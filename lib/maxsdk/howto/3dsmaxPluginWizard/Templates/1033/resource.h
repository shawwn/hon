//{{NO_DEPENDENCIES}}
// Microsoft Developer Studio generated include file.
// Used by [!output PROJECT_NAME].rc
//


#define IDS_LIBDESCRIPTION              1
#define IDS_CATEGORY                    2
#define IDS_CLASS_NAME                  3
#define IDS_PARAMS                      4
#define IDS_SPIN                        5
[!if UI_BY_MAX]
#define IDS_COMMENT                     6
[!endif] 

[!if ATMOSPHERIC_TYPE]
#define IDS_GIZMOS						7
#define IDC_GIZMOLIST					1497
#define IDC_ADDGIZMO					1498
#define IDC_DELGIZMO					1499
[!endif]

[!if MATERIAL_TYPE]
#define IDS_MTL1						7
#define IDS_MTL1ON						8
#define IDC_MTL1						1497
#define IDC_MTLON1						1498
[!endif]

[!if SKIN_GIZMO_TYPE]
#define IDS_GIZMOPARAM_NAME				7
#define IDS_PW_GIZMOCATEGORY			8
#define IDC_NAME						1498
[!endif]

[!if SPACE_WARP_TYPE]
#define IDS_SPIN_OBJ					7
#define IDC_SPIN_OBJ					1497
#define IDC_EDIT_OBJ					1498
[!endif]


[!if TEX_TYPE]
#define IDS_COORDS                      7
[!endif]
[!if FILTER_KERNEL_TYPE]
#define IDS_PARAM1                      7
#define IDS_PARAM2                      8
[!endif]
[!if SAMPLER_TYPE]
#define IDS_QUALITY                     7
#define IDS_ENABLE                      8
#define IDS_AD_ENABLE                   9
#define IDS_AD_THRESH                   10
[!endif]
[!if SHADER_TYPE]
#define IDS_MN_DIFFUSE                  7
#define IDS_MN_BRIGHTNESS               8
#define IDS_MN_OPACITY                  9
#define IDS_MN_NONE                     10
#define IDS_COLOR                       11
#define IDS_PARAMCHG                    12
#define IDS_MN_DIFFUSE_I                13
#define IDS_MN_BRIGHTNESS_I             14
#define IDS_MN_OPACITY_I                15
#define IDD_SHADER_SKEL1                148
#define IDC_TR_EDIT                     220
#define IDC_TR_SPIN                     330
#define IDC_MAPON_TR                    1038
#define IDC_HIGHLIGHT                   1062
#define IDC_BR_EDIT                     1807
#define IDC_BR_SPIN                     1808
#define IDC_MAPON_BR                    1809
#define IDC_MAPON_CLR                   1814
[!endif]

#define IDD_PANEL                       101
[!if IMAGE_FILTER_COMPOSITOR_TYPE]
#define IDD_ABOUT                       102
[!endif]
[!if SPACE_WARP_TYPE]
#define IDD_PANEL_OBJ                   102
[!endif]

#define IDC_CLOSEBUTTON                 1000
#define IDC_DOSTUFF                     1000
#define IDC_COLOR                       1456
#define IDC_EDIT                        1490
#define IDC_SPIN                        1496

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
[!if IMAGE_FILTER_COMPOSITOR_TYPE || SPACE_WARP_TYPE]
#define _APS_NEXT_RESOURCE_VALUE        102
[!else]
#define _APS_NEXT_RESOURCE_VALUE        101
[!endif]
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
