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

	private:
		
		// We store 2 parameters
		double param1, param2;
	public:
		
		TSTR GetName() { return GetString(IDS_CLASS_NAME); }

		// Kernel parameter setup and handling
		long GetNFilterParams();
		TCHAR * GetFilterParamName( long nParam );
		double GetFilterParam( long nParam );
		void SetFilterParam( long nParam, double val );
		TCHAR * GetDefaultComment();

		void Update(TimeValue t, Interval& valid);

		// Actual kernel function
		double KernelFn( double x, double y );

		// Kernel functionality queries
		long GetKernelSupport();
		long GetKernelSupportY();
		bool Is2DKernel();
		bool IsVariableSz();
		void SetKernelSz( double x, double y = 0.0 );
		void GetKernelSz( double& x, double& y );
		bool IsNormalized();
		bool HasNegativeLobes();
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return FILTER_KERNEL_CLASS_ID; }
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



[!output CLASS_NAME]::[!output CLASS_NAME]()
{
	[!output CLASS_NAME]Desc.MakeAutoParamBlocks(this);
}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}

/*===========================================================================*\
 |	Kernel Parameter returns
\*===========================================================================*/

// Number of parameters we support
long [!output CLASS_NAME]::GetNFilterParams() { return 2; }

// Get the name of parameter #nParam
TCHAR * [!output CLASS_NAME]::GetFilterParamName( long nParam )
{ 
	return GetString( nParam ? IDS_PARAM2:IDS_PARAM1 );
}

// Get the value of parameter #nParam
double [!output CLASS_NAME]::GetFilterParam( long nParam )
{
	return nParam ? param2 : param1;
}

// Set our parameter variables
void [!output CLASS_NAME]::SetFilterParam( long nParam, double val )
{
	if (nParam) {  
		param2 = val;	
		pblock->SetValue( pb_param2, 0, float( val ) );		
 	} else { 
		param1 = val; 
		pblock->SetValue( pb_param1, 0, float( val ) );		
	} 
}

void [!output CLASS_NAME]::Update(TimeValue t, Interval& valid){
	float val;
	pblock->GetValue( pb_param1, t, val, valid ); param1 = val;
	pblock->GetValue( pb_param2, t, val, valid ); param2 = val;
}

TCHAR * [!output CLASS_NAME]::GetDefaultComment() 
{
	return GetString( IDS_COMMENT); 
}

IOResult [!output CLASS_NAME]::Load(ILoad *iload)
{
	FilterKernel::Load(iload);
	return IO_OK;
}
IOResult [!output CLASS_NAME]::Save(ISave *isave)
{
	FilterKernel::Save(isave);
	return IO_OK;
}


RefResult [!output CLASS_NAME]::NotifyRefChanged(
		Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message) 
{
	switch (message) {
		case REFMSG_CHANGE:
			[!output CLASS_NAME_LOWER]_param_blk.InvalidateUI();
			break;
		}
	return REF_SUCCEED;
}

RefTargetHandle [!output CLASS_NAME]::Clone( RemapDir &remap )
{
	[!output CLASS_NAME]* newObj = new [!output CLASS_NAME]();
	newObj->ReplaceReference(0,remap.CloneRef(pblock));
	BaseClone(this, newObj, remap);
	return (RefTargetHandle)newObj;
}



/*===========================================================================*\
 |	Calculate a result
\*===========================================================================*/

double [!output CLASS_NAME]::KernelFn( double x, double y )
{
	if ( x < param1 ) return 1.0f;
	if ( x > param2 ) return 1.0f;
	else return 0.0f;
}


/*===========================================================================*\
 |	Kernel functionality queries
\*===========================================================================*/

// Integer number of pixels from center to filter 0 edge.
// Must not truncate filter x dimension for 2D filters
long [!output CLASS_NAME]::GetKernelSupport(){ return 1; }

// For 2d returns y support, for 1d returns 0
long [!output CLASS_NAME]::GetKernelSupportY(){ return 0; }

// Are we 2D or Variable Size?
bool [!output CLASS_NAME]::Is2DKernel(){ return FALSE; }
bool [!output CLASS_NAME]::IsVariableSz(){ return FALSE; }

// 1-D filters ignore the y parameter, return it as 0.0
void [!output CLASS_NAME]::SetKernelSz( double x, double y ){}
void [!output CLASS_NAME]::GetKernelSz( double& x, double& y ){ x = 0.5; y = 0.0; }

// Returning true will disable the built-in normalizer
bool [!output CLASS_NAME]::IsNormalized(){ return FALSE; }

// This is for possible future optimizations
bool [!output CLASS_NAME]::HasNegativeLobes(){ return FALSE; }
