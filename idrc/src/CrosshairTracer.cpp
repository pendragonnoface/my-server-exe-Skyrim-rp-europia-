#include "CrosshairTracer.h"

namespace IDRC {

    RE::BSEventNotifyControl CrosshairTracer::ProcessEvent(const SKSE::CrosshairRefEvent*  a_event, RE::BSTEventSource<SKSE::CrosshairRefEvent>*) {
        if (!a_event ) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* crosshairEvent = a_event;
        if (crosshairEvent && crosshairEvent->crosshairRef) {
            m_crosshairTarget = crosshairEvent->crosshairRef.get();
log::info("{}: CrosshairTarget = {}", __FUNCTION__, m_crosshairTarget->GetBaseObject()->GetName());
        } else {
            m_crosshairTarget = nullptr;
log::info("{}: CrosshairTarget = nullptr", __FUNCTION__);
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    bool CrosshairTracer::Register() {
        if (!m_isRegistered) {
            auto* crosshairRefEventSource = SKSE::GetCrosshairRefEventSource();
            if (!crosshairRefEventSource) {
                log::error("{}: crosshairRefEventSource is null", __FUNCTION__);
                return false;
            }
            crosshairRefEventSource->AddEventSink(this);

            ForceDisplayCrosshair();
            
            m_isRegistered = true;
            log::info("{}: Registered CrosshairTracer", __FUNCTION__);
        } else {
            log::warn("{}: CrosshairTracer already registered", __FUNCTION__);
        }
        return true;
    }

    bool CrosshairTracer::Unregister() {
        if (m_isRegistered) {
            auto* crosshairRefEventSource = SKSE::GetCrosshairRefEventSource();
            if (!crosshairRefEventSource) {
                log::error("{}: crosshairRefEventSource is null", __FUNCTION__);
                return false;
            }
            crosshairRefEventSource->RemoveEventSink(this);
            
            m_isRegistered = false;
            log::info("{}: Unregistered CrosshairTracer", __FUNCTION__);
        } else {
            log::warn("{}: CrosshairTracer was not registered", __FUNCTION__);
        }
        return true;
    }

	void CrosshairTracer::InitializeData() {
        m_crosshairTarget = nullptr;
    }

    RE::TESObjectREFR* CrosshairTracer::GetCrosshairTarget() const {
        return m_crosshairTarget;
    }

    // experimental functionality below to enable crosshair while riding a dragon
    // essentially copied from "contextual crosshair" mod (author: doodlum)

    [[nodiscard]] RE::GFxValue GetGFxValue(const char* a_pathToVar)
    {
        RE::GFxValue object;

        auto ui = RE::UI::GetSingleton();
        auto hud = ui ? ui->GetMenu<RE::HUDMenu>() : nullptr;
        auto view = hud ? hud->uiMovie : nullptr;
        if (view)
            view->GetVariable(std::addressof(object), a_pathToVar);

        return object;
    }

    void CrosshairTracer::ForceDisplayCrosshair()
    // This is not working as expected yet...
    {
        log::info("{}: Forcing display of crosshair", __FUNCTION__);
        auto crosshairInstance = GetGFxValue("_root.HUDMovieBaseInstance.CrosshairInstance");
        if (crosshairInstance != nullptr) {
            log::info("{}: CrosshairInstance found", __FUNCTION__);
            RE::GFxValue::DisplayInfo displayInfo;
            crosshairInstance.GetDisplayInfo(std::addressof(displayInfo));
            displayInfo.SetAlpha(0.0);
            crosshairInstance.SetDisplayInfo(displayInfo);
        }
    }

} // namespace IDRC
