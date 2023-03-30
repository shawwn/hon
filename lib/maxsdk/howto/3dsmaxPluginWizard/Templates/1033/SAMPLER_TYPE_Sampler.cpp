/**********************************************************************
 *<
	FILE: [!output PROJECT_NAME].cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "[!output PROJECT_NAME].h"

#define [!output CLASS_NAME]_CLASS_ID	Class_ID([!output CLASSID1], [!output CLASSID2])


#define PBLOCK_REF	0

class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		// Parameter block
		IParamBlock2	*pblock;	//ref 0

		TCHAR* GetDefaultComment();
		// This samples a sequence for the area
		void DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, MASK pMask=NULL );		// This is the function that is called to get the next sample 
		// returns FALSE when out of samples
		BOOL NextSample( Point2* pOutPt, float* pSampleSz, int n );
		// Integer number of samples for current quality setting
		int GetNSamples();	

		// Get/Set quality and enable status
		int SupportsQualityLevels();
		void SetQuality( float q );
		float GetQuality();

		void SetEnable( BOOL on );
		BOOL GetEnable();

		ULONG SupportsStdParams(){ return R3_ADAPTIVE; }

		void SetAdaptiveOn( BOOL on );
		BOOL IsAdaptiveOn();

		void SetAdaptiveThreshold( float val );
		float GetAdaptiveThreshold();
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return SAMPLER_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);


		int NumSubs() { return 1; }
		TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
		Animatable* SubAnim(int i) { return pblock; }

		// TODO: Maintain the number or references here
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return pblock; }
		void SetReference(int i, RefTargetHandle rtarg) { pblock=(IParamBlock2*)rtarg; }




		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		void DeleteThis() { delete this; }		
		//Constructor/Destructor

		[!output CLASS_NAME]();
		~[!output CLASS_NAME]();		

};


[!output TEMPLATESTRING_CLASSDESC]

[!if PARAM_MAPS]
[!output TEMPLATESTRING_PARAMBLOCKDESC]
[!endif]



/*===========================================================================*\
 |	Sampler Implimentation
\*===========================================================================*/


[!output CLASS_NAME]::[!output CLASS_NAME]()
{
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}

RefTargetHandle [!output CLASS_NAME]::Clone( RemapDir &remap )
{
	[!output CLASS_NAME]* sksNew = new [!output CLASS_NAME]();
	sksNew->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, sksNew, remap);
	return (RefTargetHandle)sksNew;
}


IOResult [!output CLASS_NAME]::Load(ILoad *iload)
{
	Sampler::Load(iload);
	return IO_OK;
}


IOResult [!output CLASS_NAME]::Save(ISave *isave)
{
	Sampler::Save(isave);
	return IO_OK;
}

TCHAR* [!output CLASS_NAME]::GetDefaultComment()
{
	return GetString(IDS_COMMENT);
}

RefResult [!output CLASS_NAME]::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	return REF_SUCCEED;
}

void [!output CLASS_NAME]::DoSamples( ShadeOutput* pOut, SamplingCallback* cb, ShadeContext* sc, MASK pMask)
{
// TODO: Perform the actual sampling
}

BOOL [!output CLASS_NAME]::NextSample( Point2* pOut, float* pSampleSz, int n )
{
	//TODO: Find next sample point
	return TRUE;
}


int [!output CLASS_NAME]::GetNSamples()
{
	// TODO: Return number of samples
	return 1;
}

/*===========================================================================*\
 |	Quality and Enabling support
\*===========================================================================*/

void [!output CLASS_NAME]::SetQuality( float q )
{ 
	pblock->SetValue( pb_quality, 0, q );
}

float [!output CLASS_NAME]::GetQuality()
{ 
	return pblock->GetFloat( pb_quality, 0);
}

int [!output CLASS_NAME]::SupportsQualityLevels() 
{
	return 1; 
}

void [!output CLASS_NAME]::SetEnable( BOOL on )
{ 
	pblock->SetValue( pb_enable, 0, on );
}

BOOL [!output CLASS_NAME]::GetEnable()
{ 
	BOOL b; Interval valid;
	pblock->GetValue( pb_enable, 0, b, valid );
	return b;
}

/*===========================================================================*\
 |	Adaptive Sampling support
\*===========================================================================*/


void [!output CLASS_NAME]::SetAdaptiveOn( BOOL on )
{ 
	pblock->SetValue( pb_adapt_enable, 0, on ); 
}

BOOL [!output CLASS_NAME]::IsAdaptiveOn()
{ 
	BOOL b; Interval valid;
	pblock->GetValue( pb_adapt_enable, 0, b, valid );
	return b;
}

void [!output CLASS_NAME]::SetAdaptiveThreshold( float val )
{ 
	pblock->SetValue( pb_adapt_threshold, 0, val ); 
}

float [!output CLASS_NAME]::GetAdaptiveThreshold() 
{ 
	return pblock->GetFloat( pb_adapt_threshold, 0);
}
