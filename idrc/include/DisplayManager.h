#pragma once

#include <string>
#include "FlyingModeManager.h"

namespace IDRC {
    class DisplayManager {
        public:
            static DisplayManager& GetSingleton() {
                static DisplayManager instance;
                return instance;
            }
            DisplayManager(const DisplayManager&) = delete;
            DisplayManager& operator=(const DisplayManager&) = delete;

            bool GetDisplayAttackMessage();
            
            void SetDisplayAttackMessage(bool a_display);

            void DisplayDragonHealth();

            void DisplayLeavingBorderRegion();
            
        private:
            DisplayManager() = default;
            ~DisplayManager() = default;

            bool m_displayAttackMessage = true;
    }; // class DisplayManager
} // namespace IDRC
    