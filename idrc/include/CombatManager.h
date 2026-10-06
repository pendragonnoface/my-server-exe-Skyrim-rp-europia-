#pragma once


namespace IDRC {   
    class FlyingModeManager;

    class CombatManager {
    public:
        static CombatManager& GetSingleton() {
            static CombatManager instance;
            return instance;
        }
        CombatManager(const CombatManager&) = delete;
        CombatManager& operator=(const CombatManager&) = delete;

        void InitializeData(RE::TESShout* a_unrelentingForceShout,
                            RE::TESShout* a_attackShout);
 
        bool DragonAttack(bool a_alternateAttack = false);

        void Update();
    
        bool IsShoutActive() {return m_shoutActive;}

        float GetShoutDirection() { return m_shoutDirection; }

        RE::Actor* GetShoutTarget() { return m_shoutTarget ? m_shoutTarget.get().get() : nullptr; }

        RE::Actor* GetStoredCombatTarget() { return m_storedCombatTarget ? m_storedCombatTarget.get().get() : nullptr; }

        RE::ActorHandle GetStoredCombatTargetHandle() { return m_storedCombatTarget; }

        int GetStoredCombatTargetState() { return m_storedCombatTargetState; }

        bool IsFastTravelAttack() { return m_isFastTravelAttack; }

        void SetFastTravelAttack(bool a_value) { m_isFastTravelAttack = a_value; }
    private:
        CombatManager() = default;
        ~CombatManager() = default;

        // accessed by other classes
        // change value only via Set function to trigger PropertyUpdateEvent
        RE::TESShout* m_unrelentingForceShout = nullptr;
        RE::TESShout* m_attackShout = nullptr;
        const float m_maxTargetDistance = 2000.0f;
        float m_shoutTimer = 0.0f;
        float m_shoutDirection = 1.0f;
        bool m_shoutActive = false;
        bool m_restartCombatPending = false;
        bool m_isFastTravelAttack = false;
        RE::ActorHandle m_shoutTarget{};
        RE::ActorHandle m_storedCombatTarget{};
        int m_storedCombatTargetState = 0;

        RE::TESShout* GetShout(float a_targetDistance);

        bool SetActiveShout(float a_targetDistance,  bool a_useUnrelentingForce = false);

        bool IsValidTarget(RE::Actor* a_target);

        void DragonStartCombat(RE::Actor* a_target);

        void UpdateCombat();

//        void UpdatePlayerCell();

        void UpdateAttack();

        void ExecuteAttack();

        float GetMaxTargetDistance();
    }; // class CombatManager
} // namespace IDRC

