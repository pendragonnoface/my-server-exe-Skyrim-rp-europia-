#pragma once

#include "RE/D/DragonCameraState.h"
#include <_ts_SKSEFunctions.h>
#include "RE/F/FindTriangleForLocationFilterCheckDeltaZ.h"

namespace Hooks
{
	class ReadyWeaponHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<std::uintptr_t> ReadyWeaponHandlerVtbl{ RE::VTABLE_ReadyWeaponHandler[0] };
			auto vtblShift = REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? 2 : 0;
			_ProcessButton = ReadyWeaponHandlerVtbl.write_vfunc(0x4 + vtblShift, ProcessButton);
		}

	private:
		static void ProcessButton(RE::ReadyWeaponHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data);

		static inline REL::Relocation<decltype(ProcessButton)> _ProcessButton;
	};

    class ExtraInteractionHook
    {
    public:
        static void Hook()
        {
            REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_ExtraInteraction[0] };
            _GetType = vtbl.write_vfunc(0x1, GetType);         // 0x1 = first virtual after destructor
            _IsNotEqual = vtbl.write_vfunc(0x2, IsNotEqual);   // 0x2 = second virtual after destructor
        }

    private:
        static RE::ExtraDataType GetType(const RE::ExtraInteraction* a_this);

        static bool IsNotEqual(const RE::ExtraInteraction* a_this, const RE::BSExtraData* a_rhs);

        static inline REL::Relocation<decltype(GetType)> _GetType;
        static inline REL::Relocation<decltype(IsNotEqual)> _IsNotEqual;
    };

    // MainUpdateHook is from 'True Directional Movement':
    // https://github.com/ersh1/TrueDirectionalMovement
    // All credits go to the original author Ersh!
	class MainUpdateHook
	{
	public:
		static void Hook()
		{
			auto& trampoline = SKSE::GetTrampoline();
			REL::Relocation<uintptr_t> hook{ RELOCATION_ID(35565, 36564) };  // 5B2FF0, 5D9F50, main update
			
			_Nullsub = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET1799(0x748, 0xC26, 0xC38), Nullsub);  // 5B3738, 5DAB76		
		}

	private:
		static void Nullsub();
		static inline REL::Relocation<decltype(Nullsub)> _Nullsub;		
		static void UpdateRotationMatrixDisplay(RE::NiAVObject* m_reference3D); // for debugging
	};

    class LookHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<std::uintptr_t> LookHandlerVtbl{ RE::VTABLE_LookHandler[0] };
			auto vtblShift = REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? 2 : 0;
			_ProcessThumbstick = LookHandlerVtbl.write_vfunc(0x2 + vtblShift, ProcessThumbstick);
			_ProcessMouseMove = LookHandlerVtbl.write_vfunc(0x3 + vtblShift, ProcessMouseMove);
		}

	private:
		static void ProcessThumbstick(RE::LookHandler* a_this, RE::ThumbstickEvent* a_event, RE::PlayerControlsData* a_data);
		static void ProcessMouseMove(RE::LookHandler* a_this, RE::MouseMoveEvent* a_event, RE::PlayerControlsData* a_data);

		static inline REL::Relocation<decltype(ProcessThumbstick)> _ProcessThumbstick;
		static inline REL::Relocation<decltype(ProcessMouseMove)> _ProcessMouseMove;
	};

	class DragonCameraStateHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<std::uintptr_t> DragonCameraStateVtbl{ RE::VTABLE_DragonCameraState[0] };
			_OnEnterState = DragonCameraStateVtbl.write_vfunc(0x1, OnEnterState);
			_UpdateRotation = DragonCameraStateVtbl.write_vfunc(0xE, UpdateRotation);
        	_GetCurrentRotation  = DragonCameraStateVtbl.write_vfunc(0x4, GetCurrentRotation);
		}

	private:
		static void OnEnterState(RE::DragonCameraState* a_this);
		static void UpdateRotation(RE::DragonCameraState* a_this);
		static void GetCurrentRotation(RE::DragonCameraState* a_this, RE::NiQuaternion& a_out);

		static inline REL::Relocation<decltype(OnEnterState)> _OnEnterState;
		static inline REL::Relocation<decltype(UpdateRotation)> _UpdateRotation;
		static inline REL::Relocation<decltype(GetCurrentRotation)> _GetCurrentRotation;
	};

/* UNUSED
	class FlightGoalHook
	{
	public:
		static void Hook()
		{
			_SetGoal = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(88429, 90853, 0),
				5,
				reinterpret_cast<std::uintptr_t>(SetGoal));
		}

	private:
		static bool SetGoal(std::uintptr_t* a_this, RE::BSPathingRequest** a_request);
		static inline std::uintptr_t _SetGoal{ 0 };
	};
*/
	class PathingHook
	{
	public:
		static void Hook()
		{
// required hooks:
			_SetGroundPath = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(88302, 90713, 0),
				5,
				reinterpret_cast<std::uintptr_t>(SetGroundPath));
/* unused
			_SetFlightPath = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(90008, 92493, 0),
				6,
				reinterpret_cast<std::uintptr_t>(SetFlightPath));
			
			_ProcessPathUpdate = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(88448, 90873, 0),
				6,
				reinterpret_cast<std::uintptr_t>(ProcessPathUpdate));

*/
			_FlightPlannerUpdate = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(90006, 92491, 0),
				6,
				reinterpret_cast<std::uintptr_t>(FlightPlannerUpdate));

/* for debugging:
			_SetupPathingRequest = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(40612, 41642, 0),
				5,
				reinterpret_cast<std::uintptr_t>(SetupPathingRequest));
*/
			_GetCurrentPathingLocation  = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(29819, 30635, 0),
				7,
				reinterpret_cast<std::uintptr_t>(GetCurrentPathingLocation ));

// Hooks for exploration / debugging only:
/*
			_GetCurrentMountCellOrWorldspaceForm = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(36393, 37384, 0),
				5,
				reinterpret_cast<std::uintptr_t>(GetCurrentMountCellOrWorldspaceForm));

			_TESPackage_sub_140437ac0 = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(28783, 29559, 0),
				5,
				reinterpret_cast<std::uintptr_t>(TESPackage_sub_140437ac0));

			_BuildFlyLandPath = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(30669, 31513, 0),
				5,
				reinterpret_cast<std::uintptr_t>(BuildFlyLandPath));

			_SetPathingLocFromPos = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(87972, 90353, 0),
				6,
				reinterpret_cast<std::uintptr_t>(SetPathingLocFromPos));

			_FindNavmeshTriangleForLocation = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(87985, 90369, 0),
				7,
				reinterpret_cast<std::uintptr_t>(FindNavmeshTriangleForLocation));

			_FindNavmeshTriangleForLocation2 = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(87983, 90367, 0),
				7,
				reinterpret_cast<std::uintptr_t>(FindNavmeshTriangleForLocation2));				
*/
		}

// member functions:
		static RE::BSPathing* GetPathingSingleton() {return m_pathingSingleton;};
	private:
// required hooks:
		static void SetGroundPath(std::uintptr_t  a_subPtr, std::uintptr_t* a_newNode, std::uintptr_t* a_newData);
//		static void SetFlightPath(std::uintptr_t  a_subPtr, std::uintptr_t* a_newNode, std::uintptr_t* a_newData);
		static void FlightPlannerUpdate(std::uintptr_t a_plannerSubPtr,
									float* a_deltaTime,
									float* a_outMovementIntention,
									void* a_context);
//		static void ProcessPathUpdate(std::uintptr_t a_arbiter,
//									void* a_newNode,
//									void* a_newData,
//									void* a_param4);
//		static void SetupPathingRequest(RE::Actor* _ts_a_actor, void* _ts_a_request, RE::NiPoint3* _ts_a_targetPos, float _ts_a_speed, RE::TESObjectREFR* _ts_a_targetRef);
		static void* GetCurrentPathingLocation (RE::BSPathing* a_pathing,RE::BSPathingLocation* a_loc, RE::Actor* a_actor, std::uintptr_t param4);

// Hooks for exploration / debugging only:
//		static RE::TESForm* GetCurrentMountCellOrWorldspaceForm(RE::Actor *_ts_a_actor,void* _ts_a_param2,void* _ts_a_param3, bool _ts_a_param4);
//		static void* TESPackage_sub_140437ac0(void* a_package, void* a_param2, void* a_param3, void* a_param4);
//		static bool BuildFlyLandPath(void** a_request, RE::BSPathingSolution* a_result);
//		static void SetPathingLocFromPos(RE::BSPathingLocation* a_loc, RE::NiPoint3 a_pos);
//		static bool FindNavmeshTriangleForLocation(RE::BSPathingLocation* a_loc, RE::FindTriangleForLocationFilter* a_filter);
//		static bool FindNavmeshTriangleForLocation2(RE::BSPathingLocation* a_loc, RE::FindTriangleForLocationFilter* a_filter);
		
// required hooks:
		static inline std::uintptr_t _SetGroundPath{ 0 };
//		static inline std::uintptr_t _SetFlightPath{ 0 };
		static inline std::uintptr_t _FlightPlannerUpdate{ 0 };
//		static inline std::uintptr_t _ProcessPathUpdate{ 0 };
//		static inline std::uintptr_t _SetupPathingRequest{ 0 };
		static inline std::uintptr_t _GetCurrentPathingLocation{ 0 };

// Hooks for exploration / debugging only:
//		static inline std::uintptr_t _GetCurrentMountCellOrWorldspaceForm{ 0 };
//		static inline std::uintptr_t _TESPackage_sub_140437ac0{ 0 };
//		static inline std::uintptr_t _BuildFlyLandPath{ 0 };
//		static inline std::uintptr_t _SetPathingLocFromPos{ 0 };
//		static inline std::uintptr_t _FindNavmeshTriangleForLocation{ 0 };
//		static inline std::uintptr_t _FindNavmeshTriangleForLocation2{ 0 };

// member functions:
		static void UpdateFlightPathData(std::byte* a_agent);
		static bool IsDragonPathingRequest(std::byte* a_agent);

		// unused:
//		static void LinearPathToTarget(std::byte** a_pathData, std::uint32_t a_startIndex, const RE::NiPoint3& a_targetPos);

// members variables:
		static inline RE::BSPathing* m_pathingSingleton = nullptr; // pathing singleton is initialize with the first vanilla GetCurrentPathingLocation call
		static inline std::uintptr_t m_actorOffset = REL::Module::get().version() >= SKSE::RUNTIME_SSE_1_6_629 ? 0xC0 : 0xB8;
		const static inline float m_minShoutDistance = 1000.f;
		const static inline float m_shoutHeight = 500.f;
	};

	class ProjectileLaunchHook
	{
	public:
		static void Hook()
		{
			_Launch = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(42928, 44108, 0),
				5,
				reinterpret_cast<std::uintptr_t>(Launch));
		}
	private:
		static RE::ProjectileHandle* Launch(RE::ProjectileHandle* a_result,  RE::Projectile::LaunchData& a_data);
		static inline std::uintptr_t _Launch{ 0 };

	};

// BEGIN - patches for vanilla crashes which have amplified frequency due to rapid cell traversal during dragon flight.
//         Patches have been implemented with Claude Sonnet 5, and have been reviewed.

	// FlushQueuedFormLoadsHook patches BGSSaveLoadGame::FlushQueuedFormLoads (REL: 34645, 35567) to guard virtual
	// dispatches against use-after-free and partially-initialized-form crashes. Dragon flight
	// causes rapid exterior cell loads/unloads; forms can be freed or only partially
	// initialized when these functions try to dispatch through their vtables.
	//
	// Guarded call sites (SE offset, AE offset):
	//   SE+0x111 / AE+0x111: 'call [rdx+0x160]' — TESForm::AsReference2   1st pass, on TESForm
	//   SE+0x1DF / AE+0x1C1: 'call [rax+0x80]'  — TESForm::InitLoadGame   1st pass, on TESForm
	//   SE+0x304 / AE+0x2E4: 'call [rax+0x88]'  — TESForm::FinishLoadGame 2nd pass, on TESForm
	//   SE+0x356 / AE+0x336: 'call [rax+0x160]' — TESForm::AsReference2   2nd pass, on TESObjectREFR
	//
	// The related respawn/unload save helper (FUN_14057b120 / FUN_1405ae2d0) is guarded
	// separately by CheckSaveGameHook below — see that class for details.
	class FlushQueuedFormLoadsHook
	{
	public:
		static void Hook();

	private:
		// Replaces 'call [rdx+0x160]' — TESForm::AsReference2 virtual dispatch.
		// a_form (rcx) = TESForm* from BGSLoadFormData::GetForm (non-null).
		// rdx = *a_form = vtable pointer — null or garbage when form has been freed.
		// Returns nullptr (safe: caller skips the form) when vtable is outside the image.
		// Also sanity-checks the *returned* TESObjectREFR* before handing it back: both call
		// sites use the result as a raw `this` with no further vtable dispatch — 1st pass does
		// an inline 'lock inc [ref+0x28]' refcount bump (SE/AE+0x195/0x195 relative to
		// FlushQueuedFormLoads — crash site 35567+0x195), 2nd pass calls
		// TESObjectREFR::sub_14028a930(ref) directly. An implausible ref (too small, or
		// pointing inside our own module image instead of the heap) is treated as stale and
		// nulled out here.
		static RE::TESObjectREFR* TESForm_AsReference2_Guard(RE::TESForm* a_form);

		// Replaces 'call [rax+0x80]' — TESForm::InitLoadGame virtual dispatch.
		// a_form (rcx) = TESForm*, a_loadBuffer (rdx) = BGSLoadGameBuffer*.
		// rax = *a_form = vtable pointer — garbage when form has been freed.
		// Skips the call silently when vtable is outside the image.
		static void TESForm_InitLoadGame_Guard(RE::TESForm* a_form, void* a_loadBuffer);

		// Replaces 'call [rax+0x88]' — TESForm::FinishLoadGame virtual dispatch (2nd pass).
		// a_form (rcx) = TESForm*, a_loadBuffer (rdx) = BGSLoadGameBuffer*.
		// rax = *a_form = vtable pointer — garbage when form has been freed.
		// Skips the call silently when vtable is outside the image.
		static void TESForm_FinishLoadGame_Guard(RE::TESForm* a_form, void* a_loadBuffer);

		static inline std::uintptr_t m_imageBase{ 0 };
		static inline std::uintptr_t m_imageEnd{ 0 };
	};

	// CheckSaveGameHook patches FUN_14057b120 (SE RELOC 34648) / FUN_1405ae2d0 (AE RELOC 35570) — the
	// respawn/unload save helper, invoked both from BGSSaveLoadGame::FlushQueuedFormLoads
	// and from cell activation (QueuedPromoteLargeReferencesTask path: 19799 -> 35570).
	// Per the decompiled pseudocode, this builds a BGSSaveFormBuffer for a reference that's
	// about to be unloaded/respawned and dispatches through its vtable to
	// TESObjectREFR::CheckSaveGame (+0x68) to decide whether/how to persist it before the
	// reference and its process data are torn down.
	//
	// SE+0x153 / AE+0x1A8: 'mov rcx,rsi; call [rax+0x68]' — CheckSaveGame, 6 bytes.
	// Both SE and AE compile 'call [rax+0x68]' as 'ff 50 68' (3-byte disp8).
	// Crash 35570+0x219 was inside SaveGame (CALL [R8+0x70] at AE+0x215, 4-byte 41ff5070);
	// guarding CheckSaveGame prevents reaching SaveGame for invalid/partial forms.
	// Also noted: 'call [rax+0x790]' at SE+0x10D / AE+0x166 on Character actors — TODO guard.
	// AE-only additional vtable dispatches before CheckSaveGame (no SE equivalent):
	//   AE+0x0A0: vtable+0x160, AE+0x0BA: vtable+0x478, AE+0x0CB: vtable+0x5a0, AE+0x0DF: vtable+0x470
		class CheckSaveGameHook
	{
	public:
		static void Hook();

	private:
		// Replaces 'call [rax+0x68]' — TESObjectREFR::CheckSaveGame virtual dispatch
		// in FUN_14057b120 / FUN_1405ae2d0 (the respawn/unload save helper).
		// a_ref (rcx) = TESObjectREFR*, a_saveFormBuffer (rdx) = BGSSaveFormBuffer*.
		// Returns std::int8_t (char) — 0 = don't save, non-zero = save.
		// On SE, RCX is restored to RSI (the ref) by the JMP thunk before this is called.
		// Guards against:
		//   (1) stale vtable (use-after-free): vtable outside image range → return 0
		//   (2) null parentCell (partially-initialized actor, e.g. loaded but cell already
		//       unloaded): a_ref->parentCell == nullptr → return 0, avoids null sub-pointer
		//       crash inside CheckSaveGame implementation (33243+0x56: mov ecx,[rax+0x04])
		static std::int8_t CheckSaveGame_Guard(RE::TESObjectREFR* a_ref, void* a_saveFormBuffer);

		static inline std::uintptr_t m_imageBase{ 0 };
		static inline std::uintptr_t m_imageEnd{ 0 };
	};

	// CreateSourceTextureResultHook patches a background-thread texture/resource creation wrapper (REL: 75509, 77301).
	//
	// FUN_140d6dd70 (SE 75509) / FUN_140dbc380 (AE 77301) zero-initializes a result-pointer
	// local, calls the actual creation routine (FUN_140d7dc50 / FUN_140dccc30) which only
	// populates that local on success, then UNCONDITIONALLY writes 'result->field_0x20 = 1'
	// through it — with no null check. When creation fails (corrupt/missing .dds, device
	// lost, OOM, etc. — matches crash dumps showing NiSourceTexture / BSResource::
	// CompressedArchiveStream / .dds paths on the stack) the pointer is still null and this
	// write is a guaranteed null-pointer access violation.
	//
	// Crash site (confirmed from 10 crash dumps, RAX == 0 / write address 0x20 in every one):
	//   SE+0x6A / AE+0x6C: 'mov dword ptr [rax+0x20], 0x1'  (7-byte instruction: c7 40 20 01 00 00 00)
	//
	// The faulting instruction operates on RAX implicitly (it's not a call), so it can't be
	// patched with write_call<5> directly like the FlushQueuedFormLoads guards above. Instead
	// a small thunk moves RAX into RCX, calls the guard (which performs the write only if
	// non-null) via an absolute (imm64) call, then jumps back past the original 7-byte
	// instruction via an absolute (imm64) jump.
	class CreateSourceTextureResultHook
	{
	public:
		static void Hook();

	private:
		// Replaces 'mov dword ptr [rax+0x20], 0x1'.
		// a_result (rcx, moved from rax by the thunk) = the result pointer populated by
		// FUN_140d7dc50/FUN_140dccc30 — null when resource/texture creation failed.
		// Performs the write only when a_result is non-null; otherwise it's a no-op.
		static void SetResultLoadedFlag_Guard(void* a_result);
	};

// END - patches for vanilla crashes


/*  UNUSED HOOKS:

	class CombatHook
	{
	public:
		static void Hook()
		{
			_StartCombat = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(37608, 38561, 0),
				5,
				reinterpret_cast<std::uintptr_t>(StartCombat));	
			_StopCombat = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(37613, 38566, 0),
				5,
				reinterpret_cast<std::uintptr_t>(StopCombat));
		}
	private:
		static void StopCombat(RE::Actor* a_this);
		static void StartCombat(RE::Actor* a_this, RE::Actor* a_target, RE::CombatGroup* a_combatGroup);
		static inline std::uintptr_t _StartCombat{ 0 };
		static inline std::uintptr_t _StopCombat{ 0 };
	};


	class DragonFlyLandHook
	{
	public:
		static void Hook()
		{
			_SetAllowFlying = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(22173, 22656, 0),
				5,
				reinterpret_cast<std::uintptr_t>(SetAllowFlying));
			_SetAllowFlyingEx_papyrus = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(53923, 54745, 0),
				6,
				reinterpret_cast<std::uintptr_t>(SetAllowFlyingEx_papyrus));
			_InitiateForcedLanding = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(36417, 37411, 0),
				6,
				reinterpret_cast<std::uintptr_t>(InitiateForcedLanding));
			_RunFlyLandPackageProcedure = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(38616, 39647, 0),
				5,
				reinterpret_cast<std::uintptr_t>(RunFlyLandPackageProcedure));
			_RunFlyLandProcedure = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(38639, 39670, 0),
				5,
				reinterpret_cast<std::uintptr_t>(RunFlyLandProcedure));
		}
	private:
		static bool SetAllowFlying(void* param_1,void* param_2,RE::Actor* param_3,void* param_4,
          void* param_5,void* param_6,void* param_7,void* param_8);
		static void SetAllowFlyingEx_papyrus(RE::BSScript::IVirtualMachine* a_vm,          // Papyrus VM (unused)
											RE::VMStackID          a_stackID,     // Papyrus call stack (unused)
											RE::Actor*                     a_actor,       // Target actor
											char                       a_bAllow,      // 1 = grant flying, 0 = revoke
											char                       a_searchFlag,  // Forwarded to GetCurrentMountCellOrWorldspaceForm
											void*                 a_pkgFlag       // Forwarded to _ts_InitiateForcedLanding as param_3
			);
		static void InitiateForcedLanding(RE::Actor* a_actor, RE::TESForm* a_targetForm, bool a_packageFlag3, bool a_packageFlag4);
		static void RunFlyLandPackageProcedure(RE::AIProcess* a_this, RE::Actor* a_actor);
		static void RunFlyLandProcedure(RE::AIProcess* a_this, RE::Actor* a_actor);
		static inline std::uintptr_t _SetAllowFlying{ 0 };
		static inline std::uintptr_t _SetAllowFlyingEx_papyrus{ 0 };
		static inline std::uintptr_t _InitiateForcedLanding{ 0 };
		static inline std::uintptr_t _RunFlyLandPackageProcedure{ 0 };
		static inline std::uintptr_t _RunFlyLandProcedure{ 0 };
	};

	class TestHook
	{
	public:
		static void Hook()
		{
			_HighProcessUpdate = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(38606, 39637, 0),
				5,
				reinterpret_cast<std::uintptr_t>(HighProcessUpdate));
			_GetProcedureIndexRunning = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(38196, 39156, 0),
				6,
				reinterpret_cast<std::uintptr_t>(GetProcedureIndexRunning));
			_PlayerCharacter_Update = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(39375, 40447, 0),
				5,
				reinterpret_cast<std::uintptr_t>(PlayerCharacter_Update));
			_ObjectRefActivate = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(19369, 19796, 0),
				7,
				reinterpret_cast<std::uintptr_t>(ObjectRefActivate));
			_FlyingMountTriggerLand = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(39709, 40811, 0),
				13,
				reinterpret_cast<std::uintptr_t>(FlyingMountTriggerLand));
			_FlyingMountActivate = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(36840, 37864, 0),
				6,
				reinterpret_cast<std::uintptr_t>(FlyingMountActivate));
		}
	private:
		static void HighProcessUpdate(RE::AIProcess* a_this, RE::Actor* a_actor);
		static std::uint32_t GetProcedureIndexRunning(RE::AIProcess* a_this);
		static void PlayerCharacter_Update(RE::PlayerCharacter* a_this, float a_param2);
		static bool ObjectRefActivate(void* a_this, void* a_activator,void* a_arg2,void* a_object,void* a_count,
               bool a_defaultProcessingOnly);
		static void FlyingMountTriggerLand(void* a_this, void* a_param2, void* a_param3, void* a_param4);
		static void FlyingMountActivate(void* a_this, void* a_param2);
		static inline std::uintptr_t _HighProcessUpdate{ 0 };
		static inline std::uintptr_t _GetProcedureIndexRunning{ 0 };
		static inline std::uintptr_t _PlayerCharacter_Update{ 0 };
		static inline std::uintptr_t _ObjectRefActivate{ 0 };
		static inline std::uintptr_t _FlyingMountTriggerLand{ 0 };
		static inline std::uintptr_t _FlyingMountActivate{ 0 };
	};


FastTravel-related hooks:
*************************

/*
	< NOTE: all addresses are for SE version. AE addresses differ. >

- PapyrusFastTravelHook::FastTravel: the papyrus function "FastTravel". This fires ExecuteTeleport via queuedTargetLoc.

- ExecuteTeleport is called once per frame from the main update loop (0x1405b2ff0).
	It fires if queuedTargetLoc is valid, eg via papyrus FastTravel request, COC, and on loadgame / at startup:

		[game startup / BGS load path]
			TeleportPlayerToCell (TeleportPlayertoCellHook)
			└─ ExecuteTeleport (ExecuteTeleportHook) (called once per frame, fires if queuedTargetLoc is valid)
					├─ TeleportToCell  (TeleportHook)              ← queuedTargetLoc has interior cell
					│    └─ SwitchToInteriorCell (SwitchToCellHook)
					└─ TeleportToWorldspacePosition (TeleportHook) ← queuedTargetLoc has worldspace
						└─ TeleportToCell (TeleportHook)
							├─ SwitchToInteriorCell (SwitchToCellHook)  (if target is interior; N/A on this path)
							├─ SwitchToWorldspace (SwitchToCellHook)    (if worldspace differs from current)
							└─ SwitchToExteriorCell (SwitchToCellHook)   (if target is exterior)
								└─ SwitchToWorldspace (SwitchToCellHook)  (conditional: interior-surplus purge path)

			[console: coc <x> <y>]
			ConsoleFunc__CenterOnExterior
			└─ PlayerCharacter__CenterOnCell (CenterOnCellHook)
				└─ ExecuteTeleport (ExecuteTeleportHook)  (same subtree as above)
			
			[papyrus FastTravel]
			└─ ExecuteTeleport (ExecuteTeleportHook)  (same subtree as above)

- UpdateFlyingMountFastTravelHook::UpdateFastTravel (0x1406badd0, REL::VariantID(39716, 40818, 0)) 
    is called once per frame from the main player update loop (0x14069e580).
	UpdateFastTravel is handling the flying mount logic once airborne. 
	It calls ApproachTarget (REL::VariantID(39705, 40807, 0)), and "DispatchFlyingMountTravelMode" (not hooked here, REL::VariantID(39714, 40816, 0)).
	Those two functions in turn trigger the FastTravelState and PatrolQueuedState updates
	(UpdateFastTravelState -> EnterFlyingMountFastTravelState, UpdatePatrolQueuedState). 
	PatroQueued states are:
		0 = idle
		1 = queue FastTravel    → EnterFlyingMountFastTravelState (REL::VariantID(39703, 40805, 0))
		2 = queue dismount      → FUN_1406ba750
		3 = queue set-injured   → FUN_1406ba940
		4 = queue eval-package  → FUN_1406bac00

- UpdateFlyingMountFastTravelHook::ApproachTarget handles the flying-mount approach logic toward the target.
	It is called by UpdateFastTravel, ExecuteArrival and ExecuteTeleport.

- UpdateFlyingMountFastTravelHook::ExecuteArrival is triggered through these call chains:
	* SneakHandler::ProcessButton (0x1407094c0)->0x1406b6030->TESObjectREFR::sub_1406ba9b0->0x1406ba710-> ExecuteArrival
	* ActivateHandler::sub_140708bf0->PlayerCharacter::sub_1406a9f90->0x1406ba8e0->ExecuteArrival
*/
/*

	class ExecuteTeleportHook
	{
	public:
		static void Hook()
		{
			_ExecuteTeleport = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(39366, 40438, 0),
				5,
				reinterpret_cast<std::uintptr_t>(ExecuteTeleport)
			);
		}

		static bool ExecuteTeleport(RE::PlayerCharacter* a_this);
		static void BlockNextExecuteTeleport(bool a_block) { m_blockExecuteTeleport = a_block; }
		static void SetTeleportPending(bool a_pending) { m_teleportPending = a_pending; }
	private:
		static inline std::uintptr_t _ExecuteTeleport{ 0 };
		static inline bool m_blockExecuteTeleport{ false };
		static inline bool m_teleportPending{ false };
	};

	class UpdateFlyingMountFastTravelHook
	{
	public:
		static void Hook()
		{
			_UpdateFastTravel = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(39716, 40818, 0),
				5,
				reinterpret_cast<std::uintptr_t>(UpdateFastTravel));

			_UpdateFastTravelState = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(39714, 40816, 0),
				15,
				reinterpret_cast<std::uintptr_t>(UpdateFastTravelState));

			_UpdatePatrolQueuedState = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(39702, 40804, 0),
				13,
				reinterpret_cast<std::uintptr_t>(UpdatePatrolQueuedState));

			_ApproachTarget = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(39705, 40807, 0),
				6,
				reinterpret_cast<std::uintptr_t>(ApproachTarget));

			_ExecuteArrival = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(39706, 40808, 0),
				5,
				reinterpret_cast<std::uintptr_t>(ExecuteArrival));
		}

		static void UpdateFastTravel(float a_deltaTime);
		static void UpdateFastTravelState();
		static void UpdatePatrolQueuedState(uint32_t a_mode);
		static void ApproachTarget(RE::NiPoint3* a_targetPos,
									std::uint64_t a_modeRaw,
									std::uint64_t a3,
									std::uint64_t a4);
		static void ExecuteArrival();
	private:
		static inline std::uintptr_t _UpdateFastTravel{ 0 };
		static inline std::uintptr_t _UpdateFlyingMountFastTravelState{ 0 };
		static inline std::uintptr_t _UpdateFastTravelState{ 0 };
		static inline std::uintptr_t _UpdatePatrolQueuedState{ 0 };
		static inline std::uintptr_t _ApproachTarget{ 0 };
		static inline std::uintptr_t _ExecuteArrival{ 0 };
	};


	class PapyrusFastTravelHook
	{
	public:
		static void Hook()
		{
			_FastTravel = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(54824, 55457, 0),
				6,
				reinterpret_cast<std::uintptr_t>(FastTravel));
		}

	private:
		static void FastTravel(RE::BSScript::IVirtualMachine* a_vm,       // Papyrus VM (for error reporting)
								RE::VMStackID                  a_stackID,  // Calling script's stack ID
								RE::StaticFunctionTag*         a_staticTag, // Static function tag placeholder (unused)
								RE::TESObjectREFR*             a_location );
		static inline std::uintptr_t _FastTravel{ 0 };
	};


END FastTravel-related hooks
****************************

	class VoiceSpellCastHook
	{
	public:
		static void Hook()
		{
			_HandleVoiceSpellCast = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(41740, 42821, 0),
				6,
				reinterpret_cast<std::uintptr_t>(HandleVoiceSpellCast));
		}
	private:
		static bool HandleVoiceSpellCast(std::uintptr_t  a_this, RE::Actor* a_caster);
		static inline std::uintptr_t _HandleVoiceSpellCast{ 0 };
	};

	class VoiceShoutCastHook
	{
	public:
		static void Hook()
		{
			_VoiceShoutCast = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(33663, 34443, 0),
				6,
				reinterpret_cast<std::uintptr_t>(VoiceShoutCast));
		}
		static void VoiceShoutCast(RE::Actor* a_caster);
	private:
		static inline std::uintptr_t _VoiceShoutCast{ 0 };
	};

	class StartCastHook
	{
	public:
		static void Hook()
		{
			_StartCast = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(41823, 42904, 0),
				6,
				reinterpret_cast<std::uintptr_t>(StartCast));
		}
		static bool StartCast(RE::Actor* a_caster, RE::MagicSystem::CastingSource a_source);
	private:
		static inline std::uintptr_t _StartCast{ 0 };
	};

	class CastSpellImmediateHook
	{
	public:
		static void Hook()
		{
			_CastSpellImmediate = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(33626, 34404, 0),
				5,
				reinterpret_cast<std::uintptr_t>(CastSpellImmediate));
		}
		static void CastSpellImmediate(
    RE::MagicCaster* _a_magicCaster,
    RE::MagicItem* _a_spell,
    bool _a_loadCast,
    RE::TESObjectREFR* _a_desiredTargetRef,
    float _a_effectivenessMult,
    bool _a_adjustOnlyHostileEffectiveness,
    float _a_magnitudeOverride);
	private:
		static inline std::uintptr_t _CastSpellImmediate{ 0 };
	};

	class ApplyCastHook
	{
	public:
		static void Hook()
		{
			_ApplyCast = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(33632, 34410, 0),
				7,
				reinterpret_cast<std::uintptr_t>(ApplyCast));
		}
		static bool ApplyCast(RE::MagicCaster* _a_magicCaster,
								float _a_effectivenessMult,
								std::uint32_t* _a_targetCount,
								RE::TESBoundObject* _a_source,
								bool _a_loadCast,
								bool _a_adjustOnlyHostileEffectiveness);
	private:
		static inline std::uintptr_t _ApplyCast{ 0 };
	};

	class TestCastHook
	{
	public:
		static void Hook()
		{
			_TestCast = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(33631, 34409, 0),
				5,
				reinterpret_cast<std::uintptr_t>(TestCast));
		}
		static bool TestCast(RE::MagicCaster* a_magicCaster,
								RE::MagicItem* a_spell,
								RE::Actor* a_target,
								RE::TESBoundObject* a_source,
								bool a_loadCast);

	private:
		static inline std::uintptr_t _TestCast{ 0 };
	};

	class ProcedureShoutHook
	{
	public:
		static void Hook()
		{
			_Initiate = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(28332, 29080, 0),
				5,
				reinterpret_cast<std::uintptr_t>(Initiate));
			_SetupExecState = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(28329, 29077, 0),
				5,
				reinterpret_cast<std::uintptr_t>(SetupExecState));
		}
		static void Initiate(std::uint64_t*  a_this,     // param_1: BGSProcedureShout*
    						std::uint64_t*  a_context);
		static void SetupExecState(std::uint64_t*  a_this,     // param_1: BGSProcedureShout*
								std::uint64_t*  a_context);
	private:
		static inline std::uintptr_t _Initiate{ 0 };
		static inline std::uintptr_t _SetupExecState{ 0 };
	};

	class StartVoiceShoutCastHook
	{
	public:
		static void Hook()
		{
			_StartVoiceShoutCast = _ts_SKSEFunctions::WriteFunctionHook(
				REL::VariantID(37850, 38804, 0),
				5,
				reinterpret_cast<std::uintptr_t>(StartVoiceShoutCast));
		}
		static bool StartVoiceShoutCast(RE::Character* a_caster,
										RE::TESShout* a_shout,
										std::uint32_t a_wordIndex,
										RE::Actor* a_target);
	private:
		static inline std::uintptr_t _StartVoiceShoutCast{ 0 };
	};
*/
	void Install();
} // namespace Hooks	

