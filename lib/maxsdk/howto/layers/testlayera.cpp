/**********************************************************************
 *<
	FILE: testLayerA.cpp

	DESCRIPTION:	This Plugin Shows How To:

					Iterate over scene objects
					Query what layer each object is on

					Iterate over scene layers
					Query what objects each layer 'contains'

					Register a callback notification for a user defined
					scene event.
					
	OVERVIEW:		In 3DSMax, layers do not contain information about
					objects they are associated with, but rather, the 
					reverse. Objects contain information about what
					layers they are associated with. 
					
					Thus by demonstrating both directions of
					associativity, developers can easily manipulate the
					layer data they need.
					
					This plugin also demonstrates registering a callback
					that occurs when the user selects the Modify Command 
					Panel.

	3DSMAX USAGE:	In 3DSMax, create a scene with multiple layers, with
					a few objects on each layer. Select the Modify Panel.
					
					A dialog will open that contains the names of all the
					objects in a scene and the layer they belong to. It
					also lists the layer's in the scene and the objects' 
					that belong to each layer.

					//	IMPORTANT NOTE: This code assumes no objects in the scene are
					//  in a parent child hierarchy. 
					// 	Otherwise this code should include recursive functionality.

	CREATED BY:		Chris Johnson

	HISTORY:		Started on April 12 2005

 *>	Copyright (c) 2005, All Rights Reserved.
 **********************************************************************/

#include "testLayerA.h"
#include "notify.h"				//For callback registration
#include "maxapi.h"				//For GetCOREInterface()
#include "Tab.h"				//For Tab< >, a usefull container	

#include "inode.h"				// For INode objects

// The three classes for managing layers in 3DSMax
#include "ilayermanager.h"		// Contains a pure virtual class. 
#include "iLayer.h"				// Contains a pure virtual class.
#include "ILayerProperties.h"	// Contains two function published class's that 
								// are the core for the maxscript exposure


extern HINSTANCE hInstance;
#define LayerBoss_CLASS_ID	Class_ID(0xc82d3366, 0x350b7167)

#define OPEN_LAYERBOSS 0x01

class LayerBoss : public GUP {
	public:

		static HWND hParams;

		// GUP Methods
		DWORD	Start			( );
		void	Stop			( );
		DWORD	Control			( DWORD parameter );
		
		static BOOL BossProc(HWND hDlg, UINT msg, WPARAM  wParam, LPARAM lParam);
		void DoLayerBossDlg();
		
		// Loading/Saving
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		//Constructor/Destructor
		LayerBoss();
		~LayerBoss();		
};



class LayerBossClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new LayerBoss(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return LayerBoss_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("LayerBoss"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	

};

static LayerBossClassDesc LayerBossDesc;
ClassDesc2* GetLayerBossDesc() { return &LayerBossDesc; }



LayerBoss::LayerBoss()
{

}

LayerBoss::~LayerBoss()
{

}

//This function gets called when the user switches to the modify
//command panel in 3dsmax, since it gets registers itself as a
//callback in LayerBoss::Start(). See below.
static void myEventHandler(void* param, NotifyInfo* info)
{
	GUP* gup = (GUP*)param;
	gup->Control(OPEN_LAYERBOSS);
}

BOOL LayerBoss::BossProc(HWND hDlg, UINT msg, WPARAM  wParam, LPARAM lParam)
{
	GUP* gup = (GUP*) GetWindowLong(hDlg, DWL_USER);
	switch (msg)
	{
	case WM_INITDIALOG:
		{		
			gup = (GUP*)lParam;
			SetWindowLong(hDlg, DWL_USER, lParam);
			
			//Get the interface pointer that enables access to almost
			//all the functionality of Max
			Interface* ip = GetCOREInterface();
			
			ip->RegisterDlgWnd(hDlg);
			
			//==========================================================
			//Example of walking thru the objects in a scene
			//Example of Querying the layer that the object references
			//==========================================================
			
			//Get the pointer to the layer manager
			ILayerManager* lm = (ILayerManager*) GetCOREInterface()->GetScenePointer()->GetReference(10);
			
			int layerCount = lm->GetLayerCount();
			ILayer* zeroLayer = lm->GetLayer(_T("0"));
			
			//All objects in a scene have either another object as a parent, or
			//the root node as a parent. If an object has no in scene parent then
			//it's parent is the root node.
			INode* pRootNode = ip->GetRootNode();
			int objectCount = pRootNode->NumberOfChildren();
			
			// Update the listbox
			SendDlgItemMessage(hDlg, IDC_LAYER_LIST, LB_ADDSTRING, NULL, (LPARAM) "=========================" );
			SendDlgItemMessage(hDlg, IDC_LAYER_LIST, LB_ADDSTRING, NULL, (LPARAM) "Listing of Scene Objects" );

			//Iterate thru all the objects in the scene and report their name
			//and what layer they are on.
			for (int i = 0; i < objectCount; i++)
			{
				//Get the object
				INode* cNode = pRootNode->GetChildNode(i);
				//Find out what layer it is on
				ILayer* aLayer = (ILayer*) cNode->GetReference(NODE_LAYER_REF);

				//Get the object name
				TSTR str1 = cNode->GetName();
				TSTR str2 = ", is on layer ";
				//Get the layer name
				TSTR str3 = aLayer->GetName();
				str1 = str1 + str2 + str3;
				
				//Add the string to the list box here
				SendDlgItemMessage(hDlg, IDC_LAYER_LIST, LB_ADDSTRING, NULL, (LPARAM) (TCHAR*) str1  );
			}		
			// Update the listbox
			SendDlgItemMessage(hDlg, IDC_LAYER_LIST, LB_ADDSTRING, NULL, (LPARAM) "=========================" );
			SendDlgItemMessage(hDlg, IDC_LAYER_LIST, LB_ADDSTRING, NULL, (LPARAM) "Listing of Scene Layers" );

			//==========================================================
			//Example of walking thru the layers in a scene
			//This walks thru each layer and reports the objects it 'contains'
			//In Max, Layers don't reference the objects, but it is backwards,
			//	rather, objects reference the layers.
			//==========================================================

			//Get Access to the function published Layer Manager. This is the only SDK
			//Layer Manager that can iterate over the layers by integer index!!!
			//See ILayerProperties.h for the Interface_ID
			IFPLayerManager* LM = static_cast<IFPLayerManager *>(GetCOREInterface(LAYERMANAGER_INTERFACE));

			layerCount = LM->getCount();
			for (int i = 0; i < layerCount; i++)
			{
				//Get a pointer to the layer
				ILayerProperties* hLayer = LM->getLayer(i);
				//Get the layer name
				TCHAR* layerName = hLayer->getName();

				SendDlgItemMessage(hDlg, IDC_LAYER_LIST, LB_ADDSTRING, NULL, (LPARAM)layerName );

					Tab<INode*> objs; //A container for the objects on the layer
					hLayer->Nodes(objs); //Populate the container with the objects
					
					//Report the names of the objects on each layer
					for (int i = 0; i < objs.Count(); i++)
					{
						TSTR str3 = "   ";
						INode* hObj = objs[i];
						TSTR str4 = hObj->GetName();
						str3 += str4;
						SendDlgItemMessage(hDlg, IDC_LAYER_LIST, LB_ADDSTRING, NULL, (LPARAM) (TCHAR*) str3  );
					}
			} //End for loop
			return TRUE;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_OK:
				{
					EndDialog(hDlg, 0);
					return TRUE;
				}
			}
			break;
		}
	}
	return FALSE;
}
// Activate and Stay Resident
DWORD LayerBoss::Start( ) {
	//This callback will call the myEventHandler function defined above 
	//when the modify command panel gets opened.
	RegisterNotification(myEventHandler, this, NOTIFY_MODPANEL_SEL_CHANGED);
	return GUPRESULT_KEEP;
}

void LayerBoss::Stop( ) {
	UnRegisterNotification(myEventHandler, this, NOTIFY_MODPANEL_SEL_CHANGED);
}

DWORD LayerBoss::Control( DWORD parameter ) {
	switch (parameter)
	{
	case OPEN_LAYERBOSS: 
		{
			DoLayerBossDlg();
			break;
		}
	}
	return 0;
}
void LayerBoss::DoLayerBossDlg()
{
	DialogBoxParam(
			hInstance, 
			MAKEINTRESOURCE(IDD_LAYERBOSS),
			GetCOREInterface()->GetMAXHWnd(), 
			(DLGPROC) LayerBoss::BossProc, 
			(LPARAM) this);	
}
IOResult LayerBoss::Save(ISave *isave)
{
	return IO_OK;
}

IOResult LayerBoss::Load(ILoad *iload)
{
	return IO_OK;
}


