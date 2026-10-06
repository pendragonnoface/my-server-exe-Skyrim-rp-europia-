#pragma once

#include "CombatTargetReticle.h"


namespace IDRC {
    class TargetReticleManager {
        public:
            enum TargetMode : std::uint32_t {
                kNone = 0,
                kSelectedActor = 1,
                kCombatTarget = 2
            };

            enum ReticleMode : std::uint32_t {
                kOff = 0,
                kOn = 1,
                kOnlyCombatTarget = 2
            };

            static TargetReticleManager& GetSingleton() {
                static TargetReticleManager instance;
                return instance;
            }
            TargetReticleManager(const TargetReticleManager&) = delete;
            TargetReticleManager& operator=(const TargetReticleManager&) = delete;

            void Initialize();

            void SetPrimaryTargetMode(TargetMode a_targetMode);
            void SetMaxTargetDistance(float a_maxReticleDistance);
            void SetDistanceMultiplierSmall(float a_distanceMultiplierSmall);
            void SetDistanceMultiplierLarge(float a_distanceMultiplierLarge);
            void SetDistanceMultiplierExtraLarge(float a_distanceMultiplierExtraLarge);
            void SetMaxTargetScanAngle(float a_maxTargetScanAngle);

            void Update();

            void SetReticleMode(ReticleMode a_mode) { m_reticleMode = a_mode; }

            void SetReticleLockAnimationStyle(int a_style);

            void ToggleLockReticle();

            void TogglePrimaryTargetMode();

            void DisposeReticle();

            RE::ActorHandle GetCurrentTarget() const;

            bool GetUseTarget() const;

            void SetUseTarget(bool a_useTarget);

            bool IsReticleLocked () const;

            float GetMaxTargetDistance(RE::Actor* a_actor) const;
            
        private:
            TargetReticleManager() = default;

            void UpdateReticle();

            int GetCombatState();

            bool IsTDMLocked();

            void UpdateReticleState();

            RE::Actor* GetSelectedActor() const;

            RE::ActorHandle GetCombatTarget() const;

            TargetMode GetTargetMode(bool a_hasSelectedActor, bool a_hasCombatTarget) const;

            void SetReticleTarget();

            RE::NiPointer<RE::NiAVObject> GetTargetPoint(RE::ActorHandle a_actorHandle) const;

            float GetDistanceRaceSizeMultiplier(RE::TESRace* a_race) const;

            std::weak_ptr<CombatTargetReticle> m_TargetReticle;
            bool m_isInitialized = false;
            ReticleMode m_reticleMode = ReticleMode::kOn;
            bool m_isReticleLocked = false;
            bool m_isWidgetActive = false;
            int m_combatState = 0;
            float m_maxReticleDistance = 8000.f;
            float m_distanceMultiplierSmall = 1.0f;
            float m_distanceMultiplierLarge = 2.0f;
            float m_distanceMultiplierExtraLarge = 4.0f;
            float m_maxTargetScanAngle = 7.0f;
            RE::ActorHandle m_reticleTarget{};
            TargetMode m_primaryTargetMode = TargetMode::kCombatTarget;
            TargetMode m_currentTargetMode = TargetMode::kNone;
            int m_reticleLockAnimationStyle = 0;
            bool m_useTarget = false;
    }; // class TargetReticleManager
}  // namespace IDRC
