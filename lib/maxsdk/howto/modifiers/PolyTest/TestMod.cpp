/**********************************************************************
 *<
	FILE: TestMod.cpp

	DESCRIPTION:   Testing tool for Polygon-based Meshes.

	CREATED BY: Steve Anderson

	HISTORY: created January 2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#include "PolyTest.h"
#include "iparamm2.h"

const Class_ID kPOLY_TEST_CLASS_ID(0x153b0e75, 0x62192fe7);
const unsigned int kPTEST_PBLOCK_REF(0);

const int kLogSize = 20;

class PolyTestErrorLog : public LocalModData
{
private:
	TSTR mError[kLogSize];
	int mNumErrors;
	bool mUpdated;

public:
	PolyTestErrorLog ();

	LocalModData *Clone() { return new PolyTestErrorLog (); }

	void BeginLog ();
	void AddError (TSTR error);
	int ErrorNum () { return mNumErrors; }
	TSTR Error (int errorID) { return mError[errorID]; }
};

PolyTestErrorLog::PolyTestErrorLog ()
{
	mNumErrors = 0;
	mUpdated = false;
	for (int i=0; i<kLogSize; i++) mError[i] = TSTR("");
}

void PolyTestErrorLog::BeginLog ()
{
	mNumErrors = 0;
	mUpdated = true;
	for (int i=0; i<kLogSize; i++) mError[i] = TSTR("");
}

void PolyTestErrorLog::AddError (TSTR error)
{
	if (mNumErrors == kLogSize) return;
	mError[mNumErrors] = error;
	mNumErrors++;
}

class PolyTestMod : public Modifier {
	IParamBlock2 *mp_pblock;
	static IObjParam *mp_ip;

public:
	PolyTestMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(TSTR& s) {s = GetString(IDS_POLY_TEST_MOD);}  
	virtual Class_ID ClassID() { return kPOLY_TEST_CLASS_ID;}
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);		
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName() {return GetString(IDS_POLY_TEST_MOD);}
	int SubNumToRefNum(int subNum) { return -1; }

	// From modifier
	ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_TEXMAP|PART_SELECT|PART_SUBSEL_TYPE|PART_VERTCOLOR;}
	ChannelMask ChannelsChanged() {return PART_VERTCOLOR;}
	Class_ID InputType() {return polyObjectClassID;}
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	void CheckAllData (MNMesh & mm, UVVert *pVertColor, PolyTestErrorLog *el);
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	
	//NS: New SubObjType API
	int NumSubObjTypes() { return 0; }

	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) {return GetReference(i);}
	TSTR SubAnimName(int i);

	int NumParamBlocks () { return 1; }
	IParamBlock2* GetParamBlock(int i) { return mp_pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mp_pblock->ID() == id) ? mp_pblock : NULL; }

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message) { return REF_SUCCEED; }

	void UpdateErrorDisplay ();
};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam       *PolyTestMod::mp_ip        = NULL;

class PolyTestModClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new PolyTestMod; }
	const TCHAR *	ClassName() { return GetString(IDS_POLY_TEST_MOD); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kPOLY_TEST_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR *InternalName() { return _T("PolyTestMod"); }
	HINSTANCE HInstance() { return hInstance; }
};

static PolyTestModClassDesc ptestDesc;
extern ClassDesc* GetPolyTestModDesc() {return &ptestDesc;}

//--- Parameter map/block descriptors -------------------------------

// ParamBlock2: Enumerate the parameter blocks:
enum { kPolyTestParams };

// And enumerate the parameters within that block:
enum { kPTestShowColors };

// Parameters
static ParamBlockDesc2 poly_test_param_blk (kPolyTestParams, _T("Poly Test Parameters"), 0, &ptestDesc,
											   P_AUTO_CONSTRUCT + P_AUTO_UI, kPTEST_PBLOCK_REF,
	// rollout
	IDD_POLYTEST, IDS_PARAMETERS, 0, 0, NULL,

	kPTestShowColors, _T("showErrorsInColor"), TYPE_INT, P_RESET_DEFAULT, IDS_PTEST_SHOW_ERRORS,
		p_default, 1,
		p_ui, TYPE_SINGLECHEKBOX, IDC_PTEST_SHOW_ERRORS,
		end,

	end
);

//--- Face extude mod methods -------------------------------


PolyTestMod::PolyTestMod() {
	mp_pblock = NULL;
	ptestDesc.MakeAutoParamBlocks(this);
}

void PolyTestMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	mp_ip = ip;

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	ptestDesc.BeginEditParams(ip,this,flags,prev);

	UpdateErrorDisplay ();
}

void PolyTestMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	mp_ip = NULL;

	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ptestDesc.EndEditParams(ip,this,flags,next);
}

RefTargetHandle PolyTestMod::Clone(RemapDir& remap) {
	PolyTestMod *mod = new PolyTestMod();
	mod->ReplaceReference(kPTEST_PBLOCK_REF,remap.CloneRef(mp_pblock));
	BaseClone(this, mod, remap);
	return mod;
}

void PolyTestMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
	if (!os->obj->IsSubClassOf (polyObjectClassID)) return;

	PolyObject *pobj = (PolyObject *) os->obj;
	MNMesh & mm = pobj->GetMesh();

	UVVert *pVertColor = NULL;

	int useColor;
	mp_pblock->GetValue (kPTestShowColors, t, useColor, FOREVER);

	if (useColor)
	{
		bool hasMap;
		if (0 >= mm.MNum()) {
			mm.SetMapNum (1);
			hasMap = false;
		} else {
			hasMap = !mm.M(0)->GetFlag (MN_DEAD);
		}

		MNMap *map = mm.M(0);

		int i;
		// Initialize map channel.
		map->ClearFlag (MN_DEAD);
		map->setNumVerts(mm.VNum());
		map->setNumFaces(mm.FNum());
		for (i=0; i<mm.FNum(); i++) {
			if (mm.f[i].GetFlag (MN_DEAD)) 
				continue;
			map->f[i] = mm.f[i];
		}

		// Initialize all colors to black:
		pVertColor = map->v;
		UVVert black(0.0f, 0.0f, 0.0f);
		for (i=0; i<map->VNum(); i++) pVertColor[i]=black;
	}

	PolyTestErrorLog *el = (PolyTestErrorLog *) mc.localData;
	if (el == NULL) mc.localData = el = new PolyTestErrorLog ();
	el->BeginLog();

	// Perform the checks, setting any error regions to red:
	CheckAllData (mm, pVertColor, el);
	if (el->ErrorNum()==0)
	{
		if (pVertColor)
		{
			// Everything's ok - so we set the whole mesh to white.
			UVVert white(1.0f, 1.0f, 1.0f);
			for (int i=0; i<mm.VNum(); i++) pVertColor[i] = white;
		}
	}

	UpdateErrorDisplay ();
}

void PolyTestMod::CheckAllData (MNMesh & mm, UVVert *pVertColor, PolyTestErrorLog *el) {
	bool filled=mm.GetFlag (MN_MESH_FILLED_IN);
	int i, j, k, mapChannel, max;
	TSTR buffer;
	UVVert red(1.0f, 0.0f, 0.0f);

	if (!filled)
	{
		el->AddError ("Mesh not \"filled-in\"\n");
	}

	for (mapChannel=-NUM_HIDDENMAPS; mapChannel<mm.numm; mapChannel++) {
		if (mm.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		if (!mm.M(mapChannel)->CheckAllData (mapChannel, mm.numf, mm.f))
		{
			buffer.printf ("Map error in map %d\n", mapChannel);
			el->AddError (buffer);
		}
	}

	// Luna task 747
	MNNormalSpec *pNorms = mm.GetSpecifiedNormals();
	if (pNorms)
	{
		pNorms->SetParent(&mm);
		if (!pNorms->CheckAllData(mm.numf))
			el->AddError ("Error in Normals\n");
	}

	for (i=0; i<mm.numf; i++) {
		if (mm.f[i].GetFlag (MN_DEAD)) continue;
		for (j=0; j<mm.f[i].deg; j++) {
			int vj = mm.f[i].vtx[j];
			int ej = mm.f[i].edg[j];
			if ((vj < 0) || (vj >= mm.numv)) {
				buffer.printf ("Face %d has an out-of-range vertex: %d\n", i, vj);
				el->AddError(buffer);
				vj = -1;
			} else {
				if (mm.v[vj].GetFlag (MN_DEAD)) {
					buffer.printf ("Face %d refs dead vertex %d.\n", i, vj);
					el->AddError(buffer);
				} else {
					if (filled) {
						max = mm.vfac[vj].Count();
						for (k=0; k<max; k++) if (mm.vfac[vj][k] == i) break;
						if (k>=max) {
							buffer.printf ("Face %d refs vertex %d, but vertex doesn't ref face.\n", i, vj);
							el->AddError(buffer);
							if (pVertColor) pVertColor[vj] = red;
						}
					}
				}
			}

			if (filled) {	// Don't check edges unless filled in.
				if ((ej<0) || (ej>=mm.nume)) {
					buffer.printf ("Face %d has an out-of-range edge: %d\n", i, ej);
					el->AddError(buffer);
					if (vj>-1 && pVertColor) pVertColor[vj] = red;
				} else {
					if (mm.e[ej].GetFlag (MN_DEAD)) {
						buffer.printf ("Face %d uses dead edge %d.\n", i, ej);
						el->AddError(buffer);
						if (vj>-1 && pVertColor) pVertColor[vj] = red;
					} else {
						if ((mm.e[ej].f1 != i) && (mm.e[ej].f2 != i)) {
							buffer.printf ("Face %d refs edge %d, but edge doesn't ref face.\n", i, ej);
							el->AddError(buffer);
							if (vj>-1 && pVertColor) pVertColor[vj] = red;
						} else {
							if (mm.e[ej].OtherVert(vj) != mm.f[i].vtx[(j+1)%mm.f[i].deg]) {
								buffer.printf ("Face %d refs edge %d from vert %d to vert %d, but edge doesn't go there.\n",
									i, ej, vj, mm.f[i].vtx[(j+1)%mm.f[i].deg]);
								el->AddError(buffer);
								if (vj>-1 && pVertColor) pVertColor[vj] = red;
							} else {
								if (((mm.e[ej].v1 == vj) && (mm.e[ej].f1 != i)) ||
									((mm.e[ej].v2 == vj) && (mm.e[ej].f2 != i))) {
									buffer.printf ("Orientation mismatch between face %d and edge %d.\n", i, ej);
									el->AddError(buffer);
									if (vj>-1 && pVertColor) pVertColor[vj] = red;
								}
							}
						}
					}
				}
			}
		}
		// Check out triangulation:
		int dnum = (mm.f[i].deg-3)*2;
		for (j=0; j<dnum; j++) {
			if ((mm.f[i].diag[j]>=0) && (mm.f[i].diag[j]<mm.f[i].deg)) continue;
			buffer.printf ("Face %d has bad diagonal index: %d.\n", i, mm.f[i].diag[j]);
			el->AddError(buffer);
		}
		for (j=0; j<dnum; j+=2) {
			if (mm.f[i].diag[j] == mm.f[i].diag[j+1]) {
				buffer.printf ("Face %d has bad diagonal index: %d.\n", i, mm.f[i].diag[j]);
				el->AddError(buffer);
			}
			if (((mm.f[i].diag[j]+1)%mm.f[i].deg == mm.f[i].diag[j+1]) ||
				((mm.f[i].diag[j+1]+1)%mm.f[i].deg == mm.f[i].diag[j])) {
				buffer.printf ("Face %d has bad diagonal index: %d, %d.\n", i,
					mm.f[i].diag[j], mm.f[i].diag[i+1]);
				el->AddError(buffer);
			}
		}
	}

	if (!filled) { return; }	// Nothing else to check if not filled in.

	for (i=0; i<mm.nume; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;

		if (mm.v[mm.e[i].v1].GetFlag (MN_DEAD)) {
			buffer.printf ("Edge %d refs dead vertex %d.\n", i, mm.e[i].v1);
			el->AddError(buffer);
			if (!mm.v[mm.e[i].v2].GetFlag (MN_DEAD))
				if (pVertColor) pVertColor[mm.e[i].v2] = red;
		} else {
			if (mm.vedg) {
				max = mm.vedg[mm.e[i].v1].Count();
				for (j=0; j<max; j++) if (mm.vedg[mm.e[i].v1][j] == i) break;
				if (j>=max) {
					buffer.printf ("Edge %d refs vertex %d, but vertex doesn't ref edge.\n", i, mm.e[i].v1);
					el->AddError(buffer);
					if (pVertColor) pVertColor[mm.e[i].v1] = red;
				}
			}
		}

		if (mm.v[mm.e[i].v2].GetFlag (MN_DEAD)) {
			buffer.printf ("Edge %d refs dead vertex %d.\n", i, mm.e[i].v2);
			el->AddError(buffer);
			if (!mm.v[mm.e[i].v1].GetFlag (MN_DEAD))
				if (pVertColor) pVertColor[mm.e[i].v1] = red;
		} else {
			if (mm.vedg) {
				max = mm.vedg[mm.e[i].v2].Count();
				for (j=0; j<max; j++) if (mm.vedg[mm.e[i].v2][j] == i) break;
				if (j>=max) {
					buffer.printf ("Edge %d refs vertex %d, but vertex doesn't ref edge.\n", i, mm.e[i].v2);
					el->AddError(buffer);
					if (pVertColor) pVertColor[mm.e[i].v2] = red;
				}
			}
		}

		if (mm.e[i].f1 == -1) {
			buffer.printf ("Edge %d has no face-1", i);
			el->AddError(buffer);
			if (pVertColor) pVertColor[mm.e[i].v1] = red;
			if (pVertColor) pVertColor[mm.e[i].v2] = red;
			continue;
		}
		if (mm.f[mm.e[i].f1].GetFlag (MN_DEAD)) {
			buffer.printf ("Edge %d refs dead face %d.\n", i, mm.e[i].f1);
			el->AddError(buffer);
			if (pVertColor) pVertColor[mm.e[i].v1] = red;
			if (pVertColor) pVertColor[mm.e[i].v2] = red;
		} else {
			max = mm.f[mm.e[i].f1].deg;
			for (j=0; j<max; j++) if (mm.f[mm.e[i].f1].edg[j] == i) break;
			if (j>=max) {
				buffer.printf ("Edge %d refs face %d, but face doesn't ref edge.\n", i, mm.e[i].f1);
				el->AddError(buffer);
				if (pVertColor) pVertColor[mm.e[i].v1] = red;
				if (pVertColor) pVertColor[mm.e[i].v2] = red;
			}
		}

		if (mm.e[i].f2<0) continue;
		if (mm.f[mm.e[i].f2].GetFlag (MN_DEAD)) {
			buffer.printf ("Edge %d refs dead face %d.\n", i, mm.e[i].f2);
			el->AddError(buffer);
			if (pVertColor) pVertColor[mm.e[i].v1] = red;
			if (pVertColor) pVertColor[mm.e[i].v2] = red;
		} else {
			max = mm.f[mm.e[i].f2].deg;
			for (j=0; j<max; j++) if (mm.f[mm.e[i].f2].edg[j] == i) break;
			if (j>=max) {
				buffer.printf ("Edge %d refs face %d, but face doesn't ref edge.\n", i, mm.e[i].f2);
				el->AddError(buffer);
				if (pVertColor) pVertColor[mm.e[i].v1] = red;
				if (pVertColor) pVertColor[mm.e[i].v2] = red;
			}
		}
	}

	for (i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;

		if (mm.vedg) {
			max = mm.vedg[i].Count();
			for (j=0; j<max; j++) {
				int ej = mm.vedg[i][j];
				if ((ej>=mm.nume) || (ej<0)) {
					buffer.printf ("Vertex %d refs out-of-range edge %d\n", i, ej);
					el->AddError(buffer);
					if (pVertColor) pVertColor[i] = red;
					continue;
				}
				if (mm.e[ej].GetFlag (MN_DEAD)) {
					buffer.printf ("Vertex %d refs dead edge %d.\n", i, ej);
					el->AddError(buffer);
					if (pVertColor) pVertColor[i] = red;
					continue;
				}
				if (mm.e[ej].v1 == i) continue;
				if (mm.e[ej].v2 != i) {
					buffer.printf ("Vertex %d refs edge %d, but edge doesn't ref vertex.\n", i, ej);
					el->AddError(buffer);
					if (pVertColor) pVertColor[i] = red;
				}
			}
		}

		if (mm.vfac) {
			max = mm.vfac[i].Count();
			if (mm.vedg && (max>mm.vedg[i].Count())) {
				buffer.printf ("Vertex %d references more faces than edges.\n", i);
				el->AddError(buffer);
				if (pVertColor) pVertColor[i] = red;
			}
			for (j=0; j<max; j++) {
				int fj = mm.vfac[i][j];
				if (fj>=mm.numf) {
					buffer.printf ("Vertex %d refs out-of-range face %d\n", i, fj);
					el->AddError(buffer);
					if (pVertColor) pVertColor[i] = red;
					continue;
				}
				if (mm.f[fj].GetFlag (MN_DEAD)) {
					buffer.printf ("Vertex %d refs dead face %d.\n", i, fj);
					el->AddError(buffer);
					if (pVertColor) pVertColor[i] = red;
					continue;
				}
				for (k=0; k<mm.f[fj].deg; k++) if (mm.f[fj].vtx[k] == i) break;
				if (k>=mm.f[fj].deg) {
					buffer.printf ("Vertex %d refs face %d, but face doesn't ref vertex.\n", i, fj);
					el->AddError(buffer);
					if (pVertColor) pVertColor[i] = red;
				}
			}
		}
	}
}

Interval PolyTestMod::LocalValidity(TimeValue t) {
	if (TestAFlag(A_MOD_BEING_EDITED)) return NEVER;
	else return FOREVER;
}

RefTargetHandle PolyTestMod::GetReference(int i) {
	switch (i) {
	case kPTEST_PBLOCK_REF: return mp_pblock;
	default: return NULL;
	}
}

void PolyTestMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case kPTEST_PBLOCK_REF: mp_pblock = (IParamBlock2*)rtarg; break;
	}
}

TSTR PolyTestMod::SubAnimName(int i) {
	switch (i) {
	case kPTEST_PBLOCK_REF:  return GetString(IDS_PARAMETERS);
	default:         return _T("");
	}
}

void PolyTestMod::UpdateErrorDisplay () {	
	if (mp_ip == NULL) return;

	if (!ptestDesc.NumParamMaps ()) return;
	IParamMap2 *pmap = ptestDesc.GetParamMap (kPolyTestParams);
	if (!pmap) return;
	HWND hParams = pmap->GetHWnd();
	if (!hParams) return;

	ModContextList mcList;
	INodeTab nodes;
	mp_ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		PolyTestErrorLog *errorLog = (PolyTestErrorLog *) mcList[i]->localData;
		if (!errorLog) continue;

		int i=1;
		if (errorLog->ErrorNum() == 0)
			SetDlgItemText (hParams, IDC_TEXT_RESULT, GetString (IDS_PTEST_NO_ERRORS));
		else
		{
			for (i=0; i<errorLog->ErrorNum(); i++)
				SetDlgItemText (hParams, IDC_TEXT_RESULT+i, errorLog->Error(i));
		}
		for (; i<20; i++)
			SetDlgItemText (hParams, IDC_TEXT_RESULT+i, TSTR(""));
	}
}
