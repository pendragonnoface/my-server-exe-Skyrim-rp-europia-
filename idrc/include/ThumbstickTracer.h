#pragma once

#include "ControlsManager.h"

// ThumbstickTracer and ThumbstickTracerHook implementations are modified copies of
// the DTryKeyUtils movementInputTracer and hook_movement classes
// see: https://github.com/D7ry/DtryKeyUtil
// All credits go to DTry as the original author!

namespace IDRC
{
	class ThumbstickTracerHook
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> MovementHandlerVtbl{ RE::VTABLE_MovementHandler[0] };
			
			auto vtblShift = REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? 2 : 0;
			_ProcessThumbstick = MovementHandlerVtbl.write_vfunc(0x2 + vtblShift, ProcessThumbstick);
		}

		static void Uninstall()
        {
            REL::Relocation<std::uintptr_t> MovementHandlerVtbl{ RE::VTABLE_MovementHandler[0] };

            // Restore the original function pointer
            auto vtblShift = REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? 2 : 0;
            MovementHandlerVtbl.write_vfunc(0x2 + vtblShift, _ProcessThumbstick.address());
        }

	private:
		static void ProcessThumbstick(RE::MovementHandler* a_this, RE::ThumbstickEvent* a_event, RE::PlayerControlsData* a_data);

		static inline REL::Relocation<decltype(ProcessThumbstick)> _ProcessThumbstick;
	};


	class ThumbstickTracer
	{
	public:
		enum thumbStickZone {
			neutral = 0,
			up,
			down,
			left,
			right,
			upLeft,
			upRight,
			downLeft,
			downRight
		};

        static ThumbstickTracer& GetSingleton(){
            static ThumbstickTracer singleton;
            return singleton;
        }

		void InitializeData();

		void OnThumbStickMovement(RE::ThumbstickEvent* a_thumbstickMovementInput);
		bool IsThumbstickKeyPressed(IDRCKey a_key);
		bool IsThumbstickPressed();
		float GetThumbstickAngle() const;
	
	private:
		ThumbstickTracer() = default;
		ThumbstickTracer(const ThumbstickTracer&) = delete;
		ThumbstickTracer& operator=(const ThumbstickTracer&) = delete;

		void ProcessThumbstickKey(IDRCKey a_key);	
		inline void OnForward(bool a_activate = true);
		inline void OnBack(bool a_activate = true);
		inline void OnLeft(bool a_activate = true);
		inline void OnRight(bool a_activate = true);
		inline bool UpdateProximityThumbstickZone(thumbStickZone a_newThumbstickZone);
		inline void UpdateThumbstickInputTrace(thumbStickZone a_newThumbstickZone);
		inline thumbStickZone GetThumbStickZone(float a_x, float a_y);

		thumbStickZone m_prevThumbStickZone = thumbStickZone::neutral;
		bool m_leftPressed = false;
		bool m_rightPressed = false;
		bool m_forwardPressed = false;
		bool m_backPressed = false;

		float m_rad = 0.0f;
	};
}
