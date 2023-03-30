/**********************************************************************
 *<
	FILE: StackBoss.cpp

	DESCRIPTION:	Main loading of the Stackboss modifier including XML file 
					parser.  This is used to define how the modifier is used 

	CREATED BY:		Neil Hazzard

	HISTORY:		October 2003

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "StackBoss.h"
#include "StackBossMod.h"

#include "modstack.h"
#include "notify.h"

#include "atlbase.h"
#include "msxml2.h"

#define STACKBOSS_CLASS_ID	Class_ID(0x537cbf77, 0x6b0eb82e)
#define STACK_BOSS_PARAM_REF 0


typedef struct{
	INode* node; 
	Modifier* mod; 
	ModContext* mc;
}PreModDelete;


class StackBoss : public GUP {
	private:
		Tab<Class_ID> classList;
		bool classListLoaded;
		bool includeList;
		bool globalMod;
		bool addModOnLoad;
		int nTotalNodeCount;
		int nCurNode;
		Modifier * globalStackBoss;
		Interface * ip;


	public:
		IXMLDOMDocument * pXMLDoc;
		IXMLDOMNode * pRoot;		//this is our root node 

		static HWND hParams;



		// GUP Methods
		DWORD	Start			( );
		void	Stop			( );
		DWORD	Control			( DWORD parameter );
		
		// Loading/Saving
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		
		//Constructor/Destructor

		StackBoss();
		~StackBoss();		

		bool LoadClassList();
		void AddStackBossModifier(INode * node, Modifier * mod = NULL, ModContext * mc = NULL);
		void AddStackBossToScene(INode * parentNode);
		bool FindStackBossModifier (INode* node);
		static void NotifyHandler(void *param, NotifyInfo *info );

};


class StackBossClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new StackBoss(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return STACKBOSS_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("StackBoss"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static StackBossClassDesc StackBossDesc;
ClassDesc2* GetStackBossDesc() { return &StackBossDesc; }

static Class_ID ConvertBSTRtoClassID(BSTR id)
{
	USES_CONVERSION;
	TCHAR * b = OLE2T(id);
	TSTR cid(b);
	int len = cid.length();
	int sub = cid.first(_T(','));
	TSTR sub1 = cid.Substr(0,sub);
	TSTR sub2 = cid.Substr(sub+1,len);
	ulong aa = strtoul(sub1.data(),NULL,16);
	ulong bb = strtoul(sub2.data(),NULL,16);
	return Class_ID(aa,bb);
}


StackBoss::StackBoss()
{
	classListLoaded = false;
	includeList = true;
	addModOnLoad = false;
	classList.SetCount(0);
	classListLoaded = LoadClassList();
	ip = Max();
}

StackBoss::~StackBoss()
{

}

// Activate and Stay Resident
DWORD StackBoss::Start( ) {
	
	// TODO: Return if you want remain loaded or not
	RegisterNotification(NotifyHandler,this, NOTIFY_PRE_MODIFIER_DELETED);
	RegisterNotification(NotifyHandler,this, NOTIFY_SCENE_ADDED_NODE);
	RegisterNotification(NotifyHandler,this, NOTIFY_FILE_POST_OPEN);

	return GUPRESULT_KEEP;
}

void StackBoss::Stop( ) {
	// TODO: Do plugin un-initialization here
	UnRegisterNotification(NotifyHandler,this, NOTIFY_PRE_MODIFIER_DELETED);
	UnRegisterNotification(NotifyHandler,this, NOTIFY_SCENE_ADDED_NODE);
	UnRegisterNotification(NotifyHandler,this, NOTIFY_FILE_POST_OPEN);

}

DWORD StackBoss::Control( DWORD parameter ) {
	return 0;
}

IOResult StackBoss::Save(ISave *isave)
{
	return IO_OK;
}

IOResult StackBoss::Load(ILoad *iload)
{
	return IO_OK;
}

bool StackBoss::LoadClassList()
{
	TCHAR path[256];
	BSTR  strIncludedEntity = SysAllocString(L"included");
	BSTR  strExcludedEntity = SysAllocString(L"excluded");
	BSTR  strGlobal  = SysAllocString(L"Global");
	LONG value;
	HRESULT hr;

	IXMLDOMNode *pIDOMNode = NULL;		// the property nodes
	IXMLDOMNodeList * pIDOMNodeList;
	_tcscpy(path,GetCOREInterface()->GetDir(APP_PLUGCFG_DIR));
	_tcscat(path,_T("\\stackboss.xml"));

	
	// Check the return value, hr...
	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pXMLDoc);
	if(FAILED(hr))
		return false;
	// Check the return value, hr...
	hr = pXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&pRoot);
	if(FAILED(hr))
		return false;	

	pXMLDoc->put_preserveWhiteSpace(TRUE);

	VARIANT_BOOL bSuccess = false;

	pXMLDoc->load(CComVariant(path),&bSuccess);
	if(!bSuccess)
		return false ;

	hr = pXMLDoc->getElementsByTagName(strGlobal,&pIDOMNodeList);

	if(!FAILED(hr))
	{
		hr = pIDOMNodeList->get_length(&value);
		pIDOMNodeList->reset();

		// we now iterate through our Entities, pulling the relevent data and storing it in our array
		//although there will only be one !!
		for(int i=0;i<value;i++)
		{
			pIDOMNodeList->get_item(i, &pIDOMNode);
			//The real action starts here..
			if(pIDOMNode )
			{
				IXMLDOMNodeList * childList;  // home to our properties
				HRESULT hr;

				pIDOMNode->get_childNodes(&childList);
				LONG numberOfGlobals;
				hr = childList->get_length(&numberOfGlobals);
				childList->reset();
				for(int j=0;j<numberOfGlobals;j++)
				{
					VARIANT realval;
					VariantInit(&realval);

					IXMLDOMNode *pProperty = NULL;		// the property nodes
					childList->get_item(j,&pProperty);
					BSTR name;
					VARIANT d;
					pProperty->get_nodeName(&name);
					pProperty->get_nodeTypedValue(&d);
					USES_CONVERSION;
					TCHAR * b = OLE2T(name);

					if(_tcscmp(b,(_T("AddOnLoad")))==0)
					{
						HRESULT hr = ::VariantChangeType(&realval, &d, 0, VT_BOOL);
						addModOnLoad = realval.boolVal;

					}
				}
			}
		}
	}

	pIDOMNodeList->Release();
	pIDOMNodeList = NULL;

	hr = pXMLDoc->getElementsByTagName(strIncludedEntity,&pIDOMNodeList);
	if(FAILED(hr))
	{
		hr = pXMLDoc->getElementsByTagName(strExcludedEntity,&pIDOMNodeList);
		if(FAILED(hr))
			return false;
		else
			includeList = false;
	}

	
	hr = pIDOMNodeList->get_length(&value);
	pIDOMNodeList->reset();

	// we now iterate through our Entities, pulling the relevent data and storing it in our array
	for(int i=0;i<value;i++)
	{
		pIDOMNodeList->get_item(i, &pIDOMNode);
		if(pIDOMNode )
		{
			IXMLDOMNodeList * childList;  // home to our properties
			HRESULT hr;

			pIDOMNode->get_childNodes(&childList);
			LONG value;
			hr = childList->get_length(&value);
			if(value >0)
				childList->reset();

			for(int j=0;j<value;j++)
			{
				IXMLDOMNode * pPropNode = NULL;
				BSTR name;
				childList->get_item(j,&pPropNode);
				pPropNode->get_nodeName(&name);

				if(wcscmp(name,(L"classid"))==0)
				{
					IXMLDOMNode *pClassIDNode = NULL;
					pPropNode->get_firstChild(&pClassIDNode);
					VARIANT Classid;
					VARIANT realval;
					VariantInit(&realval);

					pClassIDNode->get_nodeTypedValue(&Classid);
					pClassIDNode->Release();
					pClassIDNode = NULL;
					USES_CONVERSION;
					TCHAR * b = OLE2T(name);
					Class_ID cid = ConvertBSTRtoClassID(Classid.bstrVal);
					classList.Append(1,&cid);
				}

			}

		}
		pIDOMNode->Release();
		pIDOMNode = NULL;
	}
	//check Global...


	return true;
}

bool StackBoss::FindStackBossModifier (INode* node)
{
	// Get object from node. Abort if no object.
	Object* pObj = node->GetObjectRef();


	if (!pObj) return false;

	// Is derived object ?
	while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* pDerObj = (IDerivedObject*) pObj;

		// Iterate over all entries of the modifier stack.
		int Idx = 0;
		while (Idx < pDerObj->NumModifiers())
		{
			// Get current modifier.
			Modifier* mod = pDerObj->GetModifier(Idx);

			// Is this Skin ?
			if (mod->ClassID() == STACKBOSSMOD_CLASS_ID)
			{
				// Yes -> Exit.
				return true;
			}

			// Next modifier stack entry.
			Idx++;
		}
		pObj = pDerObj->GetObjRef();
	}

	// Not found.
	return false;
}

void StackBoss::AddStackBossModifier(INode * node, Modifier * mod, ModContext * mContext)
{
	bool valid = false;
	Object *obj = node->GetObjectRef();

	//if we don't have a list of supported objects - simply bail.  We check the mod, if this is valid, we do let it through
	//as this will come from the delete notifier.

	if(!classListLoaded && !mod)
		return;

	for(int i=0; i<classList.Count();i++)
	{
		if(includeList && obj->ClassID()==classList[i])
		{
			valid = true;
			break;
		}
		else if(!includeList && obj->ClassID()!=classList[i])
		{
			valid = true;
			break;
		}
	}
	//only if we aren't valid and there is no modifier present already.
	if(!valid && !mod)
		return;
	

	Modifier * stackBossMod;
	ModContext *mc;

	if(mod)
		stackBossMod = mod;
	else
		stackBossMod = (Modifier*)CreateInstance(OSM_CLASS_ID,STACKBOSSMOD_CLASS_ID);

	Object *bo = node->GetObjectRef();
	IDerivedObject *derob = NULL;
	if(!bo || (bo->SuperClassID() != GEN_DERIVOB_CLASS_ID))
	{
		derob = CreateDerivedObject(obj);
		node->SetObjectRef(derob);
	}
	else
		derob = (IDerivedObject*) bo;

	stackBossMod->SetName("Stack Boss");

	if(mContext)
		mc = mContext;
	else
		mc = new ModContext(new Matrix3(1), NULL, NULL);

	derob->AddModifier(stackBossMod,mc);
	derob->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE); 

	node->SetObjectRef(derob);
	node->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	node->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	GetCOREInterface()->ForceCompleteRedraw();
}

void StackBoss::AddStackBossToScene(INode * parentNode)
{
	nCurNode++;
	ip->ProgressUpdate((int)((float)nCurNode/nTotalNodeCount*100.0f)); 
	if(!FindStackBossModifier(parentNode))
		AddStackBossModifier(parentNode);

	for(int i=0; i<parentNode->NumberOfChildren();i++)
	{
		AddStackBossToScene(parentNode->GetChildNode(i));
	}
}

void PreProcess(INode* node, int& nodeCount)
{
	nodeCount++;
	for (int c = 0; c < node->NumberOfChildren(); c++) 
	{
		PreProcess(node->GetChildNode(c), nodeCount);
	}
}
// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}

void StackBoss::NotifyHandler(void *param, NotifyInfo *info )
{
	StackBoss * sb = (StackBoss*) param;
	DbgAssert(sb != NULL);

	switch(info->intcode)
	{

		case NOTIFY_PRE_MODIFIER_DELETED:
		{
			PreModDelete * data = (PreModDelete*)info->callParam;
			if(data->mod->ClassID()== STACKBOSSMOD_CLASS_ID)
			{
				IParamBlock2 * pblock2 = (IParamBlock2*)data->mod->GetReference(STACK_BOSS_PARAM_REF);
				BOOL Delete = FALSE;
				pblock2->GetValue(StackBossMod::pb_delete,0,Delete,FOREVER);
				if(!Delete)		
				{
					sb->AddStackBossModifier(data->node,data->mod,data->mc)	;
				}
			}
		}
		break;

		case NOTIFY_SCENE_ADDED_NODE:
		{
			INode * node = (INode*)info->callParam;
			if(!node)
				return;
			if(sb->FindStackBossModifier(node))
				return;
			sb->AddStackBossModifier(node);
			break;
		}
		case NOTIFY_FILE_POST_OPEN:
		{
			if(sb->addModOnLoad)
			{
				sb->nTotalNodeCount = 0;
				sb->nCurNode = 0;
				PreProcess(sb->ip->GetRootNode(), sb->nTotalNodeCount);
				// Startup the progress bar.
				sb->ip->ProgressStart(_T("Stack Bossing"), TRUE, fn, NULL);
				INode * rootNode = sb->ip->GetRootNode();
				for(int i=0; i<rootNode->NumberOfChildren(); i++)
				{
					sb->AddStackBossToScene(rootNode->GetChildNode(i));
				}
				sb->ip->ProgressEnd();
			}
			break;
		}
	}


}