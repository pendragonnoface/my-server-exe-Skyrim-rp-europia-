#include "ThumbstickTracer.h"
#include <math.h>
#include "FlyingModeManager.h"
#include "ControlsManager.h"
#include "_ts_SKSEFunctions.h"

// ThumbstickTracer and ThumbstickTracerHook implementations are modified copies of
// the DTryKeyUtils movementInputTracer and hook_movement classes
// see: https://github.com/D7ry/DtryKeyUtil
// All credits go to DTry as the original author!

namespace IDRC {

	void ThumbstickTracerHook::ProcessThumbstick(RE::MovementHandler* a_this, RE::ThumbstickEvent* a_event, RE::PlayerControlsData* a_data) {
		ThumbstickTracer::GetSingleton().OnThumbStickMovement(a_event);
		_ProcessThumbstick(a_this, a_event, a_data);
	}

	void ThumbstickTracer::InitializeData() {
		m_prevThumbStickZone = thumbStickZone::neutral;
		m_leftPressed = false;
		m_rightPressed = false;
		m_forwardPressed = false;
		m_backPressed = false;
		m_rad = 0.0f;
	}

	void ThumbstickTracer::ProcessThumbstickKey(IDRCKey a_key) {
        log::info("{}: {}", __FUNCTION__, a_key);

        auto& controlsManager = ControlsManager::GetSingleton();
		auto& flyingModeManager = FlyingModeManager::GetSingleton();
        if ((a_key == kStrafeLeft || a_key == kStrafeRight) && controlsManager.GetIsKeyPressed(kForward) && (flyingModeManager.GetFlyingMode() == kFlying)) {
            // when in flying mode, need un-block controls when rotating thumbstick coming from Forward direction to left / right 
            controlsManager.SetControlBlocked(false);
        }

        flyingModeManager.OnKeyDown(a_key);
    }
	
	bool ThumbstickTracer::IsThumbstickKeyPressed(IDRCKey a_key) {
		switch (a_key) {
		case kForward:
			return m_forwardPressed;
		case kBack:
			return m_backPressed;
		case kStrafeLeft:
			return m_leftPressed;
		case kStrafeRight:
			return m_rightPressed;
		}

		return false;
	}

	bool ThumbstickTracer::IsThumbstickPressed() {
		return m_forwardPressed || m_backPressed || m_leftPressed || m_rightPressed;
	}

	void ThumbstickTracer::OnForward(bool a_activate) {
		if (a_activate) {
			if (!m_forwardPressed) {
				m_forwardPressed = true;

				std::thread([this]() {
					// reason for separate thread: see comment in definition of ControlsManager::ProcessEvent()
					this->ProcessThumbstickKey(kForward);
				}).detach();
log::info("{}: Forward pressed", __FUNCTION__);
			}
		}
		else {
			m_forwardPressed = false;
log::info("{}: Forward released", __FUNCTION__);
		}
	}

	void ThumbstickTracer::OnBack(bool a_activate) {
		if (a_activate) {
			if (!m_backPressed) {
				m_backPressed = true;

				std::thread([this]() {
					// reason for separate thread: see comment in definition of ControlsManager::ProcessEvent()
					this->ProcessThumbstickKey(kBack);
				}).detach();
log::info("{}: Back pressed", __FUNCTION__);
			}
		}
		else {
			m_backPressed = false;
log::info("{}: Back released", __FUNCTION__);
		}
	}

	void ThumbstickTracer::OnLeft(bool a_activate) {
		if (a_activate) {
			if (!m_leftPressed) {
				m_leftPressed = true;

				std::thread([this]() {
					// reason for separate thread: see comment in definition of ControlsManager::ProcessEvent()
					this->ProcessThumbstickKey(kStrafeLeft);
				}).detach();
log::info("{}: Left pressed", __FUNCTION__);
			}
		}
		else {
			m_leftPressed = false;
log::info("{}: Left released", __FUNCTION__);
		}
	}

	void ThumbstickTracer::OnRight(bool a_activate) {
		if (a_activate) {
			if (!m_rightPressed) {
				m_rightPressed = true;

				std::thread([this]() {
					// reason for separate thread: see comment in definition of ControlsManager::ProcessEvent()
					this->ProcessThumbstickKey(kStrafeRight);
				}).detach();
log::info("{}: Right pressed", __FUNCTION__);
			}
		}
		else {
			m_rightPressed = false;
log::info("{}: Right released", __FUNCTION__);
		}
	}
	inline bool inRange(float num, double low, double high) {
		//DEBUG("{}, {}, {}", num, low, high);
		return low < num && num <= high;
	}

	inline bool InPIRange(float rad, double low, double high) {
		return (low * PI) < rad 
			&& rad <= (high * PI);
	}
	ThumbstickTracer::thumbStickZone ThumbstickTracer::GetThumbStickZone(float a_x, float a_y) {
		if (a_x == 0 && a_y == 0) {
			return thumbStickZone::neutral;
		}
		m_rad = atan2(a_y, a_x);
		if (m_rad < 0) {
			m_rad += 2 * PI;
		}
		if (InPIRange(m_rad, 0, 1.0 / 8) || InPIRange(m_rad, 15.0 / 8, 2) || m_rad == 0) {
			return thumbStickZone::right;
		}
		else if (InPIRange(m_rad, 1.0 / 8, 7.0 / 16)) {
			return thumbStickZone::upRight;
		}
		else if (InPIRange(m_rad, 7.0 / 16, 9.0 / 16)) {
			return thumbStickZone::up;
		}
		else if (InPIRange(m_rad, 9.0 / 16, 7.0 / 8)) {
			return thumbStickZone::upLeft;
		}
		else if (InPIRange(m_rad, 7.0 / 8, 9.0 / 8)) {
			return thumbStickZone::left;
		}
		else if (InPIRange(m_rad, 9.0 / 8, 11.0 / 8)) {
			return thumbStickZone::downLeft;
		}
		else if (InPIRange(m_rad, 11.0 / 8, 13.0 / 8)) {
			return thumbStickZone::down;
		}
		else if (InPIRange(m_rad, 13.0 / 8, 15.0 / 8)) {
			return thumbStickZone::downRight;
		}
		return thumbStickZone::neutral;
		
	}

	bool ThumbstickTracer::UpdateProximityThumbstickZone(thumbStickZone a_newThumbstickZone) {
		switch (a_newThumbstickZone) {
		case thumbStickZone::down:
			switch (m_prevThumbStickZone) {
			case thumbStickZone::down: return true;
			case thumbStickZone::downLeft: OnLeft(false); return true;
			case thumbStickZone::downRight: OnRight(false); return true;
			}
			break;
		case thumbStickZone::up:
			switch (m_prevThumbStickZone) {
			case thumbStickZone::up: return true;
			case thumbStickZone::upLeft: OnLeft(false); return true;
			case thumbStickZone::upRight: OnRight(false); return true;
			}
			break;
		case thumbStickZone::left:
			switch (m_prevThumbStickZone) {
			case thumbStickZone::left: return true;
			case thumbStickZone::upLeft: OnForward(false); return true;
			case thumbStickZone::downLeft: OnBack(false); return true;
			}
			break;
		case thumbStickZone::right:
			switch (m_prevThumbStickZone) {
			case thumbStickZone::right: return true;
			case thumbStickZone::downRight: OnBack(false); return true;
			case thumbStickZone::upRight: OnForward(false); return true;
			}
			break;
		case thumbStickZone::upLeft:
			switch (m_prevThumbStickZone) {
			case thumbStickZone::upLeft: return true;
			case thumbStickZone::up: OnLeft(true); return true;
			case thumbStickZone::left: OnForward(true); return true;
			case thumbStickZone::upRight: OnRight(false); OnLeft(true); return true;
			case thumbStickZone::downLeft: OnBack(false); OnForward(true); return true;
			}
			break;
		case thumbStickZone::downLeft:
			switch (m_prevThumbStickZone) {
			case thumbStickZone::downLeft: return true;
			case thumbStickZone::down: OnLeft(true); return true;
			case thumbStickZone::left: OnBack(true); return true;
			case thumbStickZone::downRight: OnRight(false); OnLeft(true); return true;
			case thumbStickZone::upLeft: OnForward(false); OnBack(true); return true;
			}
			break;
		case thumbStickZone::upRight:
			switch (m_prevThumbStickZone) {
			case thumbStickZone::upRight: return true;
			case thumbStickZone::up: OnRight(true); return true;
			case thumbStickZone::right: OnForward(true); return true;
			case thumbStickZone::upLeft: OnLeft(false); OnRight(true); return true;
			case thumbStickZone::downRight: OnBack(false); OnForward(true); return true;
			}
			break;
		case thumbStickZone::downRight:
			switch (m_prevThumbStickZone) {
			case thumbStickZone::downRight: return true;
			case thumbStickZone::down: OnRight(true); return true;
			case thumbStickZone::right: OnBack(true); return true;
			case thumbStickZone::downLeft: OnLeft(false); OnRight(true); return true;
			case thumbStickZone::upRight: OnForward(false); OnBack(true); return true;
			}
			break;
		}
		return false;
	}


	void ThumbstickTracer::UpdateThumbstickInputTrace(thumbStickZone a_newThumbstickZone) {
		if (!UpdateProximityThumbstickZone(a_newThumbstickZone)) {
			switch (m_prevThumbStickZone) {
			case thumbStickZone::up: OnForward(false); break;
			case thumbStickZone::down: OnBack(false); break;
			case thumbStickZone::left: OnLeft(false); break;
			case thumbStickZone::right: OnRight(false); break;
			case thumbStickZone::upLeft: OnForward(false); OnLeft(false); break;
			case thumbStickZone::upRight: OnForward(false); OnRight(false); break;
			case thumbStickZone::downLeft: OnBack(false); OnLeft(false); break;
			case thumbStickZone::downRight: OnBack(false); OnRight(false); break;
			}
			switch (a_newThumbstickZone) {
			case thumbStickZone::up: OnForward(true); break;
			case thumbStickZone::down: OnBack(true); break;
			case thumbStickZone::left: OnLeft(true); break;
			case thumbStickZone::right: OnRight(true); break;
			case thumbStickZone::upLeft: OnForward(true); OnLeft(true); break;
			case thumbStickZone::upRight: OnForward(true); OnRight(true); break;
			case thumbStickZone::downLeft: OnBack(true); OnLeft(true); break;
			case thumbStickZone::downRight: OnBack(true); OnRight(true); break;
			}
		}
		m_prevThumbStickZone = a_newThumbstickZone;

	}

	void ThumbstickTracer::OnThumbStickMovement(RE::ThumbstickEvent* a_thumbStickMovementInput) {
		UpdateThumbstickInputTrace(GetThumbStickZone(a_thumbStickMovementInput->xValue, a_thumbStickMovementInput->yValue));
	}

	float ThumbstickTracer::GetThumbstickAngle() const {
// Conversion to Skyrim angle convention - not used
//		float angle = -m_rad;
//		if (angle < 0) {
//			angle += 2.0 * PI;
//		}
//		return angle;

		return m_rad; 
	}

} // namespace IDRC
