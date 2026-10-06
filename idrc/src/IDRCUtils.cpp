#include "IDRCUtils.h"

#include <SimpleIni.h>
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>

#include "_ts_SKSEFunctions.h"
#include "DataManager.h"

namespace IDRC {
    namespace Utils{

        void SetINIVars(){
            SKSE::GetTaskInterface()->AddTask([]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
                _ts_SKSEFunctions::UpdateIniSetting("fFlyingMountFastTravelDragonSpeed:General", 8500.0000f);
                _ts_SKSEFunctions::UpdateIniSetting("fFlyingMountFastTravelArrivalHeight:General", 100.0000f) ;
                _ts_SKSEFunctions::UpdateIniSetting("fFlyingMountLandingRequestTimer:General", 1.5000f);
                _ts_SKSEFunctions::UpdateIniSetting("fFlyingMountSlowestSpeedMult:General", 1.2500f);
                _ts_SKSEFunctions::UpdateIniSetting("iFlyingMountSlowestQueuedRefCount:General", 250);
                _ts_SKSEFunctions::UpdateIniSetting("fPlayerFlyingMountNothingLoadingMult:General", 10.0000f);
                _ts_SKSEFunctions::UpdateIniSetting("fPlayerFlyingMountFastBaseTargetSpeed:General", 1800.0000f) ;
                _ts_SKSEFunctions::UpdateIniSetting("fPlayerFlyingMountBaseTargetSpeed:General", 1100.0000f) ;
                _ts_SKSEFunctions::UpdateIniSetting("bFlyingMountFastTravelCruiseEnabled:General", true);
                _ts_SKSEFunctions::UpdateIniSetting("fPlayerFlyingMountTravelMaxHeight:General", 100.0000f) ;
                _ts_SKSEFunctions::UpdateIniSetting("fPlayerFlyingMountTravelMinHeight:General", 100.0000f) ; 
            });
        }

/* No longer used
        bool RegisterForSingleUpdate(float a_seconds) {
            auto* quest = DataManager::GetSingleton().GetRideQuest();
            if (!quest) {
                log::error("{}: RideQuest is null", __FUNCTION__);
                return false;
            }
            auto handle = _ts_SKSEFunctions::GetHandle(quest);
            if(!handle){
                log::error("{}: Quest handle is null", __FUNCTION__);
                return false;
            }
            SKSE::GetTaskInterface()->AddTask([handle, a_seconds]() {
            // When modifying Game objects, send task to TaskInterface to ensure thread safety
                _ts_SKSEFunctions::RegisterForSingleUpdate(handle, a_seconds);
            });
            return true;
        }


        /* loads worldspace data from ini file. ini file format:
                [Worldspace]
                name = Skyrim
                center_x = 0.0
                center_y = 0.0
                size = 300000.0
                name = Solstheim
                center_x = 10000.0
                center_y = 10000.0
                size = 200000.0
        *//*
        std::vector<WorldspaceIniData> LoadWorldspaceIniData(const std::string& a_iniFilename) {
            std::vector<WorldspaceIniData> worldspaces;
        
            std::filesystem::path iniPath = std::filesystem::current_path() / "Data" / a_iniFilename;
        
            CSimpleIniA ini;
            ini.SetMultiKey(true); // Enable support for duplicate sections
            if (ini.LoadFile(iniPath.string().c_str()) != SI_OK) {
                log::error("{}: Failed to load INI file: {}", __FUNCTION__, iniPath.string());
                return worldspaces;
            }
        
            CSimpleIniA::TNamesDepend sections;
            ini.GetAllSections(sections);
        
            for (const auto& section : sections) {
                if (std::string(section.pItem) == "Worldspace") {
                    CSimpleIniA::TNamesDepend worldspaceNames, worldspaceCentersX, worldspaceCentersY, worldspaceSizes;
                    ini.GetAllValues(section.pItem, "name", worldspaceNames);
                    ini.GetAllValues(section.pItem, "center_x", worldspaceCentersX);
                    ini.GetAllValues(section.pItem, "center_y", worldspaceCentersY);
                    ini.GetAllValues(section.pItem, "size", worldspaceSizes);
        
                    // Ensure all keys have the same number of values
                    if (worldspaceNames.size() != worldspaceCentersX.size() ||
                        worldspaceNames.size() != worldspaceCentersY.size() ||
                        worldspaceNames.size() != worldspaceSizes.size()) {
                        log::error("{}: Error - Mismatched number of values in section '{}'", __FUNCTION__, section.pItem);
                        continue;
                    }
        
                    auto nameIt = worldspaceNames.begin();
                    auto centerXIt = worldspaceCentersX.begin();
                    auto centerYIt = worldspaceCentersY.begin();
                    auto sizeIt = worldspaceSizes.begin();
        
                    while (nameIt != worldspaceNames.end()) {
                        WorldspaceIniData data;
                        data.name = nameIt->pItem;
        
                        try {
                            data.center_x = std::stof(centerXIt->pItem);
                            data.center_y = std::stof(centerYIt->pItem);
                            data.size = std::stof(sizeIt->pItem);
                        } catch (const std::exception& e) {
                            log::error("{}: Error - Failed to parse values in section '{}': {}", __FUNCTION__, section.pItem, e.what());
                            ++nameIt;
                            ++centerXIt;
                            ++centerYIt;
                            ++sizeIt;
                            continue;
                        }
        
                        worldspaces.push_back(data);
        
                        ++nameIt;
                        ++centerXIt;
                        ++centerYIt;
                        ++sizeIt;
                    }
                }
            }
        
            return worldspaces;
        }
*/
        float GetHorizontalDistance(RE::TESObjectREFR* a_from, RE::TESObjectREFR* a_to) {
            if (!a_from || !a_to) {
                log::error("{}: One or both of the provided TESObjectREFRs are null", __FUNCTION__);
                return 0.0f;
            }

            auto fromPos = a_from->GetPosition();
            auto toPos = a_to->GetPosition();

            float deltaX = toPos.x - fromPos.x;
            float deltaY = toPos.y - fromPos.y;

            return std::sqrt(deltaX * deltaX + deltaY * deltaY);
        }

        float GetDragonRoll() {
            auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
            if (!dragonActor) {
                return 0.0f;
            }

            float targetRoll = 0.f;
            RE::NiAVObject* reference3D = _ts_SKSEFunctions::GetTargetPoint(dragonActor, RE::BGSBodyPartDefs::LIMB_ENUM::kTorso).get();
			if (reference3D) {
                // Note: different actors / targetpoints use different axis conventions.
                // The roll axis for dragon kTorso bodypart is world.rotate.GetVectorZ(). 
                // eg for the dragon's kSaddle bodypart, the roll axis is world.rotate.GetVectorX().
				targetRoll = asinf(reference3D->world.rotate.GetVectorZ().z);
			} else {
				log::warn("{}: No target point found for dragonActor", __FUNCTION__);
			}

            return targetRoll;
        }

        bool IsFastTravelActive() {
            return (*g_PatrolQueuedState == 1  || *g_PatrolQueuedState == 4 || *g_FastTravelState == true);
        }

    } // namespace Utils
} // namespace IDRC
