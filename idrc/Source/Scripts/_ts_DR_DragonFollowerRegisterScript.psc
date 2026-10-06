Scriptname _ts_DR_DragonFollowerRegisterScript extends Quest  

import _ts_DR_Debug

Actor Property PlayerREF Auto

ReferenceAlias Property FollowerAlias Auto
ReferenceAlias Property dragonAlias Auto 

Scene Property DragonRideScene  Auto ; tame scene to hold dragon on ground

ObjectReference Property DLC2TameDragonOrbitMarker Auto ; default object that the orbit package uses. code also moves this to fast travel destination
ObjectReference Property DragonMovetoMarker Auto

FormList Property DLC2TameDragonAllowedWorldspaces Auto ; formlist of allowed worldspaces - dragon can only be tamed/ridden in these worldspaces

SPELL Property DLC2TameDragonNoFlyAbility  Auto  
SPELL Property DLC2abCalmDragon  Auto  

Faction Property DLC2TameDragonFaction Auto
Faction Property DismissedFollowerFaction Auto

Message Property  FollowerDismissMessage Auto

WorldSpace Property TamrielWorldSpace Auto
WorldSpace Property SolstheimWorldSpace Auto

Quest Property DragonRideQuest auto

bool Property bIsStandaloneVersion = false auto

_ts_DR_RideControlScript Property DR_Control Auto Hidden


Quest RideControlQuest
ReferenceAlias  DR_dragonAlias  

bool bRegisteredForControl = false


event OnInit()
;	DR_Control = DragonRideQuest as _ts_DR_RideControlScript
	CheckForIDRC()
endEvent

Function InitRideControlQuest()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - InitRideControlQuest")

	if bIsStandaloneVersion
		RideControlQuest = DragonRideQuest
	else
;		DR_Control = Game.GetFormFromFile(0x0004E986, "IntuitiveDragonRideControl.esp") as _ts_DR_RideControlScript

		; Could also use GetFormFromFile() - see above. But in the SE version the FormID is different because of of the ESL conversion.
		; So using GetQuest() instead. However, in case IDRC not fully loaded when calling GetQuest, GetQuest() will fail. 
		; This can happen during game load due to load sequence.
		; -> Only call InitRideControlQuest() when load game process is completed (ie not during OnInit, or OnPlayerLoadGame)

		RideControlQuest = Quest.GetQuest("_ts_DR_DragonRideQuest")
	endif
	
	if !RideControlQuest
		debug.MessageBox("Could not load RideControl Quest. Dragon riding will not work.")
		return
	endif
	
	DR_Control = RideControlQuest as _ts_DR_RideControlScript
	DR_dragonAlias = DR_Control.GetAliasByName("DragonRide") as ReferenceAlias
endFunction

bool Function CheckForIDRC()
	if !bIsStandaloneVersion
		int iIDRCIndex = Game.GetModByName("IntuitiveDragonRideControl.esp")
		if iIDRCIndex == 255
			debug.MessageBox("'Intuitive Dragon Ride Control' is not in your load order. Dragon riding will not work.")
			return false
		elseif iIDRCIndex == 0
			debug.MessageBox("SKSE is not loaded. Dragon riding will not work.")
			return false
		endif
	endif
	return true
endFunction


Function InitFollower(Actor FollowerRef) 
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - InitFollower")
	FollowerAlias.ForceRefTo(FollowerRef)
	
	FollowerAlias.GetActorRef().SetRelationshipRank(PlayerREF, 3)
	FollowerAlias.GetActorRef().AddToFaction(DismissedFollowerFaction)
	FollowerAlias.GetActorRef().SetPlayerTeammate(false)
	FollowerAlias.GetActorRef().SetActorValue("WaitingForPlayer", 0)
	FollowerAlias.GetActorRef().setActorValue("Variable03", 0) ; Variable03  (0: Orbiting, 1: Exploring, 2: come to player, 3: Hover).
	FollowerAlias.GetActorRef().setAllowFlying(true)

	FollowerAlias.GetActorRef().EvaluatePackage()
		
	Setstage (120) ; set stage to dismissed 
endFunction

function DragonRegisterForAnimations()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - DragonRegisterForAnimations")

	RegisterForAnimationEvent(PlayerREF, "MountEnd")
	RegisterForAnimationEvent(PlayerREF, "DragonMountExitOut")
	RegisterForAnimationEvent(PlayerREF, "GetUpEnd")
endFunction

function DragonUnregisterForAnimations()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - DragonUnregisterForAnimations")
	UnregisterForAnimationEvent(PlayerREF, "MountEnd")
	UnregisterForAnimationEvent(PlayerREF, "DragonMountExitOut")
	UnregisterForAnimationEvent(PlayerREF, "GetUpEnd")
endFunction

event OnAnimationEvent(ObjectReference akSource, String asEventName)
; check if the player mounting or unmounting another dragon (ie not the dragon follower)
; if the player is mounting another dragon, change Dragon follower's package to FollowDragonOrbit
; if the player is unmounting another dragon, change Dragon follower's package to FollowPlayerOrbit

	Actor dragonFollowerActor = FollowerAlias.GetActorRef()
	Actor dragonActor = dragonAlias.GetActorRef()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - OnAnimationEvent")

	
	if dragonFollowerActor && akSource == PlayerREF
		if asEventName == "MountEnd" 
			RegisterForAnimationEvent(PlayerREF, "FootLeft")

_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - OnAnimationEvent: Mounting a dragon!")
			if !dragonActor ; the player is mounting another dragon
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - OnAnimationEvent: Mounting another dragon!")
				dragonFollowerActor.setActorValue("Variable03", 5) ; change Dragon follower's package to FollowDragonOrbit
				dragonFollowerActor.EvaluatePackage()
			endif
		elseif asEventName == "DragonMountExitOut" || asEventName == "GetUpEnd" || asEventName == "FootLeft" ; unmounting a dragon
			UnregisterForAnimationEvent(PlayerREF, "FootLeft")

			if !dragonActor ; the player is unmounting another dragon
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - OnAnimationEvent: unmounting another dragon!")
				dragonFollowerActor.setActorValue("Variable03", 0) ; change Dragon follower's package to FollowPlayerOrbit
				dragonFollowerActor.EvaluatePackage()
			else
				utility.Wait(3)
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - OnAnimationEvent: unmounting follower dragon!")
				if dragonActor.GetActorValue("variable01") == 99
					TryToReleaseDragon(dragonActor)
				else
					ReleaseDragon()
				endif
			endif
		endif
	endif
endEvent


Function SetFollower(ObjectReference FollowerRef)
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - SetFollower")

    actor FollowerActor = FollowerRef as Actor

	if !FollowerActor || FollowerActor.IsDead()
		return
	endif
	if !IsFollower(FollowerActor)
			
		if getstage () != 110
			if getstage () == 120
				Reset() ; in case stage is on dismissed (120)
			endif	
			Setstage (110) ; set stage to follower 
		endif

		FollowerActor.RemoveFromFaction(DismissedFollowerFaction)
			
		If FollowerActor.GetRelationshipRank(PlayerREF) <3 && FollowerActor.GetRelationshipRank(PlayerREF) >= 0
			FollowerActor.SetRelationshipRank(PlayerREF, 3)
		EndIf
		FollowerActor.SetPlayerTeammate()
		;FollowerActor.SetActorValue("Morality", 0)
		FollowerActor.setActorValue("Variable03", 0) ; Variable03  (0: Orbiting, 1: Exploring, 2: come to player, 3: Hover).
		FollowerActor.SetActorValue("WaitingForPlayer", 0)
		FollowerActor.setAllowFlying(true)
		 
		DragonRegisterForAnimations()
		InitRideControlQuest()
		DR_Control.bIsFollowerActive = true
	;	debug.Notification("TEST: Is Alias = Actor? - " + (FollowerAlias.GetActorRef() as Actor) != FollowerActor )
		if !FollowerAlias || (FollowerAlias.GetActorRef() as Actor) != FollowerActor
;			debug.Notification("Forcing Ref to Alias! FollowerAlias = " + FollowerAlias)
			FollowerAlias.ForceRefTo(FollowerActor)
		endif
		
	;	FollowerActor.EvaluatePackage()
	endif
	
EndFunction


bool Function IsFollower(ObjectReference FollowerRef)
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - IsFollower")

	actor FollowerActor = FollowerRef as Actor
	bool bIsFollower = true

	if !FollowerActor || FollowerActor.IsDead()|| FollowerActor.IsInFaction(DismissedFollowerFaction) || !FollowerActor.IsPlayerTeammate() || FollowerActor.GetRelationshipRank(PlayerREF) < 3
		bIsFollower = false
	endif

	return bIsFollower
	
EndFunction


function GetValues (string sCallingFunction)

; This function is for debugging purposes only.

	actor FollowerActor = FollowerAlias.GetActorRef() as Actor
	debug.Notification(sCallingFunction + ":	Stage: " + Getstage() + "		Wait: " + FollowerActor.GetActorValue("waitingforplayer") + "		Dismissed: " + FollowerActor.IsInFaction(DismissedFollowerFaction))
	debug.Notification(sCallingFunction + ":	Var01: " + FollowerActor.GetActorValue("variable01") + "		Var03: " + FollowerActor.GetActorValue("variable03") + "		AllowedtoFly: " + FollowerActor.IsAllowedToFly())
endfunction



function RideDragon()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - RideDragon")
	if IsFollower(FollowerAlias.GetActorRef())
		actor FollowerActor = FollowerAlias.GetActorRef() as Actor
		
		FollowerActor.SetActorValue("WaitingForPlayer", 0)
		FollowerActor.setActorValue("Variable03", 3) ; Variable03  (0: Orbiting, 1: Exploring, 2: come to player, 3: Hover).
		TameDragon(FollowerAlias.GetActorRef() as Actor)	
	endif
endfunction
 
 
Function FollowerWait()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - FollowerWait")

	If  IsFollower(FollowerAlias.GetActorRef())
	
		actor FollowerActor = FollowerAlias.GetActorRef() as Actor

		DragonMovetoMarker.MoveTo(FollowerActor) 
		FollowerActor.SetAllowFlyingEx(abAllowed = false, abAllowCrash = false, abAllowSearch = true)

		FollowerActor.SetActorValue("WaitingForPlayer", 1)
		FollowerActor.setActorValue("Variable03", 0) ; Variable03  (0: Orbiting, 1: Exploring, 2: come to player, 3: Hover).
;		utility.wait(20)
		FollowerActor.SetAllowFlying(true)
	endif
	
EndFunction
 

Function FollowerFollow()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - FollowerFollow")

	If FollowerAlias && !FollowerAlias.GetActorReference().IsDead()

		actor FollowerActor = FollowerAlias.GetActorRef() as Actor
		bool bWasFollower = true
		

		if PlayerRef.GetWorldSpace() != FollowerActor.GetWorldSpace()
;			debug.Notification("Different Worldspaces!")
			if PlayerRef.GetWorldSpace() == SolstheimWorldSpace
				FollowerActor.MoveTo(PlayerRef, 0, 0, 5000)
				FollowerActor.SetPosition(0, 0, 1000) ; southwest corner of Solstheim
				FollowerActor.EvaluatePackage()
			elseif PlayerRef.GetWorldSpace() == TamrielWorldSpace
				FollowerActor.MoveTo(PlayerRef, 0, 0, 5000)		
				FollowerActor.SetPosition(220000, 150000, -13000) ; Northeastern part of Skyrim
				FollowerActor.EvaluatePackage()
			endif
		endif
	
;		GetValues ("FF-Start")
		
		; make sure dragon is a follower
		if !IsFollower(FollowerActor)
			SetFollower(FollowerActor)
			bWasFollower = false
		endif

		FollowerActor.setAllowFlying(true)
		FollowerActor.SetActorValue("WaitingForPlayer", 0)
		FollowerActor.setActorValue("Variable03", 2) ; Variable03  (0: Orbiting, 1: Exploring, 2: come to player, 3: Hover).

		FollowerActor.EvaluatePackage()

		while PlayerREF.GetDistance(FollowerActor) > 3200
			; wait for dragon to be close enough
		endwhile

		; Make the dragon land near the player
		FollowerActor.SetAllowFlyingEx(abAllowed = false, abAllowCrash = false, abAllowSearch = true)
		FollowerActor.AddSpell(DLC2abCalmDragon)
	
		utility.wait(20);  wait 20 secs on ground for potential player interactions.

		FollowerActor.SetAllowFlying(true)
		FollowerActor.RemoveSpell(DLC2abCalmDragon)	

		; switch value to "orbit Player" package
		FollowerActor.setActorValue("Variable03", 0) ; Variable03  (0: Orbiting, 1: Exploring, 2: come to player, 3: Hover).
		FollowerActor.EvaluatePackage()

;		GetValues ("FF-End")
	endif
	
EndFunction
 
Function FollowerExplore ()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - FollowerExplore")

	actor FollowerActor = FollowerAlias.GetActorRef() as Actor

	If FollowerActor && FollowerActor.IsDead() == False && IsFollower(FollowerActor)
		FollowerActor.SetActorValue("WaitingForPlayer", 0)
		FollowerActor.setAllowFlying(true)
		FollowerActor.setActorValue("Variable03", 1) ; Variable03  (0: Orbiting, 1: Exploring, 2: come to player, 3: Hover).
	endif

endfunction


Function DismissFollower(Int iSayLine = 1)
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - DismissFollower")

	If FollowerAlias && FollowerAlias.GetActorReference().IsDead() == False && IsFollower(FollowerAlias.GetActorRef())
		FollowerDismissMessage.Show()

		actor DismissedFollowerActor = FollowerAlias.GetActorRef() as Actor
		
		DismissedFollowerActor.StopCombatAlarm()
		DismissedFollowerActor.AddToFaction(DismissedFollowerFaction)
		DismissedFollowerActor.SetPlayerTeammate(false)

		DismissedFollowerActor.SetActorValue("WaitingForPlayer", 0)
		DismissedFollowerActor.setActorValue("Variable03", 0) ; Variable03  (0: Orbiting, 1: Exploring, 2: come to player, 3: Hover).

		DragonUnregisterForAnimations()
		
		Setstage (120) ; set stage to dismissed

		DR_Control.bIsFollowerActive = false
	endif
	
EndFunction


;--------------------------------------------
; TSC: START TAME DRAGON CODE HERE, uses DL2TameDragon as reference (mostly copied from corresponding DL2TameDragonQuest and scripts)
;--------------------------------------------

bool bAllowRestrain = false 	; this gets set to true at the start of Tame sequence, false during EndWait to prevent accidentally rerestraining him


function TameDragon(actor newDragon)	
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - TameDragon")
	if !CheckForIDRC()
		return
	endif

	InitRideControlQuest()

	if DragonAlias.GetRef() && !dragonAlias.GetActorRef().IsDead()
		; Dragon is already tamed, tell him to land
		LandDragon()
		; no matter what - don't do anything else
		return
	endif

	; if dragon is not in an interior, then check if it's in an allowed worldspace - if not, do nothing
	if !newDragon.IsInInterior() && !DLC2TameDragonAllowedWorldspaces.HasForm(newDragon.GetWorldSpace())
; 		debug.trace(self + " TameDragon: dragon is in invalid worldspace + " + newDragon.GetWorldSpace() + ", do nothing")
		return
	endif
	; Dragon has not been tamed yet. Add him to Dragon Quest alias, and make him land.
		
	; reset allow restrain flag
	bAllowRestrain = true ;TSC: Is this needed?
	; put dragon in alias
	DragonAlias.ForceRefTo(newDragon)
	DR_DragonAlias.ForceRefTo(newDragon)
	; stop combat
	newDragon.StopCombatAlarm()

	LandDragon()
	
endFunction

; call this function on an already tamed dragon, to get the dragon to briefly land next to the player
function LandDragon()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - LandDragon: dragon " + dragonAlias.GetRef() + " landing again")
	Actor dragonActor = dragonAlias.GetActorRef()
	; reset actor value
	dragonActor.SetActorValue("Variable01", 0)	

	; move orbit marker to player's position
	DLC2TameDragonOrbitMarker.Moveto(PlayerRef, 120.0 * Math.Sin(PlayerRef.GetAngleZ()), 120.0 * Math.Cos(PlayerRef.GetAngleZ()), PlayerRef.GetHeight())

	; always hold him on the ground to start
	; give "no fly" ability so it doesn't interfere with scripted "no fly"
	dragonActor.AddSpell(DLC2TameDragonNoFlyAbility)

	RegisterForSingleUpdateGameTime(0.1) ; first check is still riding after 0.1 game hours
	iWaitCount = 0 ; iWaitCount is tracking the number of times the OnUpdateGameTime function is called.
	
	dragonActor.AllowPCDialogue(false)
	; start scene
	DragonRideScene.Start()
endFunction


; check if dragon is in valid worldspace - if not, release immediately
function ValidateWorldspace()
; 	debug.trace(self + " ValidateWorldspace")
	if DLC2TameDragonAllowedWorldspaces.HasForm(Game.GetPlayer().GetWorldSpace()) == false
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - ValidateWorldspace: " + self + " player in invalid worldspace " + Game.GetPlayer().GetWorldSpace() + " - release dragon")
		EndWait()
		ReleaseDragon()
	endif

endFunction

; call this when player tries to ride him
function EndWait()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - EndWait")
	if bRegisteredForControl
		return
	endif
	
	Actor dragonActor = dragonAlias.GetActorRef()
	; if he's not in a "waiting" state, do nothing
	if dragonActor.HasSpell(DLC2TameDragonNoFlyAbility) == false
; 		debug.trace(self + 	 " EndWait: not in waiting state, do nothing")
		return
	endif
	; can't restrain him again
	bAllowRestrain = false
	; unrestrain him
	RestrainDragon(false)
	; set crime faction
	dragonActor.SetCrimeFaction(DLC2TameDragonFaction)

	; let him fly if allowed
	dragonActor.RemoveSpell(DLC2TameDragonNoFlyAbility)
	dragonActor.RemoveSpell(DLC2abCalmDragon)

	; safety check: if dragon is not in aliases, must have been released (probably by player aggroing it)
	if dragonAlias.GetActorRef() != dragonActor
; 		debug.trace(" EndWait - dragonAlias doesn't match, do nothing else")
		return
	endif

	; set "done waiting" variable
	if dragonActor.GetActorValue("variable01") == 0
		dragonActor.SetActorValue("variable01", 1)
	endif
	
	dragonActor.EvaluatePackage()
	; register for short duration
	RegisterForSingleUpdateGameTime(0.1)
	if dragonActor.IsBeingRidden() 
		dragonActor.AddSpell(DLC2TameDragonNoFlyAbility)
		dragonActor.SetAllowFlying(false)		
		DR_Control.SetFlyingState(3) ; landed

		DR_Control.DragonRegisterForControls() ; Enable control keys for dragon riding
		bRegisteredForControl = true
	endif

	; try adding the calm again in case it didn't work the first time
; 	debug.trace(self + " adding DLC2abCalmDragon to dragon")
	dragonActor.AddSpell(DLC2abCalmDragon)
	utility.wait(2)
	dragonActor.RemoveSpell(DLC2abCalmDragon)

endFunction



; call this when the player has unmounted the dragon, or dragon dies
function ReleaseDragon()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - ReleaseDragon")
	; set actor value to indicate he wants to be released when possible
	Actor dragonActor = dragonAlias.GetActorRef()
	; don't do this more than once to the same dragon
	if dragonActor.GetActorValue("variable01") != 99
		dragonActor.SetActorValue("variable01", 99)
		; try to dispel now or later
		TryToReleaseDragon(dragonActor)
	endif
endFunction


int iWaitCount = 0

; TSC: This function is different from the DLC2 (Dragonborn) version. It releases the dragon after player has unmounted.
event OnUpdateGameTime()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - OnUpdateGameTime")
		
	; is dragon waiting to be released?
	Actor dragonActor = dragonAlias.GetActorRef()

	if !dragonActor
		; Dragon follower already released - do not release again (avoid conflict  in case player already is riding a IDRC dragon)
		return	
	endif

	iWaitCount += 1
;	debug.Notification("UGT:	IsBeingRidden = " + dragonActor.IsBeingRidden() + "		Var01 = " + dragonActor.GetActorValue("variable01"))
	
	; is dragon not mounted yet? Release dragon if waiting for too long (checked by iWaitCount)
	if dragonActor.GetActorValue("variable01") == 0 && iWaitCount < 3 
		RegisterForSingleUpdateGameTime(0.1) ; check again later
		return
	endif
	
	;Player has unmounted the dragon, release him.
	if dragonActor.GetActorValue("variable01") == 99
		TryToReleaseDragon(dragonActor)
	else
		; time to dispel
		ReleaseDragon()
	endif
endEvent

function TryToReleaseDragon(actor dragonActor)
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - TryToReleaseDragon")
	; if player is not riding, release
	if dragonActor.IsBeingRidden() == 0
		; RELEASE:
		dragonActor.AllowPCDialogue(true)
		dragonActor.SetRestrained(false)	 ; just in case
		dragonActor.SetAllowFlying(true)
		dragonActor.ClearLookAt()
		dragonActor.SetActorValue("variable01", 0)
		dragonActor.RemoveSpell(DLC2TameDragonNoFlyAbility)

		DR_Control.DragonUnregisterForControls()
		bRegisteredForControl = false
	
		dragonAlias.Clear()
		DR_dragonAlias.Clear()
;/
; Likely not really needed
; But in case it needs to be re-activated: This resets all MCM settings to default in the quest. 
; They still show up as the user defined them in MCM, which is inconsistent to the values in the quest.
		RideControlQuest.Stop()
		
		while (RideControlQuest.IsStopping())
			utility.Wait(0.1)
		endwhile
		
		RideControlQuest.Reset()
		RideControlQuest.Start()
/;

		if dragonActor.IsHostileToActor(Game.GetPlayer())
			dragonActor.SetPlayerTeammate(false)
		endif
		
		dragonActor.SetCrimeFaction(none)
		dragonActor.EvaluatePackage()
	else
		; player is riding - wait until landed
		RegisterForSingleUpdateGameTime(0.1)
	endif
endFunction



function DragonDead()
	; called by OnDeath block on dragon
	; stop pending updates if any
	UnregisterForUpdateGameTime()
endFunction


function RestrainDragon(bool bRestrain)
	; this is to prevent him from responding to attacks when on the ground
	; only do this when not flying!
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonFollowerRegisterScript - RestrainDragon")

	actor dragonActor = dragonAlias.GetActorRef()
	if bRestrain && dragonActor.GetActorValue("variable01") != 99
		while dragonActor.GetFlyingState() > 0 && bAllowRestrain
			utility.wait(1.0)
		endwhile
; 		debug.trace(self + " ready to restrain dragon")
		; only restrain if the allow flag is still true
		if bAllowRestrain
; 			debug.trace(self + " restraining dragon")
			dragonActor.SetRestrained(true)
			; stop combat/alarms again to be sure
			dragonActor.StopCombatAlarm()
			; add calm ability - has to be AFTER restraining him
; 			debug.trace(self + " adding DLC2abCalmDragon to dragon")
			dragonActor.AddSpell(DLC2abCalmDragon)
		endif
		; TRY MAKING TEAMMATE here - do it even if restrain can't happen
		dragonActor.SetPlayerTeammate(true, false)
	else
; 		debug.trace(self + " UN-Restraining dragon")
		dragonActor.SetRestrained(false)
	endif
endFunction
