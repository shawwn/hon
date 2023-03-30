
Archive Contents
----------------------------
3ds max Plugin Wizard Functionality and Template files.
----------------------------


Outline
----------------------------
The 3ds max Plugin Wizard allows you to create plugin projects for 3ds max 6
through a wizard interface in the Visual Studio 7/7.1 IDE.
----------------------------


Installing
----------------------------
1. Extract the archive to your local disk.

2. Open the 3dsmaxPluginWizard.vsz file (in the 3dsmaxPluginWizard directory root) 
   in a text editor and edit the ABSOLUTE PATH parameter to reflect the new location of the 
   3dsmaxPluginWizard root directory. Do not add a backslash after the directory name.

	Param="ABSOLUTE_PATH = [Absolute Path Location of 3dsmaxPluginWizard Root Directory]"

3. Copy the following files from the 3dsmaxPluginWizard root to the 'vc7/vcprojects'
   directory under your Visual Studio install (e.g. C:\Program Files\Microsoft Visual Studio .NET\Vc7\vcprojects):
	
	3dsmaxPluginWizard.ico
	3dsmaxPluginWizard.vsdir
	3dsmaxPluginWizard.vsz

4. At this point the 3ds max Plugin Wizard project should appear under File menu:New:Projects:Visual C++ Projects
   in Visual Studio.
	
----------------------------


Usage
----------------------------
The wizard will guide you through the steps involved in setting up your 3ds max 6 plugin project.
----------------------------


Feedback
----------------------------
Please send any and all feedback to sparks_admin@discreet.com.
----------------------------




