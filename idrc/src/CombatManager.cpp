#include "CombatManager.h"
#include "_ts_SKSEFunctions.h"
#include "DataManager.h"
#include "DisplayManager.h"
#include "FlyingModeManager.h"
#include "ControlsManager.h"
#include "IDRCUtils.h"
#include "APIManager.h"
#include "FastTravelManager.h"
#include "TargetReticleManager.h"
#include "IDRCUtils.h"

#include <random>

namespace IDRC {   

    void CombatManager::InitializeData(RE::TESShout* a_unrelentingForceShout,
                                RE::TESShout* a_attackShout) {
        log::info("IDRC - {}", __FUNCTION__);
        m_unrelentingForceShout = a_unrelentingForceShout;
        m_attackShout = a_attackShout;
    }

    float CombatManager::GetMaxTargetDistance() {
        return m_maxTargetDistance;
    }

    bool CombatManager::IsValidTarget(RE::Actor* a_target) {
        if (!a_target) {
            return false;
        }

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            return false;
        }

        TargetReticleManager& targetReticleManager = TargetReticleManager::GetSingleton();

        if (Utils::GetHorizontalDistance(dragonActor, a_target) < targetReticleManager.GetMaxTargetDistance(a_target) &&
            a_target->GetParentCell() && a_target->GetParentCell()->IsAttached() &&
            !a_target->IsDead()) {
            return true;
        }

        return false;
    }

    void CombatManager::DragonStartCombat(RE::Actor* a_target) {
        if (!a_target) {
            log::error("{}: target is null", __FUNCTION__);
            return;
        }

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return;
        }

        if (!IsValidTarget(a_target)) {
            log::info("{}: Target is too far away or dead, cancel DragonStartCombat", __FUNCTION__);
            return;
        }
        
        if (dragonActor->IsInCombat()) {
            SKSE::GetTaskInterface()->AddTask([dragonActor, a_target]() {
                // When modifying Game objects, send task to TaskInterface to ensure thread safety

                // This just puts the new target on top of the combat target stack
                // need to call StartCombat() (below) to actually make the dragon attack the now target
                _ts_SKSEFunctions::UpdateCombatTarget(dragonActor, a_target);
            });
        }

        StartCombat(dragonActor, a_target, dragonActor->GetCombatGroup());
    }
    
    void CombatManager::Update() {
        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return;
        }

        if (!DataManager::GetSingleton().GetDragonActor()) {
            return;
        }

        UpdateCombat();

        UpdateAttack();
    }

    void CombatManager::UpdateCombat() {

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            return;
        }

        // restart combat with stored target when dragon leaves flying state.
        if (!Utils::IsFastTravelActive()) {
            if (m_restartCombatPending)
            {
                m_restartCombatPending = false;            

                auto* storedTarget = m_storedCombatTarget ? m_storedCombatTarget.get().get() : nullptr;
                if (storedTarget && !dragonActor->IsInCombat()) {
                    log::info("{}: Restarting combat with stored target {}", __FUNCTION__, storedTarget->GetName());
                    DragonStartCombat(storedTarget);
                }
            }

            auto* combatTarget = _ts_SKSEFunctions::GetCombatTarget(dragonActor);
            m_storedCombatTarget = combatTarget ? combatTarget->GetHandle() : RE::ActorHandle{};

            if (combatTarget) {
                m_storedCombatTargetState = _ts_SKSEFunctions::GetCombatState(dragonActor);
            } else {
                m_storedCombatTargetState = 0;
            }
        } else {
            m_restartCombatPending = true;

 //           UpdatePlayerCell();

            if (m_shoutTarget) {
                m_storedCombatTarget = m_shoutTarget;
                m_storedCombatTargetState = 1;
            }

            if (m_storedCombatTarget) {
                auto* storedTarget = m_storedCombatTarget.get().get();
                if (!IsValidTarget(storedTarget)) {
                    // clear stored combat target
                    m_storedCombatTarget = RE::ActorHandle{};
                    m_storedCombatTargetState = 0;
                }
            }
        }

        // Clear combat targets (ie stop combat) if the dragon is too far from the combat target
        if (dragonActor->IsInCombat()) {
            auto* combatTarget = _ts_SKSEFunctions::GetCombatTarget(dragonActor);
    
            if (combatTarget && !IsValidTarget(combatTarget)){
                // In case the dragon is in combat, the game's 3D data  is centered around the dragon's combat target (ie the combat target's cell and its 8 adjacent cells).
                // If the mounted dragon is in a different cell in the landscape, 
                // the 3D area of the player's/dragon's location can get unloaded (player gets "detached" from cell).
                // This then causes a lot of issues, like:
                // 		skyshots, grey screen, "You cannot go that way", frozen player in mid-air (while dragon flys on), 
                // 		teleported dragon, disappearing dragon, only LOD landscape visible, crashes, etc
                // To prevent this, we clear combat targets (stop combat).
                SKSE::GetTaskInterface()->AddTask([dragonActor]() {
                    // When modifying Game objects, send task to TaskInterface to ensure thread safety
                _ts_SKSEFunctions::ClearCombatTargets(dragonActor);
                });
            }
        }
    }
/* unused
    constexpr float CELL_SIZE = 4096.0f;

    void CombatManager::UpdatePlayerCell() { 
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            log::warn("{}: Cannot access player character", __FUNCTION__);
            return;
        } 

        auto* tes = RE::TES::GetSingleton();
        if (!tes) {
            log::warn("{}: Cannot access TES", __FUNCTION__);
            return;
        }

        RE::TESObjectCELL* cell = nullptr;
        auto* worldspace = tes->GetRuntimeData2().worldSpace;
        if (!worldspace) {
            log::warn("{}: Cannot access worldspace", __FUNCTION__);
            return;
        }

        auto playerPos = player->GetPosition();

        // Calculate cell coordinates from position
        std::int16_t targetCellX = static_cast<std::int16_t>(std::floor(playerPos.x / CELL_SIZE));
        std::int16_t targetCellY = static_cast<std::int16_t>(std::floor(playerPos.y / CELL_SIZE));
        
        RE::CellID cellID(targetCellY, targetCellX);

        // First check if cell is already in the cellMap
        const auto& map = worldspace->cellMap;
        const auto it = map.find(cellID);
        if (it != map.end()) {
            cell = it->second;
        }

        // If not in map, load it using TESWorldSpace_LoadCell
        if (!cell) {
            bool loadFromDisk;
            cell = _ts_SKSEFunctions::GetCell(targetCellX, targetCellY, worldspace, loadFromDisk);
        }

        if (cell) {
            player->SetParentCell(cell);
log::info("{}: Updated player cell to {}, {}", __FUNCTION__, targetCellX, targetCellY);
        } else {
            log::error("{}: No cell!", __FUNCTION__);
        }
    }
*/
    void CombatManager::UpdateAttack() {
        bool attackStopped = false;
        if (m_shoutTimer > 3.0f) {
            m_shoutActive = false;
            attackStopped = true;
        }
        
        m_shoutTimer += *g_deltaTimeRealTime;

        if (!m_shoutActive) {
            m_shoutTarget = RE::ActorHandle{};
            m_shoutTimer = 0.0f;
        if (attackStopped) {
            auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
            if (dragonActor) {
                SKSE::GetTaskInterface()->AddTask([dragonActor]() {
                    // When modifying Game objects, send task to TaskInterface to ensure thread safety
                    _ts_SKSEFunctions::ClearLookAt(dragonActor);
                    });
                }
            }
        }          
    }
    
    bool CombatManager::DragonAttack(bool a_alternateAttack)
    {
        log::info("IDRC - {}", __FUNCTION__);
        auto& dataManager = DataManager::GetSingleton();
        auto* dragonActor = dataManager.GetDragonActor();
        auto& controlsManager = ControlsManager::GetSingleton();
        auto& flyingModeManager = FlyingModeManager::GetSingleton();
        auto& displayManager = DisplayManager::GetSingleton();

        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return false;
        }

        //  no attack while perch is triggered
        if (flyingModeManager.GetRegisteredForPerch()) {
            log::info("{}: Dragon is perching, attack canceled", __FUNCTION__);
            return false;
        }

        if (m_shoutActive) {
            log::info("{}: Shout is ongoing, canceling new attack", __FUNCTION__);
            return false;
        }

        bool isAlternateAttack = a_alternateAttack;
        m_shoutTimer = 0.0f;
        m_shoutActive = true;
                 
        m_shoutTarget = RE::ActorHandle{};
        if (APIs::TrueDirectionalMovementV1 && APIs::TrueDirectionalMovementV1->GetTargetLockState()
            && !TargetReticleManager::GetSingleton().IsReticleLocked()) {
            auto currentTDMTarget = APIs::TrueDirectionalMovementV1->GetCurrentTarget();
            if (currentTDMTarget) {
                m_shoutTarget = currentTDMTarget;
            }
        }

        if (!m_shoutTarget) {
            // No TDM target, get the combat target from the reticle (if active)
            if (auto handle = TargetReticleManager::GetSingleton().GetCurrentTarget()) {
                m_shoutTarget = handle;
            }
        }
        auto* resolvedShoutTarget = m_shoutTarget ? m_shoutTarget.get().get() : nullptr;
        RE::Actor* currentCombatTarget = _ts_SKSEFunctions::GetCombatTarget(dragonActor);

        if (!m_shoutTarget) {
            // if no target from TDM or Reticle, use current combat target (if any)
            m_shoutTarget = currentCombatTarget ? currentCombatTarget->GetHandle() : RE::ActorHandle{};
            resolvedShoutTarget = currentCombatTarget;
        } else if (resolvedShoutTarget != currentCombatTarget) {
            DragonStartCombat(resolvedShoutTarget);
        }

        m_shoutDirection = 1.f; // pass target on the left for this attack
        if (_ts_SKSEFunctions::GetRandomFloat(0.0f, 1.0f) < 0.5f) {
            m_shoutDirection = -1.f; // pass target on the right for this attack
        }
        
        if (Utils::IsFastTravelActive()){
            // use stored combat target during flying (fastTraveling)
            // (in FastTravel mode, dragon's combat state is always 0)
            m_isFastTravelAttack = true;

            if (!resolvedShoutTarget) {
                resolvedShoutTarget = m_storedCombatTarget ? m_storedCombatTarget.get().get() : nullptr;
                m_shoutTarget = m_storedCombatTarget;
            }
        }

        while (controlsManager.GetIsKeyPressed(IDRCKey::kSneak)) {
            _ts_SKSEFunctions::WaitWhileGameIsPaused();

            if (controlsManager.GetIsKeyPressed(IDRCKey::kRun)) {
                isAlternateAttack = true;
            }
            m_shoutTimer = 0.0f;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::string displayMessage = "Commanding Attack";
        if (resolvedShoutTarget) {
            // Display attack notification
            displayMessage += " on ";
            displayMessage += std::string(resolvedShoutTarget->GetName());
        }

        float distanceMultiplier = 1.f;
        if (_ts_SKSEFunctions::GetFlyingState(dragonActor) == 2) {
            distanceMultiplier = 2.0f;
        }

        bool useUnrelentingForce = dragonActor->HasShout(m_unrelentingForceShout) && (isAlternateAttack || controlsManager.GetIsKeyPressed(IDRCKey::kRun));
        float targetDistance = resolvedShoutTarget ? _ts_SKSEFunctions::GetDistance(dragonActor, resolvedShoutTarget) : 2000.0f;
        SetActiveShout(targetDistance, useUnrelentingForce);

        if (m_attackShout == nullptr || m_attackShout->variations[0].spell == nullptr) {
            log::error("{}: Error: No Shout Found", __FUNCTION__);            
            return false;
        }

        if (displayManager.GetDisplayAttackMessage()) {
            RE::SendHUDMessage::ShowHUDMessage(displayMessage.c_str());
        }

        ExecuteAttack();

        return true;
    }

    void CombatManager::ExecuteAttack() {
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return;
        }

        SKSE::GetTaskInterface()->AddTask([this, dragonActor]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
            auto* taskShoutTarget = this->m_shoutTarget ? this->m_shoutTarget.get().get() : nullptr;
            if (taskShoutTarget) {
                _ts_SKSEFunctions::SetLookAt(dragonActor, taskShoutTarget, true);
            }
            StartVoiceShoutCast(static_cast<RE::Character*>(dragonActor), this->m_attackShout, 2, taskShoutTarget);
        });
    }

    RE::TESShout* CombatManager::GetShout(float a_targetDistance) {
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return nullptr;
        }

        auto* actorBase = dragonActor->GetActorBase();
        auto* spellList = actorBase ? actorBase->GetSpellList() : nullptr;
        if (!spellList || !spellList->shouts) {
            log::info("{}: Dragon has no shout list", __FUNCTION__);
            return nullptr;
        }

        RE::TESShout* candidateShout = nullptr;
        RE::TESShout* selectedShout = nullptr;
        float bestDistance = -1.0f;
        std::vector<std::uint32_t> selectableShoutIndices;

        for (std::uint32_t i = spellList->numShouts; i-- > 0;) {
            candidateShout = spellList->shouts[i];
            if (candidateShout && dragonActor->HasShout(candidateShout) && candidateShout != m_unrelentingForceShout) {
                if (std::size(candidateShout->variations) > 0) {
                    auto* spell = candidateShout->variations[0].spell;
                    if (!spell) {
                        continue;
                    }

                    float maxProjectileRange = -1.0f;
                    for (auto* effect : spell->effects) {
                        auto* magicEffect = effect ? effect->baseEffect : nullptr;
                        auto* projectile = magicEffect ? magicEffect->data.projectileBase : nullptr;
                        if (projectile) {
                            float projectileRange = projectile->data.range;
                            if (projectileRange > maxProjectileRange) {
                                maxProjectileRange = projectileRange;
                            }
                        }
                    }

                    if (  selectableShoutIndices.size() == 0 ||
                          ( maxProjectileRange >= a_targetDistance && (bestDistance < a_targetDistance || maxProjectileRange <= bestDistance) ) ||
                          ( maxProjectileRange < a_targetDistance && maxProjectileRange >= bestDistance )
                       ) {
                        // best distance: as close as possible to the target distance (with priority on larger than target distance)

                        if (bestDistance != maxProjectileRange) {
                            // new best distance, clear previous selectable shouts
                            selectableShoutIndices.clear();
                        }

                        selectableShoutIndices.push_back(i);
                        bestDistance = maxProjectileRange;
                    }
                }
            }

            if (!selectableShoutIndices.empty()) {
                int chosenIndex = selectableShoutIndices[0];

                auto numberofSelectableShouts = selectableShoutIndices.size();
                if (numberofSelectableShouts > 1) { // more than one shout with same best distance
                    // pick a random index from the selectable shouts
                    float randomValue = _ts_SKSEFunctions::GetRandomFloat(0.0f, 1.0f);
                    chosenIndex = selectableShoutIndices[static_cast<int>(randomValue * numberofSelectableShouts)];
                }

                selectedShout = spellList->shouts[chosenIndex];
            }
        }

        return selectedShout;
    }

    bool CombatManager::SetActiveShout(float a_targetDistance, bool a_useUnrelentingForce) {
        log::info("{}: {}, {}", __FUNCTION__, a_targetDistance, a_useUnrelentingForce);
    
        RE::TESShout* usedShout = nullptr;

        if (a_useUnrelentingForce && m_unrelentingForceShout) {
            usedShout = m_unrelentingForceShout;
        } else {
            usedShout = GetShout(a_targetDistance);
        }

        if (!usedShout) {
            log::error("{}: No Shout found!", __FUNCTION__);
            return false;
        }
        if (m_attackShout->variations && m_attackShout->variations[0].spell) {
            log::info("{}: Old AttackShout-spell0: {}", __FUNCTION__, m_attackShout->variations[0].spell->GetFormID());
        } else {
            log::info("{}: Old AttackShout-spell0 is null", __FUNCTION__);
        }

        for (int i = 0; i < 3; ++i) {
            auto* word = usedShout->variations[i].word;
            auto* spell = usedShout->variations[i].spell;
    
            SKSE::GetTaskInterface()->AddTask([this, word, spell, i]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
                if (word) {
                    this->m_attackShout->variations[i].word = word;
                }
                if (spell) {
                    this->m_attackShout->variations[i].spell = spell;
                } 
            });

        }
        int count = 0;
        while (count < 100 && m_attackShout->variations[0].spell != usedShout->variations[0].spell) {
            // wait until the spell is updated
            // TODO: is there a better way???
            _ts_SKSEFunctions::WaitWhileGameIsPaused();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            count++;
        }
        if (count >= 100) { // waited > 1sec
            log::error("{}: ERROR - Timed out while waiting for AttackShout to sync!", __FUNCTION__);
            return false;
        }

        if (m_attackShout->variations && m_attackShout->variations[0].spell) {
            log::info("{}: New AttackShout-spell0: {}", __FUNCTION__, m_attackShout->variations[0].spell->GetFormID());
        } else {
            log::info("{}: New AttackShout-spell0 is null", __FUNCTION__);
        }
    
        return true;
    }
} // namespace IDRC

