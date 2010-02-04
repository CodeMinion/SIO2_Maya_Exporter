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
// If you intend to use this software for comercial use just drop me a line,
// saying what you intend to use it for. I like some braging rights :)
//
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
// You are 
//
// The following data was relevant to the project I created this for but I think
// it still gives an idea of the export speed of this exporter.
//
// 
// SPECIAL THANKS: (Sites which made this possible...)
// Rob Betaman for his site - http://nccastaff.bournemouth.ac.uk/jmacey/RobTheBloke/www/ 
// Bryan Ewert for his site - http://ewertb.soundlinker.com/ 
// Guys from Highened3D - http://www.highend3d.com";
// Rafael - http://www.oroboro.com/rafael/docserv.php/index/maya/article/maya
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SIO2_EXP_H
#define SIO2_EXP_H

#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

#include <maya/MItDag.h>
#include <maya/MIteratorType.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItGeometry.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>

#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>


#include <maya/MFnSkinCluster.h>
#include <maya/MFnSingleIndexedComponent.h>

#include <maya/MFnTransform.h>
#include <maya/MQuaternion.h>
#include <maya/MEulerRotation.h>
#include <maya/MMatrix.h>

#include <maya/MColor.h>

#include <maya/MFloatVector.h>
#include <maya/MPointArray.h>
#include <maya/MStringArray.h>
#include <maya/MFloatArray.h>
#include <maya/MColorArray.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>

#include <maya/MFnCamera.h>
#include <maya/MFnLight.h>
#include <maya/MFnSpotLight.h>
#include <maya/MFnLightDataAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MAngle.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnIkJoint.h>

#include <maya/MPlug.h>
#include <maya/MDataHandle.h>

#include <maya/MFnPhongShader.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnBlinnShader.h>

#include <maya/MAnimControl.h>
#include <maya/MFnBlendShapeDeformer.h>

#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cassert>
#include <direct.h>
#include <vector>
#include "FileDialog.h"

#ifdef WIN32
#include "FileDialog_WIN.h"
#endif


class SIO2_ExporterCmd : public MPxCommand
{
	public:
		// Precision amount, used during rounding
		const static int PRECISION = 3;
		// This is the maximun number of textures 
		// currently supported by SIO2.
		// Used by writeObject and its helper
		// functions.
		const static int MAX_TEXTURE_CHANNELS = 3;
		
		std::vector<std::vector<int>> m_vBoneVertexInfluence;
		
		// Used while exporting blend shapes and it contains
		// the names of the meshes not exported. 
		std::vector<std::string> m_vNameMeshNotExported;
		
		bool m_bExportSelection;
		bool m_bVerbose;
		bool m_bUseBlendShapes;
		bool m_bConvert2BackFaceCulling;
		bool m_bCorrectUVs;
		MString m_sDesitnationDir;
		MPointArray meshVertices;
	
		FileDialog *fileDialog;

		// Constructor
		SIO2_ExporterCmd();
		// File Save Dialog 
		
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
		virtual MStatus doIt( const MArgList& args );

		MStatus exportSelection();

		MStatus exportAll();
		
		// This function exports a single camera object
		// into the SIO2 v1.5.3 format.
		// Currently NOT SUPPORTED:
		// -IPO
		MStatus exportCamera(MObject obj);
		
		// This function exports a single light object
		// into the SIO2 v1.5.3 format.
		// Only Supports SpotLights
		// Currently NOT SUPPORTED:
		// -IPO
		MStatus exportLight(MObject obj);

		// This function places the texture
		// images int the /images folder of the 
		// .sio2 file.
		// Currently NOT SUPPORTED:
		// -Check for file extenssion other thant .ogg
		MStatus exportImages(MObject obj);

		// This function exprots a given material
		// to the SIO2 format.
		// Refer to writeMatTextureInfo bellow for more details.
		// Currently NOT SUPPORTED:
		// -FRICTION
		// -RESTITUTION
		// -ALPHA LEVEL
		// -BLEND MODE
		MStatus exportMaterial(MObject obj);

		// This function exports a given object
		// to the SIO2 format.
		MStatus exportObject(MObject obj);

		static MSyntax pluginSyntax();

		static void * creator();

	protected:
		// Used to optimize the floating point 
		// values. Taken from the sio2_exporter.py
		float optimize_float(float num);

		// Helper function to handle rounding 
		// with precision.
		float round(float val, int precision);

		// Create SIO2 directory structure.
		MStatus createSIO2Directories(std::string base, std::string sceneName);
		
		// Use to write the type of Light into the file.
		// Maya->Blender Light estimation, needs checking.
		MStatus writeLightType( std::ofstream &osf, MObject obj);

		// Sets the softness of the spotlight edges.
		// Only Used with Spot lights.
		// Ray Trace Shadows must be enabled for this to work.
		// else you get 0.
		MStatus writeFOV( std::ofstream &osf, MObject obj);

		// Maya does not seem to have Linear/Quad attenuation like 
		// Blender. More info is needed.
		// I have used cubic as a replacment, making 
		// Linear 0.5 Quad 0.5 when cubic is selected.
		MStatus writeLightAttenuationVals( std::ofstream &osf, MObject obj);

		// Exports the softness of the spotlight edges.
		// In Maya known as the penumbra angle.
		// Only used for Spot Lights.
		MStatus writeSBlend( std::ofstream &osf, MObject obj);

		// This function writes the material channel flags.
		// Texture Channel 0 - Color
		// Texture Channel 1 - AmbientColor
		// Texture Channel 2 - Incandesence
		// Alph Value - Only the the (1.0 - Max(R,G,B)) is
		// used. In SIO2 0 means fully transparent while in
		// Maya it is 1.
		// Translucense - Used to represent the Shininess.
		MStatus writeMatTextureInfo(std::ofstream &osf, MObject obj);

		// Wrtie the mesh transforms.
		MStatus writeMeshTransforms(std::ofstream &osf, MObject obj);

		// Write the location of the mesh.
		MStatus writeMeshLocation(std::ofstream &osf, MObject obj);

		// This function writes the BOFFSET
		MStatus writeMeshBoffset(std::ofstream &osf, MObject obj);
		
		// This function writes the vertices of the mesh
		// into the format supported by SIO2.
		MStatus writeMeshVerteices(std::ofstream &osf, MObject obj);
		
		// This function writes the Color.
		MStatus writeMeshVertColor(std::ofstream &osf, MObject obj);
		
		// This function writes the Color.
		MStatus writeMeshVertNormals(std::ofstream &osf, MObject obj);

		// This function writes the UV for each channel.
		MStatus writeMeshTexCoords(std::ofstream &osf, MObject obj);

		// Used to write bone data for each deformer.
		MStatus writeMeshSkinClusters(std::ofstream &osf, MObject obj);

		// Used to write the vertex index.
		MStatus writeVertexIndices(MIntArray & outVertexIndeces, MPointArray & vertexList, MIntArray & trisData, std::ofstream &osf);
		
		MStatus writeVertexIndicesFromMesh(std::ofstream &osf, MDagPath meshDagPath, MPointArray & vertexList);

		MStatus writeMeshAnimData(std::ofstream &osf, MObject obj);

		MStatus writeFVert(std::ofstream &osf, MPoint vert);

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
		MStatus GetPointsAtTime(const MDagPath& dagPath, const MTime& mayaTime, MPointArray& points );

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
		MStatus GetPointsAtTimeContext(const MDagPath& dagPath, const MTime& mayaTime, MPointArray& points );

		MStatus findAnimKeyFrames(const MDagPath& dagPath, std::vector<double> & vKeyFrames);
		// Used to conver from Radians to Degrees
		float convertRadsToDeg(float angle);

		std::string removeUnwantedChar(std::string name);
		
		// Helper function used to retrive the filename 
		// without the directory path.
		std::string retriveTextureFileName(MFnDependencyNode dpNode);

		MStatus printParentTrace(MObject obj, std::string level);
		
		MStatus printChildTrace(MObject obj, std::string level);

		MVector retriveTranslation(MObject obj);

		// Use to detect if a file is an audio file.
		bool isSoundBuffer(std::string filename);

		void printFuntionList(MObject obj);

		// This function is used to check if the key frame list contains 
		// the given frame. When exporting Bled Shape, MAYA will find 
		// a key frame for every shape keyed so if you have 6 keyed shapes
		// and your animation contains 2 frame it will find 6*2 = 12 keyframes.
		// If you are not using all of the keyed shapes in your animation
		// then some frames will be repeated. Makes senes ??? 
		bool containsAnimationFrame(MTime frame, const std::vector<double> &vKeyFrames);

		bool containsKeyFrameAnimation(MObject obj);

		bool shouldExportMaterial(std::string matMeshName);

		void disableBlendShapes(MObject obj);

		double findMax(double a, double b);

		MIntArray GetLocalIndex( MIntArray & getVertices, MIntArray & getTriangle);

		MStatus getVertexIndices(MIntArray & outVertexIndeces, MPointArray & vertexList, MIntArray & trisData);
		

		bool containsUV(MFloatArray u_coords, MFloatArray v_coords , double u, double v);

		void createMesh();
		

};

#endif