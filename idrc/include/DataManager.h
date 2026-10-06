#pragma once

#include "FlyingModeManager.h"

namespace IDRC {
    class DataManager {
        public:
            static DataManager& GetSingleton() {
                static DataManager instance;
                return instance;
            }
            DataManager(const DataManager&) = delete;
            DataManager& operator=(const DataManager&) = delete;
    
            // Getter and setter for OrbitMarker
            RE::TESObjectREFR* GetOrbitMarker();
        
            RE::Actor* GetDragonActor();
    
            RE::TESObjectREFR* GetWordWallPerch();

            RE::TESObjectREFR* GetTowerPerch();

            RE::TESObjectREFR* GetRockPerch();

            void InitializeData(RE::TESQuest* a_rideQuest,
                                RE::TESObjectREFR* a_orbitMarker,
                                RE::BGSRefAlias* a_dragonAlias,
                                std::string a_dragonName,
                                RE::TESQuest* a_findPerchQuest,
                                RE::BGSRefAlias* a_wordWallPerch, 
                                RE::BGSRefAlias* a_towerPerch,
                                RE::BGSRefAlias* a_rockPerch, 
                                RE::BGSRefAlias* a_perchTarget);

            void SetPerchTarget(RE::TESObjectREFR* a_target);

            RE::TESQuest* GetRideQuest();

            RE::TESQuest* GetFindPerchQuest();

            bool GetAutoCombat();

            void SetAutoCombat(bool a_auto);

            void ToggleAutoCombat();

            std::string GetDragonName();

            void SendPropertyUpdateEvent(std::string a_propertyName, bool a_bValue = false, 
                float a_fValue = 0.0f, int a_iValue = 0);

            void SetDragonSpeeds(float a_speedMult);

            float GetRollAmplitude();
            void  SetRollAmplitude(float a_rollAmplitude);

            float GetRollSmoothTime();
            void  SetRollSmoothTime(float a_rollSmoothTime);
        private:
            DataManager() = default;
            RE::TESObjectREFR* m_orbitMarker = nullptr;
            RE::BGSRefAlias* m_dragonAlias = nullptr;
            RE::BGSRefAlias* m_wordWallPerch = nullptr;
            RE::BGSRefAlias* m_towerPerch = nullptr;
            RE::BGSRefAlias* m_rockPerch = nullptr;
            RE::BGSRefAlias* m_perchTarget = nullptr;
            RE::TESQuest* m_rideQuest = nullptr;
            RE::TESQuest* m_findPerchQuest = nullptr;
            bool m_autoCombat = false;
            const float m_baseSpeed = 1100.0;
            const float m_fastBaseSpeed = 1800.0;
            std::string m_dragonName = "";
            float m_rollAmplitude = 0.5f;
            float m_rollSmoothTime    = 0.5f;
    }; // class DataManager
}  // namespace IDRC
