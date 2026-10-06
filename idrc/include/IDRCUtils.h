#pragma once

#include "Offsets.h"

namespace IDRC {
    namespace Utils{
        
        struct WorldspaceIniData {
            std::string name;
            float center_x;
            float center_y;
            float size;
        };
        
        void SetINIVars();

        float GetHorizontalDistance(RE::TESObjectREFR* a_from, RE::TESObjectREFR* a_to);

        float GetDragonRoll();
        
        bool IsFastTravelActive();

//        std::vector<WorldspaceIniData> LoadWorldspaceIniData(const std::string& a_iniFilename);
    } // namespace Utils
} // namespace IDRC
