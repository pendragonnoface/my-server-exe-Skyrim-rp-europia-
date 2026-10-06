#pragma once

#include "_ts_SKSEFunctions.h"
namespace IDRC {
    class CameraLockManager {
        public:

            static CameraLockManager& GetSingleton() {
                static CameraLockManager instance;
                return instance;
            }
            CameraLockManager(const CameraLockManager&) = delete;
            CameraLockManager& operator=(const CameraLockManager&) = delete;

            void Initialize();

            void Update();

            void SetInitiallyEnabled(bool a_enabled);
            bool const IsInitiallyEnabled() const;

            void SetEnabled(bool a_enabled);
            bool const IsEnabled() const;

            void ResetEnabled();

            bool const IsCameraLocked() const;

            void SetUserTurning(bool a_moved);

            void SetIgnoredCameraPitch(float a_pitch);

            void SetYawOffsetStrength(float a_strength);

            void SetPitchOffsetStrength(float a_strength);

            void SetInvertPitchOffsetDuringFlight(bool a_invert);

        private:
            CameraLockManager() = default;

            float ComputeOffsetAngle(float a_theta, float a_cameraDistance, float a_targetDistance);

            bool m_initiallyEnabled = true;
            bool m_isEnabled = true;
            bool m_turnOngoing = false;
            bool m_cameraLocked = false;
            bool m_isUserTurning = false;
            float m_dragonYaw = 0.0f;
            float m_storedCameraYaw = 0.0f;
            int m_flyState = -1;

            bool m_wasShoutTargetingActive = false;
            float m_shoutTransitionStartYaw = 0.0f;
            float m_shoutTransitionStartPitch = 0.0f;
            float m_shoutTransitionElapsed = 0.0f;
            float m_shoutCameraTransitionDuration = 0.0f;
            float m_ignoredCameraPitch = -8.f * PI / 180.f;
            const float m_maxTargetOffset = 20.0f * PI / 180.f;
            float m_yawOffsetStrength = 0.25f;
            float m_pitchOffsetStrength = 0.25f;
            bool m_invertPitchOffsetDuringFlight = true;
    }; // class CameraLockManager
}  // namespace IDRC
