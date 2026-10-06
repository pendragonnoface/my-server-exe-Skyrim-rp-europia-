Scriptname _ts_DR_DragonTameAliasScript extends ReferenceAlias  

import _ts_DR_Debug

DLC2TameDragonScript Property TameDragonQuest  Auto  

ObjectReference Property DLC2TameDragonOrbitMarker Auto

int iGroundedCount = 0
bool bDragonDeadMessage = true
bool bLowHealthMessage = true
actor NextTarget = None

Event OnActivate(ObjectReference akActionRef)
	if akActionRef == Game.GetPlayer() && TameDragonQuest.dragonAlias.GetActorRef()
		TameDragonQuest.EndWait()
	endif
endEvent



Event OnCellDetach()
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonTameAliasScript - OnCellDetach")
endEvent



Event OnCombatStateChanged(Actor akTarget, int aeCombatState)
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonTameAliasScript - OnCombatStateChanged: " + aeCombatState + ", " + akTarget)
	
	(GetOwningQuest() as _ts_DR_RideControlScript).ToggleRegenerateHealth()
endEvent


Event OnHit(ObjectReference akAggressor, Form akSource, Projectile akProjectile, bool abPowerAttack, bool abSneakAttack, \
  bool abBashAttack, bool abHitBlocked)
	actor selfActor = GetActorRef()

	float fHealth = selfActor.GetActorValuePercentage("Health")
	if fHealth <= 0.00 && bDragonDeadMessage
		debug.Notification("Your dragon is dying...")
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonTameAliasScript - OnHit: Your dragon is dying...")
		bDragonDeadMessage = false
		(GetOwningQuest() as _ts_DR_RideControlScript).DragonLandPlayerRiding(selfActor)
	else
		if fHealth <= 0.35
			if bLowHealthMessage
				bLowHealthMessage = false
				debug.Notification("Your dragon's health gets low...")
_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonTameAliasScript - OnHit: Your dragon's health gets low...")
				(GetOwningQuest() as _ts_DR_RideControlScript).DragonLandPlayerRiding(selfActor)
			endif
		else
			bLowHealthMessage = true
		endif
	endif

_ts_Debug_Trace_Message("IDRC - _ts_DR_DragonTameAliasScript - OnHit: Targeting: " + GetActorRef().GetCombatTarget())
endEvent


