Scriptname _ts_DR_PlayerAliasScript extends ReferenceAlias

import _ts_DR_Debug

int function GetIDRCPluginVersion() global native

Quest property DragonRideQuest auto

bool Property bMagicEffectsRegistered = false auto hidden

_ts_DR_RideControlScript Property DR_Control Auto Hidden

int iSKSEVersion = 0
int iSKSEVersionMinor = 0
int iSKSEVersionBeta = 0
int iSKSEVersionRelease = 0


Event OnCellDetach()
_ts_Debug_Trace_Message("IDRC - _ts_DR_PlayerAliasScript - OnCellDetach")
endEvent

Event OnCellAttach()
_ts_Debug_Trace_Message("IDRC - _ts_DR_PlayerAliasScript - OnCellAttach")
endEvent

event OnPlayerLoadGame()
_ts_Debug_Trace_Message("IDRC - _ts_DR_PlayerAliasScript - OnPlayerLoadGame")

	CheckForIDRCPluginVersion()
	CheckForSKSE()
	CheckForPO3Tweaks()
	DR_Control.InitVariables(true)
EndEvent

Event OnInit()
_ts_Debug_Trace_Message("IDRC - _ts_DR_PlayerAliasScript - OnInit")
	; Populate variables without the need to declare as properties
	DR_Control = DragonRideQuest as _ts_DR_RideControlScript

	CheckForIDRCPluginVersion()
	CheckForSKSE()
	CheckForPO3Tweaks()
EndEvent


bool function CheckForSKSE()
	iSKSEVersion = SKSE.GetVersion()
	iSKSEVersionMinor = SKSE.GetVersionMinor()
	iSKSEVersionBeta = SKSE.GetVersionBeta()
	iSKSEVersionRelease = SKSE.GetVersionRelease()

_ts_Debug_Trace_Message("IDRC - _ts_DR_PlayerAliasScript - CheckForSKSE: " + iSKSEVersion + "." + iSKSEVersionMinor + "." + iSKSEVersionBeta + " rel " + iSKSEVersionRelease)
	
	if iSKSEVersion == 0
_ts_Debug_Trace_Message("IDRC - _ts_DR_PlayerAliasScript - CheckForSKSE: Error - SKSE not loaded!")
		debug.MessageBox("Intuitive Dragon Ride Control: SKSE is not loaded. Dragon riding will not work.")
		return false
	endif

	return true
endFunction


bool function CheckForPO3Tweaks(bool bDisplayMessage = true)
	if !PO3_Tweaks.IsTweakInstalled("Load EditorIDs")
_ts_Debug_Trace_Message("IDRC - _ts_DR_PlayerAliasScript - CheckForPO3Tweaks: po3's Tweaks not found!")
		if bDisplayMessage
			debug.MessageBox("Intuitive Dragon Ride Control: powerofthree's Tweaks not installed? Dragon ride control will not work.")	
		endif
		return false
	endif

	return true
endFunction


bool function CheckForIDRCPluginVersion()
	int iIDRCPluginVersion = GetIDRCPluginVersion()
	
	if iIDRCPluginVersion != 6
_ts_Debug_Trace_Message("IDRC - _ts_DR_PlayerAliasScript - CheckForIDRCPluginVersion: Error - IDRC plugin version not compatible: "+ iIDRCPluginVersion)
		debug.MessageBox("Intuitive Dragon Ride Control: Unexpected error - IntuitiveDragonRideControl.dll version conflict. Dragon ride control will not work properly.")	
		return false
	endif

	return true
endFunction




