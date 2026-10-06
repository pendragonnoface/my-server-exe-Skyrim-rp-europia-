#pragma once


namespace IDRC
{
 	class CombatTargetTracer: public RE::BSTEventSink<RE::TESCombatEvent>{

    public:
        static CombatTargetTracer& GetSingleton() {
            static CombatTargetTracer singleton;
            return singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>*);

        bool Register();

        bool Unregister();

        void InitializeData(RE::TESEffectShader* a_breathHitShader);

        RE::Actor* GetCombatTarget() const;
	private:
		CombatTargetTracer() = default;
		CombatTargetTracer(const CombatTargetTracer&) = delete;
		CombatTargetTracer& operator=(const CombatTargetTracer&) = delete;

        bool m_isRegistered = false;
        int m_combatState = 0;
        RE::Actor* m_combatTarget = nullptr;
        RE::TESEffectShader* m_breathHitShader = nullptr;
	};
}
