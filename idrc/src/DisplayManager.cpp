#include "DisplayManager.h"
#include "FlyingModeManager.h"
#include "DataManager.h"
#include "IDRCUtils.h"
#include "_ts_SKSEFunctions.h"


namespace IDRC {

    bool DisplayManager::GetDisplayAttackMessage() {
        return m_displayAttackMessage;
    }
     
    void DisplayManager::SetDisplayAttackMessage(const bool a_display) {
        m_displayAttackMessage = a_display;
    }

    void DisplayManager::DisplayDragonHealth() {
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: Dragon actor is null", __FUNCTION__);
            return;
        }

        float healthPercentage = _ts_SKSEFunctions::GetHealthPercentage(dragonActor) * 100.0f;
        int roundedHealth = static_cast<int>(std::round(healthPercentage));
    
        RE::SendHUDMessage::ShowHUDMessage((DataManager::GetSingleton().GetDragonName() + " Health: " + std::to_string(roundedHealth) + "%").c_str());
    }

    void DisplayManager::DisplayLeavingBorderRegion() {
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: Dragon actor is null", __FUNCTION__);
            return;
        }
    
        RE::SendHUDMessage::ShowHUDMessage("You cannot go that way :-(");
    }
} // namespace IDRC