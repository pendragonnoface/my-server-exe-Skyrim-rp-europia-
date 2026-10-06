#include "FastTravelManager.h"
#include "_ts_SKSEFunctions.h"
#include "DataManager.h"
#include "IDRCUtils.h"

#include "RE/Skyrim.h"
#include "SKSE/API.h"


namespace IDRC {

    void FastTravelManager::Update() {
        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return;
        }
                
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            return;
        }

        if (Utils::IsFastTravelActive()) {
            // in dragon-FastTravel mode - check if dragon is allowed to fly
            if (!dragonActor->AsActorState()->actorState2.allowFlying) {
                log::info("{}: in FastTravel mode, but not allowed to fly - stopping fast travel...", __FUNCTION__);

                auto* orbitMarker = DataManager::GetSingleton().GetOrbitMarker();                
                SKSE::GetTaskInterface()->AddTask([dragonActor, orbitMarker]() {
                    // When modifying Game objects, send task to TaskInterface to ensure thread safety
                    if (orbitMarker) {
                        _ts_SKSEFunctions::MoveTo(orbitMarker, dragonActor, 0.0f, 0.0f,  0.0f);
                    } else {
                        log::warn("{}: Orbit marker is null", __FUNCTION__);
                    }

                    // force stop fasttravel via StopFastTravel Package
                    dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 0); // orbit
                    dragonActor->EvaluatePackage();
                });
            }
        } else if (m_lastFastTravelState) {
            // leaving FastTravel mode
            auto& flyingModeManager = FlyingModeManager::GetSingleton();
            if (!dragonActor->AsActorState()->actorState2.allowFlying) {
                log::info("{}: Leaving FastTravel and not allowed to fly - trigger land", __FUNCTION__);
                if (flyingModeManager.GetRegisteredForLanding()) {
                    if (!flyingModeManager.GetLandingPosSearchOngoing()) {
                        flyingModeManager.TriggerLand();
                    }
                } else {
                    std::thread([dragonActor]() {
                        // send to new thread so that DragonLandPlayerRiding() is not blocking Update()
                        FlyingModeManager::GetSingleton().DragonLandPlayerRiding(dragonActor);
                    }).detach();
                }
            }
        }

        m_lastFastTravelState = Utils::IsFastTravelActive();

        if (*g_PatrolQueuedState == 1 || *g_PatrolQueuedState == 4) {
            *g_PatrolQueuedState = 0;
            *g_FastTravelState = true;
        }
    }


    void FastTravelManager::FastTravel(const RE::TESObjectREFR* a_fastTravelTarget) {
//        log::info("IDRC - {}", __FUNCTION__);
        if (!a_fastTravelTarget) {
            log::error("{}: error - FastTravelTarget is none", __FUNCTION__);
            return;
        } 
        
        auto dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: error - dragonActor is none", __FUNCTION__);
            return;
        }

        if (m_skipFastTravelRequest) {
            // Set in Hooks::PathingHook::UpdateFlightPathData() to skip FastTravel (ExecuteTeleport) requests
            // while pathData is still being updated from the previous FastTravel request.
            // FastTravel is setting pathData to nullptr until new pathData is generated.
            // This can take a few frames (in particular if many pathing requests occur in parallel, eg during combat).
            // Skipping FastTravel in this case avoids continuous nullifying of pathData.

            log::info("{}: FastTravel is skipped", __FUNCTION__);
            return;
        }

        if (!dragonActor->AsActorState()->actorState2.allowFlying) {
            log::info("{}: not allowed to fly - cancelling fast travel request", __FUNCTION__);

            if (dragonActor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kVariable03) == 2) {
                // fast travel active - force stop fasttravel via StopFastTravel Package
                SKSE::GetTaskInterface()->AddTask([dragonActor]() {
                    dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 0); // orbit
                });
            }
            return;
        }

        // Block immediately subsequent FastTravel requests to prevent multiple 
        // concurrent pathing requests for the dragon,
        // until this FastTravel request has successfully triggered UpdatePathData() with valid pathData.
        // Unclear if this is really needed but should prevent  game freeze observed in such a situation.
        // UpdateFlightPathData() will clear this flag once valid pathData is available.
        m_skipFastTravelRequest = true;

        SKSE::GetTaskInterface()->AddTask([a_fastTravelTarget, dragonActor]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player) {
                log::warn("{}: Could not get player", __FUNCTION__);
                return;
            }

            auto* tes = RE::TES::GetSingleton();
            if (!tes) {
                log::warn("{}: Cannot access TES", __FUNCTION__);
                return;
            }
            auto* worldspace = tes->GetRuntimeData2().worldSpace;
            if (!worldspace) {
                log::warn("{}: Cannot access worldspace", __FUNCTION__);
                return;
            }
/*
            // trigger fast travel to a_fastTravelTarget in next frame 
            auto& data = player->GetPlayerRuntimeData();
            data.queuedTargetLoc.world         = worldspace;
            data.queuedTargetLoc.interior      = nullptr;
            data.queuedTargetLoc.location      = a_fastTravelTarget->GetPosition();
            data.queuedTargetLoc.angle         = a_fastTravelTarget->GetAngle();
            data.queuedTargetLoc.arrivalFunc   = nullptr;
            data.queuedTargetLoc.arrivalFuncData = 0;
            data.queuedTargetLoc.furnitureRef  = RE::RefHandle{};
            GetRefHandle(a_fastTravelTarget, &data.queuedTargetLoc.fastTravelMarker);
            data.queuedTargetLoc.resetWeather  = false;
            data.queuedTargetLoc.allowAutoSave = false;
            data.queuedTargetLoc.isValid       = true;  // triggers the fast travel on next per-frame ExecuteTeleport call
*/
// Previous solution: call the Papyrus function Game.FastTravel(). 
// This works as well and is an alternative to setting the loc values.
            _ts_SKSEFunctions::CallPapyrusFunction("Game"sv, "FastTravel"sv, a_fastTravelTarget);

            // trigger fasttravel package
            dragonActor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 2);
            dragonActor->EvaluatePackage();
        });
    }
} // namespace IDRC
