#pragma once
// below offsets are from 'True Directional Movement':
// https://github.com/ersh1/TrueDirectionalMovement
// All credits go to the original author Ersh!

// variables
static uintptr_t g_worldToCamMatrix = RELOCATION_ID(519579, 406126).address();                       // 2F4C910, 2FE75F0
static RE::NiRect<float>* g_viewPort = (RE::NiRect<float>*)RELOCATION_ID(519618, 406160).address();  // 2F4DED0, 2FE8B98
static float* g_deltaTimeRealTime = (float*)RELOCATION_ID(523661, 410200).address(); 

// functions
typedef RE::NiAVObject*(__fastcall* tNiAVObject_LookupBoneNodeByName)(RE::NiAVObject* a_this, const RE::BSFixedString& a_name, bool a3);
static REL::Relocation<tNiAVObject_LookupBoneNodeByName> NiAVObject_LookupBoneNodeByName{ RELOCATION_ID(74481, 76207) };

typedef void(__fastcall* tNiQuaternion_SomeRotationManipulation)(RE::NiQuaternion& a1, float a2, float a3, float a4);
static REL::Relocation<tNiQuaternion_SomeRotationManipulation> NiQuaternion_SomeRotationManipulation{ RELOCATION_ID(69466, 70843) };

// END offsets from True Directional Movement

static float* g_maxLandTargetSearchRadius = (float*)RELOCATION_ID(509193, 381414).address();
static bool* g_FastTravelState = (bool*)RELOCATION_ID(517641, 404169).address();
static int* g_PatrolQueuedState = (int*)RELOCATION_ID(517642, 404174).address();

typedef bool*(__fastcall* StartVoiceShoutCast_t)(
    RE::Character* a_caster,
    RE::TESShout* a_shout,
    std::uint32_t a_wordIndex,
    RE::Actor* a_target);
static REL::Relocation<StartVoiceShoutCast_t> StartVoiceShoutCast{ RELOCATION_ID(37850, 38804) };

typedef bool*(__fastcall* StartCombat_t)(RE::Actor* a_this, RE::Actor* a_target, RE::CombatGroup* a_combatGroup);
static REL::Relocation<StartCombat_t> StartCombat{ RELOCATION_ID(37608, 38561) };

//typedef void*(__fastcall* MountedDragonTriggerLand_t)();
//static REL::Relocation<MountedDragonTriggerLand_t> MountedDragonTriggerLand{ RELOCATION_ID(39707, 40809) };

typedef void*(__fastcall* InitiateForcedLanding_t)(
    RE::Actor* a_actor, 
    RE::TESForm* a_targetForm, 
    bool a_allowCrash, 
    bool a_packageFlag4);
static REL::Relocation<InitiateForcedLanding_t> InitiateForcedLanding{ RELOCATION_ID(36417, 37411) };

typedef void*(__fastcall* SetAllowFlyingEx_t)(
    RE::BSScript::IVirtualMachine* a_vm,
    RE::VMStackID a_stackID,
    RE::Actor* a_actor,
    bool a_allow,
    bool a_allowSearch,
    bool a_allowCrash);
static REL::Relocation<SetAllowFlyingEx_t> SetAllowFlyingEx{ RELOCATION_ID(53923, 54745) };

typedef bool*(__fastcall* FindNavmeshTriangleForLocation_t)(
    RE::BSPathingLocation* a_loc, 
    RE::FindTriangleForLocationFilter* a_filter);
static REL::Relocation<FindNavmeshTriangleForLocation_t> FindNavmeshTriangleForLocation{ RELOCATION_ID(87985, 90369) };

typedef void**(__fastcall* GetCurrentPathingLocation_t)(
    RE::BSPathing* a_pathing,
    RE::BSPathingLocation* a_loc,
    RE::Actor* a_actor,
    std::uintptr_t param4);
static REL::Relocation<GetCurrentPathingLocation_t> GetCurrentPathingLocation{ RELOCATION_ID(29819, 30635) };
		

typedef RE::RefHandle*(__fastcall* GetRefHandle_t)(const RE::TESObjectREFR *a_objectRef,RE::RefHandle *a_handleBuffer);
static REL::Relocation<GetRefHandle_t> GetRefHandle{ RELOCATION_ID(19418, 19846) };
