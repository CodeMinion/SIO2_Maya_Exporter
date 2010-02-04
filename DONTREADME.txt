1 - Load .mll plug in in Maya 2009.

2 - From Mel prompt execute "SIO2_Exporter_1_3_5 -n SceneFolderName -d destinationDirectoryPath -bs"

3 - Once exported, you will need to compress it yourself to transfer it to Mac.

4 - Once on the Mac I found that it saves you a lot of headaches if you:

4.1 - Uncompress the folder

4.2 - cd into the uncompressed folder

4.3 - Use the command line to zip it into your .sio2 file. 
     
    Command :  zip -9 -o filename.sio2 -r *