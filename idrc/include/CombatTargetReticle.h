#pragma once
#include "API/TrueHUDAPI.h"

// The CombatTargetReticle class is based on the TargetLockReticle implementation from 'True Directional Movement':
// https://github.com/ersh1/TrueDirectionalMovement
// All credits go to the original author Ersh!

namespace IDRC
{
	class CombatTargetReticle : public TRUEHUD_API::WidgetBase
	{
	public:
		enum InterpMode : std::uint8_t
		{
			kNone,
			kCrosshairToTarget,
			kTargetToTarget,
			kTargetToCrosshair
		};

		CombatTargetReticle(uint32_t a_widgetID, RE::ObjectRefHandle a_refHandle, RE::NiPointer<RE::NiAVObject> a_targetPoint) :
			WidgetBase(a_widgetID),
			_refHandle(a_refHandle),
			_targetPoint(a_targetPoint)
		{}

		CombatTargetReticle(uint32_t a_widgetID, RE::ObjectRefHandle a_refHandle, RE::NiPointer<RE::NiAVObject> a_targetPoint,
							int a_reticleLockAnimationStyle) :
			WidgetBase(a_widgetID),
			_refHandle(a_refHandle),
			_targetPoint(a_targetPoint)
		{
			SetReticleLockAnimationStyle(a_reticleLockAnimationStyle);
		}

		virtual void Update(float a_deltaTime) override;
		virtual void Initialize() override;
		virtual void Dispose() override;
		void WidgetReadyToRemove();
		void UpdateState(bool a_isReticleLocked, bool a_isTDMLocked, int a_combatState);
		void SetReticleLockAnimationStyle(int a_style);

		virtual void SetWidgetState(WidgetState a_widgetState);
		virtual void ChangeTarget(RE::ObjectRefHandle a_refHandle, RE::NiPointer<RE::NiAVObject> a_targetPoint);

		RE::ObjectRefHandle _refHandle;
		RE::NiPointer<RE::NiAVObject> _targetPoint;

	protected:
		virtual void UpdatePosition();
		virtual void UpdateWidgetState();
		virtual void StartInterpolation(InterpMode a_interpMode);

	private:
		float _interpTimer = 0.f;
		float _interpDuration = 0.f;
		float _interpAlpha = 0.f;

		RE::NiPoint2 _lastScreenPos;
		RE::NiPoint2 _desiredScreenPos;

		InterpMode _interpMode = kNone;

		static constexpr float _fullInterpDuration = 0.5f;
	};
}
