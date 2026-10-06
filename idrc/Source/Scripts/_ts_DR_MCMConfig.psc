Scriptname _ts_DR_MCMConfig extends SKI_ConfigBase conditional

Quest property DragonRideQuest auto
string property DragonName1 auto
string property DragonName2 auto

bool property bDisplayAttackMessage auto hidden conditional
;bool bSkipOrbit
bool bTriggerMessage
bool bRegenHealth
bool bDragonRoars
int property iCombatMode auto hidden conditional
string[] CombatModeList
int iDisplayHealthKey
int iForwardKey
int iBackKey
int iStrafeLeftKey
int iStrafeRightKey
int iDragonUpKey
int iDragonDownKey
int iAutoCombatKey
int iRunKey
int iToggleAlwaysRunKey
int iToggleLockReticleKey
int iTogglePrimaryTargetModeKey
int property iTargetReticleMode auto hidden conditional
string[] TargetReticleModeList
int property iReticleLockAnimationStyle auto hidden conditional
string[] ReticleLockAnimationStyleList
int property iTDMLock auto hidden conditional
string[] TDMLockList
int property iPrimaryTargetMode auto hidden conditional
string[] PrimaryTargetModeList
float property fMaxTargetDistance auto hidden conditional
float property fDistanceMultiplierSmall auto hidden conditional
float property fDistanceMultiplierLarge auto hidden conditional
float property fDistanceMultiplierExtraLarge auto hidden conditional
float property fTargetScanAngle auto hidden conditional
string[] InitialCameraLockList
bool property bCameraLockInitiallyEnabled auto hidden conditional
float property fIgnoredCameraPitch auto hidden conditional
float property fYawOffsetStrength auto hidden conditional
float property fPitchOffsetStrength auto hidden conditional
bool property bInvertPitchOffsetDuringFlight auto hidden conditional

int iToggleCameraLockKey
;int iAttackRegularKey
;int iAttackAlternateKey
float property fDragonSpeed auto hidden conditional
float property fRollAmplitude auto hidden conditional

int iDisplayMode_OID
int iDisplayHealthKey_OID
int iForward_OID
int iBack_OID
int iStrafeLeft_OID
int iStrafeRight_OID
int iRun_OID
int iToggleAlwaysRun_OID
;int iAttackRegular_OID
;int iAttackAlternate_OID
int iDragonUp_OID
int iDragonDown_OID
int iDragonSpeed_OID
int iRollAmplitude_OID
;int iSkipOrbit_OID
int iTriggerMessage_OID
int iDragonRoars_OID
int iRegenHealth_OID
int iAutoCombat_OID
int iCombatMode_OID
int iToggleLockReticle_OID
int iTogglePrimaryTargetMode_OID
int iTargetReticleMode_OID
int iReticleLockAnimationStyle_OID
int iTDMLock_OID
int iPrimaryTargetMode_OID
int iMaxTargetDistance_OID
int iDistanceMultiplierSmall_OID
int iDistanceMultiplierLarge_OID
int iDistanceMultiplierExtraLarge_OID
int iTargetScanAngle_OID
int iCameraLockEnabled_OID
int iToggleCameraLockKey_OID
int iIgnoredCameraPitch_OID
int iYawOffsetStrength_OID
int iPitchOffsetStrength_OID
int iInvertPitchOffsetDuringFlight_OID

int function GetDisplayHealthKey()
	return iDisplayHealthKey
endFunction

int function GetForwardKey()
	return iForwardKey
endFunction

int function GetBackKey()
	return iBackKey
endFunction

int function GetStrafeLeftKey()
	return iStrafeLeftKey
endFunction

int function GetStrafeRightKey()
	return iStrafeRightKey
endFunction

int function GetDragonUpKey()
	return iDragonUpKey
endFunction

int function GetDragonDownKey()
	return iDragonDownKey
endFunction

int function GetAutoCombatKey()
	return iAutoCombatKey
endFunction

int function GetRunKey()
	return iRunKey
endFunction

int function GetToggleAlwaysRunKey()
	return iToggleAlwaysRunKey
endFunction

int function GetToggleLockReticleKey()
	return iToggleLockReticleKey
endFunction
int function GetTogglePrimaryTargetModeKey()
	return iTogglePrimaryTargetModeKey
endFunction
bool function GetTargetReticleMode()
	return iTargetReticleMode
endFunction
int function GetReticleLockAnimationStyle()
	return iReticleLockAnimationStyle
endFunction
int function GetTDMLock()
	return iTDMLock
endFunction
int function GetPrimaryTargetMode()
	return iPrimaryTargetMode
endFunction
float function GetMaxTargetDistance()
	return fMaxTargetDistance
endFunction
float function GetDistanceMultiplierSmall()
	return fDistanceMultiplierSmall
endFunction
float function GetDistanceMultiplierLarge()
	return fDistanceMultiplierLarge
endFunction
float function GetDistanceMultiplierExtraLarge()
	return fDistanceMultiplierExtraLarge
endFunction
float function GetTargetScanAngle()
	return fTargetScanAngle
endFunction

bool function GetCameraLockInitiallyEnabled()
	return bCameraLockInitiallyEnabled
endFunction
int function GetToggleCameraLockKey()
	return iToggleCameraLockKey
endFunction
float function GetIgnoredCameraPitch()
	return fIgnoredCameraPitch
endFunction
float function GetYawOffsetStrength()
	return fYawOffsetStrength
endFunction
float function GetPitchOffsetStrength()
	return fPitchOffsetStrength
endFunction
bool function GetInvertPitchOffsetDuringFlight()
	return bInvertPitchOffsetDuringFlight
endFunction

event OnConfigInit()
	iDisplayHealthKey = input.GetMappedKey("Ready Weapon", 0)
	iForwardKey = input.GetMappedKey("Forward", 0)
	iBackKey = input.GetMappedKey("Back", 0)
	iStrafeLeftKey = input.GetMappedKey("Strafe Left", 0)
	iStrafeRightKey = input.GetMappedKey("Strafe Right", 0)
	iRunKey = input.GetMappedKey("Run", 0)
	iToggleAlwaysRunKey = input.GetMappedKey("Toggle Always Run", 0)
;	iAttackRegularKey = 257 ; right mouse button
;	iAttackAlternateKey = 256 ; left mouse button
	iDragonUpKey = 22 ;  "U"
	iDragonDownKey = 35 ; "H"
	iAutoCombatKey = 34 ; "G"
	bDisplayAttackMessage = true
;	bSkipOrbit = true
	bTriggerMessage = false
	bDragonRoars = false
	bRegenHealth = false
	iCombatMode = 0
	CombatModeList = new string[2]
	CombatModeList[0] = "Manual"
	CombatModeList[1] = "Automatic"
	fDragonSpeed = 1.0
	fRollAmplitude = 0.5
	iToggleLockReticleKey = 38 ; "L"
	iTogglePrimaryTargetModeKey = 46 ; "C"
	iTargetReticleMode = 1
	iReticleLockAnimationStyle = 0
	iPrimaryTargetMode = 1
	iTDMLock = 0
	TargetReticleModeList = new string[3]
	TargetReticleModeList[0] = "Off"
	TargetReticleModeList[1] = "On"
	TargetReticleModeList[2] = "Only Combat Target"
	ReticleLockAnimationStyleList = new string[2]
	ReticleLockAnimationStyleList[0] = "Rotate"
	ReticleLockAnimationStyleList[1] = "Expand"
	TDMLockList = new string[2]
	TDMLockList[0] = "TDM Target"
	TDMLockList[1] = "IDRC Target"
	PrimaryTargetModeList = new string[2]
	PrimaryTargetModeList[0] = "From Screen Center"
	PrimaryTargetModeList[1] = "Current Combat Target"
	fMaxTargetDistance = 8000.0
	fDistanceMultiplierSmall = 1.0
	fDistanceMultiplierLarge = 2.0
	fDistanceMultiplierExtraLarge = 4.0
	fTargetScanAngle = 7.0
	InitialCameraLockList = new string[2]
	InitialCameraLockList[0] = "Off"
	InitialCameraLockList[1] = "On"
	bCameraLockInitiallyEnabled = true
	iToggleCameraLockKey = 46 ; "C"
	fIgnoredCameraPitch = 8.0
	fYawOffsetStrength = 0.25
	fPitchOffsetStrength = 0.25
	bInvertPitchOffsetDuringFlight = true
EndEvent


Event OnOptionDefault(int option)
	if option == iDisplayMode_OID
		bDisplayAttackMessage = true
		SetToggleOptionValue(iDisplayMode_OID, bDisplayAttackMessage, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetDisplayAttackMessage(bDisplayAttackMessage)
;	elseif option == iSkipOrbit_OID
;		bSkipOrbit = true
;		SetToggleOptionValue(iSkipOrbit_OID, bSkipOrbit, false)
;		(DragonRideQuest as _ts_DR_RideControlScript).SetSkipOrbit(bSkipOrbit)
	elseif option == iTriggerMessage_OID
		bTriggerMessage = false
		SetToggleOptionValue(iTriggerMessage_OID, bTriggerMessage, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetTriggerMessage(bTriggerMessage)
	elseif option == iDragonRoars_OID
		bDragonRoars = false
		SetToggleOptionValue(iDragonRoars_OID, bDragonRoars, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetDragonRoars(bDragonRoars)
	elseif option == iRegenHealth_OID
		bRegenHealth = false
		SetToggleOptionValue(iRegenHealth_OID, bRegenHealth, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetRegenerateHealth(bRegenHealth)
	elseif option == iCombatMode_OID
		iCombatMode = 0
		SetMenuOptionValue(iCombatMode_OID, CombatModeList[iCombatMode])
		(DragonRideQuest as _ts_DR_RideControlScript).SetInitialAutoCombatMode(false)
	elseif option == iDisplayHealthKey_OID
		iDisplayHealthKey = input.GetMappedKey("Ready Weapon", 0)
		SetKeyMapOptionValue(iDisplayHealthKey_OID, iDisplayHealthKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iDisplayHealthKey, "DisplayHealth")
	elseif option == iDragonSpeed_OID
		fDragonSpeed = 1.0
		SetSliderOptionValue(iDragonSpeed_OID, fDragonSpeed, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetDragonSpeed(fDragonSpeed)
	elseif option == iRollAmplitude_OID
		fRollAmplitude = 0.5
		SetSliderOptionValue(iRollAmplitude_OID, fRollAmplitude, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetRollAmplitude(fRollAmplitude)
	elseif option == iForward_OID
		iForwardKey = input.GetMappedKey("Forward", 0)
		SetKeyMapOptionValue(iForward_OID, iForwardKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iForwardKey, "Forward")
	elseif option == iBack_OID
		iBackKey = input.GetMappedKey("Back", 0)
		SetKeyMapOptionValue(iBack_OID, iBackKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iBackKey, "Back")
	elseif option == iStrafeLeft_OID
		iStrafeLeftKey = input.GetMappedKey("Strafe Left", 0)
		SetKeyMapOptionValue(iStrafeLeft_OID, iStrafeLeftKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iStrafeLeftKey, "StrafeLeft")
	elseif option == iStrafeRight_OID
		iStrafeRightKey = input.GetMappedKey("Strafe Right", 0)
		SetKeyMapOptionValue(iStrafeRight_OID, iStrafeRightKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iStrafeRightKey, "StrafeRight")
	elseif option == iRun_OID
		iRunKey = input.GetMappedKey("Run", 0)
		SetKeyMapOptionValue(iRun_OID, iRunKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iRunKey, "Run")
	elseif option == iToggleAlwaysRun_OID
		iToggleAlwaysRunKey = input.GetMappedKey("Toggle Always Run", 0)
		SetKeyMapOptionValue(iToggleAlwaysRun_OID, iToggleAlwaysRunKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iToggleAlwaysRunKey, "ToggleAlwaysRun")
;	elseif option == iAttackRegular_OID
;		iAttackRegularKey = 257 ;  right mouse button
;		SetKeyMapOptionValue(iAttackRegular_OID, iAttackRegularKey, false)
;		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iAttackRegularKey, "AttackRegular")
;	elseif option == iAttackAlternate_OID
;		iAttackAlternateKey = 256 ;  right mouse button
;		SetKeyMapOptionValue(iAttackAlternate_OID, iAttackAlternateKey, false)
;		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iAttackAlternateKey, "AttackAlternate")
	elseif option == iDragonUp_OID
		iDragonUpKey = 22
		SetKeyMapOptionValue(iDragonUp_OID, iDragonUpKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iDragonUpKey, "DragonUp")
	elseif option == iDragonDown_OID
		iDragonDownKey = 35
		SetKeyMapOptionValue(iDragonDown_OID, iDragonDownKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iDragonDownKey, "DragonDown")
	elseif option == iAutoCombat_OID
		iAutoCombatKey = 34
		SetKeyMapOptionValue(iAutoCombat_OID, iAutoCombatKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iAutoCombatKey, "ToggleAutoCombat")
	elseif option == iToggleLockReticle_OID
		iToggleLockReticleKey = 38
		SetKeyMapOptionValue(iToggleLockReticle_OID, iToggleLockReticleKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iToggleLockReticleKey, "ToggleLockReticle")
	elseif option == iTogglePrimaryTargetMode_OID
		iTogglePrimaryTargetModeKey = 46
		SetKeyMapOptionValue(iTogglePrimaryTargetMode_OID, iTogglePrimaryTargetModeKey, false)
;		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iTogglePrimaryTargetModeKey, "TogglePrimaryTargetMode")
	elseif option == iTargetReticleMode_OID
		iTargetReticleMode = 1
		SetMenuOptionValue(iTargetReticleMode_OID, TargetReticleModeList[iTargetReticleMode])
		(DragonRideQuest as _ts_DR_RideControlScript).SetTargetReticleMode(iTargetReticleMode)
	elseif option == iReticleLockAnimationStyle_OID
		iReticleLockAnimationStyle = 0
		SetMenuOptionValue(iReticleLockAnimationStyle_OID, ReticleLockAnimationStyleList[iReticleLockAnimationStyle])
		(DragonRideQuest as _ts_DR_RideControlScript).SetReticleLockAnimationStyle(iReticleLockAnimationStyle)
	elseif option == iTDMLock_OID
		iTDMLock = 1
		SetMenuOptionValue(iTDMLock_OID, TDMLockList[iTDMLock])
		(DragonRideQuest as _ts_DR_RideControlScript).SetTDMLock(iTDMLock)
	elseif option == iPrimaryTargetMode_OID
		iPrimaryTargetMode = 1
		SetMenuOptionValue(iPrimaryTargetMode_OID, PrimaryTargetModeList[iPrimaryTargetMode])
		(DragonRideQuest as _ts_DR_RideControlScript).SetPrimaryTargetMode(iPrimaryTargetMode)
	elseIf option == iMaxTargetDistance_OID
		fMaxTargetDistance = 8000.0
		SetSliderOptionValue(iMaxTargetDistance_OID, fMaxTargetDistance, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetMaxTargetDistance(fMaxTargetDistance)
	elseIf option == iDistanceMultiplierSmall_OID
		fDistanceMultiplierSmall = 1.0
		SetSliderOptionValue(iDistanceMultiplierSmall_OID, fDistanceMultiplierSmall, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetDistanceMultiplierSmall(fDistanceMultiplierSmall)
	elseIf option == iDistanceMultiplierLarge_OID
		fDistanceMultiplierLarge = 2.0
		SetSliderOptionValue(iDistanceMultiplierLarge_OID, fDistanceMultiplierLarge, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetDistanceMultiplierLarge(fDistanceMultiplierLarge)
	elseIf option == iDistanceMultiplierExtraLarge_OID
		fDistanceMultiplierExtraLarge = 4.0
		SetSliderOptionValue(iDistanceMultiplierExtraLarge_OID, fDistanceMultiplierExtraLarge, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetDistanceMultiplierExtraLarge(fDistanceMultiplierExtraLarge)
	elseIf option == iTargetScanAngle_OID
		fTargetScanAngle = 7.0
		SetSliderOptionValue(iTargetScanAngle_OID, fTargetScanAngle, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetMaxTargetScanAngle(fTargetScanAngle)
	elseif option == iCameraLockEnabled_OID
		bCameraLockInitiallyEnabled = true
		SetMenuOptionValue(iCameraLockEnabled_OID, InitialCameraLockList[1])
		(DragonRideQuest as _ts_DR_RideControlScript).SetCameraLockInitiallyEnabled(bCameraLockInitiallyEnabled)
	elseif option == iToggleCameraLockKey_OID
		iToggleCameraLockKey = 46
		SetKeyMapOptionValue(iToggleCameraLockKey_OID, iToggleCameraLockKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iToggleCameraLockKey, "ToggleCameraLock")	
	elseif option == iIgnoredCameraPitch_OID
		fIgnoredCameraPitch = 8.0
		SetSliderOptionValue(iIgnoredCameraPitch_OID, fIgnoredCameraPitch, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetIgnoredCameraPitch(fIgnoredCameraPitch)
	elseif option == iYawOffsetStrength_OID
		fYawOffsetStrength = 0.25
		SetSliderOptionValue(iYawOffsetStrength_OID, fYawOffsetStrength, "{2}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetYawOffsetStrength(fYawOffsetStrength)
	elseif option == iPitchOffsetStrength_OID
		fPitchOffsetStrength = 0.25
		SetSliderOptionValue(iPitchOffsetStrength_OID, fPitchOffsetStrength, "{2}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetPitchOffsetStrength(fPitchOffsetStrength)
	elseif option == iInvertPitchOffsetDuringFlight_OID
		bInvertPitchOffsetDuringFlight = true
		SetToggleOptionValue(iInvertPitchOffsetDuringFlight_OID, bInvertPitchOffsetDuringFlight)
		(DragonRideQuest as _ts_DR_RideControlScript).SetInvertPitchOffsetDuringFlight(bInvertPitchOffsetDuringFlight)
	endif

endevent

function OnPageReset(String page)

	if page == "Dragon Ride Controls"
		SetCursorFillMode(LEFT_TO_RIGHT)
		SetCursorPosition(0)
		AddHeaderOption("Movement Controls", 0)
		AddHeaderOption("Other Controls", 0)
		AddEmptyOption()
		AddEmptyOption()
		iForward_OID = AddKeyMapOption("Forward", iForwardKey, 0)
;		iAttackRegular_OID = AddKeyMapOption("Regular Attack", iAttackRegularKey, 0)
		iRun_OID = AddKeyMapOption("Fast Mode", iRunKey, 0)
		iBack_OID = AddKeyMapOption("Back / Land", iBackKey, 0)
;		iAttackAlternate_OID = AddKeyMapOption("Alternate Attack", iAttackAlternateKey, 0)
		iToggleAlwaysRun_OID = AddKeyMapOption("Toggle Always Fast", iToggleAlwaysRunKey, 0)
		iStrafeLeft_OID = AddKeyMapOption("Turn Left", iStrafeLeftKey, 0)
		iDisplayHealthKey_OID = AddKeyMapOption("Display " + DragonName1 + " Health", iDisplayHealthKey, 0)	
		iStrafeRight_OID = AddKeyMapOption("Turn Right", iStrafeRightKey, 0)
		AddEmptyOption()
		iDragonUp_OID = AddKeyMapOption("Fly Upwards / Take Off", iDragonUpKey, 0)
		AddEmptyOption()
		iDragonDown_OID = AddKeyMapOption("Fly Downwards", iDragonDownKey, 0)
		AddEmptyOption()
		AddEmptyOption()
		AddEmptyOption()
		int index = 0
		if bCameraLockInitiallyEnabled
			index = 1
		endIf
		iCameraLockEnabled_OID = AddMenuOption("Initial Camera Lock", InitialCameraLockList[index], 0)
		AddEmptyOption()
		iToggleCameraLockKey_OID = AddKeyMapOption("Toggle Camera Lock", iToggleCameraLockKey, 0)
		AddEmptyOption()
		iIgnoredCameraPitch_OID = AddSliderOption("Ignored Camera Pitch", fIgnoredCameraPitch, "{1}")
	elseif page == "Other Options"
		SetCursorFillMode(TOP_TO_BOTTOM)
		SetCursorPosition(0)
		AddHeaderOption("Options", 0)
;		AddHeaderOption("Controls", 1)
		AddEmptyOption()
		iDragonSpeed_OID = AddSliderOption("Flying Speed", fDragonSpeed, "{1}")
		iRollAmplitude_OID = AddSliderOption("Camera Roll Strength", fRollAmplitude, "{1}")
		AddEmptyOption()
		iRegenHealth_OID = AddToggleOption("Regenerate Health", bRegenHealth, 0)
;		iSkipOrbit_OID = AddToggleOption("Skip Orbit Mode", bSkipOrbit, 0)
		iTriggerMessage_OID = AddToggleOption("Combat Comments", bTriggerMessage, 0)
		iDragonRoars_OID = AddToggleOption("Dragon Roars", bDragonRoars, 0)
		iDisplayMode_OID = AddToggleOption("Display Attack Messages", bDisplayAttackMessage, 0)
	elseif page == "Dragon Combat Options"
		SetCursorFillMode(LEFT_TO_RIGHT)
		SetCursorPosition(0)
		AddHeaderOption("Combat Mode", 0)
		AddHeaderOption("Combat Target", 0)
		iCombatMode_OID = AddMenuOption("Initial Combat Mode", CombatModeList[iCombatMode])
		iPrimaryTargetMode_OID = AddMenuOption("Primary Target", PrimaryTargetModeList[iPrimaryTargetMode])
		iAutoCombat_OID = AddKeyMapOption("Toggle Combat Mode", iAutoCombatKey, 0)
		iTargetScanAngle_OID = AddSliderOption("Target Scan Angle", fTargetScanAngle, "{1}")
		AddEmptyOption()
		iMaxTargetDistance_OID = AddSliderOption("Maximum Target Distance", fMaxTargetDistance, "{1}")
		AddHeaderOption("Target Reticle (Requires TrueHUD)", 0)
		iDistanceMultiplierSmall_OID = AddSliderOption("Distance Multiplier Small Targets", fDistanceMultiplierSmall, "{1}")
		iTargetReticleMode_OID = AddMenuOption("Target Reticle", TargetReticleModeList[iTargetReticleMode])
		iDistanceMultiplierLarge_OID = AddSliderOption("Distance Multiplier Large Targets", fDistanceMultiplierLarge, "{1}")
		iToggleLockReticle_OID = AddKeyMapOption("Lock / Unlock Target Reticle", iToggleLockReticleKey, 0)
		iDistanceMultiplierExtraLarge_OID = AddSliderOption("Distance Multiplier Extra Large Targets", fDistanceMultiplierExtraLarge, "{1}")
		iReticleLockAnimationStyle_OID = AddMenuOption("Reticle Lock Animation", ReticleLockAnimationStyleList[iReticleLockAnimationStyle])
		AddEmptyOption()
		AddEmptyOption()
		AddHeaderOption("Camera Offsets During Attacks", 0)
		AddHeaderOption("True Directional Movement Interaction", 0)
		iYawOffsetStrength_OID = AddSliderOption("Yaw Offset Strength", fYawOffsetStrength, "{2}")
		iTDMLock_OID = AddMenuOption("TDM Target Lock Focus", TDMLockList[iTDMLock])
		iPitchOffsetStrength_OID = AddSliderOption("Pitch Offset Strength", fPitchOffsetStrength, "{2}")
		AddEmptyOption()
;		iTogglePrimaryTargetMode_OID = AddKeyMapOption("Toggle Primary Target", iTogglePrimaryTargetModeKey, 0)
		iInvertPitchOffsetDuringFlight_OID = AddToggleOption("Invert Pitch Offset During Flight Attacks", bInvertPitchOffsetDuringFlight, 0)
	endIf	
endFunction

event OnOptionSliderOpen(int option)
	if option == iDragonSpeed_OID
		SetSliderDialogStartValue(fDragonSpeed)
		SetSliderDialogDefaultValue(1.0)
		SetSliderDialogRange(0.5, 2.0)
		SetSliderDialogInterval(0.1)
	elseif option == iRollAmplitude_OID
		SetSliderDialogStartValue(fRollAmplitude)
		SetSliderDialogDefaultValue(0.5)
		SetSliderDialogRange(0.0, 1.0)
		SetSliderDialogInterval(0.1)
	elseif option == iMaxTargetDistance_OID
		SetSliderDialogStartValue(fMaxTargetDistance)
		SetSliderDialogDefaultValue(8000.0)
		SetSliderDialogRange(0.0, 8000.0)
		SetSliderDialogInterval(10.0)
	elseif option == iDistanceMultiplierSmall_OID
		SetSliderDialogStartValue(fDistanceMultiplierSmall)
		SetSliderDialogDefaultValue(1.0)
		SetSliderDialogRange(0.0, 10.0)
		SetSliderDialogInterval(0.1)
	elseif option == iDistanceMultiplierLarge_OID
		SetSliderDialogStartValue(fDistanceMultiplierLarge)
		SetSliderDialogDefaultValue(2.0)
		SetSliderDialogRange(0.0, 10.0)
		SetSliderDialogInterval(0.1)
	elseif option == iDistanceMultiplierExtraLarge_OID
		SetSliderDialogStartValue(fDistanceMultiplierExtraLarge)
		SetSliderDialogDefaultValue(4.0)
		SetSliderDialogRange(0.0, 10.0)
		SetSliderDialogInterval(0.1)
	elseif option == iTargetScanAngle_OID
		SetSliderDialogStartValue(fTargetScanAngle)
		SetSliderDialogDefaultValue(7.0)
		SetSliderDialogRange(0.0, 90.0)
		SetSliderDialogInterval(1.0)
	elseif option == iIgnoredCameraPitch_OID
		SetSliderDialogStartValue(fIgnoredCameraPitch)
		SetSliderDialogDefaultValue(8.0)
		SetSliderDialogRange(0.0, 90.0)
		SetSliderDialogInterval(1.0)
	elseif option == iYawOffsetStrength_OID
		SetSliderDialogStartValue(fYawOffsetStrength)
		SetSliderDialogDefaultValue(0.25)
		SetSliderDialogRange(-1.0, 1.0)
		SetSliderDialogInterval(0.05)
	elseif option == iPitchOffsetStrength_OID
		SetSliderDialogStartValue(fPitchOffsetStrength)
		SetSliderDialogDefaultValue(0.25)
		SetSliderDialogRange(0.0, 1.0)
		SetSliderDialogInterval(0.05)
	endif
endevent

event OnOptionSliderAccept(int option, float value)
	if option == iDragonSpeed_OID
		fDragonSpeed = value
		SetSliderOptionValue(iDragonSpeed_OID, fDragonSpeed, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetDragonSpeed(fDragonSpeed)
	elseif option == iRollAmplitude_OID
		fRollAmplitude = value
		SetSliderOptionValue(iRollAmplitude_OID, fRollAmplitude, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetRollAmplitude(fRollAmplitude)
	elseif option == iMaxTargetDistance_OID
		fMaxTargetDistance = value
		SetSliderOptionValue(iMaxTargetDistance_OID, fMaxTargetDistance, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetMaxTargetDistance(fMaxTargetDistance)
	elseif option == iDistanceMultiplierSmall_OID
		fDistanceMultiplierSmall = value
		SetSliderOptionValue(iDistanceMultiplierSmall_OID, fDistanceMultiplierSmall, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetDistanceMultiplierSmall(fDistanceMultiplierSmall)
	elseif option == iDistanceMultiplierLarge_OID
		fDistanceMultiplierLarge = value
		SetSliderOptionValue(iDistanceMultiplierLarge_OID, fDistanceMultiplierLarge, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetDistanceMultiplierLarge(fDistanceMultiplierLarge)
	elseif option == iDistanceMultiplierExtraLarge_OID
		fDistanceMultiplierExtraLarge = value
		SetSliderOptionValue(iDistanceMultiplierExtraLarge_OID, fDistanceMultiplierExtraLarge, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetDistanceMultiplierExtraLarge(fDistanceMultiplierExtraLarge)
	elseif option == iTargetScanAngle_OID
		fTargetScanAngle = value
		SetSliderOptionValue(iTargetScanAngle_OID, fTargetScanAngle, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetMaxTargetScanAngle(fTargetScanAngle)
	elseif option == iIgnoredCameraPitch_OID
		fIgnoredCameraPitch = value
		SetSliderOptionValue(iIgnoredCameraPitch_OID, fIgnoredCameraPitch, "{1}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetIgnoredCameraPitch(fIgnoredCameraPitch)
	elseif option == iYawOffsetStrength_OID
		fYawOffsetStrength = value
		SetSliderOptionValue(iYawOffsetStrength_OID, fYawOffsetStrength, "{2}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetYawOffsetStrength(fYawOffsetStrength)
	elseif option == iPitchOffsetStrength_OID
		fPitchOffsetStrength = value
		SetSliderOptionValue(iPitchOffsetStrength_OID, fPitchOffsetStrength, "{2}")
		(DragonRideQuest as _ts_DR_RideControlScript).SetPitchOffsetStrength(fPitchOffsetStrength)
	endif
endevent

event OnOptionMenuOpen(int option)
	if (option == iTargetReticleMode_OID)
		SetMenuDialogOptions(TargetReticleModeList)
		SetMenuDialogStartIndex(iTargetReticleMode)
		SetMenuDialogDefaultIndex(1)
	elseif (option == iReticleLockAnimationStyle_OID)
		SetMenuDialogOptions(ReticleLockAnimationStyleList)
		SetMenuDialogStartIndex(iReticleLockAnimationStyle)
		SetMenuDialogDefaultIndex(0)
	elseif (option == iTDMLock_OID)
		SetMenuDialogOptions(TDMLockList)
		SetMenuDialogStartIndex(iTDMLock)
		SetMenuDialogDefaultIndex(0)
	elseif (option == iPrimaryTargetMode_OID)
		SetMenuDialogOptions(PrimaryTargetModeList)
		SetMenuDialogStartIndex(iPrimaryTargetMode)
		SetMenuDialogDefaultIndex(1)
	elseif (option == iCombatMode_OID)
		SetMenuDialogOptions(CombatModeList)
		SetMenuDialogStartIndex(iCombatMode)
		SetMenuDialogDefaultIndex(0)
	elseif (option == iCameraLockEnabled_OID)
		SetMenuDialogOptions(InitialCameraLockList)
		int index = 0
		if bCameraLockInitiallyEnabled
			index = 1
		endif
		SetMenuDialogStartIndex(index)
		SetMenuDialogDefaultIndex(1)
	endIf
endEvent

event OnOptionMenuAccept(int option, int index)
	if (option == iTargetReticleMode_OID)
		iTargetReticleMode = index
		SetMenuOptionValue(iTargetReticleMode_OID, TargetReticleModeList[iTargetReticleMode])
		(DragonRideQuest as _ts_DR_RideControlScript).SetTargetReticleMode(iTargetReticleMode)
	elseif (option == iReticleLockAnimationStyle_OID)
		iReticleLockAnimationStyle = index
		SetMenuOptionValue(iReticleLockAnimationStyle_OID, ReticleLockAnimationStyleList[iReticleLockAnimationStyle])
		(DragonRideQuest as _ts_DR_RideControlScript).SetReticleLockAnimationStyle(iReticleLockAnimationStyle)
	elseif (option == iTDMLock_OID)
		iTDMLock = index
		SetMenuOptionValue(iTDMLock_OID, TDMLockList[iTDMLock])
		(DragonRideQuest as _ts_DR_RideControlScript).SetTDMLock(iTDMLock)
	elseif (option == iPrimaryTargetMode_OID)
		iPrimaryTargetMode = index
		SetMenuOptionValue(iPrimaryTargetMode_OID, PrimaryTargetModeList[iPrimaryTargetMode])
		(DragonRideQuest as _ts_DR_RideControlScript).SetPrimaryTargetMode(iPrimaryTargetMode)
	elseif (option == iCombatMode_OID)
		iCombatMode = index
		SetMenuOptionValue(iCombatMode_OID, CombatModeList[iCombatMode])
		bool bAutoCombat = false
		if (iCombatMode == 1)
			bAutoCombat = true
		endIf
		(DragonRideQuest as _ts_DR_RideControlScript).SetInitialAutoCombatMode(bAutoCombat)
	elseif (option == iCameraLockEnabled_OID)
		if index == 0
			bCameraLockInitiallyEnabled = false
		else
			bCameraLockInitiallyEnabled = true
		endIf
		SetMenuOptionValue(iCameraLockEnabled_OID, InitialCameraLockList[index])
		(DragonRideQuest as _ts_DR_RideControlScript).SetCameraLockInitiallyEnabled(bCameraLockInitiallyEnabled)
	endif
endEvent

function OnOptionSelect(Int option)

	if option == iDisplayMode_OID
		bDisplayAttackMessage = !bDisplayAttackMessage
		SetToggleOptionValue(iDisplayMode_OID, bDisplayAttackMessage, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetDisplayAttackMessage(bDisplayAttackMessage)
;	elseif option == iSkipOrbit_OID
;		bSkipOrbit = !bSkipOrbit
;		SetToggleOptionValue(iSkipOrbit_OID, bSkipOrbit, false)
;		(DragonRideQuest as _ts_DR_RideControlScript).SetSkipOrbit(bSkipOrbit)
	elseif option == iTriggerMessage_OID
		bTriggerMessage = !bTriggerMessage
		SetToggleOptionValue(iTriggerMessage_OID, bTriggerMessage, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetTriggerMessage(bTriggerMessage)
	elseif option == iDragonRoars_OID
		bDragonRoars = !bDragonRoars
		SetToggleOptionValue(iDragonRoars_OID, bDragonRoars, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetDragonRoars(bDragonRoars)
	elseif option == iRegenHealth_OID
		bRegenHealth = !bRegenHealth
		SetToggleOptionValue(iRegenHealth_OID, bRegenHealth, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetRegenerateHealth(bRegenHealth)
	elseif option == iInvertPitchOffsetDuringFlight_OID
		bInvertPitchOffsetDuringFlight = !bInvertPitchOffsetDuringFlight
		SetToggleOptionValue(iInvertPitchOffsetDuringFlight_OID, bInvertPitchOffsetDuringFlight, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetInvertPitchOffsetDuringFlight(bInvertPitchOffsetDuringFlight)
	endIf
endFunction

function OnOptionHighlight(Int option)

	if option == iDisplayMode_OID
		SetInfoText("Displays messages when commanding attacks.")
;	elseif option == iSkipOrbit_OID
;		SetInfoText("If activated, Orbit mode is skipped when accelerating or decelerating the dragon.")
	elseif option == iInvertPitchOffsetDuringFlight_OID
		SetInfoText("If activated, the camera is offset below the center during flight attacks, otherwise above center.")
	elseif option == iTriggerMessage_OID
		SetInfoText("If activated, the dragon comments when commanded to attack.")
	elseif option == iDragonRoars_OID
		SetInfoText("If activated, the dragon roars while riding it.")
	elseif option == iRegenHealth_OID
		SetInfoText("If activated, the dragon regenerates health during combat.")
	elseif option == iCombatMode_OID
		SetInfoText("Defines if initial combat mode when mounting a dragon is 'Automatic' or 'Manual'.")
	elseif option == iDisplayHealthKey_OID
		SetInfoText("Press this key to show "+ DragonName2 +" health.")
	elseif option == iDragonSpeed_OID
		SetInfoText("Sets "+ DragonName1 + " flying speed.")
	elseif option == iRollAmplitude_OID
		SetInfoText("Defines the strength of the camera roll while mounted.")
	elseif option == iForward_OID
		SetInfoText("Press this key to accelerate the dragon (Hover->Fly), and to move forward while grounded.")
	elseif option == iBack_OID
		SetInfoText("Press this key to decelerate the dragon (Fly->Hover->Land), and to take off while grounded.")
	elseif option == iStrafeLeft_OID
		SetInfoText("Press this key to turn left.")
	elseif option == iStrafeRight_OID
		SetInfoText("Press this key to turn right.")
	elseif option == iRun_OID
		SetInfoText("While keeping this key pressed the dragon takes tighter turns and faster up/down movements.")
	elseif option == iToggleAlwaysRun_OID
		SetInfoText("Press this key to toggle on/off tighter turns, and faster up and down movements.")
;	elseif option == iAttackRegular_OID
;		SetInfoText("Press this key to command a regular dragon attack.")
;	elseif option == iAttackAlternate_OID
;		SetInfoText("Press this key to command an alternate dragon attack (if applicable).")
	elseif option == iDragonUp_OID
		SetInfoText("Press this key to steer the dragon upwards during flight, and to take off while grounded.")
	elseif option == iDragonDown_OID
		SetInfoText("Press this key to steer the dragon downwards during flight.")
	elseif option == iAutoCombat_OID
		SetInfoText("Press this key to toggle between automatic and manual dragon combat mode.")
	elseif option == iToggleLockReticle_OID
		SetInfoText("Press this key to lock / unlock the target reticle on the current target.")
	elseif option == iTogglePrimaryTargetMode_OID
		SetInfoText("Press this key to toggle the primary target between 'From Screen Center' and 'Current Combat Target'.")
	elseif option == iTargetReticleMode_OID
		SetInfoText("Defines Target Reticle visibility: Off - disabled; On - visible on selected actor and current combat target; Only Combat Target - visible only on current combat target. Requires TrueHUD.")
	elseif option == iReticleLockAnimationStyle_OID
		SetInfoText("Defines the animation style of the target reticle when locking on a target.")
	elseif option == iTDMLock_OID
		SetInfoText("Defines which target the True Directional Movement (TDM) Target Lock focuses on when activated.")
	elseif option == iPrimaryTargetMode_OID
		SetInfoText(" The primary target for the the dragon's next commanded attack. The other option is the fallback in case no primary target exists.")
	elseif option == iMaxTargetDistance_OID
		SetInfoText("The maximum distance for targeting. If set to 0, the search range is not limited.")
	elseif option == iDistanceMultiplierSmall_OID
		SetInfoText("The maximum distance will be multiplied with this value for small targets.")
	elseif option == iDistanceMultiplierLarge_OID
		SetInfoText("The maximum distance will be multiplied with this value for large targets.")
	elseif option == iDistanceMultiplierExtraLarge_OID
		SetInfoText("The maximum distance will be multiplied with this value for extra large targets.")
	elseif option == iTargetScanAngle_OID
		SetInfoText("The maximum scan angle for the 'From Screen Center' target search. If set to 0, the search is omnidirectional.")
	elseif option == iCameraLockEnabled_OID
		SetInfoText("If enabled, camera movements are used to control the dragon movements and the camera follows user's movement commands")
	elseif option == iToggleCameraLockKey_OID
		SetInfoText("Press this key to toggle camera lock on/off.")
	elseif option == iIgnoredCameraPitch_OID
		SetInfoText("Downward camera pitches until this angle will not trigger height changes of the flying dragon.")
	elseif option == iYawOffsetStrength_OID
		SetInfoText("Strength of the camera yaw offset during a dragon attack.")
	elseif option == iPitchOffsetStrength_OID
		SetInfoText("Strength of the camera pitch offset during a dragon attack.")
	else
		SetInfoText("")
		; no change
		;SetInfoText("No information available for this option.")
		;Debug.Notification("No information available for this option.")
	endIf
endFunction
 

function OnOptionKeyMapChange(Int option, Int keyCode, String conflictControl, String conflictName)

	if keyCode == 1
		return 
	endIf

	if option == iDisplayHealthKey_OID
		iDisplayHealthKey = keyCode
		SetKeyMapOptionValue(iDisplayHealthKey_OID, iDisplayHealthKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iDisplayHealthKey, "DisplayHealth")
	elseif option == iForward_OID
		iForwardKey = keyCode
		SetKeyMapOptionValue(iForward_OID, iForwardKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iForwardKey, "Forward")
	elseif option == iBack_OID
		iBackKey = keyCode
		SetKeyMapOptionValue(iBack_OID, iBackKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iBackKey, "Back")
	elseif option == iStrafeLeft_OID
		iStrafeLeftKey = keyCode
		SetKeyMapOptionValue(iStrafeLeft_OID, iStrafeLeftKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iStrafeLeftKey, "StrafeLeft")
	elseif option == iStrafeRight_OID
		iStrafeRightKey = keyCode
		SetKeyMapOptionValue(iStrafeRight_OID, iStrafeRightKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iStrafeRightKey, "StrafeRight")
	elseif option == iRun_OID
		iRunKey = keyCode
		SetKeyMapOptionValue(iRun_OID, iRunKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iRunKey, "Run")
	elseif option == iToggleAlwaysRun_OID
		iToggleAlwaysRunKey = keyCode
		SetKeyMapOptionValue(iToggleAlwaysRun_OID, iToggleAlwaysRunKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iToggleAlwaysRunKey, "ToggleAlwaysRun")
;	elseif option == iAttackRegular_OID
;		iAttackRegularKey = keyCode
;		SetKeyMapOptionValue(iAttackRegular_OID, iAttackRegularKey, false)
;		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iAttackRegularKey, "AttackRegular")
;	elseif option == iAttackAlternate_OID
;		iAttackAlternateKey = keyCode
;		SetKeyMapOptionValue(iAttackAlternate_OID, iAttackAlternateKey, false)
;		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iAttackAlternateKey, "AttackAlternate")
	elseif option == iDragonUp_OID
		iDragonUpKey = keyCode
		SetKeyMapOptionValue(iDragonUp_OID, iDragonUpKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iDragonUpKey, "DragonUp")
	elseif option == iDragonDown_OID
		iDragonDownKey = keyCode
		SetKeyMapOptionValue(iDragonDown_OID, iDragonDownKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iDragonDownKey, "DragonDown")
	elseif option == iAutoCombat_OID
		iAutoCombatKey = keyCode
		SetKeyMapOptionValue(iAutoCombat_OID, iAutoCombatKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iAutoCombatKey, "ToggleAutoCombat")
	elseif option == iToggleLockReticle_OID
		iToggleLockReticleKey = keyCode
		SetKeyMapOptionValue(iToggleLockReticle_OID, iToggleLockReticleKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iToggleLockReticleKey, "ToggleLockReticle")
	elseif option == iTogglePrimaryTargetMode_OID
		iTogglePrimaryTargetModeKey = keyCode
		SetKeyMapOptionValue(iTogglePrimaryTargetMode_OID, iTogglePrimaryTargetModeKey, false)
;		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iTogglePrimaryTargetModeKey, "TogglePrimaryTargetMode")
	elseif option == iToggleCameraLockKey_OID
		iToggleCameraLockKey = keyCode
		SetKeyMapOptionValue(iToggleCameraLockKey_OID, iToggleCameraLockKey, false)
		(DragonRideQuest as _ts_DR_RideControlScript).SetKey(iToggleCameraLockKey, "ToggleCameraLock")
	else
		return ; no change
	endif
endFunction

