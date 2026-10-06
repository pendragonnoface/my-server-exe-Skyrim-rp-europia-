Scriptname _ts_DR_RideControlScript extends Quest conditional

import _ts_DR_Debug

int property iFlyingState = 0 auto hidden
bool property bRegenerateHealth = false auto hidden
bool property bTriggerMessage = false auto conditional hidden
bool property bDragonRoars = false auto conditional hidden
bool property bAutoCombat = false auto hidden conditional
bool property bIsFollowerActive = false auto hidden conditional

ReferenceAlias Property dragonAlias  Auto
ReferenceAlias Property PerchTarget Auto
ReferenceAlias Property WordWallPerch Auto
ReferenceAlias Property RockPerch Auto
ReferenceAlias Property TowerPerch Auto

ObjectReference Property DLC2TameDragonOrbitMarker Auto ; default object that the orbit package uses. code also moves this to fast travel destination
ObjectReference Property FlyToTargetMarker Auto

quest Property MCMConfigQuest auto
Quest Property FindPerchQuest auto

ObjectReference Property DragonTurnMarker Auto
ObjectReference Property DragonTravelToMarker Auto

Spell Property RestoreHealthSpell Auto

string property DragonName auto

Spell Property DLC2TameDragonNoFlyAbility  Auto

Shout Property AttackShout auto
Shout Property UnrelentingForceShout auto



; Initialization 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

int ScriptVersion = 8
int function GetVersion()
	return ScriptVersion
endfunction


event OnInit()
	InitVariables(false)
endevent

function InitializeData_SKSE(Quest rideQuest, Quest findPerchQuest, \
							ReferenceAlias dragonAlias, ObjectReference OrbitMarker,\
							ReferenceAlias wordWallPerch, \
							ReferenceAlias TowerPerch, \
							ReferenceAlias RockPerch, \
							ReferenceAlias PerchTarget, \							
							Shout UnrelentingForceShout, Shout AttackShout, \
							ObjectReference DragonTurnMarker, ObjectReference DragonTravelToMarker,\
							ObjectReference FlyToTargetMarker, \
							Spell TameDragonNoFlyAbility, \
							string dragonName, \
							bool cameraLockInitiallyEnabled) global native

function InitVariables(bool bIsOnLoad)
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - InitVariables: IsUsingGamepad = " + game.UsingGamepad())
	
	SetIniVars()

	RegisterCustomEvents()
	CheckIDRCVersion()

	InitializeData_SKSE((self as quest), FindPerchQuest, dragonAlias, DLC2TameDragonOrbitMarker, \
						WordWallPerch, TowerPerch, RockPerch, PerchTarget, \
						UnrelentingForceShout, AttackShout, \
						DragonTurnMarker, DragonTravelToMarker, FlyToTargetMarker,\
						DLC2TameDragonNoFlyAbility, DragonName, \
						(MCMConfigQuest as _ts_DR_MCMConfig).bCameraLockInitiallyEnabled)

	SetTargetReticleMode((MCMConfigQuest as _ts_DR_MCMConfig).iTargetReticleMode)
	SetReticleLockAnimationStyle((MCMConfigQuest as _ts_DR_MCMConfig).iReticleLockAnimationStyle)
	SetTDMLock((MCMConfigQuest as _ts_DR_MCMConfig).iTDMLock)
	SetPrimaryTargetMode((MCMConfigQuest as _ts_DR_MCMConfig).iPrimaryTargetMode)
	SetMaxTargetDistance((MCMConfigQuest as _ts_DR_MCMConfig).fMaxTargetDistance)
	SetDistanceMultiplierSmall((MCMConfigQuest as _ts_DR_MCMConfig).fDistanceMultiplierSmall)
	SetDistanceMultiplierLarge((MCMConfigQuest as _ts_DR_MCMConfig).fDistanceMultiplierLarge)
	SetDistanceMultiplierExtraLarge((MCMConfigQuest as _ts_DR_MCMConfig).fDistanceMultiplierExtraLarge)
	SetMaxTargetScanAngle((MCMConfigQuest as _ts_DR_MCMConfig).fTargetScanAngle)
	SetIgnoredCameraPitch((MCMConfigQuest as _ts_DR_MCMConfig).fIgnoredCameraPitch)
	SetYawOffsetStrength((MCMConfigQuest as _ts_DR_MCMConfig).fYawOffsetStrength)
	SetPitchOffsetStrength((MCMConfigQuest as _ts_DR_MCMConfig).fPitchOffsetStrength)
	SetInvertPitchOffsetDuringFlight((MCMConfigQuest as _ts_DR_MCMConfig).bInvertPitchOffsetDuringFlight)

	if bIsOnLoad
		if (MCMConfigQuest as _ts_DR_MCMConfig) != None
			; This ensures that the values from the MCM menu are used,
			; instead of the initial values :
			SetDragonSpeed((MCMConfigQuest as _ts_DR_MCMConfig).fDragonSpeed)
			SetRollAmplitude((MCMConfigQuest as _ts_DR_MCMConfig).fRollAmplitude)
			bool bAuto = false
			if (MCMConfigQuest as _ts_DR_MCMConfig).iCombatMode == 1
				bAuto = true
			endif
			SetInitialAutoCombatMode(bAuto)
			SetDisplayAttackMessage((MCMConfigQuest as _ts_DR_MCMConfig).bDisplayAttackMessage)
		else
			; fallback in case SkyUI is not in the load order
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - InitVariables: MCMConfigQuest not available")
			SetDragonSpeed(1.0)
			SetRollAmplitude(0.5)
		endif

		bool  bDragonIsBeingRidden = false
		if dragonAlias.GetActorReference()
			bDragonIsBeingRidden = dragonAlias.GetActorReference().IsBeingRidden()
		endif			

		if game.GetPlayer().IsOnMount() && bDragonIsBeingRidden
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - InitVariables: Re-registering ride control. FlyingState: " + iFlyingState + ", AutoCombat: " + bAutoCombat)
			DragonRegisterForControls(bReRegisterOnLoad = true)
			SetFlyingState(iFlyingState)
			SetAutoCombat(bAutoCombat)
			dragonAlias.GetActorRef().EvaluatePackage()
		endif
	endif
endFunction


function SetINIVars_SKSE() global native
function SetIniVars()
	SetINIVars_SKSE()
endfunction


bool Function CheckIDRCVersion()
	
	if GetVersion() < 8
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - CheckIDRCVersion: Savegame with old IDRC version.")
		debug.MessageBox("Intuitive Dragon Ride Control: Your savegame is using an older version of IDRC. Ride controls will not work properly! Please clean the savegame before continuing to use IDRC. Check the mod's Nexus page for instructions.")
		return false	
	endif

	return true
endFunction


Function RegisterCustomEvents()
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - RegisterCustomEvents")
	; Use custom events to run time-consuming tasks asynchronosly, ie not gating progress of the calling scripts
	; NOTE: These custom events MUST be registered each time the game is loaded - see OnPlayerLoadGame() in _ts_DR_PlayerAliasScript
	
	RegisterForModEvent("OnProperyUpdate_SKSE", "OnPropertyUpdate_SKSE")
EndFunction


; Dragon Ride Control registration
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

bool function RegisterForControls_SKSE(bool bReRegisterOnLoad = false, bool bRegisterFromGoTDragonCompanions = false) global native
function DragonRegisterForControls(bool bReRegisterOnLoad = false, bool bRegisterFromGoTDragonCompanions = false)
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - DragonRegisterForControls")

	if dragonAlias.GetActorRef().IsBeingRidden()
		SetKeyMappings(bRegisterFromGoTDragonCompanions)
		RegisterForControls_SKSE(bReRegisterOnLoad, bRegisterFromGoTDragonCompanions)	
	endif	
endfunction


bool function UnregisterForControls_SKSE() global native
function DragonUnregisterForControls()
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - DragonUnregisterForControls")
	UnregisterForControls_SKSE()
endfunction


function SetKeyMapping_SKSE(string skey, int iMappedKey) global native
function SetKeyMappings(bool bRegisterFromGoTDragonCompanions = false)
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - SetKeyMappings")

	SetKeyMapping_SKSE("Forward", (MCMConfigQuest as _ts_DR_MCMConfig).GetForwardKey())
	SetKeyMapping_SKSE("Back", (MCMConfigQuest as _ts_DR_MCMConfig).GetBackKey())
	SetKeyMapping_SKSE("StrafeLeft", (MCMConfigQuest as _ts_DR_MCMConfig).GetStrafeLeftKey())
	SetKeyMapping_SKSE("StrafeRight", (MCMConfigQuest as _ts_DR_MCMConfig).GetStrafeRightKey())
	SetKeyMapping_SKSE("DisplayHealth", (MCMConfigQuest as _ts_DR_MCMConfig).GetDisplayHealthKey())
	SetKeyMapping_SKSE("DragonUp", (MCMConfigQuest as _ts_DR_MCMConfig).GetDragonUpKey())
	SetKeyMapping_SKSE("DragonDown", (MCMConfigQuest as _ts_DR_MCMConfig).GetDragonDownKey())
	SetKeyMapping_SKSE("Run", (MCMConfigQuest as _ts_DR_MCMConfig).GetRunKey())
	SetKeyMapping_SKSE("ToggleAlwaysRun", (MCMConfigQuest as _ts_DR_MCMConfig).GetToggleAlwaysRunKey())
	SetKeyMapping_SKSE("ToggleLockReticle", (MCMConfigQuest as _ts_DR_MCMConfig).GetToggleLockReticleKey())
	SetKeyMapping_SKSE("ToggleCameraLock", (MCMConfigQuest as _ts_DR_MCMConfig).GetToggleCameraLockKey())
;	SetKeyMapping_SKSE("PrimaryTargetMode", (MCMConfigQuest as _ts_DR_MCMConfig).GetTogglePrimaryTargetModeKey())
	if !bRegisterFromGoTDragonCompanions
		SetKeyMapping_SKSE("ToggleAutoCombat", (MCMConfigQuest as _ts_DR_MCMConfig).GetAutoCombatKey())
	endif
endfunction


; Interface with SKSE plugin -> Synchronization of SKSE property updates with Papyrus properties
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Event OnProperyUpdate_SKSE(string propertyName, bool bValue, float fValue, int iValue)
	if propertyName == "AutoCombat"
		bAutoCombat = bValue
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - OnProperyUpdate_SKSE: bAutoCombat = " + bAutoCombat)
	elseif propertyName == "FlyingState"
		iFlyingState = iValue
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - OnProperyUpdate_SKSE: iFlyingState = " + iFlyingState)
	else
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - OnProperyUpdate_SKSE: Error - Unknown property: " + propertyName)
	endif

	; ensure any updates that involve package changes are evaluated
	dragonAlias.GetActorRef().EvaluatePackage()
endevent


; MCM Interface
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

function SetInitialAutoCombatMode_SKSE(bool bValue) global native
function SetInitialAutoCombatMode(bool bValue)
	SetInitialAutoCombatMode_SKSE(bValue)
endFunction


function SetTriggerMessage(bool bValue)
	bTriggerMessage = bValue
endFunction


function SetDragonRoars(bool bValue)
	bDragonRoars = bValue
endFunction


function SetRegenerateHealth(bool bValue)
	bRegenerateHealth = bValue
	ToggleRegenerateHealth()
endfunction


function ToggleRegenerateHealth()
	actor dragonActor = dragonAlias.GetActorRef()
	
	if dragonActor
		if bRegenerateHealth &&  dragonActor.GetCombatState() == 1
			float fMagnitude = 0.5 * dragonActor.GetLevel()
			RestoreHealthSpell.SetNthEffectMagnitude(0, fMagnitude)
			dragonActor.AddSpell(RestoreHealthSpell)		
		else
			dragonActor.RemoveSpell(RestoreHealthSpell)
		endif
	endif
endFunction


function SetKey (int ikey, string sKeyName)
	SetKeyMapping_SKSE(sKeyName, ikey)
endFunction


function SetDragonSpeeds_SKSE (float fMult) global native
function SetDragonSpeed (float fvalue)
SetDragonSpeeds_SKSE (fvalue)
endFunction

function SetRollAmplitude_SKSE (float fAmplitude) global native
function SetRollAmplitude (float fvalue)
	SetRollAmplitude_SKSE (fvalue)
endFunction

function SetTargetReticleMode_SKSE(int bTargetReticleMode) global native
function SetTargetReticleMode(int bTargetReticleMode)
	SetTargetReticleMode_SKSE(bTargetReticleMode)
endfunction

function SetReticleLockAnimationStyle_SKSE(int iReticleLockAnimationStyle) global native
function SetReticleLockAnimationStyle(int iReticleLockAnimationStyle)
	SetReticleLockAnimationStyle_SKSE(iReticleLockAnimationStyle)
endfunction

function SetTDMLock_SKSE(int iTDMLock) global native
function SetTDMLock(int iTDMLock)
	SetTDMLock_SKSE(iTDMLock)
endfunction

function SetPrimaryTargetMode_SKSE(int iPrimaryTargetMode) global native
function SetPrimaryTargetMode(int iPrimaryTargetMode)
	SetPrimaryTargetMode_SKSE(iPrimaryTargetMode)
endfunction

function SetMaxTargetDistance_SKSE(float fMaxTargetDistance) global native
function SetMaxTargetDistance(float fMaxTargetDistance)
	SetMaxTargetDistance_SKSE(fMaxTargetDistance)
endfunction

function SetDistanceMultiplierSmall_SKSE(float fDistanceMultiplierSmall) global native
function SetDistanceMultiplierSmall(float fDistanceMultiplierSmall)
	SetDistanceMultiplierSmall_SKSE(fDistanceMultiplierSmall)
endfunction

function SetDistanceMultiplierLarge_SKSE(float fDistanceMultiplierLarge) global native
function SetDistanceMultiplierLarge(float fDistanceMultiplierLarge)
	SetDistanceMultiplierLarge_SKSE(fDistanceMultiplierLarge)
endfunction

function SetDistanceMultiplierExtraLarge_SKSE(float fDistanceMultiplierExtraLarge) global native
function SetDistanceMultiplierExtraLarge(float fDistanceMultiplierExtraLarge)
	SetDistanceMultiplierExtraLarge_SKSE(fDistanceMultiplierExtraLarge)
endfunction

function SetMaxTargetScanAngle_SKSE(float fTargetScanAngle) global native
function SetMaxTargetScanAngle(float fTargetScanAngle)
	SetMaxTargetScanAngle_SKSE(fTargetScanAngle)
endfunction

function SetCameraLockInitiallyEnabled_SKSE(bool bEnabled) global native
function SetCameraLockInitiallyEnabled(bool bEnabled)
	SetCameraLockInitiallyEnabled_SKSE(bEnabled)
endfunction

function SetIgnoredCameraPitch_SKSE(float fIgnoredCameraPitch) global native
function SetIgnoredCameraPitch(float fIgnoredCameraPitch)
	SetIgnoredCameraPitch_SKSE(fIgnoredCameraPitch)
endfunction

function SetYawOffsetStrength_SKSE(float fYawOffsetStrength) global native
function SetYawOffsetStrength(float fYawOffsetStrength)
	SetYawOffsetStrength_SKSE(fYawOffsetStrength)
endfunction

function SetPitchOffsetStrength_SKSE(float fPitchOffsetStrength) global native
function SetPitchOffsetStrength(float fPitchOffsetStrength)
	SetPitchOffsetStrength_SKSE(fPitchOffsetStrength)
endfunction

function SetInvertPitchOffsetDuringFlight_SKSE(bool bInvert) global native
function SetInvertPitchOffsetDuringFlight(bool bInvert)
	SetInvertPitchOffsetDuringFlight_SKSE(bInvert)
endfunction

; Interface to other scripts
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; DragonFollowAliasScript, DragonRegisterScript
; GoTDragonCompanions
function SetFlyingMode_SKSE(int iState) global native
Function SetFlyingState(int iState)
	iFlyingState = iState
	SetFlyingMode_SKSE(iFlyingState)
endFunction


; DragonTameAliasScript
bool function DragonLandPlayerRiding_SKSE(ObjectReference LandTarget) global native
bool function DragonLandPlayerRiding(ObjectReference LandTarget)
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - DragonLandPlayerRiding")
return DragonLandPlayerRiding_SKSE(LandTarget)
endFunction


; Interface to GoTDragonCompanions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;Function SetFlyingState(int iState) (see above)

function SetAutoCombat_SKSE(bool bAuto) global native
function SetAutoCombat(bool bValue)
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - SetAutoCombat: bAutoCombat = " + bValue)
	bAutoCombat = bValue
	SetAutoCombat_SKSE(bValue)
endfunction

; ShoutList functions are only kept to keep API compatible for GoTDragonCompanions
; They do no longer provide any functionality, nor have any effect
; Can be removed whenever the function calls in GoTDragonCompanions are removed.
function SetBreathShoutList(Formlist ShoutList)
endfunction

function SetBallShoutList(Formlist ShoutList)
endfunction

Formlist function GetBreathShoutList()
	Formlist  ShoutList
	return ShoutList
endfunction

Formlist function GetBallShoutList()
	Formlist  ShoutList
	return ShoutList
endfunction
; END legacy ShoutList functions

bool function GetInitialAutoCombatMode_SKSE() global native
bool function GetInitialAutoCombatMode()
	return GetInitialAutoCombatMode_SKSE()
endfunction

function SetDisplayAttackMessage_SKSE(bool bDisplay) global native
function SetDisplayAttackMessage(bool bDisplay)
	SetDisplayAttackMessage_SKSE(bDisplay)
endfunction

bool function DragonAttack_SKSE(bool bAlternateAttack = false) global native
bool function DragonAttack(bool bAlternateAttack = false)
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - DragonAttack")
	return DragonAttack_SKSE(bAlternateAttack)
endfunction

bool function DragonTakeOffPlayerRiding_SKSE(ObjectReference TakeOffTarget) global native
bool function DragonTakeOffPlayerRiding(ObjectReference TakeOffTarget)
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - DragonTakeOffPlayerRiding")
	return DragonTakeOffPlayerRiding_SKSE(TakeOffTarget)
endfunction

bool function DragonHoverPlayerRiding_SKSE(ObjectReference HoverTarget) global native
bool function DragonHoverPlayerRiding(ObjectReference HoverTarget)
_ts_Debug_Trace_Message("IDRC - _ts_DR_RideControlScript - DragonHoverPlayerRiding")
	return DragonHoverPlayerRiding_SKSE(HoverTarget)
endfunction

