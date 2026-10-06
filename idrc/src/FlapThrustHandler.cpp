#include "FlapThrustHandler.h"
#include "DataManager.h"

namespace IDRC {
    void FlapThrustHandler::Register() {
        if (m_isRegistered) {
            log::warn("{}: FlapThrustHandler already registered", __FUNCTION__);
            return;
        }
        
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::error("{}: dragonActor is null", __FUNCTION__);
            return;
        }

        bool bSuccess = dragonActor->AddAnimationGraphEventSink(&FlapThrustHandler::GetSingleton());
        if (bSuccess) {
            m_isRegistered = true;
            log::info("{}: Registered FlapThrustHandler", __FUNCTION__);
        } else {
            RE::BSAnimationGraphManagerPtr graphManager;
            dragonActor->GetAnimationGraphManager(graphManager);
            bool bSinked = false;
            if (graphManager) {			
                for (auto& animationGraph : graphManager->graphs) {
                    if (bSinked) {
                        m_isRegistered = true;
                        break;
                    }
                    auto eventSource = animationGraph->GetEventSource<RE::BSAnimationGraphEvent>();
                    for (auto& sink : eventSource->sinks) {
                        if (sink == &FlapThrustHandler::GetSingleton()) {
                            bSinked = true;
                            break;
                        }
                    }
                }
            }
            
            if (!bSinked) {
                log::warn("{}: Failed to register FlapThrustHandler", __FUNCTION__);
            }		
        }
    }
    
    void FlapThrustHandler::Unregister() {
        if (m_isRegistered) {
            auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
            if (!dragonActor) {
                log::error("{}: dragonActor is null", __FUNCTION__);
                return;
            }

            dragonActor->RemoveAnimationGraphEventSink(&FlapThrustHandler::GetSingleton());
            m_isRegistered = false;
            log::info("{}: Unregistered FlapThrustHandler", __FUNCTION__);
        } else {
            log::warn("{}: FlapThrustHandler was not registered", __FUNCTION__);
        }
    }

    constexpr uint32_t hash(const char* data, size_t const size) noexcept
    {
        uint32_t hash = 5381;

        for (const char* c = data; c < data + size; ++c) {
            hash = ((hash << 5) + hash) + (unsigned char)*c;
        }

        return hash;
    }

    constexpr uint32_t operator"" _h(const char* str, size_t size) noexcept
    {
        return hash(str, size);
    }

    RE::BSEventNotifyControl FlapThrustHandler::ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>*)
    {
        if (a_event) {
            std::string_view eventTag = a_event->tag.data();

            auto& flyingModeManager = FlyingModeManager::GetSingleton();

            auto flyingMode = flyingModeManager.GetFlyingMode();
            if (hash(eventTag.data(), eventTag.size()) == "FlapThrustBegin"_h && flyingMode == FlyingMode::kFlying) {
                m_timeSinceLastFlap = 0.0f;
                m_previousThrust = RE::NiPoint3(0.0f, 0.0f, 0.0f);
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    void FlapThrustHandler::Update() {
        if (m_isRegistered) {
            m_timeSinceLastFlap += _ts_SKSEFunctions::GetRealTimeDeltaTime();

            UpdateDragonPosition();       
        }
    }

    void FlapThrustHandler::UpdateDragonPosition() {

        RE::NiPoint3 currentThrust = GetThrust(m_timeSinceLastFlap);
        RE::NiPoint3 deltaThrust = currentThrust - m_previousThrust;
        m_previousThrust = currentThrust;

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (dragonActor) {
            RE::NiPoint3 currentPosition = dragonActor->GetPosition();
            currentPosition += deltaThrust;
// TODO: SetPosition is only keeping the position for this frame
//       See comment below in GetThrust() 
            dragonActor->SetPosition(currentPosition, true);
        }
    }

    
    RE::NiPoint3 FlapThrustHandler::GetThrust(float a_time) {
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if(!dragonActor) {
            return RE::NiPoint3(0.0f, 0.0f, 0.0f);
        }

        RE::NiPoint3 thrustStrength(0.0f, 0.0f, 0.0f);
        if (a_time < m_timeBeforeLift) {
            return RE::NiPoint3(0.0f, 0.0f, 0.0f);
        } else if (a_time < m_timeBeforeLift + m_liftTime) {
            thrustStrength.x = m_thrustAmplitude * GetNormalizedThrust(a_time - m_timeBeforeLift, m_liftTime, true);
            thrustStrength.y = thrustStrength.x;
            thrustStrength.z = thrustStrength.x;
            
// TODO: find a solution for lateral and forward thrust
// Current issue is that SetPosition() is only keeping the dragon at the new position for the current frame
// As long as UpdateDragonPosition modifies the position every frame, this is okay.
// But we do not want to modify x and y during drop phase 
// This means that the dragon's horizontal position is reset by another logic in the frame to its original position
// and the dragon visually snaps back to that horizontal position after lift phase is completed.
// Would need to understand where in the frame the dragon's position is reset, 
// or how to modify the position in a way that is not reset by other logic.

// -> For now,only apply thrust in the z direction during lift and drop phases
thrustStrength.x = 0.0f;
thrustStrength.y = 0.0f;

        } else if (a_time < m_timeBeforeLift + m_liftTime + m_dropTime) {
            thrustStrength.x = 0.0f;  // No forward thrust during drop phase
            thrustStrength.y = 0.0f;  // No lateral thrust during drop phase
            thrustStrength.z = m_thrustAmplitude * GetNormalizedThrust(a_time - m_timeBeforeLift - m_liftTime, m_dropTime, false);
        } else {
            return RE::NiPoint3(0.0f, 0.0f, 0.0f);
        }

        float dragonYaw = dragonActor->GetHeading(false);
        float dragonPitch = - _ts_SKSEFunctions::NormalRelativeAngle(dragonActor->GetAngleX());
        float dragonRoll = IDRC::Utils::GetDragonRoll();

        RE::NiPoint3 thrustForward(0.0f, std::cos(m_thrustDirection), std::sin(m_thrustDirection));

        // rotate thrustForward vector based on pitch and roll
        float xAfterRoll = std::cos(dragonRoll) * thrustForward.x + std::sin(dragonRoll) * thrustForward.z;
        float yAfterRoll = thrustForward.y;
        float zAfterRoll = -std::sin(dragonRoll) * thrustForward.x + std::cos(dragonRoll) * thrustForward.z;

        RE::NiPoint3 thrustAfterRollAndPitch(
            xAfterRoll,
            std::cos(dragonPitch) * yAfterRoll - std::sin(dragonPitch) * zAfterRoll,
            std::sin(dragonPitch) * yAfterRoll + std::cos(dragonPitch) * zAfterRoll);

        // rotate around z-axis by dragon heading
        float xAfterHeading = std::cos(dragonYaw) * thrustAfterRollAndPitch.x + std::sin(dragonYaw) * thrustAfterRollAndPitch.y;
        float yAfterHeading = -std::sin(dragonYaw) * thrustAfterRollAndPitch.x + std::cos(dragonYaw) * thrustAfterRollAndPitch.y;
        float zAfterHeading = thrustAfterRollAndPitch.z;

        // scale by thrustStrength
        RE::NiPoint3 finalThrust(thrustStrength.x * xAfterHeading,
                                 thrustStrength.y * yAfterHeading,
                                 thrustStrength.z * zAfterHeading);

        return finalThrust;
    }

    float FlapThrustHandler::GetNormalizedThrust(float a_elapsedTime, float a_totalTime, bool a_isLift)
    {
        constexpr float kDecayAtEnd = 4.7f; // 95% decay at a_totalTime
        float duration = std::max(1e-4f, a_totalTime);
        float normalizedTime = std::clamp(a_elapsedTime, 0.0f, duration);
        float k = kDecayAtEnd / duration;

        auto g = [k](float t) {
            return std::exp(-k * t) * (1.0f + k * t);
        };

        float gT = g(duration);
        float gNow = g(normalizedTime);
        float denom = std::max(1e-6f, 1.0f - gT);

        if (a_isLift) {
            return std::clamp((1.0f - gNow) / denom, 0.0f, 1.0f);
        }

        return std::clamp((gNow - gT) / denom, 0.0f, 1.0f);
    }

} // namespace IDRC
