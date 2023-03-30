/*
This file contains the array that is referenced by the plugin wizard.
Changes to this array will be reflected in the wizard directly.
*/

	var plugitem_caption	= 0; // caption of the plugin class
	var plugitem_key		= 1; // Associate key type
	var plugitem_ext		= 2; // Extension of this plugin (eg: dlr, dlo, etc.)
	var plugitem_sid		= 3; // Super Class ID
	var plugitem_bclass1	= 4; // super class name that plugin should be derived from
	var plugitem_bclass2	= 5; // alternative super class name
	var plugitem_supported	= 6; // if this type of plugin is fully supported by appwizard
	var plugitem_rollout	= 7; // TRUE if the dialog is a rollout (eg:object plugins)
	var plugitem_staticType = 8; // TRUE if static instance of this plugin type has to be created
	
	var plugin_count = 35;
	var PlugArray	 = new Array();
	PlugArray = [	
	//	caption						key								 ext	sid							bclass1					bclass2		supported  rollout staticType
	[ "Anti-Aliasing Filters",		"FILTER_KERNEL_TYPE",			"dlk",	"FILTER_KERNEL_CLASS_ID",	"FilterKernel",			"",				true,	true,  false ],
	[ "Atmospheric",				"ATMOSPHERIC_TYPE",				"dlr",	"ATMOSPHERIC_CLASS_ID",		"Atmospheric",			"",				true,	true,  false ],
	[ "Cameras",					"CAMERA_TYPE",					"dlo",	"CAMERA_CLASS_ID",			"CameraObject",			"GenCamera",	false,	true,  false ],
	[ "Color Selector",				"COLPICK_TYPE",					"dlu",	"COLPICK_CLASS_ID",			"ColPick",				"",				true,	false, false ],	
	[ "Construction Grid Objects",	"GRID_OBJECT_TYPE",				"dlo",	"UNKNOWN_CLASS_ID",			"None",					"",				false,	true,  false ],
	[ "Controllers",				"CONTROLLER_TYPE",				"dlc",	"CTRL_POSITION_CLASS_ID",	"Control",				"StdControl",	false,	true,  false ],
	[ "File Export",				"FILE_EXPORT_TYPE",				"dle",	"SCENE_EXPORT_CLASS_ID",	"SceneExport",			"",				true,	false, false ],
	[ "File Import",				"FILE_IMPORT_TYPE",				"dli",	"SCENE_IMPORT_CLASS_ID",	"SceneImport",			"",				true,	false, false ],
	[ "File List",					"FILE_LIST_TYPE",				"",		"UNKNOWN_CLASS_ID",			"None",					"",				false,	true,  false ],
//	[ "Front End Controllers",		"FRONT_END_CONTROLLER_TYPE",	"dlu",	"FRONTEND_CONTROL_CLASS_ID","FrontEndController",	"",				true,	true,  true  ],
	[ "Global Utility Plug-Ins",	"GUP_TYPE",						"gup",	"GUP_CLASS_ID",				"GUP",					"",				true,	false, false ],	
	[ "Helper Objects",				"HELPER_OBJECT_TYPE",			"dlo",	"HELPER_CLASS_ID",			"HelperObject",			"ConstObject",	false,	true,  false ],
	[ "IK Solvers",					"IK_TYPE",						"dlc",	"IK_SOLVER_CLASS_ID",		"IKSolver",				"",				true,	false,  false ],
	[ "Image Filter / Compositor",	"IMAGE_FILTER_COMPOSITOR_TYPE", "flt",	"FLT_CLASS_ID",				"ImageFilter",			"",				true,	false, false ],
	[ "Image Loader / Saver",		"IMAGE_LOADER_SAVER_TYPE",		"bmi",	"BMM_IO_CLASS_ID",			"BitmapIO",				"",				false,	true,  false ],
	[ "Image Viewer",				"IMAGE_VIEWER_TYPE",			"dlf",	"BMM_FILTER_CLASS_ID",		"ViewFile",				"",				false,	true,  false ],
	[ "Lights",						"LIGHT_TYPE",					"dlo",	"LIGHT_CLASS_ID",			"LightObject",			"GenLight",		false,	true,  false ],
	[ "Manipulators",				"MANIP_TYPE",					"dlo",	"HELPER_CLASS_ID",			"Manipulator",			"SimpleManipulator",		true,	true,  false ],
	[ "Materials",					"MATERIAL_TYPE",				"dlt",	"MATERIAL_CLASS_ID",		"Mtl",					"",				true,	true,  false ],
	[ "Modifiers",					"MODIFIER_TYPE",				"dlm",	"OSM_CLASS_ID",				"Modifier",				"SimpleMod2",	true,	true,  false ],
	[ "NURBS Objects",				"NURBS_OBJECT_TYPE",			"dlo",	"UNKNOWN_CLASS_ID",			"None",					"",				false,	true,  false ],
	[ "Particle Systems / Effects",	"PARTICLE_TYPE",				"dlo",	"GEOMOBJECT_CLASS_ID",		"ParticleObject",		"SimpleParticle",false, true,  false ],
	[ "Patch Objects",				"PATCH_OBJECT_TYPE",			"dlo",	"GEOMOBJECT_CLASS_ID",		"PatchObject",			"",				false,	true,  false ],
	[ "Procedural Objects",			"PROCEDURAL_OBJECT_TYPE",		"dlo",	"GEOMOBJECT_CLASS_ID",		"GeomObject",			"SimpleObject2",true,	true,  false ],
	[ "Renderer",					"RENDERER_TYPE",				"dlr",	"RENDERER_CLASS_ID",		"Renderer",				"",				false,	true,  false ],
	[ "Rendering Effects",			"RENDER_EFFECT_TYPE",			"dlv",	"RENDER_EFFECT_CLASS_ID",	"Effect",				"",				true,	true,  false ],
	[ "Samplers",					"SAMPLER_TYPE",					"dlh",	"SAMPLER_CLASS_ID",			"Sampler",				"",				true,	true,  false ],
	[ "Shaders",					"SHADER_TYPE",					"dlb",  "SHADER_CLASS_ID ",			"Shader",				"",				true,	true,  false ],	
	[ "Shadow Generator",			"SHADOW_TYPE",					"dlo",	"SHADOW_TYPE_CLASS_ID",		"ShadowType",			"",				true,	true,  false ],	
	[ "Skin Deformer Gizmo",		"SKIN_GIZMO_TYPE",				"dlm",	"REF_TARGET_CLASS_ID",		"GizmoClass",			"",				true,	true,  false ],	
	[ "Sound Plug-Ins",				"SOUND_TYPE",					"dlo",	"SOUNDOBJ_CLASS_ID",		"SoundObj",				"",				false,	true,  false ],
	[ "Space Warps",				"SPACE_WARP_TYPE",				"dlm",	"WSM_CLASS_ID",				"SimpleWSMMod",			"",				true,	true,  false ],
//	[ "Spline Shapes",				"SPLINE_SHAPE_TYPE",			"dlo",	"SHAPE_CLASS_ID",			"SimpleSpline",			"SimpleShape",	true,	true,  false ],
	[ "Textures 2D",				"TEXTURE_2D_TYPE",				"dlt",	"TEXMAP_CLASS_ID",			"Texmap",				"",				true,	true,  false ],
	[ "Textures 3D",				"TEXTURE_3D_TYPE",				"dlt",	"TEXMAP_CLASS_ID",			"Texmap",				"Tex3D",		true,	true,  false ],
	[ "Track View Utility",			"TRACK_VIEW_UTILITY_TYPE",		"dlu",	"TRACKVIEW_UTILITY_CLASS_ID","TrackViewUtility",	"",				true,	false, true  ],
	[ "Utility",					"UTILITY_TYPE",					"dlu",	"UTILITY_CLASS_ID",			"UtilityObj",			"",				true,	true,  true  ]
];

