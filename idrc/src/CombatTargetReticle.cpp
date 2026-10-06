#include "CombatTargetReticle.h"
#include "Offsets.h"
#include "_ts_SKSEFunctions.h"

// The CombatTargetReticle class is based on the TargetLockReticle implementation from 'True Directional Movement':
// https://github.com/ersh1/TrueDirectionalMovement
// All credits go to the original author Ersh!

namespace IDRC
{
	void CombatTargetReticle::Update(float a_deltaTime)
	{
		if (_interpTimer > 0) {
			_interpTimer -= a_deltaTime;

			if (_interpTimer < 0) {
				_interpTimer = 0;
			}

			_interpAlpha = 1.f - _interpTimer / _interpDuration;
		}

		if (_interpTimer == 0.f && _widgetState != WidgetState::kPendingRemoval) {
			_interpMode = InterpMode::kNone;
		}

		RE::GFxValue result;
		_object.Invoke("isReadyToRemove", &result);
		if (result.GetBool()) {
			_widgetState = WidgetState::kRemoved;
			return;
		}

		UpdatePosition();
		UpdateWidgetState();
	}

	void CombatTargetReticle::Initialize()
	{
		SetWidgetState(WidgetState::kActive);

		RE::GRectF rect = _view->GetVisibleFrameRect();
		_lastScreenPos.x = rect.right * 0.5f;
		_lastScreenPos.y = rect.bottom * 0.5f;
	}

	void CombatTargetReticle::WidgetReadyToRemove()
	{
		AddWidgetTask([=]() {
			RE::GFxValue arg;
			arg.SetBoolean(true);
			_object.Invoke("setWidgetReadyToRemove", nullptr, &arg, 1);
		});
	}	

	void CombatTargetReticle::Dispose()
	{
		_object.Invoke("cleanUp", nullptr, nullptr, 0);
	}

	void CombatTargetReticle::SetWidgetState(WidgetState a_widgetState)
	{
		_widgetState = a_widgetState;

		if (_widgetState == WidgetState::kActive) {
			StartInterpolation(InterpMode::kCrosshairToTarget);
		} else if (_widgetState == WidgetState::kPendingRemoval) {
			StartInterpolation(InterpMode::kTargetToCrosshair);
		}
	}

	void CombatTargetReticle::ChangeTarget(RE::ObjectRefHandle a_refHandle, RE::NiPointer<RE::NiAVObject> a_targetPoint)
	{

		AddWidgetTask([=]() {
			_refHandle = a_refHandle;
			_targetPoint = a_targetPoint;	

			if (_widgetState >= WidgetState::kPendingRemoval) {
				SetWidgetState(WidgetState::kActive);

				RE::GFxValue arg;
				arg.SetBoolean(false);
				_object.Invoke("setWidgetReadyToRemove", nullptr, &arg, 1);
			} else {
				StartInterpolation(InterpMode::kTargetToTarget);
				_object.Invoke("changeTarget", nullptr, nullptr, 0);
			}
		});
	}

	void CombatTargetReticle::UpdatePosition()
	{

		if (!_refHandle) {
			_widgetState = WidgetState::kRemoved;
			return;
		}

		RE::GRectF rect = _view->GetVisibleFrameRect();

		if (_interpMode == InterpMode::kTargetToCrosshair) {
			_desiredScreenPos.x = rect.right * 0.5f;
			_desiredScreenPos.y = rect.bottom * 0.5f;
		} else {
			RE::NiPoint3 targetWorldPos = _targetPoint ? _targetPoint->world.translate : _refHandle.get()->GetLookingAtLocation();

			RE::NiCamera::WorldPtToScreenPt3((float(*)[4])g_worldToCamMatrix, *g_viewPort, targetWorldPos, _desiredScreenPos.x, _desiredScreenPos.y, _depth, 1e-5f);

			_desiredScreenPos.y = 1.0f - _desiredScreenPos.y;  // Flip y for Flash coordinate system
			_desiredScreenPos.y = rect.top + (rect.bottom - rect.top) * _desiredScreenPos.y;
			_desiredScreenPos.x = rect.left + (rect.right - rect.left) * _desiredScreenPos.x;
		}

		RE::NiPoint2 screenPos;

		// if we're interpolating, lerp between the positions
		if (_interpTimer > 0) {
			screenPos.x = _ts_SKSEFunctions::InterpEaseIn(_lastScreenPos.x, _desiredScreenPos.x, _interpAlpha, 2);
			screenPos.y = _ts_SKSEFunctions::InterpEaseIn(_lastScreenPos.y, _desiredScreenPos.y, _interpAlpha, 2);
		} else {
			screenPos = _desiredScreenPos;
		}

		float scale = 100.f;

		RE::GFxValue::DisplayInfo displayInfo;
		displayInfo.SetPosition(screenPos.x, screenPos.y);
		displayInfo.SetScale(scale, scale);
		_object.SetDisplayInfo(displayInfo);
	}

	void CombatTargetReticle::UpdateWidgetState()
	{
		if (!_refHandle || !_refHandle.get()) {
			_widgetState = WidgetState::kRemoved;
			return;
		}

		auto actor = _refHandle.get()->As<RE::Actor>();
		if (!actor) {
			_widgetState = WidgetState::kRemoved;
			return;
		}

		AddWidgetTask([=]() {
			RE::GFxValue arg;
			arg.SetBoolean(_widgetState == kPendingRemoval ? true : false);

			_object.Invoke("updateWidgetState", nullptr, &arg, 1);
		});		
	}

	void CombatTargetReticle::StartInterpolation(InterpMode a_interpMode)
	{
		RE::GFxValue::DisplayInfo displayInfo;
		_object.GetDisplayInfo(&displayInfo);
		_lastScreenPos.x = static_cast<float>(displayInfo.GetX());
		_lastScreenPos.y = static_cast<float>(displayInfo.GetY());

		if (_interpMode == InterpMode::kCrosshairToTarget && a_interpMode == InterpMode::kTargetToCrosshair ||
			_interpMode == InterpMode::kTargetToCrosshair && a_interpMode == InterpMode::kCrosshairToTarget) {
			_interpDuration = _fullInterpDuration * _interpAlpha;
		} else {
			_interpDuration = _fullInterpDuration;
		}

		_interpTimer = _interpDuration;
		_interpMode = a_interpMode;
	}

	void CombatTargetReticle::UpdateState(bool a_isReticleLocked, bool a_isTDMLocked, int a_combatState)
	{
		AddWidgetTask([this, a_isReticleLocked, a_isTDMLocked, a_combatState]() {
			RE::GFxValue args[3];
			args[0].SetBoolean(a_isReticleLocked);
			args[1].SetBoolean(a_isTDMLocked);
			args[2].SetNumber(static_cast<double>(a_combatState));
			_object.Invoke("updateReticle", nullptr, args, 3);
		});
	}

	void CombatTargetReticle::SetReticleLockAnimationStyle(int a_style)
	{
		// a_style == 0: "Rotate"; a_style == 1: "Expand"
    	float circleInitZoom = 1.2f;
		if (a_style == 1) {
			circleInitZoom = 0.5f;
		}

		AddWidgetTask([this, circleInitZoom]() {
			RE::GFxValue arg;
			arg.SetNumber(circleInitZoom);
			_object.Invoke("SetCircleInitZoom", nullptr, &arg, 1);
		});
	}
}
