#include "CombatTargetTracer.h"
#include "DataManager.h"
#include "_ts_SKSEFunctions.h"

namespace IDRC {

    RE::BSEventNotifyControl CombatTargetTracer::ProcessEvent(const RE::TESCombatEvent*  a_event, RE::BSTEventSource<RE::TESCombatEvent>*) {
        if (!a_event ) {
            return RE::BSEventNotifyControl::kContinue;
        }

        RE::Actor* dragonActor = DataManager::GetSingleton().GetDragonActor();

        if (!dragonActor) {
            log::warn("{}: Dragon actor is nullptr", __FUNCTION__);
            return RE::BSEventNotifyControl::kContinue;
        }

        int combatState = _ts_SKSEFunctions::GetCombatState(dragonActor);
        RE::Actor* currentCombatTarget = dragonActor->GetActorRuntimeData().currentCombatTarget.get().get();

        if (currentCombatTarget != m_combatTarget) {
            if (m_combatTarget) {
                log::info("{}: Previous CombatTarget: {} ({} - {})", __FUNCTION__, m_combatTarget->GetName(),  m_combatTarget->GetFormID(),  m_combatTarget->GetHandle().native_handle());
            } else {
                log::info("{}: No previous CombatTarget", __FUNCTION__);
            }
            m_combatTarget = currentCombatTarget;
        }
 
        if (combatState != m_combatState) {
            m_combatState = combatState;
            log::info("{}: Combat state changed to: {}", __FUNCTION__, m_combatState);
        }

        if (m_combatTarget) {
            log::info("{}: Current CombatTarget: {} ({})", __FUNCTION__, m_combatTarget->GetName(),  m_combatTarget->GetFormID());
        }
 
        return RE::BSEventNotifyControl::kContinue;
    }

    bool CombatTargetTracer::Register() {
        if (!m_isRegistered) {

            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESCombatEvent>(this);
            m_isRegistered = true;

            RE::Actor* dragonActor = DataManager::GetSingleton().GetDragonActor();
            if (!dragonActor) {
                log::warn("{}: Dragon actor is nullptr", __FUNCTION__);
                m_combatState = 0;
                m_combatTarget = nullptr;
                return true;
            }
            if (!m_breathHitShader) {
                log::warn("{}: m_breathHitShader is nullptr", __FUNCTION__);
                m_combatState = 0;
                m_combatTarget = nullptr;
                return true;
            }

            m_combatState = _ts_SKSEFunctions::GetCombatState(dragonActor);
            m_combatTarget = dragonActor->GetActorRuntimeData().currentCombatTarget.get().get();

            if (m_combatState > 0 && m_combatTarget) {
                SKSE::GetTaskInterface()->AddTask([this]() {
                // When modifying Game objects, send task to TaskInterface to ensure thread safety
                    this->m_combatTarget->ApplyEffectShader(this->m_breathHitShader);
                });
                log::info("{}: Applied shader to target {} ({})", __FUNCTION__, m_combatTarget->GetName(),  m_combatTarget->GetFormID());
            }
            log::info("{}: registered CombatTargetTracer", __FUNCTION__);
        } else {
            log::warn("{}: CombatTargetTracer already registered", __FUNCTION__);
        }
        return true;
    }

    bool CombatTargetTracer::Unregister() {
        if (m_isRegistered) {
            RE::ScriptEventSourceHolder::GetSingleton()->RemoveEventSink<RE::TESCombatEvent>(this);
            
            if (m_combatTarget) {
                SKSE::GetTaskInterface()->AddTask([this]() {
                    // When modifying Game objects, send task to TaskInterface to ensure thread safety
                    this->m_combatTarget->ApplyEffectShader(nullptr);
                });
                log::info("{}: Removed shader from target {} ({})", __FUNCTION__, m_combatTarget->GetName(),  m_combatTarget->GetFormID());
                m_combatTarget = nullptr;
            }

            m_combatState = 0;

            m_isRegistered = false;
            log::info("{}: Unregistered CombatTargetTracer", __FUNCTION__);

        } else {
            log::warn("{}: CombatTargetTracer was not registered", __FUNCTION__);
        }
        return true;
    }

    void CombatTargetTracer::InitializeData(RE::TESEffectShader* a_breathHitShader) {
        log::info("IDRC - {}", __FUNCTION__);
        m_breathHitShader = a_breathHitShader;
    }

    RE::Actor* CombatTargetTracer::GetCombatTarget() const {
        return m_combatTarget;
    }
} // namespace IDRC
