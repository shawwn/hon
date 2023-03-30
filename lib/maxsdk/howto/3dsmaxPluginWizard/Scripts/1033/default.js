function OnFinish(selProj, selObj)
{
	//wizard.YesNoAlert( "BREAKPOINT" ); //for testing
	
	try
	{
		var strProjectPath = wizard.FindSymbol('PROJECT_PATH');
		var strProjectName = wizard.FindSymbol('PROJECT_NAME');

		SetupSymbols(); //create symbols needed when rendering
		
		var InfFile = CreateCustomInfFile();
		RenderTemplates( strProjectName, strProjectPath, InfFile );
		InfFile.Delete();

		selProj = CreateCustomProject(strProjectName, strProjectPath);
		//PchSettings(selProj);
		selProj.Object.Save();
	}
	catch(e)
	{
		if (e.description.length != 0)
			SetErrorInfo(e);
		return e.number
	}
}

function CreateCustomProject(strProjectName, strProjectPath)
{
	try
	{
		var strProjTemplatePath = wizard.FindSymbol('PROJECT_TEMPLATE_PATH');
		var strProjTemplate = '';

		var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');
		var strProjFile = strProjectPath + '\\' + strProjectName + '.vcproj'
		var strSolutionFile = strProjectPath + '\\' + strProjectName + '.sln';

		var Solution = dte.Solution;
		var strSolutionName = "";
		if (wizard.FindSymbol("CLOSE_SOLUTION"))
		{
			Solution.Close();
			Solution.Create(strProjectPath, strProjectName);
			//solution.SaveAs(strSolutionFile);
		}

		var strProjectNameWithExt = '';
		strProjectNameWithExt = strProjectName + '.vcproj';

		var oTarget = wizard.FindSymbol("TARGET");
		var prj;
		if (wizard.FindSymbol("WIZARD_TYPE") == vsWizardAddSubProject)  // vsWizardAddSubProject
		{
			var prjItem = oTarget.AddFromFile(strProjFile);
			prj = prjItem.SubProject;
		}
		else
		{
			prj = oTarget.AddFromFile(strProjFile);
		}
		
		return prj;
	}
	catch(e)
	{
		throw e;
	}
}

function GetPluginLibs(ext)
{
	var libs = "";
	var pmap = "";
	if (ext == "bmi" || ext == "bmf" || ext == "dlv" || ext == "bms" || ext == "dlh")
	{
		libs = "bmm.lib core.lib maxutil.lib maxscrpt.lib";
	}
	else if (ext == "dlc")
	{
		libs = "core.lib expr.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib";
	}
	else if(ext == "dle" || ext == "dlf" || ext == "dli")
	{
		libs = "core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib";
	}
	else if (ext == "dlm" || ext == "dlo" || ext == "dlr" || ext == "dlt" || ext == "dlb" || ext == "dlk")
	{
		libs = "bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib manipsys.lib";
	}
	else if (ext == "dlu" || ext == "gup")
	{
		libs = "bmm.lib core.lib geom.lib gfx.lib mesh.lib maxutil.lib maxscrpt.lib gup.lib";
	}
	else if (ext == "flt")
	{
		libs = "bmm.lib core.lib flt.lib maxutil.lib maxscrpt.lib";
	}
	libs += " paramblk2.lib";
	
	if( wizard.FindSymbol('IMAGE_VIEWER_TYPE') )
		libs += " viewFile.lib";
	
	return libs;

}

function PchSettings(proj)
{
	// TODO: specify pch settings
}

function DelFile(fso, strWizTempFile)
{
	try
	{
		if (fso.FileExists(strWizTempFile))
		{
			var tmpFile = fso.GetFile(strWizTempFile);
			tmpFile.Delete();
		}
	}
	catch(e)
	{
		throw e;
	}
}

function CreateCustomInfFile()
{
	try
	{
		var fso, TemplatesFolder, TemplateFiles, strTemplate;
		fso = new ActiveXObject('Scripting.FileSystemObject');

		var TemporaryFolder = 2;
		var tfolder = fso.GetSpecialFolder(TemporaryFolder);
		var strTempFolder = tfolder.Drive + '\\' + tfolder.Name;

		var strWizTempFile = strTempFolder + "\\" + fso.GetTempName();

		var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');
		var strInfFile = strTemplatePath + '\\Templates.inf';
		wizard.RenderTemplate(strInfFile, strWizTempFile);

		var WizTempFile = fso.GetFile(strWizTempFile);
		return WizTempFile;
	}
	catch(e)
	{
		throw e;
	}
}

function GetTargetName(strName, strProjectName)
{
	try
	{
		// TODO: set the name of the rendered file based on the template filename
		var strTarget = strName;

		if (strName == 'readme.txt')
			strTarget = 'ReadMe.txt';

		if (strName == 'sample.txt')
			strTarget = 'Sample.txt';
			
		if (strName == 'dllentry.cpp')
			strTarget = 'DllEntry.cpp';
		
		if (strName == wizard.FindSymbol('PLUGINTYPE_TEMPLATE') + '.cpp')
			strTarget = wizard.FindSymbol('PROJECT_NAME') + '.cpp';
			
		if (strName == 'root.h')
			strTarget = wizard.FindSymbol('PROJECT_NAME') + '.h';
		
		if (strName == 'root.def')
			strTarget = wizard.FindSymbol('PROJECT_NAME') + '.def';
		
		if (strName == 'root.rc')
			strTarget = wizard.FindSymbol('PROJECT_NAME') + '.rc';

		if (strName == 'root.vcproj')
			strTarget = wizard.FindSymbol('PROJECT_NAME') + '.vcproj';

		return strTarget; 
	}
	catch(e)
	{
		throw e;
	}
}

function RenderTemplates(strProjectName, strProjectPath, InfFile)
{
	try
	{
		var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');
		var strTpl = '';
		var strName = '';

		var strTextStream = InfFile.OpenAsTextStream(1, -2);
		while (!strTextStream.AtEndOfStream)
		{
			strTpl = strTextStream.ReadLine();
			if (strTpl != '')
			{
				strName = strTpl;
				var strTarget = GetTargetName(strName, strProjectName);
				var strTemplate = strTemplatePath + '\\' + strTpl;
				var strFile = strProjectPath + '\\' + strTarget;

				var bCopyOnly = false;  //"true" will only copy the file from strTemplate to strTarget without rendering/adding to the project
				var strExt = strName.substr(strName.lastIndexOf("."));
				if(strExt==".bmp" || strExt==".ico" || strExt==".gif" || strExt==".rtf" || strExt==".css")
					bCopyOnly = true;
				wizard.RenderTemplate(strTemplate, strFile, bCopyOnly);
			}
		}
		strTextStream.Close();
		
		//Render the template holding user default settings
		var userDefaultsOutput =  wizard.FindSymbol('HTML_PATH') + '\\userDefaults.js';
		var userDefaultsTemplate = wizard.FindSymbol('TEMPLATES_PATH') + '\\userDefaults_template.js'
		wizard.RenderTemplate( userDefaultsTemplate, userDefaultsOutput, false, true );		
	}
	catch(e)
	{
		throw e;
	}
}

// Converts any backslash characters to double-backslash,
// so the string looks like a string constant
function MakePrintable( s_input ) {
	var s = (""+s_input);
	var ret = "";
	for( var i = 0; i<s.length; i++ )
	{
		if( s.substring(i,i+1)=='\\' )
			ret += '\\\\';
		else ret += s.substring(i,i+1);
   }
   return ret;
}

function RenderToString(strTemplate)
{
	var strTemplateFile = wizard.FindSymbol("TEMPLATES_PATH") + "\\" + strTemplate;
	var strInsertText = wizard.RenderTemplateToString(strTemplateFile);
	return strInsertText;
}

function SetupSymbols( )
{
	try
	{
		// The name of the template for the main .cpp file of the plugin
		var plugintypeTemplate = wizard.FindSymbol("PLUGINTYPE") + '_' + wizard.FindSymbol("SUPER_CLASS_NAME");
		wizard.AddSymbol("PLUGINTYPE_TEMPLATE", plugintypeTemplate);
		
		// Symbol for template classdesc include
		var classdescString = RenderToString("MASTER_CLASSDESC.txt");
		wizard.AddSymbol("TEMPLATESTRING_CLASSDESC", classdescString);
		
		// Symbol for template paramdesc include
		var paramdescString = RenderToString("MASTER_PARAMBLOCKDESC.txt");
		wizard.AddSymbol("TEMPLATESTRING_PARAMBLOCKDESC", paramdescString);
		
		// Symbols for project template
		pExt  = wizard.FindSymbol("PLUGEXT");
		wizard.AddSymbol("PLUGLIBS", GetPluginLibs(pExt));
		var strSDKPath = wizard.FindSymbol("SDKPATH");
		var strSDKPathInclude = strSDKPath + '\\' + 'include';	
		var strSDKPathLib = strSDKPath + '\\' + 'lib';	
		wizard.AddSymbol("SDKPATH_INCLUDE", strSDKPathInclude);
		wizard.AddSymbol("SDKPATH_LIB", strSDKPathLib);
		wizard.AddSymbol("TEMPLATE_COMMENTS", true);
		
		wizard.AddSymbol("SDKPATH_PRINTABLE", MakePrintable( wizard.FindSymbol("SDKPATH") ) );
		wizard.AddSymbol("PLGPATH_PRINTABLE", MakePrintable( wizard.FindSymbol("PLGPATH") ) );
		wizard.AddSymbol("EXEPATH_PRINTABLE", MakePrintable( wizard.FindSymbol("EXEPATH") ) );
	}
	catch(e)
	{
		throw e;
	}
}
