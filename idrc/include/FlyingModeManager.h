#pragma once

#include "ControlsManager.h"
#include "IDRCUtils.h"
#include "_ts_SKSEFunctions.h"

namespace IDRC {
    class CombatManager;
    class FastTravelManager;

    enum FlyingMode {
        kFlying = 0,
        kOrbiting = 1,  // unused
        kHovering = 2,
        kLanded = 3,
        kPerching = 4
    };
    
    class FlyingModeManager {
    public:
        static FlyingModeManager& GetSingleton() {
            static FlyingModeManager instance;
            return instance;
        }

        FlyingModeManager(const FlyingModeManager&) = delete;
        FlyingModeManager& operator=(const FlyingModeManager&) = delete;

        void InitializeData(RE::TESObjectREFR* a_dragonTurnMarker, 
                            RE::TESObjectREFR* a_dragonTravelToMarker,
                            RE::TESObjectREFR* a_flyToTargetMarker,
                            RE::SpellItem* a_noFlyAbility);

        void Update();

        RE::TESObjectREFR* GetDragonTurnMarker();

        void OnKeyDown(IDRCKey a_key);

        void OnBackKeyUp();

        void ToggleAutoCombat();

        bool DragonTravelTo(RE::TESObjectREFR* a_directionMarker);

        bool TriggerLand(RE::TESObjectREFR* a_landTarget = nullptr);

        bool DragonHoverPlayerRiding(RE::TESObjectREFR* a_hoverTarget);

        bool ForceHover();

        bool DragonTakeOffPlayerRiding(RE::TESObjectREFR* a_takeOffTarget);

        bool DragonLandPlayerRiding(RE::TESObjectREFR* a_landTarget);

        bool DragonFlyTo(float a_angle);

        bool CheckForTurn() const;

        bool CheckForHeightChange() const;

        void SetYawOffset(float a_angle);

        void SetTargetPitch(float a_angle);

        float GetTargetYaw();

        float GetTargetPitch();

        FlyingMode GetFlyingMode();

        void SetFlyingModeFromPapyrus(int a_flyingState);

        bool GetRegisteredForLanding();

        bool GetRegisteredForPerch();

        void SetRegisteredForPerch(bool a_registeredForPerch);

        void ChangeDragonHeight();

        bool DragonTurnPlayerRiding(float a_turnAngle);

        bool GetLandingPosSearchOngoing() { return m_landingPosSearchOngoing; }
    private:
        FlyingModeManager() = default;
        ~FlyingModeManager() = default;

        RE::TESObjectREFR* m_dragonTurnMarker = nullptr;
        RE::TESObjectREFR* m_dragonTravelToMarker = nullptr;
        RE::TESObjectREFR* m_flyToTargetMarker = nullptr;

        float m_minHeight = 1000.0f;  
        float m_yawOffset = 0.0;
        float m_targetPitch = 0.0;
        float m_turnSpeed = 25.0;
        float m_deltaPitch = 20.0f * PI / 180.0f;
        bool m_toggleAlwaysRun = true;
        bool m_registeredForLanding = false;
        bool m_registeredForPerch = false;
        bool m_toggledAutoCombatLand = false;
        bool m_vanillaAttack = false;
        bool m_finalizeTriggerLand = false;
        bool m_landingPosSearchOngoing = false;
        bool m_waitForLanded = false;
        bool m_backKeyPressed = false;
        bool m_isModeTransitioning = false;

        FlyingMode m_mode = kLanded;
        RE::SpellItem* m_noFlyAbility = nullptr;
        RE::NiPoint3 m_landingPos;

        class WorldSpaceData {
        public:
            float m_minX;
            float m_maxX;
            float m_minY;
            float m_maxY;
            float m_centerX;
            float m_centerY;
            float m_seaLevel;
            std::string m_borderRegionName;

            WorldSpaceData() = default;
            WorldSpaceData(const RE::TESWorldSpace* a_worldSpace);

            void InitializeData(const std::string& a_regionName);
        };

        void CheckModeTransition();

        float GetRunFactor(float a_modifier = 1.0f);

        bool FlyingModeUp(IDRCKey a_key);

        bool FlyingModeDown();

        void UpdateFlyingMode();

        bool CancelDragonLandPlayerRiding();

        void SetRegisteredForLanding(bool a_registeredForLanding);

        void PlaceTravelToMarker(RE::TESObjectREFR* a_ref, float a_distance = 0.0f, float a_angle = 0.0f, float a_offsetZ = 0.0f);

        bool DragonNewDirection(float a_angle);

        bool DragonPerchPlayerRiding();
        
        void TriggerTurn();
        
        float NormalizeAngle(float a_angle);

        float GetTurnFactor();

        float GetAngleToCoordinate(float a_posX, float a_posY);

        float GetWorldSpaceCenterX();

        float GetWorldSpaceCenterY();

        bool IsInBorderRegion() const;

        void SetFlyingMode(FlyingMode a_mode);

        void FinalizeTriggerLand();

        void FinalizeLand();

        void SetLandedCompleted();

        void GetValidLandingPosition();
    
    }; // class FlyingModeManager

} // namespace IDRC