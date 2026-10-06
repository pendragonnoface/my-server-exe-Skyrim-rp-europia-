#include "CameraLockManager.h"
#include "DataManager.h"
#include "FlyingModeManager.h"
#include "FastTravelManager.h"
#include "ControlsManager.h"
#include "_ts_SKSEFunctions.h"
#include "APIManager.h"
#include "DataManager.h"
#include "CombatManager.h"


namespace IDRC {

    void CameraLockManager::Initialize()
    {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (playerCamera && playerCamera->currentState) {
            RE::TESCameraState* playerCameraState = playerCamera->currentState.get();
            if (playerCameraState) {
                RE::NiQuaternion cameraRotation;
                playerCameraState->GetRotation(cameraRotation);
                m_storedCameraYaw = _ts_SKSEFunctions::GetYaw(cameraRotation);
            }
        }

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (dragonActor) {
            m_flyState = _ts_SKSEFunctions::GetFlyingState(dragonActor);
            m_dragonYaw = dragonActor->GetAngleZ();
        }
    }

    void CameraLockManager::Update()
    {
        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return;
        }

        if (!m_isEnabled) {
            m_cameraLocked = false;
            return;
        }

        auto& flyingModeManager = FlyingModeManager::GetSingleton();
        auto& dataManager = DataManager::GetSingleton();
        
        auto* dragonActor = dataManager.GetDragonActor();
        if (!dragonActor) {
            m_cameraLocked = false;
            return;
        }

        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        RE::ThirdPersonState* dragonCameraState = nullptr;

        if (playerCamera && playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kDragon)) {
            dragonCameraState = static_cast<RE::ThirdPersonState*>(playerCamera->currentState.get());
            if (!dragonCameraState) {
                log::warn("{}: Dragon camera state is null", __FUNCTION__);
                m_cameraLocked = false;
                return;
            }
        } else {
            m_cameraLocked = false;
            return;
        }

        bool isKeyboardTurn = flyingModeManager.CheckForTurn();
        float currentCameraYaw = _ts_SKSEFunctions::GetYaw(dragonCameraState->rotation);
        if (m_isUserTurning || isKeyboardTurn) {
            m_storedCameraYaw = currentCameraYaw;
        } else {
            currentCameraYaw = m_storedCameraYaw;
        }

        auto& combatManager = CombatManager::GetSingleton();
        if (combatManager.IsShoutActive() && combatManager.GetShoutTarget()) {
            // If a shout target exists, use its position to calculate the camera yaw to keep 
            // the dragon oriented towards the target while shouting.
            // Also see the similar logic in Hooks::UpdateFlightPathData(), 
            // which uses the shout target position to update flight path waypoints.
            auto dragonPos = dragonActor->GetPosition();
            auto shoutTargetPos = combatManager.GetShoutTarget()->GetPosition();
            currentCameraYaw = atan2(shoutTargetPos.x - dragonPos.x, shoutTargetPos.y - dragonPos.y);
        }

        float currentDragonYaw = dragonActor->GetAngleZ();
        float currentDragonYawOffset = currentCameraYaw - currentDragonYaw;
        if (APIs::TrueDirectionalMovementV4 && APIs::TrueDirectionalMovementV4->IsTargetLockBehindTarget()) {
            currentDragonYawOffset += PI;
        }
        currentDragonYawOffset = _ts_SKSEFunctions::NormalRelativeAngle(currentDragonYawOffset);

        int flyState = _ts_SKSEFunctions::GetFlyingState(dragonActor);
        auto flyMode = flyingModeManager.GetFlyingMode();

        bool isTDMLocked = false;
        if (APIs::TrueDirectionalMovementV1) {
            isTDMLocked = APIs::TrueDirectionalMovementV1->GetTargetLockState();
        }

        bool isDragonTurning = false;
        if (fabs(_ts_SKSEFunctions::NormalRelativeAngle(currentDragonYaw - m_dragonYaw)) > 0.1f * PI / 180.f) {
            isDragonTurning = true;
        }
        m_dragonYaw = currentDragonYaw;

        auto* orbitMarker = DataManager::GetSingleton().GetOrbitMarker();
        if (!orbitMarker) {
            log::warn("{}: Could not obtain OrbitMarker", __FUNCTION__);
            m_cameraLocked = false;
            return;
        }
        auto* turnMarker = flyingModeManager.GetDragonTurnMarker();
        if (!turnMarker) {
            log::warn("{}: Could not obtain TurnMarker", __FUNCTION__);
            m_cameraLocked = false;
            return;
        }

        float targetDragonYaw = currentDragonYaw;
        if (flyState == 0 || flyState == 3) { // hovering or landed
            targetDragonYaw = _ts_SKSEFunctions::GetAngleZ(dragonActor->GetPosition(), turnMarker->GetPosition());
        } else if (flyState == 2) {  // flying
            targetDragonYaw = _ts_SKSEFunctions::GetAngleZ(dragonActor->GetPosition(), orbitMarker->GetPosition());
        }
        float targetDragonYawOffset = currentDragonYaw - targetDragonYaw;
        targetDragonYawOffset = _ts_SKSEFunctions::NormalRelativeAngle(targetDragonYawOffset);

        if (m_turnOngoing && fabs(targetDragonYawOffset) < 2.f * PI / 180.f) {
            m_turnOngoing = false;
        }

        if (flyState != m_flyState) { // reset turnMarker to avoid dragon rotation after reaching new state
            if (flyState == 0) { // landed
                flyingModeManager.DragonTurnPlayerRiding(180.f / PI * currentDragonYawOffset);
            } else if (flyState == 3) {  // hovering
                SKSE::GetTaskInterface()->AddTask([turnMarker, orbitMarker]() {
                    // When modifying Game objects, send task to TaskInterface to ensure thread safety
                    _ts_SKSEFunctions::MoveTo(turnMarker, orbitMarker, 2500.0f * std::sin(orbitMarker->GetAngleZ()), 2500.0f * std::cos(orbitMarker->GetAngleZ()), 0.0f);
                });
            }
        }
        m_flyState = flyState;

        if (( (flyState == 3 && flyMode == FlyingMode::kHovering ) ||
              (flyState == 0 && flyMode == FlyingMode::kLanded) ||
              (flyState == 2 && flyMode == FlyingMode::kFlying) )  // only trigger camera-induced movements if dragon is in one of these flying states 
           ) {

            // Turning
            if  ((isTDMLocked || m_isUserTurning || m_turnOngoing || flyState == 2) // only if user is actively triggering a turn (via mouse or gamepad, or TDM Lock), or such a user-triggered turn is not yet completed
                  && (flyState == 2 ||fabs(currentDragonYawOffset) > 2.f * PI / 180.f) // ignore turn angles smaller than 2 degrees
                  && !isKeyboardTurn
                ) {
                // dragon yaw follows user-triggered camera rotation
                flyingModeManager.SetYawOffset(180.f / PI * currentDragonYawOffset);
                m_turnOngoing = true;
            } 
        }

        if ((combatManager.IsShoutActive() && combatManager.GetShoutTarget()) || 
            isKeyboardTurn ||
            (isDragonTurning && !m_isUserTurning && !m_turnOngoing && !isTDMLocked)) {
            // camera rotation follows dragon yaw
            m_cameraLocked = true;

            float freeRotationX = _ts_SKSEFunctions::NormalRelativeAngle(dragonCameraState->freeRotation.x);
            float freeRotationY = _ts_SKSEFunctions::NormalRelativeAngle(dragonCameraState->freeRotation.y);
            float realTimeDeltaTime = _ts_SKSEFunctions::GetRealTimeDeltaTime() < 0.05f ? _ts_SKSEFunctions::GetRealTimeDeltaTime() : 0.05f;
            float damping = 1.0f - 2.5f * realTimeDeltaTime;

            auto* shoutTarget = combatManager.GetShoutTarget();
            bool shoutTargetingActive = combatManager.IsShoutActive() && shoutTarget;
            if (shoutTargetingActive && !isTDMLocked) {
                auto dragonPos = dragonActor->GetPosition();
                auto shoutTargetPos = shoutTarget->GetPosition();
                float cameraDistance = dragonPos.GetDistance(_ts_SKSEFunctions::GetCameraPos());
                float dragonHeading = dragonActor->GetHeading(false);
                float distanceToTarget = dragonActor->GetDistance(shoutTarget);
                float yawToTarget = _ts_SKSEFunctions::GetAngleZ(dragonPos, shoutTargetPos);
                
                float yawOffset = 0.f;
                float pitchOffset = 0.0f;
//                if (cameraDistance > 10.0f) { TODO: ImprovedCamera firstPerson support - Check SmoothCam / TDM sources _ImprovedCamera_IsFirstPerson for reference.
                    const float yawTheta =  m_maxTargetOffset * m_yawOffsetStrength;
                    auto flyingMode = FlyingModeManager::GetSingleton().GetFlyingMode();
                    float pitchOffsetSign = 1.0f;
                    if (m_invertPitchOffsetDuringFlight && flyingMode != FlyingMode::kLanded && flyingMode != FlyingMode::kPerching) {
                        pitchOffsetSign = -1.0f;
                    }
                    const float pitchTheta =  m_maxTargetOffset * m_pitchOffsetStrength * pitchOffsetSign;
                    float dX = shoutTargetPos.x - dragonPos.x;
                    float dY = shoutTargetPos.y - dragonPos.y;
                    float deltaXY = std::sqrt(dX * dX + dY * dY);
                    yawOffset = ComputeOffsetAngle(yawTheta, cameraDistance, deltaXY);
                    pitchOffset = ComputeOffsetAngle(pitchTheta, cameraDistance, distanceToTarget);
//                }

                float shoutTargetYaw = _ts_SKSEFunctions::NormalRelativeAngle(yawToTarget + yawOffset - dragonHeading);

                float pitchToTarget = _ts_SKSEFunctions::GetAngleX(dragonPos, shoutTargetPos) + pitchOffset;
                if (!m_wasShoutTargetingActive) {
                    m_shoutTransitionStartYaw = freeRotationX;
                    m_shoutTransitionStartPitch = freeRotationY;
                    m_shoutTransitionElapsed = 0.0f;

                    float maxAngleDelta = std::max(std::fabs(shoutTargetYaw), std::fabs(pitchToTarget - m_shoutTransitionStartPitch));    
                    m_shoutCameraTransitionDuration = std::max(0.2f, 0.5f * std::fabs(maxAngleDelta) / PI); // up to 0.5 sec for 180 deg camera turn towards target.
                }

                if (m_shoutTransitionElapsed < m_shoutCameraTransitionDuration) {
                    m_shoutTransitionElapsed = std::min(m_shoutTransitionElapsed + realTimeDeltaTime, m_shoutCameraTransitionDuration);
                    float t = _ts_SKSEFunctions::ApplyEasing(m_shoutTransitionElapsed / m_shoutCameraTransitionDuration, true, true);
                    float yawDelta = _ts_SKSEFunctions::NormalRelativeAngle(shoutTargetYaw - m_shoutTransitionStartYaw);
                    float pitchDelta = _ts_SKSEFunctions::NormalRelativeAngle(pitchToTarget - m_shoutTransitionStartPitch);
                    freeRotationX = _ts_SKSEFunctions::NormalRelativeAngle(m_shoutTransitionStartYaw + yawDelta * t);
                    freeRotationY = _ts_SKSEFunctions::NormalRelativeAngle(m_shoutTransitionStartPitch + pitchDelta * t);
                } else {
                    freeRotationX = shoutTargetYaw;
                    freeRotationY = pitchToTarget;
                }

                dragonCameraState->freeRotation.x = _ts_SKSEFunctions::NormalRelativeAngle(freeRotationX);
                freeRotationY = _ts_SKSEFunctions::NormalRelativeAngle(freeRotationY);
                dragonCameraState->freeRotation.y = std::clamp(freeRotationY, -70.f * PI / 180.f, 45.f * PI / 180.f); // max vanilla pitch range for the dragon camera
            } else {
                dragonCameraState->freeRotation.x = _ts_SKSEFunctions::NormalRelativeAngle(damping * freeRotationX);
            }
            
            m_wasShoutTargetingActive = shoutTargetingActive;
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player) {
                // Later in this frame, vanilla DragonCameraStateHook::HandleLookInput() is called,
                // via PlayerCamera::Update(), ThirdPersonState::Update().
                // DragonCameraStateHook::HandleLookInput() updates dragonCameraState->freeRotation.y based on player rotation
                // to keep the dragon camera oriented towards the player rotation.
                // Need to update player rotation here to match the updated dragonCameraState->freeRotation.y.
                // This avoids dragonCameraState->freeRotation.y being overridden by player rotation in HandleLookInput()
                player->data.angle.x = -dragonCameraState->freeRotation.y;
            }
        } else {
            m_cameraLocked = false;
            m_wasShoutTargetingActive = false;
        }

        m_isUserTurning = false; // reset flag. Is set to true in LookHook::ProcessMouseMove() in case of user-triggered camera rotation
    }

    float CameraLockManager::ComputeOffsetAngle(float a_theta, float a_cameraDistance, float a_targetDistance) {
        if (a_targetDistance < 1.0f) {
            return 0.0f;
        }
        float sinArg = std::clamp(-a_cameraDistance / a_targetDistance * std::sin(a_theta), -1.0f, 1.0f);
        return std::asin(sinArg) - a_theta;
    }

    void CameraLockManager::SetInitiallyEnabled(bool a_enabled)
    {
        m_initiallyEnabled = a_enabled;
    }

    bool const CameraLockManager::IsInitiallyEnabled() const
    {
        return m_initiallyEnabled;
    }

    void CameraLockManager::SetEnabled(bool a_enabled)
    {
        m_isEnabled = a_enabled;
    }

    bool const CameraLockManager::IsEnabled() const
    {
        return m_isEnabled;
    }

    void CameraLockManager::ResetEnabled()
    {
        m_isEnabled = m_initiallyEnabled;
    }

    bool const CameraLockManager::IsCameraLocked() const
    {
        return m_cameraLocked;
    }

    void CameraLockManager::SetUserTurning(bool a_moved) {
        m_isUserTurning = a_moved;
    }

    void CameraLockManager::SetIgnoredCameraPitch(float a_pitch) {
        m_ignoredCameraPitch = -a_pitch * PI / 180.f;
    }

    void CameraLockManager::SetYawOffsetStrength(float a_strength) {
        m_yawOffsetStrength = a_strength;
    }

    void CameraLockManager::SetPitchOffsetStrength(float a_strength) {
        m_pitchOffsetStrength = a_strength;
    }

    void CameraLockManager::SetInvertPitchOffsetDuringFlight(bool a_invert) {
        m_invertPitchOffsetDuringFlight = a_invert;
    }
}  // namespace IDRC
