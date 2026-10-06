#pragma once

#include "ControlsManager.h"

namespace IDRC
{

    // CURRENTLY NOT USED
	class CrosshairTracer: public RE::BSTEventSink<SKSE::CrosshairRefEvent> {

    public:
        static CrosshairTracer& GetSingleton() {
            static CrosshairTracer singleton;
            return singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* a_event, RE::BSTEventSource<SKSE::CrosshairRefEvent>*);

        bool Register();

        bool Unregister();

		void InitializeData();	
         RE::TESObjectREFR* GetCrosshairTarget() const;

	private:
		CrosshairTracer() = default;
		CrosshairTracer(const CrosshairTracer&) = delete;
		CrosshairTracer& operator=(const CrosshairTracer&) = delete;

        void ForceDisplayCrosshair(); // EXPERIMENTAL

         RE::TESObjectREFR* m_crosshairTarget = nullptr;
         bool m_isRegistered = false;
	};
}
