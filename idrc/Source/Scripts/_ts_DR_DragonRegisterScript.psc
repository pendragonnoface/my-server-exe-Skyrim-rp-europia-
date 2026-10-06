Scriptname _ts_DR_DragonRegisterScript extends Quest conditional

import _ts_DR_Debug

ReferenceAlias Property PlayerAlias  Auto

ReferenceAlias Property dragonAlias  Auto

_ts_DR_RideControlScript Property DR_Control Auto Hidden

ReferenceAlias property DLC2Dragon Auto
WorldSpace Property DLC2ApocryphaWorld Auto

Spell Property DLC2TameDragonNoFlyAbility  Auto

bool property bIsMounted = false auto hidden conditional

bool bUseAnimationEvents = false


event OnInit()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonRegisterScript - OnInit: IsUsingGamepad = " + game.UsingGamepad())

	if !PlayerAlias
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonRegisterScript - OnInit: Error - PlayerAlias not filled!")
		debug.MessageBox("Intuitive Dragon Ride Control: Unexpected Error - PlayerAlias not filled. Dragon ride control non-functional.")
	endif

	RegisterForAnimationEvent(PlayerAlias.GetActorRef(), "DragonMountEnter")
	RegisterForAnimationEvent(PlayerAlias.GetActorRef(), "DragonMountEnterInstant")
	RegisterForAnimationEvent(PlayerAlias.GetActorRef(), "MountEnd")
	RegisterForAnimationEvent(PlayerAlias.GetActorRef(), "DragonMountExitOut")
	RegisterForAnimationEvent(PlayerAlias.GetActorRef(), "GetUpEnd")
		
;	RegisterForAnimationEvent(PlayerAlias.GetActorRef(),"pa_Ground_Mount_Dragon")
	DR_Control = (self as quest) as _ts_DR_RideControlScript
	RegisterForSingleUpdate(2.0)
endEvent


event OnAnimationEvent(ObjectReference akSource, String asEventName)
; check if the player mounting or unmounting
; if the player is mounting a tamed dragon, initialize ride controls
; if the player is unmounting a tamed dragon, clear the ride controls
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonRegisterScript - OnAnimationEvent: IsUsingGamepad = " + game.UsingGamepad())

	if akSource == PlayerAlias.GetActorRef() && PlayerAlias.GetActorRef().GetWorldSpace() != DLC2ApocryphaWorld
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonRegisterScript - OnAnimationEvent: EventName = " + asEventName)
		if asEventName == "MountEnd" || asEventName == "DragonMountEnter" || asEventName == "DragonMountEnterInstant"
			InitializeRideControl()
			 ; in case the dragon mounting is caught by OnAnimationEvent(), no need to regularly check for the mounting via OnUpdate()
			 ; instead, rely on OnAnimationEvent() to detect the mounting
			bUseAnimationEvents = true

		elseif asEventName == "DragonMountExitOut" || asEventName == "GetUpEnd"
			ClearRideControl()
		elseif asEventName == "FootLeft"
			ClearRideControl()
			UnregisterForAnimationEvent(PlayerAlias.GetActorRef(), "FootLeft")
		endif
	endif
endEvent


Event OnUpdate()

	if !bUseAnimationEvents 
		if utility.GetIniBool("bEnableLogging:Papyrus") && utility.GetIniBool("bEnableTrace:Papyrus")
			bool bPlayerMounted = false
			if DLC2Dragon.GetActorReference()
				bPlayerMounted = DLC2Dragon.GetActorReference().IsBeingRidden()
			endIf
;debug.trace("IDRC.DragonRegister-OnUpdate: " + DLC2Dragon.GetActorReference() + ", " + bIsMounted + ", " + PlayerAlias.GetActorRef().IsOnMount() + "," + bPlayerMounted)
		endIf
	
		if DLC2Dragon.GetActorReference() && !bIsMounted  && PlayerAlias.GetActorRef().GetWorldSpace() != DLC2ApocryphaWorld
			if DLC2Dragon.GetActorRef().IsBeingRidden() && PlayerAlias.GetActorRef().IsOnMount()
				InitializeRideControl()
			endIf
		elseif bIsMounted || dragonAlias.GetActorReference()
			if !dragonAlias.GetActorReference().IsBeingRidden() || !PlayerAlias.GetActorRef().IsOnMount()
				ClearRideControl()
			endIf
		endIf

		; Check again in 2secs for the mounting/unmounting, but only if not yet clear if OnAnimationEvent() can detect it as well
		; This is relevant for VR: In the VR version I did not find out if there's an animation event which is triggered when mounting a dragon
		; So in VR the mounting currently can only be detected via OnUpdate()
		; but for the other versions, OnAnimationEvent() works fine, and will be used instead of OnUpdate() because this is the less expensive method
		RegisterForSingleUpdate(2.0)
	endIf
	
endEvent

function InitializeRideControl()
	bool bDragonIsBeingRidden = false
	
	if DLC2Dragon.GetActorReference()
		bDragonIsBeingRidden = DLC2Dragon.GetActorReference().IsBeingRidden()
	endif
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonRegisterScript - InitializeRideControl: " + bIsMounted + ", " + PlayerAlias.GetActorRef().IsOnMount() + "," + bDragonIsBeingRidden)
		
	if !bIsMounted && PlayerAlias.GetActorRef().IsOnMount() && bDragonIsBeingRidden && PlayerAlias.GetActorRef().GetWorldSpace() != DLC2ApocryphaWorld
;		debug.Notification("InitializeRideControl...")

		bIsMounted = true

		dragonAlias.ForceRefTo(DLC2Dragon.GetRef())
	
		dragonAlias.GetActorReference().AddSpell(DLC2TameDragonNoFlyAbility)
		dragonAlias.GetActorReference().SetAllowFlying(false)		
		DR_Control.SetFlyingState(3) ; landed
		DR_Control.DragonRegisterForControls() ; Enable control keys for dragon riding
		RegisterForAnimationEvent(PlayerAlias.GetActorRef(), "FootLeft")
	endif
endfunction


function ClearRideControl()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonRegisterScript - ClearRideControl")
	if bIsMounted && dragonAlias
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonRegisterScript - ClearRideControl: Cleaning up!")
		bIsMounted = false
		
		if dragonAlias.GetActorReference().HasSpell(DLC2TameDragonNoFlyAbility)
			dragonAlias.GetActorReference().RemoveSpell(DLC2TameDragonNoFlyAbility)
		endIf

;		DragonSprint(false)
		DR_Control.DragonUnregisterForControls()

		dragonAlias.GetActorReference().SetAllowFlying(true)

		dragonAlias.GetActorReference().ClearLookAt()

		dragonAlias.Clear()
	endIf
endFunction

