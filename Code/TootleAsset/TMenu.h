/*------------------------------------------------------

	Menu asset. Layout for a menu

-------------------------------------------------------*/
#pragma once

#include "TLAsset.h"
#include "TAsset.h"


namespace TLAsset
{
	class TMenu;
};



//----------------------------------------------
//	a menu rendered to the screen
//----------------------------------------------
class TLAsset::TMenu : public TLAsset::TAsset
{
public:
	class TMenuItem;

public:
	TMenu(TRefRef MenuAssetRef);
	
	FORCEINLINE TPtrArray<TMenuItem>&		GetMenuItems()							{	return m_MenuItems;	}
	FORCEINLINE const TPtrArray<TMenuItem>&	GetMenuItems() const					{	return m_MenuItems;	}
	FORCEINLINE TPtr<TMenuItem>&	GetMenuItem(TRefRef MenuItemRef)				{	return GetMenuItems().FindPtr( MenuItemRef );	}
	FORCEINLINE Bool				GetMenuItemExists(TRefRef MenuItemRef) const	{	return GetMenuItems().Exists( MenuItemRef );	}
	FORCEINLINE Bool				GetMenuItemExists(TRefRef MenuItemRef)			{	return GetMenuItems().Exists( MenuItemRef );	}

	TPtr<TMenuItem>&				AddMenuItem(TRefRef MenuItemRef);				//	create a menu item - returns NULL if duplicated menu item ref

	FORCEINLINE TRefRef				GetSchemeRef() const							{	return m_SchemeRef;	}
	FORCEINLINE void				SetSchemeRef(TRefRef SchemeRef)					{	m_SchemeRef = SchemeRef;	}

protected:
	virtual SyncBool				ImportData(TBinaryTree& Data);	//	load asset data out binary data
	virtual SyncBool				ExportData(TBinaryTree& Data);	//	save asset data to binary data

protected:
	TRef							m_SchemeRef;									//	ref of a scheme associated with the menu
	TPtrArray<TMenuItem>			m_MenuItems;									//	list of menu items
};



//----------------------------------------------
//	individual menu item
//	gr: consider turning this into an asset in it's own right to make it easier to import/export
//----------------------------------------------
class TLAsset::TMenu::TMenuItem
{
public:
	friend class TMenu;

public:
	TMenuItem(TRefRef MenuItemRef);

	TRefRef						GetMenuItemRef() const		{	return m_MenuItemRef;	}
	const TString&				GetText() const				{	return m_Text;	}
	DEPRECATED const TString&	GetString() const 	{	return m_Text;	}
	TRefRef						GetMenuCommand() const		{	return m_Command;	}
	TRefRef						GetNextMenu() const			{	return m_NextMenu;	}
	TRefRef						GetMeshRef() const			{	return m_MeshRef; }

	DEPRECATED void				SetString(const TString& String) 		{	m_Text = String;	}
	void						SetText(const TString& String)		{	m_Text = String;	}
	void						SetMenuCommand(TRefRef Command)		{	m_Command = Command;	}
	void						SetNextMenu(TRefRef NextMenu)		{	m_NextMenu = NextMenu;	SetMenuCommand("open");	};
	void						SetMeshRef(TRefRef MeshRef)			{	m_MeshRef = MeshRef;	}

	Bool						IsHighlightable() const				{	return m_Command.IsValid();	}	

	inline Bool					operator==(TRefRef MenuItemRef) const	{	return GetMenuItemRef() == MenuItemRef;	}

protected:
	TRef						m_MenuItemRef;		//	ref for menu item
	TRef						m_Command;			//	menu command, invalid commands cannot be highlighted

	//	todo: gr: replace all this with TBinaryData?
	TString						m_Text;				//	string displayed on menu
	TRef						m_NextMenu;			//	if menu command is "open" then this is the menu we open
	TRef						m_MeshRef;			//	Mesh icon reference - for use instead of a string
};
