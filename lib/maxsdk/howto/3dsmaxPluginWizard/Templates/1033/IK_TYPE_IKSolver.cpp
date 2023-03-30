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

using namespace IKSys;

class [!output CLASS_NAME] : public [!output SUPER_CLASS_NAME] {
	public:

		static HWND hParams;
		
		bool IsHistoryDependent() const;
		bool DoesOneChainOnly()const;

		bool IsInteractive() const;
		bool UseSlidingJoint() const;
		bool UseSwivelAngle() const;
  		bool IsAnalytic() const ;

		bool SolveEERotation() const;

		const IKSys::ZeroPlaneMap*
			GetZeroPlaneMap(const Point3& a0, const Point3& n0) const
			{ return NULL; }

		float		GetPosThreshold() const;
		float		GetRotThreshold() const;
		unsigned	GetMaxIteration() const;
		void		SetPosThreshold(float);
		void		SetRotThreshold(float);
		void		SetMaxIteration(unsigned);

		ReturnCondition Solve(IKSys::LinkChain&);

		Class_ID ClassID() {return [!output CLASS_NAME]_CLASS_ID;}		
		SClass_ID SuperClassID() { return IK_SOLVER_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

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

}

[!output CLASS_NAME]::~[!output CLASS_NAME]()
{

}


bool [!output CLASS_NAME]::IsHistoryDependent() const
{
	//TODO:  Let the IK system know whether you a History dependent or not.
	// Return TRUE for History Dependent
	return false;
}

bool [!output CLASS_NAME]::DoesOneChainOnly() const
{
	//TODO:  Specify whether you can work on more than one chain or not
	return true; 
}


bool [!output CLASS_NAME]::IsInteractive() const
{
	//TODO: Return whether you support Interactive ?
	return false; 
}

bool [!output CLASS_NAME]::UseSlidingJoint() const
{
	//TODO: Do you use a sliding joint

	return false; 
}
	
bool [!output CLASS_NAME]::UseSwivelAngle() const
{
	//TODO: DO you use the Swivel angle

	return true; 
}
  		
bool [!output CLASS_NAME]::IsAnalytic() const 
{
	//TODO: Is this an Analytic solver - Return TRUE for yes
	return true; 
}


/******************************************************************************************************
*

	RotThreshold() is not relevant to solver that does not SolveEERotation().
	UseSwivelAngle() and SolveEERotation() cannot both be true.

*
\******************************************************************************************************/
bool [!output CLASS_NAME]::SolveEERotation() const{

	//TODO: 

	return false; 
}

float [!output CLASS_NAME]::GetPosThreshold() const
{
	//TODO: Return the Position Threshold of the solver

	return 0.0f; 
}
	
float [!output CLASS_NAME]::GetRotThreshold() const
{

	//TODO: Return the Rotation Threshold of the solver


	return 0.0f; 
}
		
unsigned [!output CLASS_NAME]::GetMaxIteration() const
{
	return 0; 
}

void [!output CLASS_NAME]::SetPosThreshold(float)
{
	//TODO: Set the Position Threshold

}

void [!output CLASS_NAME]::SetRotThreshold(float)
{
	//TODO: Set the Rotation Threshold
}

void [!output CLASS_NAME]::SetMaxIteration(unsigned)
{

}

/*************************************************************************************************
*
	This the method that is actually called to perform the IK specific task.
   
	This only use this method if you answers true to DoesOneChainOnly() and false to 
	HistoryDependent(). The solver is not designed to be invoked recursively. The
    recursion logic existing among the ik chains is taken care of by
    the Max IK (sub-)System.
*
\*************************************************************************************************/

IKSolver :: ReturnCondition [!output CLASS_NAME] :: Solve (IKSys::LinkChain& linkChain){

	//TODO: Implement the actual IK Solver

	return 0;
}


