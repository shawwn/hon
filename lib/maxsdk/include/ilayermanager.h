/**********************************************************************
 *<
	FILE: ILayerManager.h

	DESCRIPTION: Declaration of the ILayerManager interface

	CREATED BY:	Peter Sauerbrei

	HISTORY: Created 19 October 1998

 *>	Copyright (c) 1998-99, All Rights Reserved.
 **********************************************************************/

#ifndef __ILAYERMANAGER_H__
#define __ILAYERMANAGER_H__

class ILayer;
class LayerIterator;
class ConstLayerIterator;

class ILayerManager : public ReferenceTarget
{
public:
	static const SClass_ID kLayerManagerSuperClassID;

	// from Animatable
	SClass_ID SuperClassID() { return kLayerManagerSuperClassID; }

	// local methods
	virtual bool AddLayer(ILayer * layer) = 0;
	virtual ILayer * CreateLayer(void) = 0;					// creates a new layer
	virtual BOOL DeleteLayer(const TSTR & name) = 0;		// deletes a layer
	virtual void SetCurrentLayer(const TSTR & name) = 0;	// sets the current layer
	virtual void SetCurrentLayer(void) = 0;
	virtual ILayer * GetCurrentLayer(void) const = 0;		// gets the current layer
	virtual void EditLayer(const TSTR & name) = 0;
	virtual void DoLayerPropDialog(HWND hWnd) = 0;
	virtual LayerIterator * MakeIterator(void) = 0;
	virtual ConstLayerIterator * MakeConstIterator(void) const = 0;
	virtual int GetLayerCount(void) = 0;
	virtual ILayer * GetLayer(const TSTR & name) const = 0;
	virtual void DoLayerSelDialog(HWND hWnd) = 0;
	//virtual void SetupToolList(HWND hWnd) = 0;
	virtual void SetupToolList2(HWND hWnd, HWND hParent) = 0;
	virtual void ExtendMenu(HMENU hMenu, bool geometry = true, bool grid = false) = 0;
	virtual TSTR GetSavedLayer(int i) const = 0;
	virtual ILayer * GetRootLayer() const = 0;
	virtual void Reset(BOOL fileReset = FALSE) = 0;
	virtual void SelectObjectsByLayer(HWND hWnd) = 0;

	// new Hide/Freeze logic, 030516  --prs.
	virtual void SetPropagateToLayer(int prop) = 0;
	virtual int GetPropagateToLayer(void) = 0;
};

#endif //__ILAYERMANAGER_H__
