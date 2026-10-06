#include "DataManager.h"
#include "_ts_SKSEFunctions.h"
#include "IDRCUtils.h"
#include "FlyingModeManager.h"
#include "EuropiaBridge.h"

namespace IDRC {
    RE::TESObjectREFR* DataManager::GetOrbitMarker() {
        return m_orbitMarker;
    }

    RE::Actor* DataManager::GetDragonActor() {
        // Europia: once the bridge is in use, only report a dragon while the player rides it.
        if (EuropiaBridge::IsInUse()) {
            if (!EuropiaBridge::IsActive()) {
                return nullptr;
            }
            if (m_dragonAlias) {
                if (auto* aliasActor = m_dragonAlias->GetActorReference()) {
                    return aliasActor;
                }
            }
            return EuropiaBridge::GetDirectDragon();
        }
        if (m_dragonAlias) {
            // Note: if the alias does not contain an actor (ie another form), this will crash:
            return m_dragonAlias->GetActorReference();
        }
        return nullptr;
    }

    void DataManager::InitializeData(RE::TESQuest* a_rideQuest,
                                    RE::TESObjectREFR* a_orbitMarker,
                                    RE::BGSRefAlias* a_dragonAlias,
                                    std::string a_dragonName,
                                    RE::TESQuest* a_findPerchQuest,
                                    RE::BGSRefAlias* a_wordWallPerch, 
                                    RE::BGSRefAlias* a_towerPerch,
                                    RE::BGSRefAlias* a_rockPerch,
                                    RE::BGSRefAlias* a_perchTarget) {
        m_rideQuest = a_rideQuest;
        m_orbitMarker = a_orbitMarker;
        m_dragonAlias = a_dragonAlias;
        m_dragonName = a_dragonName;
        m_findPerchQuest = a_findPerchQuest;
        m_wordWallPerch = a_wordWallPerch;
        m_towerPerch = a_towerPerch;
        m_rockPerch = a_rockPerch;
        m_perchTarget = a_perchTarget;
    }

    RE::TESObjectREFR* DataManager::GetWordWallPerch() {
        if (m_wordWallPerch) {
            return m_wordWallPerch->GetReference();
        }
        return nullptr;
    }

    RE::TESObjectREFR* DataManager::GetTowerPerch() {
        if (m_towerPerch) {
            return m_towerPerch->GetReference();
        }
        return nullptr;
    }

    RE::TESObjectREFR* DataManager::GetRockPerch() {
        if (m_rockPerch) {
            return m_rockPerch->GetReference();
        }
        return nullptr;
    }

    RE::TESQuest* DataManager::GetFindPerchQuest() {
        return m_findPerchQuest;
    }

    void DataManager::SetPerchTarget(RE::TESObjectREFR* a_target) {
        RE::Actor* dragonActor = GetDragonActor();
        SKSE::GetTaskInterface()->AddTask([this, dragonActor, a_target]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            this->m_perchTarget->ForceRefTo(a_target);
            dragonActor->EvaluatePackage();
        });        
    }

    RE::TESQuest* DataManager::GetRideQuest() {
        return m_rideQuest;
    }

    bool DataManager::GetAutoCombat() {
        return m_autoCombat;
    }

    void DataManager::SetAutoCombat(bool a_auto) {
        m_autoCombat = a_auto;
        SendPropertyUpdateEvent("AutoCombat", m_autoCombat, 0.0f, 0);
    }

    void DataManager::ToggleAutoCombat() {
        m_autoCombat = !m_autoCombat;
        SendPropertyUpdateEvent("AutoCombat", m_autoCombat, 0.0f, 0);
    }


    std::string DataManager::GetDragonName() {
        return m_dragonName;
    }
        
    void DataManager::SendPropertyUpdateEvent(std::string a_propertyName, bool a_bValue, float a_fValue, int a_iValue) {
        SKSE::GetTaskInterface()->AddTask([this, a_propertyName, a_bValue, a_fValue, a_iValue]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
            if (vm) {
                auto* args = RE::MakeFunctionArguments(std::string(a_propertyName), bool(a_bValue), float(a_fValue), int(a_iValue));
                auto handle = _ts_SKSEFunctions::GetHandle(this->GetRideQuest());
                if (handle) {
                    vm->SendEvent(handle, "OnProperyUpdate_SKSE", args);
                } else {
                    log::error("{}: ERROR - invalid handle ({}, {})", "SendPropertyUpdateEvent", a_propertyName, handle);
                }
            }
        });
    }

    float DataManager::GetRollAmplitude() {
        return m_rollAmplitude;
    }

    void DataManager::SetRollAmplitude(float a_rollAmplitude) {
        log::info("{}: {}", __FUNCTION__, a_rollAmplitude);
        m_rollAmplitude = a_rollAmplitude;
    }

    float DataManager::GetRollSmoothTime() {
        return m_rollSmoothTime;
    }

    void DataManager::SetRollSmoothTime(float a_rollSmoothTime) {
        log::info("{}: {}", __FUNCTION__, a_rollSmoothTime);
        m_rollSmoothTime = a_rollSmoothTime;
    }

    void DataManager::SetDragonSpeeds(float a_speedMult) {
        log::info("{}: Multiplier = {}", __FUNCTION__, a_speedMult);

        float mult = a_speedMult;
        if (mult < 0.5f || mult > 2.0f) {
            log::warn("{}: Speed multiplier out of range. Resetting to default (1.0)", __FUNCTION__);
            mult = 1.0f;
        }
        SKSE::GetTaskInterface()->AddTask([this, mult]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            _ts_SKSEFunctions::UpdateIniSetting("fPlayerFlyingMountBaseTargetSpeed:General", this->m_baseSpeed * mult);
            _ts_SKSEFunctions::UpdateIniSetting("fPlayerFlyingMountFastBaseTargetSpeed:General",  this->m_fastBaseSpeed * mult);
        });
    }

}  // namespace IDRC
