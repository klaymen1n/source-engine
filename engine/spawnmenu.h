//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef SPAWNMENU_H
#define SPAWNMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/PHandle.h>

class CSpawnMenuButton;
class CSpawnMenuOptionsPanel;

//-----------------------------------------------------------------------------
// Purpose: A simple panel to contain a debug menu button w/ cascading menus
//-----------------------------------------------------------------------------
class CSpawnMenuPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:

	CSpawnMenuPanel( vgui::Panel *parent, char const *panelName );

	// Trap visibility so that we can force the cursor on
	virtual void SetVisible( bool state );
	virtual void OnCommand( char const *command );

private:
	CSpawnMenuButton	*m_pSpawnMenu;

	vgui::DHANDLE< CSpawnMenuOptionsPanel >	m_hSpawnOptions;
};

#endif // SPAWNMENU_H
