#pragma once
#include "_ts_SKSEFunctions.h"

namespace IDRC
{

	class FlapThrustHandler: public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
    public:
        static FlapThrustHandler& GetSingleton() {
            static FlapThrustHandler singleton;
            return singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*);

        void Register();

        void Unregister();

        void Update();

	private:
        FlapThrustHandler() = default;
        FlapThrustHandler(const FlapThrustHandler&) = delete;
        FlapThrustHandler(FlapThrustHandler&&) = delete;
        ~FlapThrustHandler() = default;

        FlapThrustHandler& operator=(const FlapThrustHandler&) = delete;
        FlapThrustHandler& operator=(FlapThrustHandler&&) = delete;

        void UpdateDragonPosition();

        RE::NiPoint3 GetThrust(float a_time);
        float GetNormalizedThrust(float a_elapsedTime, float a_totalTime, bool a_isLift);

		bool m_isRegistered = false;
        float m_timeSinceLastFlap = 0.0f;
        const float m_timeBeforeLift = 0.3f;
        const float m_liftTime = 1.5f;
        const float m_dropTime = 10.0f;
        const float m_thrustAmplitude = 500.0f;
// TODO: currently m_thrustDirection is set to 0.5*PI, ie 'up'.
//       The idea is to set m_thrustDirection to something like 0.25 * PI (45 degrees) 
//       to provide a combined forward & lift thrust.
//       This is de-activated for now, see the comment in GetThrust() for more details.
        const float m_thrustDirection = 0.5f * PI; // 0: forward; PI/2: up
        RE::NiPoint3 m_previousThrust = RE::NiPoint3(0.0f, 0.0f, 0.0f);
	};  
}
