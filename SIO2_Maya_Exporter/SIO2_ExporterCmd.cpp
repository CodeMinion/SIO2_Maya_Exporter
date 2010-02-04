//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2009 Frank Hernandez
// 
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the 
// Free Software Foundation; either version 2 of the License, or 
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but 
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
// for more details.
//
// You should have received a copy of the GNU General Public License along 
// with this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// File: SIO2_ExporterCmd.cpp
//
// MEL Command: SIO2_Exporter
//
// Author: Frank Hernandez
//
// Date: 02 July 2009
//
// Site: www.cis.fiu.edu/~fhern006
//
// This exporter is provided 'as is' and no responsibility can be taken for
// any damages that may be caused while using it. 
//
// NOT Yet SUPPORTED:
// - IPO
// - Scripts
// - Some Object Data. Refer to Export Object for more.
//
// KNOWN ISSUES:
// 1- Since the lot, and rot are extracted from the mesh, if the mesh
//    is binded to a rig this information is lost and the results are 
//    0 in the final object file. This was not an issues in the project
//    this was created for so it was not fixed. 
//    
//
// This is a Maya esporter for the SIO2 Interactive engine SDK 1.3.5;
// 
// SPECIAL THANKS: (Sites which made this possible...)
// Rob Betaman for his site - http://nccastaff.bournemouth.ac.uk/jmacey/RobTheBloke/www/ 
// Bryan Ewert for his site - http://ewertb.soundlinker.com/ 
// Guys from Highened3D - http://www.highend3d.com";
// Rafael - http://www.oroboro.com/rafael/docserv.php/index/maya/article/maya
//
///////////////////////////////////////////////////////////////////////////////////////////////////



#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MPoint.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MFileIO.h>

#include "SIO2_ExporterCmd.h"





const char * g_cRegisterName = "SIO2_Exporter_1_3_5";
const char * g_cSelectionFlag = "-s"; 
const char * g_cSelectionLongFlag = "-selection";

const char * g_cDestDirectoryFlag = "-d";
const char * g_cDestDirectoryLongFlag = "-destination";

const char * g_cSceneNameFlag = "-n";
const char * g_cSceneNameLongFlag = "-sceneName";

const char * g_cHelpFlag = "-h";
const char * g_cHelpLongFlag = "-help";

const char * g_cCreditsFlag = "-z";
const char * g_cCreditsLongFlag = "-credits";

const char * g_cVerboseFlag = "-v";
const char * g_cVerboseLongFlag = "-verbose";

const char * g_cFPSFlag = "-fps";
const char * g_cFPSLongFlag = "-frameRate";

const char * g_cBlendShapeFlag = "-bs";
const char * g_cBlendShapeLongFlag = "-blendShape";

const char * g_cBackFaceCullingFlag = "-bf"; 
const char * g_cBackFaceCullingLongFlag = "-convert2BFC";


const char * g_cHelpText = 
"\nSIO SDK Version: 1.3.5 \
\nThis is a Maya exporter designed for the SIO2Interactive \
game engine for the iPhone. \
\n\nUse -bs to export blend shape keyframe animation. \
\nIf not then you will also export the mesh for the blend shapes \
affecting the original. \nThis will slow down export since it will \
export the vertex position for each frame untill it reaches the end playback range.\
\nIn short, if you \
want to export blend shape nicely use the -bs flag.\
Note: If you do not have keyframes when you export using -bs, nothing \
will be exported. \
\n\nMake sure the the version number \
at the end of the exporter's name matches the version of the \
SDK being used for maximun compatibility.\
\nDo Not forget to triangulate your meshes before exporting.\
\n\nKeep chacking the site for updates.\n";

const char * g_cCreditText =
"\nDeveloper: Frank Hernandez \
\nDate: July 02 2009 \
\nSite: www.cis.fiu.edu/~fhern006\
\n\nSPECIAL THANKS: \
\nRob Betaman for his site - http://nccastaff.bournemouth.ac.uk/jmacey/RobTheBloke/www/ \
\nBryan Ewert for his site - http://ewertb.soundlinker.com/ \
\nGuys from Highened3D - http://www.highend3d.com";

// SIO2 Relative Directories
// These are the directories that appear in side the .sio2 file.
const char * g_cCamerasDir = "camera";
const char * g_cLightDir = "lamp";
const char * g_cImageDir = "image";
const char * g_cIpoDir = "ipo";
const char * g_cMaterialDir = "material";
const char * g_cObjectDir = "object";
const char * g_cSoundDir = "sound";
const char * g_cScriptDir = "script";

// Project Specific, Face UV mapping should not exceed this separation.
const float g_fUVMaxSep = 0.15;

std::string g_sDestDir = "C:/temp/";
std::string g_sSceneDir;	
std::string g_sSceneDirName = "TempScene";	
int g_nFrameRate = 0;


//
//	Description:
//		implements the MEL SIO2_Exporter command.
//
//	Arguments:
//		args - the argument list that was passes to the command from MEL
//
//	Return Value:
//		MS::kSuccess - command succeeded
//		MS::kFailure - command failed (returning this value will cause the 
//                     MEL script that is being run to terminate unless the
//                     error is caught using a "catch" statement.
//
MStatus SIO2_ExporterCmd::doIt( const MArgList& args )
{
	
	MStatus stat = MS::kSuccess;
	m_bExportSelection = false;
	m_bVerbose = false;
	m_bUseBlendShapes = false;
	MArgDatabase argData( syntax(), args);
	bool bDestSet = false, bSceneSet= false, bAnimRateSet = false; 
	m_bConvert2BackFaceCulling = false;
	m_bCorrectUVs = false;
	MString msSecneName;

	if(argData.isFlagSet(g_cHelpFlag))
	{
		setResult(g_cHelpText);
		return MS::kSuccess;
	}
	if(argData.isFlagSet(g_cCreditsFlag))
	{
		setResult(g_cCreditText);
		return MS::kSuccess;
	}

	// Check if SELECTION ONLY flag has been
	// set. Export only selected items.
	// Currently only Select All - Export All 
	// is supported.
	if(argData.isFlagSet(g_cSelectionFlag))
		m_bExportSelection = true;


	if(argData.isFlagSet(g_cVerboseFlag))
		m_bVerbose = true;

	if(argData.isFlagSet(g_cBlendShapeFlag))
		m_bUseBlendShapes = true;

	// Create the SIO2 folder structure
	//stat = createSIO2Directories(g_sDestDir, "TestScene");
	
	//if(stat == MStatus::kFailure)
	//	return stat;

	if(argData.isFlagSet(g_cFPSFlag))
	{
		argData.getFlagArgument(g_cFPSFlag, 0, g_nFrameRate); 
		bAnimRateSet = true;
	}


	//Retreive destination directory.
	if(argData.isFlagSet(g_cDestDirectoryFlag))
	{
		argData.getFlagArgument(g_cDestDirectoryFlag, 0,  m_sDesitnationDir);

		if(m_sDesitnationDir.length() >0)
		{
		   g_sDestDir = m_sDesitnationDir.asChar();
		   bDestSet = true;
		}
	}
	if(argData.isFlagSet(g_cSceneNameFlag))
	{
		argData.getFlagArgument(g_cSceneNameFlag, 0, msSecneName);

		if(msSecneName.length()>0)
		{
			bSceneSet = true;
			g_sSceneDirName = msSecneName.asChar();
		}
	}
	
	if(argData.isFlagSet(g_cBackFaceCullingFlag))
	{
		m_bConvert2BackFaceCulling = true;
		MGlobal::displayInfo("Flag Set");
	}
	
	//if(m_bVerbose)
	{

		if(!bSceneSet)
		{
			MGlobal::displayWarning(MString("No Scene Folder Name Set, Using Default: ")+ g_sSceneDirName.c_str());
		}
		if(!bDestSet)
		{
			MGlobal::displayWarning(MString("No Destination Folder Set, Using Default: ")+ g_sDestDir.c_str());
		}
		if(!bAnimRateSet && !m_bUseBlendShapes)
		{
			MGlobal::displayWarning(MString("No Animation Rate Set, Using Default: ")+ g_nFrameRate+MString(" FPS. No Animations Will Be Exported."));
		}

	}


	// Create the SIO2 folder structure
	stat = createSIO2Directories(g_sDestDir, g_sSceneDirName);
	
	if(stat == MStatus::kFailure)
		return stat;

	if(m_bExportSelection)
		stat = exportSelection();
	else
		stat = exportAll();


	MGlobal::displayInfo("SIO2_Exporter command executed!\n");


	return stat;
}
MSyntax SIO2_ExporterCmd::pluginSyntax()
{
	MSyntax syntax;

	syntax.addFlag(g_cSelectionFlag, g_cSelectionLongFlag);
	syntax.addFlag(g_cDestDirectoryFlag, g_cDestDirectoryLongFlag, MSyntax::kString);
	syntax.addFlag(g_cHelpFlag, g_cHelpLongFlag);
	syntax.addFlag(g_cCreditsFlag, g_cCreditsLongFlag);
	syntax.addFlag(g_cVerboseFlag, g_cVerboseLongFlag);
	syntax.addFlag(g_cFPSFlag, g_cFPSLongFlag, MSyntax::kLong);
	syntax.addFlag(g_cSceneNameFlag, g_cSceneNameLongFlag, MSyntax::kString);
	syntax.addFlag(g_cBlendShapeFlag, g_cBlendShapeLongFlag);
	syntax.addFlag(g_cBackFaceCullingFlag, g_cBackFaceCullingLongFlag);
	return syntax;
}

void * SIO2_ExporterCmd::creator()
{
	return new SIO2_ExporterCmd;

}

SIO2_ExporterCmd::SIO2_ExporterCmd()
{

#ifdef WIN32
	fileDialog = new FileDialog_WIN();
#endif
}
// The version of the plugin must match the version of
// the SIO2 SDK since the format is constantly being updated.
MStatus initializePlugin(MObject obj)
{
	MFnPlugin pluginFn(obj, "Frank Hernandez" , "1.0");
	
	MStatus stat;
	stat = pluginFn.registerCommand(g_cRegisterName, 
									SIO2_ExporterCmd::creator,
									SIO2_ExporterCmd::pluginSyntax );

	if(!stat)
		stat.perror("SIO2_Exporter registerCommand failed");

	return stat;
}

MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin pluginFn(obj);
	
	MStatus stat;
	stat = pluginFn.deregisterCommand(g_cRegisterName);

	if(!stat)
		stat.perror("SIO2_Exporter deregisterCommand failed");

	return stat;
}
// Export only selected items in the scene.
MStatus SIO2_ExporterCmd::exportSelection()
{
	MSelectionList selection;
	MGlobal::getActiveSelectionList(selection);

	if(selection.length() <=0)
	{
		MGlobal::displayError("No Items Selected.");
		return MS::kFailure;

	}
	//Iterate through selected objects.
	MItSelectionList iter (selection);

	for(; !iter.isDone(); iter.next())
	{
		//iter.
		MObject item;
		iter.getDependNode(item);
		
		//Add code to process nodes
		//e.i Camera, light, polygon etc.
		MGlobal::displayInfo(MString("API Type: ")+item.apiTypeStr());

		// TODO: Modify selection to get the correct objects to export
		//switch(item.apiType())
		//{
		//	case MFn::kCamera:
		//		exportCamera(item);
		//		
		//		if(m_bVerbose)
		//			MGlobal::displayInfo("Camera Exported");
		//		break;

		//	case MFn::kLight:
		//	case MFn::kAmbientLight:
		//	case MFn::kSpotLight:
		//	case MFn::kPointLight:
		//	case MFn::kDirectionalLight:
		//	case MFn::kAreaLight:
		//		exportLight(item);
		//		
		//		if(m_bVerbose)
		//			MGlobal::displayInfo("Light Exported");
		//		break;

		//	case MFn::kFileTexture:
		//		exportImages(item);
		//		
		//		if(m_bVerbose)
		//			MGlobal::displayInfo("Texture File Exported");
		//		break;

		//	case MFn::kLambert:
		//	case MFn::kPhong:
		//	case MFn::kBlinn:
		//		exportMaterial(item);
		//		
		//		if(m_bVerbose)
		//			MGlobal::displayInfo("Material Exported");	
		//		break;

		//	case MFn::kAnimCurve:
		//		MGlobal::displayInfo("Animation Curve Found:");
		//		break;
		//	
		//	case MFn::kMesh:
		//		MFnMesh mesh(item);
		//		if(!mesh.isIntermediateObject())		
		//			exportObject(item);
		//		break;

		//}
	}

	return MS::kSuccess;
}
// Export all items in the scene
MStatus SIO2_ExporterCmd::exportAll()
{
	MGlobal::displayInfo("Exporting ALL");
	MStatus stat;
	MDagPath dagPath;
	
	MObject obj;
	disableBlendShapes(obj);

	MItDependencyNodes it(MFn::kInvalid);
	while(!it.isDone())
	{	
		MObject item = it.item();
		switch(item.apiType())
		{
			case MFn::kCamera:
				stat = exportCamera(item);
				
				//if(m_bVerbose)
					//MGlobal::displayInfo(MStrin("Camera Found"));
				break;

			case MFn::kLight:
			case MFn::kAmbientLight:
			case MFn::kSpotLight:
			case MFn::kPointLight:
			case MFn::kDirectionalLight:
			case MFn::kAreaLight:
				exportLight(item);
				
				//if(m_bVerbose)
					//MGlobal::displayInfo("Light Found");
				break;

			case MFn::kFileTexture:
				exportImages(item);
				
				//if(m_bVerbose)
					//MGlobal::displayInfo("Texture File Found");
				break;

			case MFn::kLambert:
			case MFn::kPhong:
			case MFn::kBlinn:
				exportMaterial(item);
				
				//if(m_bVerbose)
					//MGlobal::displayInfo("Material Found");	
				break;

			case MFn::kAnimCurve:
				//MGlobal::displayInfo("Animation Curve Found:");
				break;

			case MFn::kMesh:
				MFnMesh meshObj(item);
				//meshObj.getPath(dagPath);

				//if(!dagPath.hasFn(MFn::kBlendShape))
				//{
				//MFnMesh mesh(item);
				//MGlobal::displayInfo("Exporting Mesh");
					if(!meshObj.isIntermediateObject())		
						exportObject(item);
				//}
				break;

		}
		it.next();
	}

	m_vNameMeshNotExported.clear();

	MGlobal::displayInfo("Done Exporting ALL");
	
	return MS::kSuccess;
}
//const int PRECISION = 3;
float SIO2_ExporterCmd::optimize_float(float num)
{
	int i = (int) num;
	float r = round(num, PRECISION);

	if (i == r)	
		return i;
	else
		return r;

}

float SIO2_ExporterCmd::round(float val, int precision)
{
	std::stringstream s;
	s<<std::setprecision(precision)<<std::setiosflags(std::ios_base::fixed)<<val;
	s>>val;
	return val;
}

MStatus SIO2_ExporterCmd::createSIO2Directories(std::string base, std::string sceneName)
{

	std::string fullDir(base);
	
	fullDir = fullDir+sceneName;
	int status;

	status = _mkdir(fullDir.c_str());

	if(status != 0 )
	{
		MGlobal::displayError("Failed to create g_sDestDir: "+ MString(fullDir.c_str()));
		return MStatus::kFailure;
	}
	else
	{
		g_sSceneDir = fullDir+"/";
		std::string localDir;
		// Create /camera inside the .soi2 file
		localDir = fullDir +"/"+g_cCamerasDir;
		status = _mkdir(localDir.c_str());
		if(status != 0)
			return MStatus::kFailure;

		// Create /lamp inside the .soi2 file
		localDir = fullDir +"/"+g_cLightDir;
		status = _mkdir(localDir.c_str());
		if(status !=0)
			return MStatus::kFailure;

		// Create /image inside the .soi2 file
		localDir = fullDir +"/"+g_cImageDir;
		status = _mkdir(localDir.c_str());
		if(status !=0)
			return MStatus::kFailure;

		// Create /ipo inside the .soi2 file
		localDir = fullDir +"/"+g_cIpoDir;
		status = _mkdir(localDir.c_str());
		if(status !=0)
			return MStatus::kFailure;

		// Create /material inside the .soi2 file
		localDir = fullDir +"/"+g_cMaterialDir;
		status = _mkdir(localDir.c_str());
		if(status !=0)
			return MStatus::kFailure;

		// Create /object inside the .soi2 file
		localDir = fullDir +"/"+g_cObjectDir;
		status = _mkdir(localDir.c_str());
		if(status !=0)
			return MStatus::kFailure;

		// Create /sound inside the .soi2 file
		localDir = fullDir +"/"+g_cSoundDir;
		status = _mkdir(localDir.c_str());
		if(status !=0)
			return MStatus::kFailure;

		// Create /script inside the .soi2 file
		localDir = fullDir +"/"+g_cScriptDir;
		status = _mkdir(localDir.c_str());
		if(status !=0)
			return MStatus::kFailure;



	}

	return MStatus::kSuccess;
}
MStatus SIO2_ExporterCmd::exportCamera(MObject obj)
{
	MStatus stat;
	// Make sure that the object is not null
	// and that it is a camera object.
	assert(!obj.isNull());
	assert(obj.hasFn(MFn::kCamera));


	MFnDagNode dagFn(obj);

	MFnCamera cam = MFnCamera(obj);
	MDagPath path;
	cam.getPath(path);
	
	MMatrix worldSpace = path.inclusiveMatrix();

	MFnTransform transformFn(path);

	MFnDependencyNode camParent(cam.parent(0));
	// The parent of the camera object is the one that holds
	// the transformation information of the camera.
	MFnTransform camTransforms(cam.parent(0));
	// Get the camera translations. i.e Position
	MVector camTranslation = camTransforms.translation(MSpace::kTransform);

	// Get the camera rotations, to know the direction in which is looking.
	MEulerRotation camRotation;
	camTransforms.getRotation(camRotation);
	
	// Use the name of the parent to save the camera.
	// The camera object usually contain shape in it,
	// while the parent is the one that whold the name
	// given in the Maya editor.
	std::string name = removeUnwantedChar(camParent.name().asChar());
	std::string dirFinal = g_sSceneDir + g_cCamerasDir + "/"+ name;
	
	std::ofstream osf(dirFinal.c_str());
	
	MVector camDir( worldSpace[2][0], worldSpace[2][1] , worldSpace[2][2] );

	camDir.normalize();
	// Calculate FOV
	double deg = 360.0 * atan( (16.0 / cam.focalLength()) ) / M_PI;// math.pi
	
	osf<<"camera( \""<<g_cCamerasDir<<"/"<<name<<"\" )"<<endl
		<<"{"<<endl;
		
		osf<<"\tloc( " <<optimize_float( camTranslation.x) << " " <<optimize_float(-1*camTranslation.z) << " " <<optimize_float(camTranslation.y) << " "<<")"<<endl;
		osf<<"\tdir( " <<optimize_float(-1*camDir.x) << " " <<optimize_float(camDir.z) << " " <<optimize_float(-1*camDir.y) << " "<<")"<<endl;
		osf<<"\tfov( " <<optimize_float(deg) << " )"<<endl;
		osf<<"\tcstart( " <<optimize_float(cam.nearClippingPlane()) << " )"<<endl;
		osf<<"\tcend( " <<optimize_float(cam.farClippingPlane()) << " )"<<endl;

		osf<<"}";

	osf.close();
	return MS::kSuccess;
}

MStatus SIO2_ExporterCmd::exportLight(MObject obj)
{
	// Make sure that the object is not null
	// and that it is a light object.
	assert(!obj.isNull());
	assert(obj.hasFn(MFn::kLight));

	float fLightRadius = 0;

	MFnLight light(obj);
	MFnDependencyNode lightParent(light.parent(0));
	// The parent of the light object is the one that holds
	// the transformation information of the light.
	MFnTransform lightTransforms(light.parent(0));
	// Get the light translations. i.e Position
	MVector lightTranslation = lightTransforms.translation(MSpace::kTransform);
	// Get the light rotations, to know the direction in which is looking.
	MQuaternion lightRotation (0,0,0,1);
	lightTransforms.getRotation(lightRotation);
	

	// Use the name of the parent to save the light.
	// The light object usually contain shape in it,
	// while the parent is the one that whold the name
	// given in the Maya editor.
	std::string name = removeUnwantedChar(lightParent.name().asChar());
	std::string dirFinal = g_sSceneDir + g_cLightDir + "/" + name;
	std::ofstream osf(dirFinal.c_str());	

	// Lamp Type
	// I am not too familiar with Blender so this is a guestimation.
	// feel free to fix any mistakes in my assumption.
	//     Maya					Blender
	// 0 - Point				Lamp
	// 1 - Ambient Light		Sun
	// 2 - Spot					Spot
	// 3 - Directional Light	Hemi					
	// 4 - Area					Area
	//
	//TODO: ADD Light Type Support
	//MFnSpotLight spotLight(obj);
	
	// Find the value of the Light Radius Attribute
	// MEL lightRadius
	MFnDependencyNode lightRData(light);
	MPlug lightPlug = lightRData.findPlug("lightRadius");
	lightPlug.getValue(fLightRadius);
	// Convert Radius to Degrees.
	fLightRadius = optimize_float(convertRadsToDeg(fLightRadius));

	osf<<"lamp( \""<<g_cLightDir<<"/"<<name<<"\" )"<<endl
	<<"{"<<endl;
	writeLightType(osf, obj);
	osf<<"\tloc( " <<optimize_float(lightTranslation.x) << " " <<optimize_float(-1*lightTranslation.z) << " " <<optimize_float(lightTranslation.y) << " "<<")"<<endl;
	osf<<"\tdir( " <<optimize_float(light.lightDirection().x) << " " <<optimize_float(-1*light.lightDirection().z) << " " <<optimize_float(light.lightDirection().y) << " "<<")"<<endl;
	osf<<"\tcol( " <<light.color().r << " " <<light.color().g << " " <<light.color().b << " "<<")"<<endl;
	osf<<"\tnrg( " <<light.intensity()<< " "<<")"<<endl;
	osf<<"\tdst( " <<fLightRadius<< " "<<")"<<endl;
	writeFOV(osf, obj);
	writeSBlend(osf, obj);
	writeLightAttenuationVals(osf, obj);
	osf<<"}";

	osf.close();
	return MS::kSuccess;
}
MStatus SIO2_ExporterCmd::writeLightType(std::ofstream &osf, MObject obj)
{
	int nLightType = 0;

	// Lamp Type
	// I am not too familiar with Blender so this is a guestimation.
	// feel free to fix any mistakes in my assumption.
	//     Maya					Blender
	// 0 - Point				Lamp
	// 1 - Ambient Light		Sun
	// 2 - Spot					Spot
	// 3 - Directional Light	Hemi					
	// 4 - Area					Area
	switch(obj.apiType())
	{
		case MFn::kPointLight:
			nLightType = 0;
			break;

		case MFn::kAmbientLight:
			nLightType = 1;
			break;

		case MFn::kSpotLight:
			nLightType = 2;
			break;

		case MFn::kDirectionalLight:
			nLightType = 3;
			break;
			
		case MFn::kAreaLight:
			nLightType = 4;
			break;

	}

	osf<<"\ttype( " << nLightType<< " "<<")"<<endl;

	return MStatus::kSuccess;
	

}
MStatus SIO2_ExporterCmd::writeFOV(std::ofstream &osf, MObject obj)
{
	float fov = 0;
	if(obj.apiType() == MFn::kSpotLight)
	{
		MFnSpotLight spotLight(obj);
		fov = spotLight.coneAngle();
	}
	osf<<"\tfov( " << optimize_float(convertRadsToDeg(fov))<< " "<<")"<<endl;
	
	return MStatus::kSuccess;
}

MStatus SIO2_ExporterCmd::writeLightAttenuationVals(std::ofstream &osf, MObject obj)
{
	float nAtt1 = 0.0, nAtt2 = 0.0;
	int nDecay;
	if(obj.apiType() == MFn::kSpotLight
		|| obj.apiType() == MFn::kPointLight)
	{
		MFnDependencyNode lightRData(obj);
		MPlug lightPlug = lightRData.findPlug("decayRate");
		lightPlug.getValue(nDecay);

		switch(nDecay)
		{
			case 1:
				nAtt1 = 1.0;
				break;

			case 2:
				nAtt2 = 1.0;
				break;

			case 3:
				nAtt1 = 0.5;
				nAtt2 = 0.5;
				break;

		}
	
	}
	osf<<"\tatt1( " << nAtt1 << " "<<")"<<endl;
	osf<<"\tatt2( " << nAtt2 << " "<<")"<<endl;

	return MStatus::kSuccess;
}
MStatus SIO2_ExporterCmd::writeSBlend(std::ofstream &osf, MObject obj)
{
	double fSBlend = 0.0;
	if(obj.apiType() == MFn::kSpotLight)
	{
		MFnDependencyNode lightRData(obj);
		MPlug lightPlug = lightRData.findPlug("penumbraAngle");
		MAngle tempAngle = lightPlug.asMAngle();
		fSBlend = tempAngle.asDegrees();
		MFnNumericAttribute tempAtt(lightRData.attribute("penumbraAngle"));
	}
	osf<<"\tsblend( " << optimize_float(fSBlend)<< " "<<")"<<endl;
	
	return MStatus::kSuccess;
}

float SIO2_ExporterCmd::convertRadsToDeg(float angle)
{
	return 180 * (angle/M_PI);
}

MStatus SIO2_ExporterCmd::exportImages(MObject obj)
{
	MStatus stat = MStatus::kSuccess;

	std::string dirFinalFile;
	MFnDependencyNode textureFn(obj);
	
	MPlug textureFileName = textureFn.findPlug("ftn");
	MString filenameTex =textureFileName.asString();

	filenameTex = textureFileName.asString();
	
	std::string fileName = filenameTex.asChar();
	
	// Find Directory End
	int found = fileName.find_last_of("/");
	std::string file = fileName.substr(found+1);
	
	if(file.find(".OGG")>-1 || file.find(".ogg")>-1)
	{
		dirFinalFile = g_sSceneDir + g_cSoundDir + "/"+file ;	
	}
	else
		dirFinalFile = g_sSceneDir + g_cImageDir + "/"+file ;
	
	// Copy Files to /Image folder in the .sio2
	// file.
	fileDialog->CopyFileOver(filenameTex.asChar(), dirFinalFile);
	
	return stat;

}

MStatus SIO2_ExporterCmd::exportMaterial(MObject obj)
{
	MStatus stat = MStatus::kSuccess;
	
	switch(obj.apiType())
	{
		case MFn::kPhong:
		
			break;
	}
	MFnDependencyNode depNode(obj);

	std::string name = removeUnwantedChar(depNode.name().asChar());

	if(m_bUseBlendShapes)
	{
		
		if(!shouldExportMaterial(name))
			return MS::kFailure;
	}

	std::string dirFinal = g_sSceneDir + g_cMaterialDir + "/" + name;
	
	std::ofstream osf(dirFinal.c_str());	

	osf<<"material( \""<<g_cMaterialDir<<"/"<<name<<"\" )"<<endl
	<<"{"<<endl;

	writeMatTextureInfo(osf, obj);

	osf<<"}";

	osf.close();
	return stat;

}
bool SIO2_ExporterCmd::shouldExportMaterial(std::string matName)
{
	for(int i=0; i<m_vNameMeshNotExported.size(); i++)
	{
		if(strcmp(m_vNameMeshNotExported[i].c_str(), matName.c_str()) == 0)
		{
			return false;
		}
	}
	return true;
}
std::string SIO2_ExporterCmd::removeUnwantedChar(std::string name)
{
	int loc = name.find(":");
	while(loc>-1)
	{
		name.replace(loc, 1, "_");
		loc = name.find(":");
	}
	return name;
}
MStatus SIO2_ExporterCmd::writeMatTextureInfo(std::ofstream &osf, MObject obj)
{
	MStatus stat = MStatus::kSuccess;
	
	
	MFnDependencyNode matFn(obj);
	
	MPlug matPlug;
	MPlugArray plugs;
	
	// Texture Channel 0
	matPlug = matFn.findPlug("color");
	matPlug.connectedTo(plugs, true,false);

	std::string nameTex;
	for(int i=0; i<plugs.length(); i++)
	{
		if(plugs[i].node().apiType() == MFn::kFileTexture)
		{
			MFnDependencyNode fnDep(plugs[i].node());
			nameTex = retriveTextureFileName(fnDep);
			osf<<"\ttfalgs0( " <<1<< " "<<")"<<endl;
			osf<<"\ttname0( \"" <<g_cImageDir<< "/" +nameTex << "\" "<<")"<<endl;
	
		}
	}

	// Texture Channel 1
	matPlug = matFn.findPlug("ambientColor");
	matPlug.connectedTo(plugs, true,false);

	for(int i=0; i<plugs.length(); i++)
	{
		if(plugs[i].node().apiType() == MFn::kFileTexture)
		{
			MFnDependencyNode fnDep(plugs[i].node());
			nameTex = retriveTextureFileName(fnDep);
			osf<<"\ttfalgs1( " <<1<< " "<<")"<<endl;
			osf<<"\ttname1( " <<g_cImageDir<< "/" +nameTex << " "<<")"<<endl;
		}
	}

	// Texture Channel 2
	matPlug = matFn.findPlug("incandescence");
	matPlug.connectedTo(plugs, true,false);

	for(int i=0; i<plugs.length(); i++)
	{
		if(plugs[i].node().apiType() == MFn::kFileTexture)
		{
			MFnDependencyNode fnDep(plugs[i].node());
			nameTex = retriveTextureFileName(fnDep);
			if(isSoundBuffer(nameTex))
			{
				osf<<"\tsfalgs( " <<1<< " "<<")"<<endl;
				osf<<"\tsbname( " <<g_cImageDir<< "/" +nameTex << " "<<")"<<endl;
			

			}		
		}
	}

	MColor color;
	matPlug = matFn.findPlug("ambientColorR");
	matPlug.getValue(color.r);
	matPlug = matFn.findPlug("ambientColorG");
	matPlug.getValue(color.g);
	matPlug = matFn.findPlug("ambientColorB");
	matPlug.getValue(color.b);

	if(color.r <= 0 && color.b <=0 && color.g <=0)
	{
		MGlobal::displayWarning("Diffuse: Ambient Color is <= 0 , the object will not reflect light.");
		MGlobal::displayWarning("Diffuse: Ambient Color is <= 0 , using default :0.8. Source: Ln#1089");
		color.r = 0.8 , color.b = 0.8, color.g = 0.8; 
	}
	osf<<"\tdiffuse( " <<optimize_float(color.r) << " " <<optimize_float(color.g) << " " <<optimize_float(color.b) << " "<<")"<<endl;
	

	MColor color1;
	matPlug = matFn.findPlug("specularColorR");
	matPlug.getValue(color1.r);
	//color.r = matPlug.asDouble();
	matPlug = matFn.findPlug("specularColorG");
	matPlug.getValue(color1.g);
	matPlug = matFn.findPlug("specularColorB");
	matPlug.getValue(color1.b);
	matPlug = matFn.findPlug("specularColorA");
	matPlug.getValue(color1.a);
	osf<<"\tspecular( " <<optimize_float(color1.r) << " " <<optimize_float(color1.g) << " " <<optimize_float(color1.b) << " "<<")"<<endl;
	

	matPlug = matFn.findPlug("transparencyR");
	matPlug.getValue(color.r);
	matPlug = matFn.findPlug("transparencyG");
	matPlug.getValue(color.g);
	matPlug = matFn.findPlug("transparencyB");
	matPlug.getValue(color.b);
	double transp = findMax(color.r, color.g);
	transp = findMax(transp, color.b);

	osf<<"\talpha( " <<optimize_float(1.0 - transp)<< " "<<")"<<endl;
	
	matPlug = matFn.findPlug("translucence");
	double transl;
	matPlug.getValue(transl);
	osf<<"\tshininess( " <<optimize_float(transl*128.0)<< " "<<")"<<endl;

	// FRICTION
	if(false)
	{
		double friction;
		osf<<"\tfriction( " <<optimize_float(friction)<< " "<<")"<<endl;
	}
	// RESTITUTION
	if(false)
	{
		double restuttion;
		osf<<"\trestitution( " <<optimize_float(restuttion)<< " "<<")"<<endl;
	}
	// ALPHA LEVEL
	if(false)
	{
		double alphaLvl;
		osf<<"\talvl( " <<optimize_float(alphaLvl)<< " "<<")"<<endl;
	}
	// BLEND MODE
	if(false)
	{
		char blendMode;
		osf<<"\tblend( " <<blendMode<< " "<<")"<<endl;
	}
	

	return stat;
}
double SIO2_ExporterCmd::findMax(double a, double b)
{
	if(a>b)
		return a;

	return b;
}
std::string SIO2_ExporterCmd::retriveTextureFileName(MFnDependencyNode dpNode)
{
	MPlug textureFileName = dpNode.findPlug("ftn");
	MString filenameTex =textureFileName.asString();

	std::string nameTex = removeUnwantedChar(filenameTex.asChar());

	int found = nameTex.find_last_of("/");
	nameTex = nameTex.substr(found+1);
	
	return nameTex;
}
bool SIO2_ExporterCmd::isSoundBuffer(std::string filename)
{
	return filename.find(".ogg")>-1 || filename.find(".OGG")>-1;
}

MStatus SIO2_ExporterCmd::exportObject(MObject obj)
{
	MStatus stat = MStatus::kSuccess;

	MFnMesh meshObject(obj);
	MFnDependencyNode meshParentNode(meshObject.parent(0));

	if(m_bUseBlendShapes)
	{
		if(!containsKeyFrameAnimation(obj))
		{
			// If we are using blend shapes with key frames 
			// then only the base shape being animated has 
			// key frames. However the exporter will try to 
			// export all the other shapes as meshes as well.
			// These are not needed in SIO2 so we do not export
			// them. Check the -h help.
			MObjectArray shaders;
			MIntArray FaceIndices;
			MFnMesh meshObj(obj);
			meshObj.getConnectedShaders(0, shaders, FaceIndices);
			//Get material file name
			int size = shaders.length();
			MString matName;
			for(int kk=0; kk<shaders.length(); kk++)
			{
				MFnDependencyNode fnShader (shaders[kk]);
				MPlug sshader = fnShader.findPlug("surfaceShader");
				MPlugArray materials;
				sshader.connectedTo(materials,true, true);
				for(int ii=0; ii<materials.length(); ii++)
				{
					MFnDependencyNode fnMat(materials[ii].node());
					m_vNameMeshNotExported.push_back(removeUnwantedChar( fnMat.name().asChar()).c_str());
				}

			}
			return MS::kFailure;

		}
	}
	std::string name = removeUnwantedChar(meshParentNode.name().asChar());
	std::string dirFinal = g_sSceneDir + g_cObjectDir + "/" + name;
	
	std::ofstream osf(dirFinal.c_str());	

	osf<<"object( \""<<g_cObjectDir<<"/"<<name<<"\" )"<<endl
	<<"{"<<endl;

	// Write loc( %s %s %s )
	// Write rot( %f %f %f )
	// Write scl( %f %f %f )
	writeMeshTransforms(osf, obj);	
	
	// Write rad( %f )
	// TODO
	// Currently Magic Numbers:
	osf<<"\trad( " <<optimize_float(1.732)<< " "<<")"<<endl;
	
	// Write flags( %d )
	// TODO

	// Write bounds( %c )
	// TODO
	// Currently Magic Numbers:
	osf<<"\tbounds( " <<optimize_float(4)<< " "<<")"<<endl;
	
	// Write mass( %f )
	// TODO

	// Write damp( %f )
	// TODO

	// Write rotdamp( %f )
	// TODO

	// Write margin( %f )
	// TODO

	// Write dim( %f %f %f )
	// TODO
	// Currently Magic Numbers:
	osf<<"\tdim( " <<optimize_float(1) << " " <<optimize_float(1) << " " <<optimize_float(1) << " "<<")"<<endl;
	
	// Write instname( “%s” )
	// TODO

	// Write iponame(“%s”)
	// TODO

	// Write linstiff( “%f” )
	// TODO

	// Write shapematch( “%f” )
	// TODO

	// Write citerations(“%c”)
	// TODO

	// Write piterations(“%f”)
	// TODO

	// Write bendconst(“%c”)
	// TODO

	// Write vbo_offset( %d %d %d %d %d )
	writeMeshBoffset(osf, obj);

	// Write vert( %f %f %f )
	writeMeshVerteices(osf, obj);

	// Write vcol( %c %c %c %c )
	writeMeshVertColor(osf, obj);

	// Write vnor( %f %f %f )
	writeMeshVertNormals(osf, obj);

	// Write uv#
	writeMeshTexCoords(osf, obj);

	// Write n_vgroup( %d )
	// Write vgroup( “%s” )
	// Write mname( “%s” )
	// Write n_ind( %d )
	// Write ind( %h %h %h )
	writeMeshSkinClusters(osf, obj);

	// Write n_frame( %d )
	// Write frame( %f %s )
	// Write fvert( %f %f %f )
	writeMeshAnimData(osf,obj);

	osf<<"}";

	osf.close();
	return stat;
}
MStatus SIO2_ExporterCmd::writeMeshLocation(std::ofstream &osf, MObject obj)
{
	MStatus stat = MS::kSuccess;

	MFnMesh meshObj(obj);
	
	MFnTransform fn;

	for(int i=0; i<meshObj.parentCount(); i++)
	{
		if(meshObj.parent(i).hasFn(MFn::kTransform))
		{
			fn.setObject(meshObj.parent(i));
		}
	}
	
	MVector vec = fn.translation(MSpace::kTransform, &stat);
	
	osf<<"\tloc( " <<optimize_float(vec.x) << " " <<optimize_float(-1*vec.z) << " " <<optimize_float(vec.y) << " "<<")"<<endl;
	return stat;
}

MStatus SIO2_ExporterCmd::writeMeshTransforms(std::ofstream &osf, MObject obj)
{
	MStatus stat = MS::kSuccess;

	MFnMesh meshObj(obj);
	
	MFnTransform fn;//(obj);

	for(int i=0; i<meshObj.parentCount(); i++)
	{
		if(meshObj.parent(i).hasFn(MFn::kTransform))
		{
			fn.setObject(meshObj.parent(i));
		}
	}
	double meshScale [] = {0.0, 0.0, 0.0};
	MEulerRotation meshRot;
	fn.getScale(meshScale);
	fn.getRotation(meshRot);
	MVector vec = fn.translation(MSpace::kTransform, &stat);

	osf<<"\tloc( " <<optimize_float(vec.x) << " " <<optimize_float(-1*vec.z) << " " <<optimize_float(vec.y) << " "<<")"<<endl;
	osf<<"\trot( " <<optimize_float(convertRadsToDeg( meshRot.x)) << " " <<optimize_float(convertRadsToDeg(-1* meshRot.z)) << " " <<optimize_float(convertRadsToDeg(meshRot.y)) << " "<<")"<<endl;
	osf<<"\tscl( " <<optimize_float(meshScale[0]) << " " <<optimize_float(meshScale[1]) << " " <<optimize_float(meshScale[2]) << " "<<")"<<endl;

	
	return stat;
}

MVector SIO2_ExporterCmd::retriveTranslation(MObject obj)
{
	MFnTransform fn(obj);
	MVector transl = fn.getTranslation(MSpace::kTransform);

	MGlobal::displayInfo(MString("")+transl.x);

	if(fn.parentCount()>0)
		return retriveTranslation(fn.parent(0));

	return transl;

}
MStatus SIO2_ExporterCmd::printParentTrace(MObject obj, std::string level)
{
	MStatus stat = MS::kSuccess;

	MFnMesh meshObj(obj);
	
	for(int i=0; i<meshObj.parentCount(); i++)
	{
		MObject pObj = meshObj.parent(i);

		MFnDagNode fnParent(pObj);

		MGlobal::displayInfo(MString("Parent Name: ")+fnParent.name());
		MGlobal::displayInfo(MString("Parent Type: ")+fnParent.type());


		printParentTrace(pObj, level+"*");

	}
	return stat;
}
MStatus SIO2_ExporterCmd::writeMeshSkinClusters(std::ofstream &osf, MObject obj)
{
	MStatus stat = MStatus::kSuccess;

	MFnMesh meshObj(obj);
	std::string toFile;

	
	int nVertexGourps = 0;

	bool m_bSkinFound = false;

	//Nasty Hack Must Fix
	MObjectArray shaders;
	MIntArray FaceIndices;

	meshObj.getConnectedShaders(0, shaders, FaceIndices);
	//Get material file name
	int size = shaders.length();
	MString matName;
	for(int kk=0; kk<shaders.length(); kk++)
	{
		MFnDependencyNode fnShader (shaders[kk]);
		MPlug sshader = fnShader.findPlug("surfaceShader");
		MPlugArray materials;
		sshader.connectedTo(materials,true, true);
		for(int ii=0; ii<materials.length(); ii++)
		{
			MFnDependencyNode fnMat(materials[ii].node());
			matName = matName+MString("\tmname( \"material/")+ removeUnwantedChar( fnMat.name().asChar()).c_str()+ MString("\" )\n");
		}

	}

	
	// Find skin clusters
	MItDependencyNodes it(MFn::kSkinClusterFilter);
	// Check if it has skin cluster.
	if(!it.isDone())
	{
		for(; !it.isDone(); it.next())
		{
			if(m_bSkinFound) break;

			MObject obj = it.item();
			
			MFnSkinCluster fn(obj);
			MDagPathArray infs;
			
			unsigned int nInfs = fn.influenceObjects(infs);
			unsigned int nGeoms = fn.numOutputConnections();
			
			
			for(unsigned int i = 0; i<nGeoms; i++)
			{
				if(m_bSkinFound)break;

				unsigned int index;
				index = fn.indexForOutputConnection(i);

				MDagPath skinPath;
				fn.getPathAtIndex(index, skinPath);

				MItGeometry gIter(skinPath);

				if(strcmp(skinPath.partialPathName().asChar(), meshObj.name().asChar())!=0)
					continue;
				
				// Get the index of vertices affected by each deform.
				// the equivalent of vertex groups in Blender... 
				// I guess.... Not really familiar with Blender.
				osf<<"\tn_vgroup( " <<nInfs<< " "<<")"<<endl;
				for(int j=0; j<nInfs; j++)
				{
					osf<<"\tvgroup( \"" <<infs[j].partialPathName()<<"\" )"<<endl;
					osf<<matName.asChar();
				
					MSelectionList geoList;
					MDoubleArray geoWeights;
					MDagPath affectedPath;
					MStringArray verts;
					MIntArray intVerts;
					int nAffectedTris = 0;
					
					fn.getPointsAffectedByInfluence(infs[j],geoList, geoWeights);
					MObject vertComp;
					geoList.getDagPath(0, affectedPath, vertComp);

					
					MFnSingleIndexedComponent vertices(vertComp);
					vertices.getElements(intVerts);
					writeVertexIndicesFromMesh(osf, affectedPath, meshVertices);

				}			
			}
		
		}
	}
	else
	{
		//Create null vertex group
		osf<<"\tn_vgroup( " <<1<< " "<<")"<<endl;
		if(m_bUseBlendShapes)
		{
			osf<<"\tvgroup( \"" <<"blendShape"<<"\")"<<endl;
		}
		else
		{
			osf<<"\tvgroup( \"" <<"null"<<"\")"<<endl;
		}
		osf<<matName.asChar();
		MDagPath meshDagPath;
		meshObj.getPath(meshDagPath);			

		writeVertexIndicesFromMesh(osf, meshDagPath,meshVertices);
		meshVertices.clear();

	}
	return stat;
}
MStatus SIO2_ExporterCmd::writeVertexIndicesFromMesh(std::ofstream &osf, MDagPath meshDagPath, MPointArray &vertexList)
{
	MStatus stat = MS::kSuccess;
	int numTriangles = 0;
	MIntArray outVertIndices; 
	MString indCmdWrite("");
	MPointArray verIndTris;

	// Keep track of the number of triangels
	// matching the vertices passed.
	int nCountTriangles = 0; 
	
	MItMeshPolygon itPoly(meshDagPath, MObject::kNullObj);
	for(; !itPoly.isDone(); itPoly.next())
	{
		itPoly.numTriangles(numTriangles);
		for(int i= 0; i<numTriangles; i++)
		{
			MPointArray nonTweaked;
			MIntArray triangleVertices;
			MIntArray localIndex;

			stat = itPoly.getTriangle(i, nonTweaked, triangleVertices, MSpace::kObject);

			if(stat == MS::kSuccess)
			{
				
				stat = writeVertexIndices(outVertIndices, vertexList, triangleVertices, osf);
				if(stat == MS::kSuccess)
				{
					nCountTriangles++;
					verIndTris.append(MPoint(outVertIndices[0], outVertIndices[1], outVertIndices[2]));
				}
				
			}
			// Preapare the list for the next triangle.
			outVertIndices.clear();
					
		}
	}
	if(nCountTriangles > 0)
	{
		// If we found triangles matching the vertices
		// then write them to file.

		// Write the number of indices needed to create 
		// the triangles.
		osf<<"\tn_ind( "<<nCountTriangles*3<<" )"<<endl;
		for(int i=0; i<verIndTris.length(); i++)
		{
			osf<<"\tind( "<<verIndTris[i].x<< " " << verIndTris[i].y << " " << verIndTris[i].z<<" )"<<endl;

		}
		
		// Write the indices.
		osf<<indCmdWrite.asChar();
		
	}

	return stat;
}

MStatus SIO2_ExporterCmd::writeVertexIndices(MIntArray & outVertexIndeces, MPointArray & vertexList, MIntArray & trisData, std::ofstream &osf)
{
	MStatus stat = MS::kSuccess;

	if(trisData.length()!=3)
	{
		stat = MS::kFailure;
		return stat;
	}
	for(unsigned int i = 0; i<trisData.length(); i++)
	{
		for(unsigned int j = 0; j<vertexList.length(); j++)
		{
			if(trisData[i] == j)
			{
				outVertexIndeces.append(j);
				break;
			}

		}
	}
	if(outVertexIndeces.length() != trisData.length())
	{
		// Wrong triangle
		stat = MS::kFailure;
	}
	else
	{
		if(m_bConvert2BackFaceCulling)
		{
			//Write indices in conter-clockwise order
			int ind2 = outVertexIndeces[1];
			int ind3 = outVertexIndeces[2];
			outVertexIndeces[1] = ind3;
			outVertexIndeces[2] = ind2;
		}	
	}


	return stat;
}
MStatus SIO2_ExporterCmd::getVertexIndices(MIntArray & outVertexIndeces, MPointArray & vertexList, MIntArray & trisData)
{
	//MIntArray vertIndex;
	MStatus stat = MS::kSuccess;

	if(trisData.length()!=3)
	{
		stat = MS::kFailure;
		return stat;
	}
	for(unsigned int i = 0; i<trisData.length(); i++)
	{
		for(unsigned int j = 0; j<vertexList.length(); j++)
		{
			if(trisData[i] == j)//vertexList[j])
			{
				outVertexIndeces.append(j);
				break;
			}

		}
				
	}
	if(outVertexIndeces.length() != trisData.length())
	{
		// Wrong triangle 
		stat = MS::kFailure;
	}

	return stat;
}

MIntArray SIO2_ExporterCmd::GetLocalIndex( MIntArray & getVertices, MIntArray & getTriangle)
{
  MIntArray   localIndex;
  unsigned    gv, gt;

  assert ( getTriangle.length() == 3 );    // Should always deal with a triangle

  for ( gt = 0; gt < getTriangle.length(); gt++ )
  {
    for ( gv = 0; gv < getVertices.length(); gv++ )
    {
      if ( getTriangle[gt] == getVertices[gv] )
      {
		localIndex.append( gv );
        break;
      }
    }

    // if nothing was added, add default "no match"
    if ( localIndex.length() == gt )
      localIndex.append( -1 );
  }

  return localIndex;
}

MStatus SIO2_ExporterCmd::printChildTrace(MObject obj, std::string level)
{
	MStatus stat = MS::kSuccess;

	MFnMesh meshObj(obj);
	
	for(int i=0; i<meshObj.childCount(); i++)
	{
		MObject pObj = meshObj.child(i);

		MFnDagNode fnParent(pObj);

		MGlobal::displayInfo(MString("Child Name: ")+fnParent.name());
		MGlobal::displayInfo(MString("Child Type: ")+fnParent.type());


		printParentTrace(pObj, level+"*");

	}
	return stat;
}
MStatus SIO2_ExporterCmd::writeMeshBoffset(std::ofstream &osf, MObject obj)
{
	MStatus stat = MS::kSuccess;
	// Attach function set to the object.
	MFnMesh meshObj(obj);
	
	// Vertex Array
	MPointArray vts;
	// Get the vertices.
	meshObj.getPoints(vts);

	// Vertex Color Array
	MColorArray vcols;
	// Get the vertices colors.
	meshObj.getVertexColors(vcols);

	// Vertex Color Array
	MFloatVectorArray vnor;
	// Get the vertices colors.
	meshObj.getVertexNormals(false,vnor);

	// UV set Array
	MStringArray uvsets;
	// Get the name of the UV sets.
	meshObj.getUVSetNames(uvsets);

	MIntArray vbo_offset(4);
	for(int i=0; i<vbo_offset.length(); i++)
		vbo_offset[i]=0;

	MInt64 vbo_size  = vts.length() * 3 * 4;


	if(vcols.length()>0)
	{
		vbo_offset[0] = vbo_size;
		vbo_size = vbo_size + vts.length() * 4;
	}
	if(vnor.length()>0)
	{
		vbo_offset[ 1 ] = vbo_size;
		vbo_size = vbo_size + vts.length() * 12 ;
	}

	if(uvsets.length()>0 && meshObj.numUVs(uvsets[0])>0)
	{
		vbo_offset[ 2 ] = vbo_size;
		vbo_size = vbo_size + vts.length() * 8 ;

		if(uvsets.length()>1)
		{
			vbo_offset[ 3 ] = vbo_size;
			vbo_size = vbo_size + vts.length() * 8 ;
		}
	}

	
	osf<<"\tvbo_offset( " <<vbo_size 
		<< " " <<optimize_float(vbo_offset[0])
		<< " " <<optimize_float(vbo_offset[1])
		<< " " <<optimize_float(vbo_offset[2])
		<< " " <<optimize_float(vbo_offset[3]) 
		<< " "<<")"<<endl;

	return stat;


}
MStatus SIO2_ExporterCmd::writeMeshVerteices(std::ofstream &osf, MObject obj)
{
	MStatus stat = MS::kSuccess;
	// Attach function set to the object.
	MFnMesh meshObj(obj);

	// Vertex Array
	MPointArray vts;

	// Get the vertices.
	meshObj.getPoints(vts);

	for(int i=0; i<vts.length(); i++)
	{

		osf<<"\tvert( " <<optimize_float(vts[i].x) 
			<< " " <<optimize_float(-1*vts[i].z)
			<< " " <<optimize_float(vts[i].y) 
			<< " "<<")"<<endl;
	}
	meshVertices = vts;
	return stat;
}
MStatus SIO2_ExporterCmd::writeMeshVertColor(std::ofstream &osf, MObject obj)
{
	MStatus stat = MS::kSuccess;
	// Attach function set to the object.
	MFnMesh meshObj(obj);

	// Vertex Color Array
	MColorArray vcols;
	// Get the vertices colors.
	meshObj.getVertexColors(vcols);

	for(int i=0; i<vcols.length(); i++)
	{
		osf<<"\tvcol( " <<optimize_float(vcols[i].r) 
			<< " " <<optimize_float(vcols[i].g) 
			<< " " <<optimize_float(vcols[i].b)
			<< " " <<optimize_float(vcols[i].a) << " "<<")"<<endl;
	}

	return stat;

}

MStatus SIO2_ExporterCmd::writeMeshVertNormals(std::ofstream &osf, MObject obj)
{
	MStatus stat = MS::kSuccess;
	// Attach function set to the object.
	MFnMesh meshObj(obj);

	// Vertex Normal Array
	MFloatVectorArray vnor;

	// Get the vertices normals.
	meshObj.getVertexNormals(false,vnor);

	for(int i=0; i<vnor.length(); i++)
	{
		//RHS
//		osf<<"\tvnor( " <<optimize_float(vnor[i].x) 
//			<< " " <<optimize_float(vnor[i].y) 
//			<< " " <<optimize_float(vnor[i].z)
//			<< " "<<")"<<endl;
		//LHS
		osf<<"\tvnor( " <<optimize_float(vnor[i].x) 
			<< " " <<optimize_float(-1*vnor[i].z) 
			<< " " <<optimize_float(vnor[i].y)
			<< " "<<")"<<endl;
	}

	return stat;

}

MStatus SIO2_ExporterCmd::writeMeshTexCoords(std::ofstream &osf, MObject obj)
{

	MStatus stat = MS::kSuccess;
	
	// Attach function set to the object.
	MFnMesh meshObj(obj);
	
	MDagPath dagForMesh;
	meshObj.getPath(dagForMesh);

	MIntArray outVertIndices; 
					
	// UV set Array
	MStringArray uvsets;

	// Get the name of the UV sets.
	meshObj.getUVSetNames(uvsets);
	if(uvsets.length()==0 || meshObj.numUVs(uvsets[0])==0)
	{
		stat= MS::kFailure;
		return stat;
	}
	 int  vtxInPolygon;

	for(int i =0; i<uvsets.length() && i<MAX_TEXTURE_CHANNELS; i++)
	{
		MFloatArray u_coords;
		MFloatArray v_coords;

		MFloatArray u_coordsOut(meshVertices.length());
		MFloatArray v_coordsOut(meshVertices.length());

		for(int i =0 ; i<u_coordsOut.length(); i++)
		{
			u_coordsOut[i] = -1;
			v_coordsOut[i] = -1;
		}

		meshObj.getUVs(u_coords, v_coords, &uvsets[i]);

		MStatus status;
		MItMeshPolygon  itPolygon( dagForMesh, MObject::kNullObj );
		for ( /* nothing */; !itPolygon.isDone(); itPolygon.next() )
		{
			
			MIntArray  polygonVertices;
			itPolygon.getVertices( polygonVertices );



			// Get triangulation of this poly.
			int numTriangles;
			itPolygon.numTriangles(numTriangles);
			while ( numTriangles-- )
			{
					MPointArray                     nonTweaked;
					// object-relative vertex indices for each triangle
					MIntArray                       triangleVertices;
					// face-relative vertex indices for each triangle
					MIntArray                       localIndex;

		       status = itPolygon.getTriangle( numTriangles,
			                                nonTweaked,
				                            triangleVertices,
					                        MSpace::kObject );

				if ( status == MS::kSuccess )
				{

					// Get face-relative vertex indices for this triangle
					localIndex = GetLocalIndex( polygonVertices,
						                      triangleVertices );

					status = getVertexIndices(outVertIndices, meshVertices, triangleVertices);
					if(status == MS::kSuccess)
					{
						// Preapare the list for the next triangle.
						//outVertIndices.clear();
						int uvID[3];

						// Get UV values for each vertex within this polygon
						  
						for ( vtxInPolygon = 0; vtxInPolygon < 3; vtxInPolygon++ )
						{
							itPolygon.getUVIndex( localIndex[vtxInPolygon],
												  uvID[vtxInPolygon],
												  &uvsets[i] );
						}

						if(m_bCorrectUVs)
						{
						
						}
						else
						{
							for(int i = 0; i<3; i++)
							{
								int vertInd = outVertIndices[i];
								u_coordsOut[vertInd] = u_coords[uvID[i]];
								v_coordsOut[vertInd] = 1 - v_coords[uvID[i]];

							}
						}

					}

			
				}
				outVertIndices.clear();
		
			}
		}
		// Write UVS
		for(int j=0; j<u_coordsOut.length(); j++)
		{
			osf<<"\tuv"<<i<<"( " <<optimize_float(u_coordsOut[j]) 
				<< " " <<optimize_float(v_coordsOut[j])
				<< " "<<")"<<endl;	
		}
		u_coordsOut.clear();
		v_coordsOut.clear();

	}

	return stat;

}
bool SIO2_ExporterCmd::containsUV(MFloatArray u_coords, MFloatArray v_coords, double u, double v)
{
	for(int i=0; i<u_coords.length(); i++)
	{
		if(u_coords[i] == u && v_coords[i] == v)
			return true;
	}

	return false;
}

// Function taken from : http://ewertb.soundlinker.com/api/api.009.htm
// ************************************************************************************************
//    Function: GetPointsAtTime
//
// Description: Sets Maya to the specified Time and gets the object-space vertex coordinates
//              from the specified DAG at the that Time.
//
//       Input: const MDagPath& dagPath: The DAG path for the mesh object.
//              const MTime& mayaTime: The Time at which to query the vertex coordinates.
//              MPointArray& points: Storage for the array of vertex coordinates.
//
//      Output: (MStatus): MS::kSuccess if it worked; else MS::kFailure.
// ************************************************************************************************
MStatus SIO2_ExporterCmd::GetPointsAtTime(const MDagPath &dagPath, const MTime &mayaTime, MPointArray &points)
{
	  MStatus status = MS::kSuccess;

	  points.clear();

	  MFnMesh fnMesh;

	  // Move Maya to current frame
	  MGlobal::viewFrame( mayaTime );

	  // You MUST reinitialize the function set after changing time!
	  fnMesh.setObject( dagPath );

	  // Get vertices at this time
	  status = fnMesh.getPoints( points );

	  return status;

}
MStatus SIO2_ExporterCmd::GetPointsAtTimeContext(const MDagPath &dagPath, const MTime &mayaTime, MPointArray &points)
{
	  MStatus status = MS::kSuccess;

	  points.clear();

	  MFnDependencyNode fnDependNode( dagPath.node(), &status );

	  MPlug plugMesh;
	  MObject meshData;

	  // Get the .outMesh plug for this mesh
	  plugMesh = fnDependNode.findPlug( MString( "outMesh" ), &status );

	  // Get its value at the specified Time.
	  status = plugMesh.getValue( meshData, MDGContext( mayaTime ) );

	  // Use its MFnMesh function set 
	  MFnMesh fnMesh( meshData, &status );

	  // And query the point coordinates
	  status = fnMesh.getPoints( points );

	  return status;

}
// Function taken from : http://ewertb.soundlinker.com/api/api.009.htm
// ************************************************************************************************
//    Function: GetPointsAtTimeContext
//
// Description: Gets the object-space vertex coordinates from the specified DAG at
//              the specified Time.
//
//       Input: const MDagPath& dagPath: The DAG path for the mesh object.
//              const MTime& mayaTime: The Time at which to query the vertex coordinates.
//              MPointArray& points: Storage for the array of vertex coordinates.
//
//      Output: (MStatus): MS::kSuccess if it worked; else MS::kFailure.
// ************************************************************************************************
MStatus SIO2_ExporterCmd::writeMeshAnimData(std::ofstream &osf, MObject obj)
{
	MStatus stat = MS::kSuccess;

	MPointArray points;
	MTime currentFrame, maxFrame;
	std::vector<double> vKeyFrames;
	std::string defAnimName = "DefAnimName";

	MFnMesh mesh(obj);

	MDagPath dagPath;
	mesh.getPath(dagPath);
	// Find key frames.
	stat = findAnimKeyFrames(dagPath, vKeyFrames);

	// If no key frames were found then used the FrameRate
	// either set thorugh the flag or the default of 1;
	if(stat == MS::kFailure)
	{
		// Only export animation if no anim curve was found
		// but a frame rate was specified.
		if(g_nFrameRate > 0)
		{
			// Get Start Frame
			currentFrame = MAnimControl::minTime();
			// Write number of Frames
			osf<<"\tn_frame( " <<optimize_float(ceil( maxFrame.value()/g_nFrameRate))<< " "<<")"<<endl;
			
			while(currentFrame <= maxFrame)
			{
			
				stat = GetPointsAtTimeContext( dagPath, currentFrame, points );
				if(stat == MS::kSuccess)
				{
					// Get Frame data if any
					osf<<"\tframe( " <<optimize_float( currentFrame.value())<<" \""<< defAnimName<<"\" )"<<endl;
					for(int i=0; i<points.length(); i++)
					{
						writeFVert(osf, points[i]);
					}
				}
				currentFrame+= g_nFrameRate;
			}
		}
	}
	else
	{
		// Write number of Frames
		osf<<"\tn_frame( " <<optimize_float( vKeyFrames.size())<< " "<<")"<<endl;
		for(int i =0 ; i<vKeyFrames.size(); i++)
		{
			currentFrame = vKeyFrames[i];
			stat = GetPointsAtTimeContext( dagPath, currentFrame, points );
			if(stat == MS::kSuccess)
			{
				// Get Frame data if any
				osf<<"\tframe( " <<optimize_float( currentFrame.value())<<" \""<< defAnimName<<"\" )"<<endl;
				for(int i=0; i<points.length(); i++)
				{

					writeFVert(osf, points[i]);
				}
		
			}
		}
	}


	return stat;
}
MStatus SIO2_ExporterCmd::writeFVert(std::ofstream &osf, MPoint vert)
{
	MStatus stat = MStatus::kSuccess;

	osf<<"\tfvert( " << optimize_float(vert.x)<<" "
		<<optimize_float(-1*vert.z)<<" "
		<<optimize_float(vert.y)<<" )"<<endl;

	return stat;
}
MStatus SIO2_ExporterCmd::findAnimKeyFrames(const MDagPath &dagPath, std::vector<double> &vKeyFrames)
{
	MStatus stat = MS::kSuccess;

	MFnDagNode dn(dagPath);

	// Find motion nodes.
	MItDependencyGraph dgIter(dagPath.node(), 
							MFn::kAnimCurve, 
							MItDependencyGraph::kUpstream,
							MItDependencyGraph::kBreadthFirst,
							MItDependencyGraph::kNodeLevel,
							&stat);

	if(stat == MS::kSuccess)
	{
		for(; !dgIter.isDone(); dgIter.next())
		{
			MObject anim = dgIter.thisNode(&stat);
			MFnAnimCurve animCurve (anim, &stat);

			if(stat == MS::kSuccess)
			{
				unsigned int numKeys = animCurve.numKeyframes(&stat);
				for(unsigned int currKey =0; currKey<numKeys; currKey++)
				{
					MTime keyTime = animCurve.time(currKey, &stat);
					// If this isnt a duplicate frame add it to the list.
					// Used during blend shape export. 
					// See: containsAnimationFrame.
					if(!containsAnimationFrame(keyTime, vKeyFrames))
						vKeyFrames.push_back(keyTime.value());
				}

			}
		}
	}

	return stat;
}
bool SIO2_ExporterCmd::containsAnimationFrame(MTime frame, const std::vector<double> &vKeyFrames)
{
	for(int i=0; i<vKeyFrames.size(); i++)
	{
		if(vKeyFrames[i] == frame.value())
		{
			return true;
		}
	}
	return false;
}

bool SIO2_ExporterCmd::containsKeyFrameAnimation(MObject obj)
{

	MStatus stat;
	MFnMesh mesh(obj);
	MDagPath dagPath;
	mesh.getPath(dagPath);

	// Find motion nodes.
	MItDependencyGraph dgIter(dagPath.node(), 
							MFn::kAnimCurve, 
							MItDependencyGraph::kUpstream,
							MItDependencyGraph::kBreadthFirst,
							MItDependencyGraph::kNodeLevel,
							&stat);

	if(stat == MS::kSuccess)
	{
		return true;
	}

	return false;
}
void SIO2_ExporterCmd::printFuntionList(MObject obj)
{
	MStringArray list;

	MGlobal::getFunctionSetList(obj, list);

	for(int i=0; i<list.length(); i++)
	{
		MGlobal::displayInfo(list[i]);
	}

}
void SIO2_ExporterCmd::disableBlendShapes(MObject obj)
{
	MItDependencyNodes it(MFn::kBlendShape);
	while(!it.isDone())
	{
		MFnBlendShapeDeformer fn(it.item());
		MPlug plug = fn.findPlug("en");
		plug.setValue(0.0f);

		it.next();

	}
}

