#pragma once


namespace IDRC
{
    // CURRENTLY NOT USED
	class MagicEffectTracer: public RE::BSTEventSink<RE::TESMagicEffectApplyEvent>{

    public:
        static MagicEffectTracer& GetSingleton() {
            static MagicEffectTracer singleton;
            return singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>*);

        bool Register();

        bool Unregister();

        void InitializeData(RE::SpellItem* a_spell);

        RE::Actor* FindTarget();
	private:
		MagicEffectTracer() = default;
		MagicEffectTracer(const MagicEffectTracer&) = delete;
		MagicEffectTracer& operator=(const MagicEffectTracer&) = delete;

        bool m_isRegistered = false;
        RE::SpellItem* m_spell = nullptr;
	};
}
