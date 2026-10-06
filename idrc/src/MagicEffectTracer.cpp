#include "MagicEffectTracer.h"
#include "DataManager.h"
#include "_ts_SKSEFunctions.h"

namespace IDRC {

    RE::BSEventNotifyControl MagicEffectTracer::ProcessEvent(const RE::TESMagicEffectApplyEvent*  a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*) {
        if (!a_event ) {
            return RE::BSEventNotifyControl::kContinue;
        }
        if (!m_spell) {
            log::info("{}: no spell defined for tracing...", __FUNCTION__);
            return RE::BSEventNotifyControl::kContinue;
        }

        for (const auto* effect : m_spell->effects) {
            if (effect && effect->baseEffect && effect->baseEffect->GetFormID() == a_event->magicEffect) {

                if (a_event->target && a_event->target.get()) {
                        log::info("{}: MagicEffect {} applied to {}", __FUNCTION__, m_spell->GetName(), a_event->target.get()->GetName());
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    bool MagicEffectTracer::Register() {
        if (!m_isRegistered) {

            RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESMagicEffectApplyEvent>(this);
            m_isRegistered = true;
            log::info("{}: Registered MagicEffectTracer", __FUNCTION__);
        } else {
            log::warn("{}: MagicEffectTracer already registered", __FUNCTION__);
        }
        return true;
    }

    bool MagicEffectTracer::Unregister() {
        if (m_isRegistered) {
            RE::ScriptEventSourceHolder::GetSingleton()->RemoveEventSink<RE::TESMagicEffectApplyEvent>(this);
            
//            m_spell = nullptr;
            m_isRegistered = false;
            log::info("{}: Unregistered MagicEffectTracer", __FUNCTION__);
        } else {
            log::warn("{}: MagicEffectTracer was not registered", __FUNCTION__);
        }
        return true;
    }

    void MagicEffectTracer::InitializeData(RE::SpellItem* a_spell) {
        m_spell = a_spell;
        m_isRegistered = false;
    }
} // namespace IDRC
