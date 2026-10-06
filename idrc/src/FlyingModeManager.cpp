#include "FlyingModeManager.h"
#include "CombatManager.h"
#include "DataManager.h"
#include "DisplayManager.h"
#include "FastTravelManager.h"
#include "IDRCUtils.h"
#include "APIManager.h"

#include "RE/Skyrim.h"
#include "SKSE/API.h"
#include "CLIBUtil/EditorID.hpp"
#include "Hooks.h"

namespace IDRC {

    void FlyingModeManager::InitializeData(RE::TESObjectREFR* a_dragonTurnMarker, 
                                    RE::TESObjectREFR* a_dragonTravelToMarker,
                                    RE::TESObjectREFR* a_flyToTargetMarker,
                                    RE::SpellItem* a_noFlyAbility) {
        log::info("IDRC - {}", __FUNCTION__);
        m_dragonTurnMarker = a_dragonTurnMarker;
        m_dragonTravelToMarker = a_dragonTravelToMarker;
        m_flyToTargetMarker = a_flyToTargetMarker;
        m_noFlyAbility = a_noFlyAbility;

        m_yawOffset = 0.0;
        m_targetPitch = 0.0;
        m_turnSpeed = 25.0;
        m_deltaPitch = 20.0f * PI / 180.0f;
        m_toggleAlwaysRun = true;
        m_registeredForLanding = false;
        m_registeredForPerch = false;
        m_toggledAutoCombatLand = false;
        m_vanillaAttack = false;
        m_mode = kLanded;
    }

    void FlyingModeManager::Update() {
        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return;
        }
                
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (dragonActor) {

            // Border region check
            if (!IsInBorderRegion()) {
                DisplayManager::GetSingleton().DisplayLeavingBorderRegion();
                ForceHover();
            }
/*
if (this->m_noFlyAbility) {
bool hasNoFlyAbility = dragonActor->HasSpell(this->m_noFlyAbility);
log::info("{}: Dragon has NoFlyAbility = {}", __FUNCTION__, hasNoFlyAbility ? "true" : "false");
} else {
log::warn("{}: NoFlyAbility is null", __FUNCTION__);
}
log::info("{}: FFlyingMode = {}", __FUNCTION__, m_mode);
*/
            // process ongoing landing
            if (GetRegisteredForLanding()) {
                if (!m_finalizeTriggerLand && !m_landingPosSearchOngoing) {
                    auto* currentPackage = dragonActor->GetCurrentPackage();
                    if (currentPackage && currentPackage->procedureType != RE::PACKAGE_PROCEDURE_TYPE::kLanding) {
                        log::info("{}: Dragon dropped out of landing package during landing - re-trigger land", __FUNCTION__);
                        // Dragon dropped out of landing package during landing.
                        // This can happen eg during ongoing combat,
                        // when dragon sometimes changes to kSpectator package / kObserveCombat procedure.
                        // Re-Trigger landing to switch back to Landing package
                        TriggerLand();
                    } 
                } else {
                    FinalizeTriggerLand();
                }

                FinalizeLand();
            }

            // process direction and height changes
            TriggerTurn();

            if (_ts_SKSEFunctions::GetFlyingState(dragonActor) == 2) {
                ChangeDragonHeight();
            }

            // handle map-triggered fasttravel
            if (m_mode != kFlying && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 2 &&
                (Utils::IsFastTravelActive())) {
                // Can happen eg when player triggers fast travel via map.
                // Map-based fasttravel currently not supported, because the dynamic
                // repathing in PathingHook::UpdateFlightPathData will override any 
                // player-triggered fasttravel request.
                // Switching back to hover to ensure consistent flying state. 
                DragonHoverPlayerRiding(dragonActor);
            }

            // process flying mode transitions
            CheckModeTransition();

            // handle auto-combat during flying
            auto& combatManager = CombatManager::GetSingleton();
            auto* storedCombatTarget = combatManager.GetStoredCombatTarget();
            if ( DataManager::GetSingleton().GetAutoCombat() && 
                 storedCombatTarget &&
                 (Utils::IsFastTravelActive()) &&
                 combatManager.IsFastTravelAttack()
                ) {
                DragonHoverPlayerRiding(storedCombatTarget);
                combatManager.SetFastTravelAttack(false);
            }
        }
    }

    void FlyingModeManager::CheckModeTransition() {
        if (!m_isModeTransitioning) {
            return;
        }

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: Dragon actor is null", __FUNCTION__);
            return;
        }

        int dragonFlyingState = _ts_SKSEFunctions::GetFlyingState(dragonActor);

        // Check flying state transitions
        if ((m_mode == FlyingMode::kFlying && dragonFlyingState == 2) || 
            (m_mode == FlyingMode::kHovering && dragonFlyingState == 3) ||
            (m_mode == FlyingMode::kLanded && dragonFlyingState == 0) ||
            (m_mode == FlyingMode::kPerching && dragonFlyingState == 5)) {

            auto* turnMarker = FlyingModeManager::GetSingleton().GetDragonTurnMarker();

            if (m_mode == FlyingMode::kHovering) {
/* TODO: This is a leftover from the DisplayManager::UpdateDisplay() function, which was removed.
         This fired ONCE when the transition to Hovering was completed.
         TBD if this is still needed, or if it can be removed.
*/

                // workaround to ensure that the dragon does not start turning when starting to hover
                // this frequently happens when using the thumbstick control to
                // trigger hover mode from flying - reason unknown.
                // turnMarker has been put ahead of the OrbitMarker position when hovering is triggered
                // (in FlyingModeManager::DragonHoverPlayerRiding() )
                SKSE::GetTaskInterface()->AddTask([turnMarker, dragonActor]() {
                    // When modifying Game objects, send task to TaskInterface to ensure thread safety
                    if (turnMarker) {
                        _ts_SKSEFunctions::SetLookAt(dragonActor, turnMarker, true);
                        dragonActor->EvaluatePackage();
                    }
                });
            }
                
            m_isModeTransitioning = false;

            ControlsManager::GetSingleton().SetControlBlocked(false);

            if (m_mode == FlyingMode::kPerching && dragonFlyingState == 5) {
                FlyingModeManager::GetSingleton().SetRegisteredForPerch(false);
            }
        }
    }
    
    void FlyingModeManager::ChangeDragonHeight() {
        auto& controlsManager =ControlsManager::GetSingleton();
        if (controlsManager.GetIsKeyPressed(IDRCKey::kUp)) {
            m_targetPitch = GetRunFactor() * m_deltaPitch;
        } else if (controlsManager.GetIsKeyPressed(IDRCKey::kDown)) {
            m_targetPitch = -GetRunFactor() * m_deltaPitch;
        } else {
            m_targetPitch = 0.0f;
        }
    }

    RE::TESObjectREFR* FlyingModeManager::GetDragonTurnMarker() {
        return m_dragonTurnMarker;
    }

    FlyingMode FlyingModeManager::GetFlyingMode() {
        return m_mode;
    }

    void FlyingModeManager::SetFlyingModeFromPapyrus(int a_flyingState) {
        m_mode = static_cast<FlyingMode>(a_flyingState);
    }

    void FlyingModeManager::SetFlyingMode(FlyingMode a_mode) {

        if (m_mode == a_mode) {
            return;
        }

        m_isModeTransitioning = true;
        m_mode = a_mode;

        DataManager::GetSingleton().SendPropertyUpdateEvent("FlyingState", false, 0.0f, static_cast<int>(m_mode));
    }

    float FlyingModeManager::GetRunFactor(float a_modifier) {
        bool run = false;

        // Check if the run key is pressed
        if (ControlsManager::GetSingleton().GetIsKeyPressed(IDRCKey::kRun)) {
            run = true;
        }

        // Toggle the run state if "Always Run" is enabled
        if (m_toggleAlwaysRun) {
            run = !run;
        }

        // Calculate the run factor
        float runFactor = run ? (2.0f * a_modifier) : a_modifier;

        return runFactor;
    }

    void FlyingModeManager::UpdateFlyingMode() {
        log::info("IDRC - {}", __FUNCTION__);
    
        if (m_registeredForLanding) {
            log::info("{}: Registered for Landing - cancel update", __FUNCTION__);
            return;
        }
    
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: Dragon actor is null", __FUNCTION__);
            return;
        }
    
        int currentFlyingState = _ts_SKSEFunctions::GetFlyingState(dragonActor);
    
        if (currentFlyingState == 0 || currentFlyingState == 4) { // Landed or landing
            DragonLandPlayerRiding(dragonActor);
        } else {
            DragonHoverPlayerRiding(dragonActor);
        }
    }

    void FlyingModeManager::ToggleAutoCombat() {
        if (m_registeredForLanding) {
            // Autocombat is always turned off during landing - do not allow to toggle
            return;
        }
    
        auto& dataManager = DataManager::GetSingleton();
        dataManager.ToggleAutoCombat();
        log::info("{}: AutoCombat toggled to {}", __FUNCTION__, dataManager.GetAutoCombat());
    
        // Display notification if flying mode display is enabled
        if (dataManager.GetAutoCombat()) {
            RE::SendHUDMessage::ShowHUDMessage("Combat - Auto");
        } else {
            RE::SendHUDMessage::ShowHUDMessage("Combat - Manual");
        }
    
        auto* dragonActor = dataManager.GetDragonActor();
        if (!dragonActor) {
            log::error("{}: Dragon actor is null", __FUNCTION__);
            return;
        }
    
        SKSE::GetTaskInterface()->AddTask([dragonActor]() {
        // When modifying Game objects, send task to TaskInterface to ensure thread safety
            dragonActor->EvaluatePackage();
        });
    
        if (!dataManager.GetAutoCombat() && _ts_SKSEFunctions::GetCombatState(dragonActor) > 0 && dragonActor->IsBeingRidden()) {
            UpdateFlyingMode();
        }
    }

    void FlyingModeManager::OnKeyDown(IDRCKey a_key) {
        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return; // Exit if the game is in menu mode
        }
    
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: Dragon actor is null", __FUNCTION__);
            return;
        }
    
        auto& controlsManager = ControlsManager::GetSingleton();
        auto& combatManager = CombatManager::GetSingleton();
        auto& dataManager = DataManager::GetSingleton();
        auto& displayManager = DisplayManager::GetSingleton();
        log::info("{}: OnKeyDown: keycode = {}, controlBlocked = {}, registeredForLanding = {}", 
                  __FUNCTION__, a_key, controlsManager.GetControlBlocked(), m_registeredForLanding);
    
        // Handle Activate Key
        if (a_key == kActivate && _ts_SKSEFunctions::GetFlyingState(dragonActor) != 0 && 
                 _ts_SKSEFunctions::GetFlyingState(dragonActor) != 5) {
            SetRegisteredForLanding(false);
            SetRegisteredForPerch(false);
            controlsManager.SetControlBlocked(false);
            DragonHoverPlayerRiding(dragonActor);
        }
    
        // Handle Toggle Always Run Key
        if (a_key == kToggleAlwaysRun) {
            m_toggleAlwaysRun = !m_toggleAlwaysRun;

            if (m_toggleAlwaysRun) {
                RE::SendHUDMessage::ShowHUDMessage("Fast Mode - On");
            } else {
                RE::SendHUDMessage::ShowHUDMessage("Fast Mode - Off");
            }
        } else if (a_key == kToggleAutoCombat) {
            ToggleAutoCombat();
    
            if (dragonActor->IsBeingRidden()) {
                dragonActor->EvaluatePackage();
            }
        } else if (a_key == kJump) {
            m_vanillaAttack = !m_vanillaAttack;
        } else if (a_key == kSneak) {
            combatManager.DragonAttack(controlsManager.GetIsKeyPressed(kRun));
        }
    
        if (a_key == kForward || a_key == kStrafeLeft || a_key == kStrafeRight) {
    
            if (m_registeredForLanding && a_key == kForward) {
                if (CancelDragonLandPlayerRiding()) {
                    controlsManager.SetControlBlocked(false);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                } else {
                    return;
                }
            }
        }
    
        bool flyingModeNotification = false;
        if (!controlsManager.GetControlBlocked() && IsInBorderRegion()) {
            if (dragonActor->IsBeingRidden()) {
                if ((a_key == kForward || a_key == kBack) && 
                        dataManager.GetAutoCombat() && _ts_SKSEFunctions::GetCombatState(dragonActor) > 0) {
                    UpdateFlyingMode();
                }
    
                if (a_key == kForward) {
                    if (m_mode == kLanded && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 0) { // Landed
                        while (controlsManager.GetIsKeyPressed(a_key)) {
                            if (!(controlsManager.GetIsKeyPressed(kStrafeLeft) || controlsManager.GetIsKeyPressed(kStrafeRight))) {
                                RE::NiPoint3 forwardVector{ 0.f, 1.f, 0.f };
                                float angle = _ts_SKSEFunctions::GetAngleZ(dragonActor->GetPosition() - m_dragonTurnMarker->GetPosition(), forwardVector);
                                angle = _ts_SKSEFunctions::NormalRelativeAngle(angle - dragonActor->GetAngleZ());
                                PlaceTravelToMarker(dragonActor, 500.0f, angle, 0.0f);
                                DragonTravelTo(m_dragonTravelToMarker);
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        }
                    } else {
                        FlyingModeUp(a_key);
                        flyingModeNotification = true;
                    }
                } else if (a_key == kBack) {
                    if ((m_mode == kLanded && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 0) || 
                        (m_mode == kPerching && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 5)) {
                            FlyingModeUp(a_key);
                    } else {
                        FlyingModeDown();
                    }
                    flyingModeNotification = true;
                } else if (a_key == kStrafeLeft || a_key == kStrafeRight) {
                    flyingModeNotification = (GetFlyingMode() != FlyingMode::kLanded && GetFlyingMode() != FlyingMode::kPerching);
                } else if (a_key == IDRCKey::kUp) {
                    if ((m_mode == kLanded && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 0) || 
                               (m_mode == kPerching && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 5)) { // Landed or perching
                        FlyingModeUp(a_key);
                        flyingModeNotification = true;
                    } else if (m_mode == kHovering && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 3) { // Hovering
                        FlyingModeUp(a_key);
                        flyingModeNotification = true;
                    }
                } else if (a_key == kDisplayHealth) {
                    displayManager.DisplayDragonHealth();
                }
            }
        }
    
        _ts_SKSEFunctions::ClearLookAt(dragonActor);
        _ts_SKSEFunctions::SetLookAt(dragonActor, dragonActor, false);
    }

    void FlyingModeManager::OnBackKeyUp() {
        m_backKeyPressed = false;
    }

    bool FlyingModeManager::FlyingModeUp(IDRCKey a_key) {
        log::info("IDRC - {}", __FUNCTION__);
    
        if (!GetRegisteredForLanding() && !GetRegisteredForPerch()) {
            auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
            if (!dragonActor) {
                log::error("{}: Dragon actor is null", __FUNCTION__);
                return false;
            }
    
            auto& controlsManager =ControlsManager::GetSingleton();
            FlyingMode mode = GetFlyingMode();
    
            while (controlsManager.GetIsKeyPressed(a_key)) {
                float wait = 0.75f;
                if (!(controlsManager.GetIsKeyPressed(IDRCKey::kStrafeLeft) || controlsManager.GetIsKeyPressed(IDRCKey::kStrafeRight))) {
                    controlsManager.SetControlBlocked(true);

                    if (mode == FlyingMode::kFlying && *g_FastTravelState == false) {
                        DragonNewDirection(dragonActor->GetAngleZ());
                        wait = 0.1f;
                    } else if (mode == FlyingMode::kHovering && dragonActor->AsActorState()->actorState2.allowFlying) {
                        mode = FlyingMode::kFlying;
                        SetFlyingMode(kFlying);
                        DragonNewDirection(dragonActor->GetAngleZ());
                        wait = 0.1f;
                    } else if ((mode == FlyingMode::kLanded && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 0) ||
                               (mode == FlyingMode::kPerching && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 5)) {
                        mode = FlyingMode::kHovering;
                        wait = 0.75f;
                        if (!_ts_SKSEFunctions::IsFlying(dragonActor)) {
                            DragonTakeOffPlayerRiding(dragonActor);
                        } else {
                            DragonHoverPlayerRiding(dragonActor);
                        }
                    }
                } else {
                    break;
                }
    
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(wait * 1000)));
            }
    
            controlsManager.SetControlBlocked(false);
        }

        return true;
    }

    bool FlyingModeManager::FlyingModeDown() {
        log::info("IDRC - {}", __FUNCTION__);
    
        if (!GetRegisteredForPerch()) {
            auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
            if (!dragonActor) {
                log::error("{}: Dragon actor is null", __FUNCTION__);
                return false;
            }

            auto& controlsManager = ControlsManager::GetSingleton();
            FlyingMode mode = GetFlyingMode();
            auto* currentPackage = dragonActor->GetCurrentPackage();
            auto packageType = currentPackage ? currentPackage->packData.packType : RE::PACKAGE_TYPE::kNone;
   
            while (controlsManager.GetIsKeyPressed(IDRCKey::kBack)) {
                if (!(controlsManager.GetIsKeyPressed(IDRCKey::kStrafeLeft) || controlsManager.GetIsKeyPressed(IDRCKey::kStrafeRight))) {
                    controlsManager.SetControlBlocked(true);
    
                    if (mode == FlyingMode::kFlying) {
                        mode = FlyingMode::kHovering;
    
                        std::thread([this, dragonActor]() {
                            // send to new thread so that FlyingModeDown keeps progressing while Hover is ongoing
                            this->DragonHoverPlayerRiding(dragonActor);
                        }).detach();
                    
                    } else if (mode == FlyingMode::kHovering && packageType != RE::PACKAGE_TYPE::kGetUpFromChairBed) {
                        // don't trigger perching or landing if the a previous take-off procedure is still ongoing
                        // Take-off would continue with the dragon ending up in FlyingMode == kLanded, isAllowedToFly==fals
                        // but actually flying (orbiting)
                        if (controlsManager.GetIsKeyPressed(IDRCKey::kRun)) {
                            DragonPerchPlayerRiding();
                            mode = FlyingMode::kPerching;
                        } else if (DragonLandPlayerRiding(dragonActor)) {
                            m_backKeyPressed = true; // avoid repeated landing attempts while Back key is held down
                            mode = FlyingMode::kLanded;
                        }
                    } else if (mode == FlyingMode::kLanded && !m_backKeyPressed) {
                        DragonLandPlayerRiding(dragonActor);
                        m_backKeyPressed = true; // avoid repeated landing attempts while Back key is held down
                    }
                }
    
                std::this_thread::sleep_for(std::chrono::milliseconds(750));
            }
    
        controlsManager.SetControlBlocked(false);
        }
        return true;
    }

    void FlyingModeManager::TriggerTurn() {
//        log::info("IDRC - {}", __FUNCTION__);

        auto& controlsManager = ControlsManager::GetSingleton();
        auto& combatManager = CombatManager::GetSingleton();
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: Dragon actor is null", __FUNCTION__);
            return;
        }
        
        if (!CheckForTurn()) {
            // m_yawOffset and m_targetPitch are already set in CameraLockManager::Update() via camera movement earlier in this frame
            controlsManager.SetControlBlocked(false);
        } else {
            // set m_yawOffset  and m_targetPitch directly in case user turns via keyboard
            controlsManager.SetControlBlocked(true);
            m_yawOffset = GetTurnFactor() * m_turnSpeed;
        }

        if (combatManager.IsShoutActive() && combatManager.GetShoutTarget() 
            && _ts_SKSEFunctions::GetFlyingState(dragonActor) != 2) {

            auto shoutTargetPos = combatManager.GetShoutTarget()->GetPosition();
            auto dragonPos = dragonActor->GetPosition();
            float yawOffset = std::atan2f(shoutTargetPos.x - dragonPos.x, shoutTargetPos.y - dragonPos.y) - dragonActor->GetHeading(false);
            m_yawOffset = 180.f / PI * yawOffset;
        }

        DragonTurnPlayerRiding(m_yawOffset);

        if (_ts_SKSEFunctions::GetFlyingState(dragonActor) != 2) {
            // do not reset while flying so that FlightPath can be updated
            // in PathingHook::UpdateFlightPathData
            m_yawOffset = 0.0f;
        }
    }

    bool FlyingModeManager::CheckForTurn() const {
        auto& controlsManager = ControlsManager::GetSingleton();
        bool turn = false;
        if (controlsManager.IsThumbstickPressed()) {
            if (controlsManager.GetIsKeyPressed(kForward) &&
                !controlsManager.GetIsKeyPressed(kStrafeLeft) && 
                !controlsManager.GetIsKeyPressed(kStrafeRight)) {
                turn = false;
            } else {
                turn = true;
            }
        } else {
            turn = controlsManager.GetIsKeyPressed(IDRCKey::kStrafeLeft) || controlsManager.GetIsKeyPressed(IDRCKey::kStrafeRight);
        }
        return turn;
    }

    bool FlyingModeManager::CheckForHeightChange() const {
        auto& controlsManager = ControlsManager::GetSingleton();
        return controlsManager.GetIsKeyPressed(IDRCKey::kUp) || controlsManager.GetIsKeyPressed(IDRCKey::kDown);
    }

    float FlyingModeManager::GetTurnFactor() {
        float turnFactor = 0.0f;
        auto& controlsManager = ControlsManager::GetSingleton();

        if (controlsManager.IsThumbstickPressed()) {
            float thumbstickAngle = controlsManager.GetThumbstickAngle();
            // Thumbstick angle convention: right = 0, forward = 90,  left = 180, back = 270 (in radians)
            if (thumbstickAngle > 270.0f * 0.0174533f) {thumbstickAngle -= 360.0f * 0.0174533f;}
            // Forward: turnFactor = 0; Back: turnFactor = 2.0
            // Left: turnfactor negative; Right: turnFactor positive
            turnFactor = ( 90.0f * 0.0174533f - thumbstickAngle) / (90.0f * 0.0174533f);
        }
        else {
            turnFactor = 0.5f;
            if (!controlsManager.GetIsKeyPressed(IDRCKey::kForward)) {
                if (!controlsManager.GetIsKeyPressed(IDRCKey::kBack)) {
                    turnFactor = 1.0f;
                } else {
                    turnFactor = 2.0f;
                }
            }
            float direction = controlsManager.GetIsKeyPressed(kStrafeLeft) ? -1.0f : 1.0f;
            if (APIs::TrueDirectionalMovementV4 && APIs::TrueDirectionalMovementV4->IsTargetLockBehindTarget()) {
                direction *= -1.0f;
            }
            turnFactor *= direction;
        }
        turnFactor *= GetRunFactor();
        return turnFactor;
    }

    bool FlyingModeManager::DragonTravelTo(RE::TESObjectREFR* a_directionMarker) {
        log::info("IDRC - {}", __FUNCTION__);
    
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return false;
        }
    
        if (!a_directionMarker) {
            log::error("{}: Direction marker is null", __FUNCTION__);
            return false;
        }
    
        // Switch to travel package
        SKSE::GetTaskInterface()->AddTask([dragonActor]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 4); // Travel package
            dragonActor->EvaluatePackage();
        });
    
        log::info("{}: Switched to Travel Package", __FUNCTION__);
    
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
        // Switch back to orbit package
        SKSE::GetTaskInterface()->AddTask([dragonActor]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 0); // Orbit package
            dragonActor->EvaluatePackage();
        });
        
        return true;
    }

    bool FlyingModeManager::TriggerLand(RE::TESObjectREFR* a_landTarget) {
        log::info("IDRC - {}", __FUNCTION__);
    
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return false;
        }
    
        // If no a_andTarget is provided, use the dragonActor as the target
        if (!a_landTarget) {
            a_landTarget = dragonActor;
        }
    
        // Move the orbit marker to the landing position
        auto* orbitMarker = DataManager::GetSingleton().GetOrbitMarker();
        if (!orbitMarker) {
            log::error("{}: orbitMarker is null", __FUNCTION__);
        }

        m_landingPosSearchOngoing = true;
        m_finalizeTriggerLand = true;
    
        SKSE::GetTaskInterface()->AddTask([this, orbitMarker, a_landTarget]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety

            // MoveTo ensures orbit marker is in same worldspace as a_landTarget:
            _ts_SKSEFunctions::MoveTo(orbitMarker, a_landTarget); 

            m_landingPos = a_landTarget->GetPosition(); // fallback in case GetValidLandingPosition() fails to find a valid landing position
            std::thread([this]() {
                // send to new thread so that TriggerLand is not blocking in case search takes longer
                // (this can happen if additional cells get loaded in GetValidLandingPosition()
                // Once GetValidLandingPosition returns, FinalizeTriggerLand() is triggered in the next Update() 
                // by checking for m_landingPosSearchOngoing == true
                this->GetValidLandingPosition();
                this->m_landingPosSearchOngoing = false;
            }).detach();
        });
        return true;
    }

    void FlyingModeManager::FinalizeTriggerLand() {
        log::info("IDRC - {}, m_finalizeTriggerLand: {}, m_landingPosSearchOngoing: {}", __FUNCTION__,
            m_finalizeTriggerLand ? "true" : "false", m_landingPosSearchOngoing ? "true" : "false");

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return;
        }

        if (m_finalizeTriggerLand && !m_landingPosSearchOngoing) {
            if (Utils::IsFastTravelActive()) {
                // still in fast travel or patrol, ensure orbit package 
                // to force stop fast travel via package conditions
                SKSE::GetTaskInterface()->AddTask([dragonActor]() {
                    // When modifying Game objects, send task to TaskInterface to ensure thread safety
                    dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 0); // Orbit package
                    dragonActor->EvaluatePackage();
                });

                // don't initiate landing while fast travel or patrol
                // or else dragon will be still in that mode while grounded
                return;              
            }

            m_finalizeTriggerLand = false;

            auto* orbitMarker = DataManager::GetSingleton().GetOrbitMarker();
            SKSE::GetTaskInterface()->AddTask([this, dragonActor, orbitMarker]() {
                orbitMarker->SetPosition(this->m_landingPos);
                
                // Instead of calling SetAllowFlying(false), which 
                //  * determines a landing location via GetCurrentMountCellOrWorldspaceForm()
                //  * calls InitiateForcedLanding() with that location to put dragon into kLanding package
                //  * sets actorState2.allowFlying = false
                // call InitiateForcedLanding directly with a_landTarget as landing target.
                // This avoids 'awkward' landing locations, eg very far away. 
                // But this also means that the dragon will land almost anywhere directly on the spot, also when it shouldn't.
                // Eg it will land on water, and then start to sink.
                
                InitiateForcedLanding(dragonActor, orbitMarker, true, false);
                m_waitForLanded = true;
            });        
        }
    }

    void FlyingModeManager::FinalizeLand() {
        log::info("IDRC - {}", __FUNCTION__);

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return;
        }

        if (m_waitForLanded && _ts_SKSEFunctions::GetFlyingState(dragonActor) == 0) {
            m_waitForLanded = false;
            if (Utils::IsFastTravelActive()) {
                // This should never happen!
                log::warn("{}: Dragon is still in fast travel or patrol after landing, triggering take-off", __FUNCTION__);
                SetLandedCompleted();
                DragonTakeOffPlayerRiding(dragonActor);
                return;
            }

            log::info("{}: Dragon has landed", __FUNCTION__);
            SKSE::GetTaskInterface()->AddTask([this, dragonActor]() {
                if (this->m_noFlyAbility) {
                    dragonActor->AddSpell(this->m_noFlyAbility);
                } else {
                    log::warn("{}: NoFlyAbility is null", __FUNCTION__);
                }

                // need to set allowFlying explicitly, because SetAllowFlying is not used
                dragonActor->AsActorState()->actorState2.allowFlying = false;
                dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 0); // Orbit package
                dragonActor->EvaluatePackage();
            });      
            
            SetLandedCompleted();
        }
    }


    void FlyingModeManager::SetLandedCompleted() {
        // Restore auto-combat if it was toggled off
        if (m_toggledAutoCombatLand) {
            log::info("{}: AutoCombat toggled to TRUE", __FUNCTION__);
            DataManager::GetSingleton().SetAutoCombat(true);
            m_toggledAutoCombatLand = false;
        }

        PlaceTravelToMarker(IDRC::DataManager::GetSingleton().GetDragonActor());

        // Clear landing registration and register for updates
        SetRegisteredForLanding(false);
    }


    void FlyingModeManager::GetValidLandingPosition()
	{
        log::info("IDRC - {}", __FUNCTION__);

        auto* pathingSingleton = Hooks::PathingHook::GetPathingSingleton();
		if (!pathingSingleton) {
			log::warn("{}: Pathing singleton not initialized!", __FUNCTION__);
			return;
		}

		auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (!dragonActor) {
			log::warn("{}: Dragon actor not found!", __FUNCTION__);
			return;
		}
        
        RE::BSTSmartPointer<RE::BSPathingCell> cell;
		RE::BSPathingLocation loc(RE::NiPoint3(0.0f, 0.0f, 0.0f), cell);
		GetCurrentPathingLocation(pathingSingleton, &loc, dragonActor, 0);

		if (loc.navMeshInfo == nullptr || loc.triangle < 0) {
			log::warn("{}: Current pathing location is invalid!", __FUNCTION__);
			return;
		}

		RE::NiPoint3 searchPos = loc.location;

		// FindNavmeshTriangleForLocation always returns the closest point with navmesh, but does not filter for water level
		if (FindNavmeshTriangleForLocation(&loc, &pathingSingleton->defaultTriangleFilter)) {
			if (loc.location.z > RE::PlayerCharacter::GetSingleton()->GetWaterHeight()) {
				m_landingPos = loc.location;
//log::info("{}: Found valid landing position at {}", __FUNCTION__, m_landingPos);
//if (APIs::TrueHUD) {
//	APIs::TrueHUD->DrawPoint(m_landingPos, 5.0f, 20.0f, 0xFFFFFFFF);
//}
				return;
			}
		}

		// Fallback: no landing spot found around dragon location, or found spot under water
		// start area search for alternative landing location
		
		float dragonForwardAngle = dragonActor->GetHeading(false);
		const float searchDistanceStep = 1000.f;
		int angleSteps = 16;
		const float searchAngleStep = PI / angleSteps;

		RE::NiPoint3 candidatePos = { searchPos.x, searchPos.y, searchPos.z };
		float offsetX = 0.f;
		float offsetY = 0.f;
		float offset = 0.f;
		float angleOffset = 0.f;
		float searchAngle = 0.f;
		for (int distanceStep = 1; distanceStep <= 50; ++distanceStep) {
			// max search distance (50 * searchDistanceStep = 50000 units) should be sufficient to find a valid spot
			offset = searchDistanceStep * distanceStep;

			for (int step = 0; step <= angleSteps; ++step) {
				angleOffset = step * searchAngleStep;
				searchAngle = dragonForwardAngle + angleOffset;

				offsetX = std::sin(searchAngle) * offset;
				offsetY = std::cos(searchAngle) * offset;

				candidatePos.x = searchPos.x + offsetX;
				candidatePos.y = searchPos.y + offsetY;

				if (_ts_SKSEFunctions::HasNavmesh(candidatePos, true)) {
					m_landingPos = candidatePos;
                    m_landingPos.z = _ts_SKSEFunctions::GetLandHeightWithWater(candidatePos, true);
//log::info("{}: Found valid landing position via Navmesh (right) {}", __FUNCTION__, m_landingPos);
//if (APIs::TrueHUD) {
//	APIs::TrueHUD->DrawPoint(m_landingPos, 5.0f, 20.0f, 0xFFFFFFFF);
//}
					return;
				}

				// check the other direction
				if (step != 0 && step != angleSteps) { // skip 0 and 180 degrees, already checked
					searchAngle = dragonForwardAngle - angleOffset;

					offsetX = std::sin(searchAngle) * offset;
					offsetY = std::cos(searchAngle) * offset;

					candidatePos.x = searchPos.x + offsetX;
					candidatePos.y = searchPos.y + offsetY;

					if (_ts_SKSEFunctions::HasNavmesh(candidatePos, true)) {
						m_landingPos = candidatePos;
                        m_landingPos.z = _ts_SKSEFunctions::GetLandHeightWithWater(candidatePos, true);
//log::info("{}: Found valid landing position via Navmesh (left) {}", __FUNCTION__, m_landingPos);
//if (APIs::TrueHUD) {
//	APIs::TrueHUD->DrawPoint(m_landingPos, 5.0f, 20.0f, 0xFFFFFFFF);
//}
                        return;
					}
				}
			}
		}
	}


    bool FlyingModeManager::CancelDragonLandPlayerRiding() {
        log::info("IDRC - {}", __FUNCTION__);
    
        if (!GetRegisteredForLanding()) {
            return false;
        }

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return false;
        }

        if (GetFlyingMode() != FlyingMode::kLanded) {
            SetLandedCompleted();
            return false;
        }
        
        if (!(_ts_SKSEFunctions::GetFlyingState(dragonActor) == 2 ||
              _ts_SKSEFunctions::GetFlyingState(dragonActor) == 3)) {
            log::info("{}: Already landing (4) or landed (1). Ignore CancelLand request.", __FUNCTION__);
            return false;
        }
    
        log::info("{}: Trying to stop landing", __FUNCTION__);

        SetLandedCompleted();
        SKSE::GetTaskInterface()->AddTask([this, dragonActor]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            auto currentProcess = dragonActor->GetActorRuntimeData().currentProcess;
            if (currentProcess) {
                currentProcess->SetRunOncePackage(nullptr, dragonActor);
            } else {
                log::warn("{}: currentProcess is null", __FUNCTION__);
            }

            dragonActor->AsActorState()->actorState2.allowFlying = true;
            if (this->m_noFlyAbility) {
                dragonActor->RemoveSpell(this->m_noFlyAbility);
            } else {
                log::warn("{}: NoFlyAbility is null", __FUNCTION__);
            }

            dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 3);
            dragonActor->EvaluatePackage();

            this->DragonHoverPlayerRiding(dragonActor);
        });
    
        return true;
    }

    bool FlyingModeManager::GetRegisteredForLanding() {
        return m_registeredForLanding;
    }

    bool FlyingModeManager::GetRegisteredForPerch() {
        return m_registeredForPerch;
    }

    void FlyingModeManager::SetRegisteredForLanding(bool a_registeredForLanding) {
        m_registeredForLanding = a_registeredForLanding;
    }

    void FlyingModeManager::SetRegisteredForPerch(bool a_registeredForPerch) {
        m_registeredForPerch = a_registeredForPerch;
    }

    bool FlyingModeManager::DragonHoverPlayerRiding(RE::TESObjectREFR* a_hoverTarget) {
        // Check if the dragon is registered for landing
        if (m_registeredForLanding) {
            log::info("{}: Registered for Landing - cancel hover", __FUNCTION__);
            return false;
        }
    
        auto& dataManager = DataManager::GetSingleton();
        auto* dragonActor = dataManager.GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return false;
        }
    
        log::info("{}: DragonHoverPlayerRiding, IsBeingRidden = {}", __FUNCTION__, dragonActor->IsBeingRidden());
    
        auto flyingMode = GetFlyingMode();

        float injuredHealthPercentage = dragonActor->GetRace()->data.injuredHealthPercent;

        if (_ts_SKSEFunctions::GetHealthPercentage(dragonActor) > injuredHealthPercentage &&
            (flyingMode != FlyingMode::kHovering || _ts_SKSEFunctions::GetFlyingState(dragonActor) != 3)) {

            SetFlyingMode(FlyingMode::kHovering);
    
            SKSE::GetTaskInterface()->AddTask([this, dragonActor]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
                dragonActor->AsActorState()->actorState2.allowFlying = true;
                if (this->m_noFlyAbility) {
                    dragonActor->RemoveSpell(this->m_noFlyAbility);
                } else {
                    log::warn("{}: NoFlyAbility is null", __FUNCTION__);
                }
                dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kWaitingForPlayer, 0);
                dragonActor->EvaluatePackage();
            });
        
            // Check if flying state is still valid
            flyingMode = GetFlyingMode();
            if (flyingMode != FlyingMode::kHovering) {
                // another flying mode change has been triggered since the hover command was initiated
                // (this can happen in case Hover is triggered as event, from FlyingModeDown())
                log::info("{}: No longer valid - cancel hover", __FUNCTION__);
                return false;
            }
    
            auto* orbitMarker = dataManager.GetOrbitMarker();
            if (!orbitMarker) {
                log::error("{}: Orbit marker is null", __FUNCTION__);
                return false;
            }

            // Determine the dragon's current height above ground
            float dragonPosZ = dragonActor->GetPositionZ();
            float heightAboveGround = dragonPosZ - _ts_SKSEFunctions::GetLandHeightWithWater(RE::PlayerCharacter::GetSingleton());
    
            if (heightAboveGround < m_minHeight) {
                heightAboveGround = m_minHeight;
            }
    
            // Adjust the hover target's position
            float angleZ = a_hoverTarget->GetAngleZ();
            float distance = 4600.0f;

            SKSE::GetTaskInterface()->AddTask([this, dragonActor, a_hoverTarget, orbitMarker, 
                                            distance, angleZ, dragonPosZ, heightAboveGround]() {
                // When modifying Game objects, send task to TaskInterface to ensure thread safety
                if (_ts_SKSEFunctions::GetFlyingState(dragonActor) != 3) { // not hovering
                    RE::NiPoint3 angle = {0.0f, 0.0f, angleZ};
                    _ts_SKSEFunctions::SetAngle(orbitMarker, angle);

                    // move Hover Target a bit ahead, so that dragon (hopefully) does not need to take a turn to reach the hover target
                    _ts_SKSEFunctions::MoveTo(orbitMarker, a_hoverTarget, distance * std::sin(angleZ), distance * std::cos(angleZ), 0.0f);

                    float markerPosZ = _ts_SKSEFunctions::GetLandHeightWithWater(orbitMarker) + heightAboveGround;

                    if (markerPosZ < dragonPosZ) {
                        markerPosZ = (dragonPosZ + markerPosZ) / 2.0f;
                    }

                    // move the hover target to the calculated height
                    orbitMarker->SetPosition(orbitMarker->GetPositionX(), orbitMarker->GetPositionY(), markerPosZ);
                } else {
                    _ts_SKSEFunctions::MoveTo(orbitMarker, dragonActor);
                }

                // move turn marker ahead of orbit marker to fix new look-at position
                _ts_SKSEFunctions::MoveTo(this->m_dragonTurnMarker, orbitMarker, 2500.0f * std::sin(angleZ), 2500.0f * std::cos(angleZ), 0.0f);
                dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 3); // Hover package
                dragonActor->EvaluatePackage();
            });
        }
    
        return true;
    }

    bool FlyingModeManager::ForceHover() {
        log::info("IDRC - {}", __FUNCTION__);
    
        auto& dataManager = DataManager::GetSingleton();
        auto* dragonActor = dataManager.GetDragonActor();
    
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return false;
        }
    
        if (!m_flyToTargetMarker) {
            log::error("{}: FlyToTargetMarker is null", __FUNCTION__);
            return false;
        }
    
        ControlsManager::GetSingleton().SetControlBlocked(true);

        float centerX = GetWorldSpaceCenterX();
        float centerY = GetWorldSpaceCenterY();
    
        float angleZ = GetAngleToCoordinate(centerX, centerY);
    
        float offsetX = 6000.0f * std::sin(angleZ);
        float offsetY = 6000.0f * std::cos(angleZ);
    
        // Move the FlyToTargetMarker behind the dragon, directed at the worldspace center
        SKSE::GetTaskInterface()->AddTask([this, dragonActor, offsetX, offsetY, angleZ]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            _ts_SKSEFunctions::MoveTo(this->m_flyToTargetMarker, dragonActor, offsetX, offsetY, 0.0f);
            RE::NiPoint3 angle = { 0.0f, 0.0f, angleZ };
            _ts_SKSEFunctions::SetAngle(this->m_flyToTargetMarker, angle);
            DragonHoverPlayerRiding(m_flyToTargetMarker);
        });
    
        ControlsManager::GetSingleton().SetControlBlocked(false);

        return true;
    }

    bool FlyingModeManager::DragonTakeOffPlayerRiding(RE::TESObjectREFR* a_takeOffTarget) {
        log::info("IDRC - {}", __FUNCTION__);
    
        // Check if the dragon is registered for landing
        if (m_registeredForLanding) {
            log::info("{}: Registered for Landing - cancel takeoff", __FUNCTION__);
            return false;
        }
    
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return false;
        }
        
        float injuredHealthPercent = dragonActor->GetRace()->data.injuredHealthPercent;
        if (_ts_SKSEFunctions::GetHealthPercentage(dragonActor) > injuredHealthPercent) {
            // Set flying state to hovering
            SetFlyingMode(FlyingMode::kHovering);
    
            auto* orbitMarker = DataManager::GetSingleton().GetOrbitMarker();
            if (!orbitMarker) {
                log::error("{}: Orbit marker is null", __FUNCTION__);
                return false;
            }

            if (!a_takeOffTarget) {
                log::info("{}: No TakeOffTarget provided - cancel takeoff", __FUNCTION__);
                return false;
            }
            
            float angleZ = dragonActor->GetAngleZ();    
            RE::NiPoint3 angle = { dragonActor->GetAngleX(), dragonActor->GetAngleY(), angleZ };
            
            SKSE::GetTaskInterface()->AddTask([this, dragonActor, orbitMarker, angle, a_takeOffTarget, angleZ]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
                _ts_SKSEFunctions::SetAngle(orbitMarker, angle);
                _ts_SKSEFunctions::MoveTo(orbitMarker, a_takeOffTarget, 
                    100.0f * std::sin(angleZ), 100.0f * std::cos(angleZ),  this->m_minHeight);

                dragonActor->AsActorState()->actorState2.allowFlying = true;
                if (m_noFlyAbility) {
                    dragonActor->RemoveSpell(this->m_noFlyAbility);
                } else {
                    log::warn("{}: NoFlyAbility is null", __FUNCTION__);
                }

                dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 3); // Hover package
                dragonActor->EvaluatePackage();
            });
        } else {
            log::info("{}: Dragon health is low - cannot take off", __FUNCTION__);
            RE::SendHUDMessage::ShowHUDMessage("Dragon health is low - cannot take off");
            return false;
        }
    
        return true;
    }

    void FlyingModeManager::PlaceTravelToMarker(RE::TESObjectREFR* a_ref, float a_distance, float a_angle, float a_offsetZ) {
        log::info("IDRC - {}", __FUNCTION__);
    
        if (!a_ref) {
            log::error("{}: a_ref is null", __FUNCTION__);
            return;
        }
    
        if (!m_dragonTravelToMarker) {
            log::error("{}: DragonTravelToMarker is null", __FUNCTION__);
            return;
        }
    
        float markerPosX = a_ref->GetPositionX() + a_distance * std::sin(a_ref->GetAngleZ() + a_angle);
        float markerPosY = a_ref->GetPositionY() + a_distance * std::cos(a_ref->GetAngleZ() + a_angle);
        float markerPosZ = _ts_SKSEFunctions::GetLandHeight(markerPosX, markerPosY, a_ref->GetPositionZ()) + a_offsetZ;
    
        SKSE::GetTaskInterface()->AddTask([this, a_ref, markerPosX, markerPosY, markerPosZ]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            this->m_dragonTravelToMarker->MoveTo(a_ref); // ensures TravelToMarker is in same worldspace as a_ref
            this->m_dragonTravelToMarker->SetPosition(markerPosX, markerPosY, markerPosZ);
        });
    }

    bool FlyingModeManager::DragonLandPlayerRiding(RE::TESObjectREFR* a_landTarget) {
        log::info("IDRC - {}", __FUNCTION__);

        bool isReTriggered = false;
        if (GetRegisteredForLanding()) {
            log::info("{}: Already registered for landing - re-triggering landing", __FUNCTION__);
            isReTriggered = true;
        }
        
        auto& dataManager = DataManager::GetSingleton();
        auto* dragonActor = dataManager.GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return false;
        }
        
        // Check if the dragon is already landing or landed
        if (GetFlyingMode() == FlyingMode::kLanded && 
            _ts_SKSEFunctions::GetFlyingState(dragonActor) == 0) {
            log::info("{}: Already in Landing or landed. Ignore Land request.", __FUNCTION__);
            return false;
        }

        if (!isReTriggered) {
            SetRegisteredForLanding(true);

            SetFlyingMode(FlyingMode::kLanded);

            // Handle auto-combat toggling
            m_toggledAutoCombatLand = false;
            if (dataManager.GetAutoCombat()) {
                log::info("{}: AutoCombat toggled to FALSE", __FUNCTION__);
                dataManager.SetAutoCombat(false);
                m_toggledAutoCombatLand = true;
            }
        }

        PlaceTravelToMarker(dragonActor);

        TriggerLand(a_landTarget);
        return true;
    }


    bool FlyingModeManager::DragonPerchPlayerRiding() {
        log::info("IDRC - {}", __FUNCTION__);
    
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return false;
        }
    
        // Only trigger perch from hover
        if (_ts_SKSEFunctions::GetFlyingState(dragonActor) == 3 && !m_registeredForPerch) {
            SetRegisteredForPerch(true);
            auto oldState = GetFlyingMode();
    
            SetFlyingMode(FlyingMode::kPerching);

            auto& dataManager = DataManager::GetSingleton();

            // Start the perch quest
            auto* findPerchQuest = dataManager.GetFindPerchQuest();
            if (!findPerchQuest) {
                log::error("{}: findPerchQuest is null", __FUNCTION__);
                SetFlyingMode(oldState);
                SetRegisteredForPerch(false);
                return false;
            }

            SKSE::GetTaskInterface()->AddTask([findPerchQuest]() {
                // When modifying Game objects, send task to TaskInterface to ensure thread safety
                findPerchQuest->Start();
            });

            int count = 0;
            while (count < 100 && findPerchQuest->IsStopped()) 
            {
                _ts_SKSEFunctions::WaitWhileGameIsPaused();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                count++;
            }
            if (count >= 100) { // waited > 1sec
                log::error("{}: ERROR - Timed out while waiting for FindPerchQuest to start!", __FUNCTION__);
                SetFlyingMode(oldState);
                SetRegisteredForPerch(false);
                return false;
            }

            // Retrieve potential perch targets
            auto* wordWall = dataManager.GetWordWallPerch();
            auto* tower = dataManager.GetTowerPerch();
            auto* rock = dataManager.GetRockPerch();
    
            if (!wordWall && !tower && !rock) {
                // No valid perch targets found
                dataManager.SetPerchTarget(nullptr);
                SetFlyingMode(oldState);
                SetRegisteredForPerch(false);
            } else {
                // Find the closest perch target
                float minDistance = 99999.9f;
                RE::TESObjectREFR* closestPerch = nullptr;
    
                auto* player = RE::PlayerCharacter::GetSingleton();
    
                if (wordWall && _ts_SKSEFunctions::GetDistance(wordWall, player) < minDistance) {
                    minDistance = _ts_SKSEFunctions::GetDistance(wordWall, player);
                    closestPerch = wordWall;
                }
                if (tower && _ts_SKSEFunctions::GetDistance(tower, player) < minDistance) {
                    minDistance = _ts_SKSEFunctions::GetDistance(tower, player);
                    closestPerch = tower;
                }
                if (rock && _ts_SKSEFunctions::GetDistance(rock, player) < minDistance) {
                    minDistance = _ts_SKSEFunctions::GetDistance(rock, player);
                    closestPerch = rock;
                }
                if (closestPerch) {
                    log::info("{}: Found perch: {}", __FUNCTION__, closestPerch->GetFormID());      

                    dataManager.SetPerchTarget(closestPerch);

                    SKSE::GetTaskInterface()->AddTask([dragonActor]() {
                    // When modifying Game objects, send task to TaskInterface to ensure thread safety
                        dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 1); // Perching
                        dragonActor->EvaluatePackage();
                    });
                }
            }

            SKSE::GetTaskInterface()->AddTask([findPerchQuest]() {
                // When modifying Game objects, send task to TaskInterface to ensure thread safety
                findPerchQuest->Stop();
            });
        }
        return true;
    }


    bool FlyingModeManager::DragonTurnPlayerRiding(float a_turnAngle) {
//        log::info("IDRC - {}, angle: {}", __FUNCTION__, a_turnAngle);

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        auto* orbitMarker = DataManager::GetSingleton().GetOrbitMarker();
    
        if (!dragonActor) {
            log::error("{}: dragonActor is None", __FUNCTION__);
            return false;
        }
    
        if (!m_dragonTurnMarker) {
            log::error("{}: DragonTurnMarker is None", __FUNCTION__);
            return false;
        }

        if (!orbitMarker) {
            log::error("{}: OrbitMarker is None", __FUNCTION__);
            return false;
        }
        float angleZ = dragonActor->GetAngleZ() + a_turnAngle* 0.01745329252f;
    
        int flyingState = _ts_SKSEFunctions::GetFlyingState(dragonActor);
        if (flyingState == 0 || flyingState == 3 || flyingState == 5) { // Landed, hovering, or perching
            SKSE::GetTaskInterface()->AddTask([this, dragonActor, orbitMarker, angleZ]() {
                // When modifying Game objects, send task to TaskInterface to ensure thread safety
                _ts_SKSEFunctions::MoveTo(this->m_dragonTurnMarker, dragonActor, 2500.0f * std::sin(angleZ), 2500.0f * std::cos(angleZ), 0.0f);
                _ts_SKSEFunctions::SetLookAt(dragonActor, this->m_dragonTurnMarker, true);
                auto angle = dragonActor->GetAngle();
                _ts_SKSEFunctions::SetAngle(orbitMarker, angle);
                _ts_SKSEFunctions::SetAngleZ(orbitMarker, angleZ);
                dragonActor->EvaluatePackage();
            });    
        } else if (flyingState == 2 && m_mode == kFlying  && !m_registeredForLanding && !m_registeredForPerch) {
            return DragonNewDirection(angleZ);
        }
        return true;
    }

    bool FlyingModeManager::DragonNewDirection(float a_angle) {
//        log::info("IDRC - {}", __FUNCTION__);
    
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is None", __FUNCTION__);
            return false;
        }
    
        if (dragonActor->AsActorState()->actorState2.allowFlying == 0) { // Check if dragon is allowed to fly
            log::info("{}: Dragon is not allowed to fly (injured) - cancel new direction", __FUNCTION__);
            return false;
        }
    
        return DragonFlyTo(a_angle);
    }

    bool FlyingModeManager::DragonFlyTo(float a_angle){
//        log::info("IDRC - {}", __FUNCTION__);

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        auto* orbitMarker = DataManager::GetSingleton().GetOrbitMarker();

        if (!dragonActor) {
            log::error("{}: dragonActor is None", __FUNCTION__);
            return false;
        }
        if (!orbitMarker) {
            log::error("{}: OrbitMarker is None", __FUNCTION__);
            return false;
        }
        if (dragonActor->AsActorState()->actorState2.allowFlying == 0) {
            log::info("{}: Dragon is not allowed to fly - cancel FlyTo", __FUNCTION__);
            return false;
        }
        
        WorldSpaceData worldSpaceData; 
        try {
            worldSpaceData = WorldSpaceData(dragonActor->GetWorldspace()); 
        } catch (const std::invalid_argument& e) {
            log::error("{}: {}", __FUNCTION__, e.what());
            return false;
        }
    
        // Current player coordinates
        auto* player = RE::PlayerCharacter::GetSingleton();
        float posX = player->GetPositionX();
        float posY = player->GetPositionY();
        float angleNorm = NormalizeAngle(a_angle);

        // keep the target position close to the dragon, to avoid the need for the engine to frequently re-compute long paths
        float distance = 10000.f; // previously: GetDistanceToRegionBoundingBox(worldSpaceData, posX, posY, angleNorm);
        if(!IsInBorderRegion()){
            log::info("{}: Player is not in border region {} - cancel FlyTo...", __FUNCTION__, worldSpaceData.m_borderRegionName);
            return false;
        }


        if (GetFlyingMode() == kFlying) { // no other flying state triggered yet
            // move dragon orbit marker to the Skyrim borderline which is in the direction the player has defined.
            float markerPosX = posX + distance * std::sin(angleNorm);
            float markerPosY = posY + distance * std::cos(angleNorm);

            float height = worldSpaceData.m_seaLevel + m_minHeight;
            SKSE::GetTaskInterface()->AddTask([dragonActor, orbitMarker, markerPosX, markerPosY, height]() {
                // When modifying Game objects, send task to TaskInterface to ensure thread safety
                orbitMarker->MoveTo(dragonActor); // ensures orbitMarker is in same worldspace as dragonActor
                orbitMarker->SetPosition(markerPosX, markerPosY, height);
                dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 2); // FastTravel

                dragonActor->EvaluatePackage();
//if (APIs::TrueHUD) {
//    APIs::TrueHUD->DrawPoint(orbitMarker->GetPosition(), 15.0f, 0.0f, 0xAA00AAFF);
//}
                // Start the FastTravel mode
                FastTravelManager::GetSingleton().FastTravel(orbitMarker);
            });
        }

        return true;
    }

    void FlyingModeManager::SetYawOffset(float a_angle) {
        m_yawOffset = a_angle;
    }

    void FlyingModeManager::SetTargetPitch(float a_angle) {
        m_targetPitch = a_angle;
    }

    float FlyingModeManager::GetTargetYaw() {
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return 0.0f;
        }

        float targetYaw = dragonActor->GetHeading(false) + PI/180.f * m_yawOffset;
        return targetYaw;
    }

    float FlyingModeManager::GetTargetPitch() {
        return m_targetPitch;
    }

    float FlyingModeManager::GetAngleToCoordinate(float a_posX, float a_posY) {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            log::error("{}: Player reference is null", __FUNCTION__);
            return 0.0f;
        }
    
        // Calculate deltas
        float deltaX = a_posX - player->GetPositionX();
        float deltaY = a_posY - player->GetPositionY();
    
        float angle = 0.0f;
    
        // Avoid division by zero
        if (deltaY != 0.0f) {
            angle = std::atan2(deltaX, deltaY);
            if (angle < 0.0f) {
                angle +=  2.0f * PI;
            }
        } else if (deltaX > 0.0f) {
            angle = 0.5f * PI;  // 90 degrees
        } else {
            angle = 1.5f * PI; // 270 degrees
        }
        return angle;
    }
    
    float FlyingModeManager::GetWorldSpaceCenterX() {
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: Dragon actor is null", __FUNCTION__);
            return 0.0f;
        }
    
        auto* worldSpace = dragonActor->GetWorldspace();
        if (!worldSpace) {
            log::error("{}: Worldspace is null", __FUNCTION__);
            return 0.0f;
        }
    
        if (strcmp(worldSpace->GetFullName(), "Solstheim") == 0) {
            return 50000.0f;
        } else if (strcmp(worldSpace->GetFullName(), "Skyrim") == 0) {
            return 0.0f;
        }
    
        log::warn("{}: Unknown worldspace - returning default center X = 0.0", __FUNCTION__);
        return 0.0f;
    }
    
    float FlyingModeManager::GetWorldSpaceCenterY() {
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: Dragon actor is null", __FUNCTION__);
            return 0.0f;
        }
    
        auto* worldSpace = dragonActor->GetWorldspace();
        if (!worldSpace) {
            log::error("{}: Worldspace is null", __FUNCTION__);
            return 0.0f;
        }
    
        if (strcmp(worldSpace->GetFullName(), "Solstheim") == 0) {
            return 60000.0f;
        } else if (strcmp(worldSpace->GetFullName(), "Skyrim") == 0) {
            return 0.0f;
        }
    
        log::warn("{}: Unknown worldspace - returning default center Y = 0.0", __FUNCTION__);
        return 0.0f;
    }

    void FlyingModeManager::WorldSpaceData::InitializeData(const std::string& a_regionName) {
        float sealevel= 0.0f;
        float min_x = 0.0f;
        float min_y = 0.0f;
        float max_x = 0.0f;
        float max_y = 0.0f;
        float center_x = 0.0f;
        float center_y = 0.0f;

/* currently not used: Scan through all regions and finde the BorderRegions for all worldspaces
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            log::error("{}: DataHandler is null", __FUNCTION__);
            return;
        }
        auto* regionList = dataHandler->regionList;
        if (!regionList) {
            log::error("{}: RegionList is null", __FUNCTION__);
            return;
        }
        for (const auto& region : *regionList) {
currently not scanning through all regions of all worldspaces - instead look up region by given name: */         
            auto* form = RE::TESForm::LookupByEditorID(a_regionName); // LookupByEditorID() only works with powerofthree's Tweaks ("Load EditorID" tweak)
            auto* region = form ? form->As<RE::TESRegion>() : nullptr;

            if (region && ((region->GetFormFlags() & RE::TESRegion::RecordFlags::kBorderRegion) != 0)) { 
                // && region->worldSpace == a_worldSpace
log::info("{}: Worldspace {} - BorderRegion: {} / {}", __FUNCTION__, region->worldSpace->GetFullName(), region->GetFormEditorID(), region->GetFormID());
                
                sealevel= region->worldSpace->GetDefaultWaterHeight();

                auto* regionPoints  = region->pointLists;
                if (!regionPoints) {
                    log::info("{}: region pointLists is null", __FUNCTION__);
                } else {
                    for (const auto& pointList : *regionPoints) {
                        if (pointList) {
                            log::info("{}: worldspace: {}, PointList minimums: x = {}, y = {}, maximums: x = {}, y = {}", 
                                    __FUNCTION__, region->worldSpace->GetFullName(), pointList->minimums.x, pointList->minimums.y, pointList->maximums.x, pointList->maximums.y);

                            min_x = pointList->minimums.x - 10000.0f;
                            min_y = pointList->minimums.y - 10000.0f;
                            max_x = pointList->maximums.x + 10000.0f;
                            max_y = pointList->maximums.y + 10000.0f;

                            int iCount = 0;
                            float sumX = 0.0f;
                            float sumY = 0.0f;
                            for (const auto& regionPoint : *pointList) {
                                if (regionPoint) {
                                    sumX += regionPoint->point.x;
                                    sumY += regionPoint->point.y;
                                    iCount++;
                                }
                            }
                            if (iCount > 0) {
                                log::info("{}: region FormID {}: avg-center ({}, {}), min/max-center ({}, {})", __FUNCTION__, 
                                    region->GetFormID(), sumX / iCount, sumY / iCount, 
                                    (pointList->minimums.x + pointList->maximums.x)/2.0f, (pointList->minimums.y + pointList->maximums.y)/2.0f);

                                center_x = sumX / iCount;
                                center_y = sumY / iCount;
                            } else {
                                log::info("{}: PointList has no points", __FUNCTION__);
                            }
                        }
                    }
                }
            } else {
                log::warn("{}: Form is not a TESRegion or is not a BorderRegion", __FUNCTION__);
            }
//        }  end scan through all regions - currently not used.
        m_minX = min_x;
        m_maxX = max_x;
        m_minY = min_y;
        m_maxY = max_y;
        m_seaLevel = sealevel;
        m_borderRegionName = a_regionName;
        m_centerX = center_x;
        m_centerY = center_y;
    }

    FlyingModeManager::WorldSpaceData::WorldSpaceData(const RE::TESWorldSpace* a_worldSpace) {
        if (!a_worldSpace) {
            log::error("{}: Worldspace is null", __FUNCTION__);
            return;
        }
        
        std::string worldspace_EDID = clib_util::editorID::get_editorID(a_worldSpace);

        // The Border region of the worldSpace MUST be completely within the bounding box defined by fMinX, fMaxX, fMinY, fMaxY
        if (strcmp(worldspace_EDID.c_str(), "Tamriel") == 0) {
            m_minX = -220000;
            m_maxX = 240000;
            m_minY = -150000;
            m_maxY = 200000;
            m_seaLevel = -14000;
            m_borderRegionName = "BorderRegionSkyrim";
        }  else if (strcmp(worldspace_EDID.c_str(), "DLC2SolstheimWorld") == 0) {
            m_minX = -20000;
            m_maxX = 140000;
            m_minY = -20000;
            m_maxY = 140000;
            m_seaLevel = 0;
            m_borderRegionName = "DLC2SolstheimBorderRegion";
        } else {
            log::error("{}: Trying to use dragon fast travel in invalid worldspace: {}!", __FUNCTION__, worldspace_EDID.c_str());
       }
    }

    bool FlyingModeManager::IsInBorderRegion() const {

        auto player = RE::PlayerCharacter::GetSingleton();
        WorldSpaceData worldSpaceData; 
        worldSpaceData = WorldSpaceData(player->GetWorldspace()); 

        bool isInBorderRegion = _ts_SKSEFunctions::IsPlayerInRegion(worldSpaceData.m_borderRegionName);

        if (!isInBorderRegion && worldSpaceData.m_borderRegionName == "BorderRegionSkyrim") {
            // if in Worldspace Tamriel / Skyrim, also check for Castle Volkihar
            isInBorderRegion = _ts_SKSEFunctions::IsPlayerInRegion("DLC1BorderRegionSkyrim");
        }

        return isInBorderRegion;
    }

    float FlyingModeManager::NormalizeAngle(float a_angle){
        // Normalize angle
        float angleNorm = a_angle;
        if (angleNorm < 0.0f) {
            angleNorm += 2.0f *PI;
        } else if (angleNorm > 2.0f *PI) {
            angleNorm -= 2.0f *PI;
        }
        return angleNorm;
    }
} // namespace IDRC
