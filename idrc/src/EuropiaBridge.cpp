// Europia multiplayer bridge for Intuitive Dragon Ride Control.
// Licensed GPL-3.0-or-later, same as IDRC. Original IDRC by staalo18.
#include "EuropiaBridge.h"
#include "DataManager.h"
#include "FlyingModeManager.h"
#include "CombatManager.h"
#include "ControlsManager.h"
#include "TargetReticleManager.h"
#include "CameraLockManager.h"
#include "DisplayManager.h"
#include "IDRCUtils.h"
#include "_ts_SKSEFunctions.h"
#include <chrono>
#include <atomic>

namespace IDRC::EuropiaBridge {
    namespace {
        constexpr std::string_view kPlugin = "IntuitiveDragonRideControl.esp";
        constexpr std::string_view kDragonborn = "Dragonborn.esm";
        constexpr std::string_view kSkyrim = "Skyrim.esm";

        bool s_formsTried = false;
        bool s_formsOk = false;
        bool s_dataInit = false;
        bool s_inUse = false;
        bool s_active = false;
        bool s_aliasOk = false;
        bool s_pending = false;

        RE::TESQuest* s_rideQuest = nullptr;
        RE::TESQuest* s_perchQuest = nullptr;
        RE::BGSRefAlias* s_dragonAlias = nullptr;
        RE::BGSRefAlias* s_perchTarget = nullptr;
        RE::BGSRefAlias* s_wordWall = nullptr;
        RE::BGSRefAlias* s_tower = nullptr;
        RE::BGSRefAlias* s_rock = nullptr;
        RE::TESObjectREFR* s_orbit = nullptr;
        RE::TESObjectREFR* s_turn = nullptr;
        RE::TESObjectREFR* s_travelTo = nullptr;
        RE::TESObjectREFR* s_flyTo = nullptr;
        RE::SpellItem* s_noFly = nullptr;
        RE::TESShout* s_attack = nullptr;
        RE::TESShout* s_uf = nullptr;

        RE::ActorHandle s_dragon;
        std::chrono::steady_clock::time_point s_lastTick{};
        std::chrono::steady_clock::time_point s_mountSince{};

        RE::BGSRefAlias* FindRefAlias(RE::TESQuest* a_quest, std::uint32_t a_id) {
            if (!a_quest) return nullptr;
            for (auto* alias : a_quest->aliases) {
                if (alias && alias->aliasID == a_id && alias->GetVMTypeID() == RE::BGSRefAlias::VMTYPEID) {
                    return static_cast<RE::BGSRefAlias*>(alias);
                }
            }
            return nullptr;
        }

        template <class T>
        T* Lookup(RE::FormID a_id, std::string_view a_file, const char* a_what) {
            auto* dh = RE::TESDataHandler::GetSingleton();
            T* form = dh ? dh->LookupForm<T>(a_id, a_file) : nullptr;
            log::info("EuropiaBridge: {} {:06X}@{} -> {}", a_what, a_id, a_file, form ? "ok" : "MISSING");
            return form;
        }

        bool LoadForms() {
            s_formsTried = true;
            s_rideQuest  = Lookup<RE::TESQuest>(0x800, kPlugin, "ride quest");
            s_perchQuest = Lookup<RE::TESQuest>(0x807, kPlugin, "perch quest");
            s_turn       = Lookup<RE::TESObjectREFR>(0x80C, kPlugin, "turn marker");
            s_travelTo   = Lookup<RE::TESObjectREFR>(0x811, kPlugin, "travel-to marker");
            s_flyTo      = Lookup<RE::TESObjectREFR>(0x812, kPlugin, "fly-to marker");
            s_attack     = Lookup<RE::TESShout>(0x803, kPlugin, "attack shout");
            s_orbit      = Lookup<RE::TESObjectREFR>(0x01EF71, kDragonborn, "orbit marker");
            s_noFly      = Lookup<RE::SpellItem>(0x0200EB, kDragonborn, "no-fly ability");
            s_uf         = Lookup<RE::TESShout>(0x016CF0, kSkyrim, "unrelenting force");
            s_dragonAlias = FindRefAlias(s_rideQuest, 1);
            s_perchTarget = FindRefAlias(s_rideQuest, 7);
            s_wordWall    = FindRefAlias(s_perchQuest, 1);
            s_tower       = FindRefAlias(s_perchQuest, 2);
            s_rock        = FindRefAlias(s_perchQuest, 3);
            log::info("EuropiaBridge: aliases dragon={} perchTarget={} wordWall={} tower={} rock={}",
                s_dragonAlias != nullptr, s_perchTarget != nullptr, s_wordWall != nullptr, s_tower != nullptr, s_rock != nullptr);
            s_formsOk = s_rideQuest && s_dragonAlias && s_turn && s_travelTo && s_flyTo && s_noFly && s_orbit;
            log::info("EuropiaBridge: forms ready = {}", s_formsOk);
            return s_formsOk;
        }

        std::uint32_t MappedKey(std::string_view a_event, std::uint32_t a_fallback) {
            auto* cm = RE::ControlMap::GetSingleton();
            if (cm) {
                auto key = cm->GetMappedKey(a_event, RE::INPUT_DEVICE::kKeyboard);
                if (key != 0xFF && key != 0 && key < 0x100) return key;
            }
            return a_fallback;
        }

        void SetKey(ControlsManager& a_cm, const char* a_name, std::uint32_t a_code) {
            a_cm.SetKeyMapping(a_name, DXScanCode(a_code));
        }

        // Mirrors _ts_DR_RideControlScript.InitVariables(), using the MCM defaults.
        void InitData() {
            if (s_dataInit) return;
            s_dataInit = true;
            log::info("EuropiaBridge: initialising IDRC data natively");
            Utils::SetINIVars();
            DataManager::GetSingleton().InitializeData(s_rideQuest, s_orbit, s_dragonAlias, "DRAGON", s_perchQuest,
                s_wordWall, s_tower, s_rock, s_perchTarget);
            FlyingModeManager::GetSingleton().InitializeData(s_turn, s_travelTo, s_flyTo, s_noFly);
            CombatManager::GetSingleton().InitializeData(s_uf, s_attack);
            auto& cm = ControlsManager::GetSingleton();
            cm.InitializeData();
            TargetReticleManager::GetSingleton().Initialize();
            auto& cam = CameraLockManager::GetSingleton();
            cam.SetInitiallyEnabled(true);
            cam.ResetEnabled();
            cam.SetIgnoredCameraPitch(8.0f);
            cam.SetYawOffsetStrength(0.25f);
            cam.SetPitchOffsetStrength(0.25f);
            cam.SetInvertPitchOffsetDuringFlight(true);
            auto& tr = TargetReticleManager::GetSingleton();
            tr.SetReticleMode(TargetReticleManager::ReticleMode::kOn);
            tr.SetReticleLockAnimationStyle(0);
            tr.SetUseTarget(false);
            tr.SetPrimaryTargetMode(static_cast<TargetReticleManager::TargetMode>(1 + 1));
            tr.SetMaxTargetDistance(8000.0f);
            tr.SetDistanceMultiplierSmall(1.0f);
            tr.SetDistanceMultiplierLarge(2.0f);
            tr.SetDistanceMultiplierExtraLarge(4.0f);
            tr.SetMaxTargetScanAngle(7.0f);
            DataManager::GetSingleton().SetDragonSpeeds(1.0f);
            DataManager::GetSingleton().SetRollAmplitude(0.5f);
            cm.SetInitialAutoCombatMode(false);
            DisplayManager::GetSingleton().SetDisplayAttackMessage(true);

            SetKey(cm, "Forward",          MappedKey("Forward", 0x11));        // W
            SetKey(cm, "Back",             MappedKey("Back", 0x1F));           // S
            SetKey(cm, "StrafeLeft",       MappedKey("Strafe Left", 0x1E));    // A
            SetKey(cm, "StrafeRight",      MappedKey("Strafe Right", 0x20));   // D
            SetKey(cm, "DisplayHealth",    MappedKey("Ready Weapon", 0x13));   // R
            SetKey(cm, "Run",              MappedKey("Run", 0x2A));            // LShift
            SetKey(cm, "ToggleAlwaysRun",  MappedKey("Toggle Always Run", 0x3A));
            SetKey(cm, "DragonUp",         0x16);  // U
            SetKey(cm, "DragonDown",       0x23);  // H
            SetKey(cm, "ToggleAutoCombat", 0x22);  // G
            SetKey(cm, "ToggleLockReticle",0x26);  // L
            SetKey(cm, "ToggleCameraLock", 0x2E);  // C
        }

        bool IsDragon(RE::Actor* a_actor) {
            if (!a_actor) return false;
            auto* race = a_actor->GetRace();
            return race && race->HasKeywordString("ActorTypeDragon");
        }

        void Begin(RE::Actor* a_dragon) {
            log::info("EuropiaBridge: player is riding dragon {:08X} '{}' - taking control", a_dragon->GetFormID(), a_dragon->GetName());
            InitData();

            if (!s_rideQuest->IsRunning()) {
                bool result = false;
                bool ok = s_rideQuest->EnsureQuestStarted(result, true);
                log::info("EuropiaBridge: ride quest was not running; EnsureQuestStarted ok={} result={} running={}", ok, result, s_rideQuest->IsRunning());
            } else {
                log::info("EuropiaBridge: ride quest already running");
            }

            s_dragonAlias->ForceRefTo(a_dragon);
            s_aliasOk = s_dragonAlias->GetActorReference() == a_dragon;
            log::info("EuropiaBridge: dragon alias filled = {}", s_aliasOk);

            s_dragon = a_dragon->GetHandle();
            s_inUse = true;
            s_active = true;

            if (s_noFly && !a_dragon->HasSpell(s_noFly)) {
                a_dragon->AddSpell(s_noFly);
            }
            a_dragon->AsActorState()->actorState2.allowFlying = false;
            a_dragon->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable03, 0);
            a_dragon->EvaluatePackage();

            FlyingModeManager::GetSingleton().SetFlyingModeFromPapyrus(3);  // landed
            bool reg = ControlsManager::GetSingleton().RegisterForControls(false, false);
            log::info("EuropiaBridge: controls registered = {} (W take off, mouse steer, U up, H down, S slow/land)", reg);
        }

        void End() {
            log::info("EuropiaBridge: player left the dragon - releasing control");
            ControlsManager::GetSingleton().UnregisterForControls();
            auto dragon = s_dragon.get();
            if (dragon) {
                if (s_noFly && dragon->HasSpell(s_noFly)) {
                    dragon->RemoveSpell(s_noFly);
                }
                dragon->AsActorState()->actorState2.allowFlying = true;
                dragon->EvaluatePackage();
            }
            s_active = false;
            s_dragon.reset();
        }
    }

    namespace {
        std::atomic<bool> s_turnBack{ false };
        std::atomic<float> s_turnBackYaw{ 0.0f };
        std::atomic<int> s_mouseN{ 0 }, s_mouseSwallowed{ 0 }, s_pathN{ 0 };
        std::atomic<float> s_pathYaw{ 0.0f }, s_pathPitch{ 0.0f };
        std::chrono::steady_clock::time_point s_lastCamDiag{};
    }
    void SetTurnBack(bool a_on, float a_yaw) { s_turnBackYaw = a_yaw; s_turnBack = a_on; }
    bool GetTurnBack(float& a_yaw) { a_yaw = s_turnBackYaw; return s_turnBack; }
    void NoteMouse(bool a_swallowed) { ++s_mouseN; if (a_swallowed) ++s_mouseSwallowed; }
    namespace { std::atomic<int> s_stage[6]{}; }
    void NoteStage(int a_stage) { if (a_stage >= 0 && a_stage < 6) ++s_stage[a_stage]; }
    void NotePath(float a_yaw, float a_pitch) { ++s_pathN; s_pathYaw = a_yaw; s_pathPitch = a_pitch; }

    bool IsInUse() { return s_inUse; }
    bool IsActive() { return s_active; }

    RE::Actor* GetDirectDragon() {
        if (!s_active) return nullptr;
        auto ptr = s_dragon.get();
        return ptr.get();
    }

    namespace {
        std::chrono::steady_clock::time_point s_lastDiag{}, s_lastLift{};
        bool s_noProcTold = false;

        float GroundAt(RE::NiPoint3 a_pos) {
            float h = -1.0e9f;
            if (auto* tes = RE::TES::GetSingleton()) {
                if (auto* ws = tes->GetRuntimeData2().worldSpace) {
                    float out = 0.0f;
                    if (ws->GetMaxHeightAt(a_pos, out)) h = out;
                }
            }
            return h;
        }

        // Europia: never let a ridden dragon fly into (or under) the ground. Multiplayer can push him below the terrain.
        void GroundGuard(std::chrono::steady_clock::time_point a_now) {
            auto dragon = s_dragon.get();
            if (!dragon) return;
            auto* d = dragon.get();
            bool hasProc = d->GetActorRuntimeData().currentProcess != nullptr;
            if (!hasProc && !s_noProcTold) { s_noProcTold = true; log::warn("EuropiaBridge: dragon has no AI process (multiplayer unloaded his AI?)"); }
            if (hasProc) s_noProcTold = false;
            auto pos = d->GetPosition();
            float ground = GroundAt(pos);
            int fs = _ts_SKSEFunctions::GetFlyingState(d);
            if (a_now - s_lastDiag > std::chrono::seconds(5)) {
                s_lastDiag = a_now;
                log::info("EuropiaBridge: dragon z={:.0f} ground={:.0f} flyingState={} process={} ridden={}", pos.z, ground, fs, hasProc, d->IsBeingRidden());
            }
            if (ground < -1.0e8f) return;
            bool airborne = (fs == 1 || fs == 2 || fs == 3);  // taking off, cruising, hovering (not landing/landed/perching)
            if (airborne && pos.z < ground + 150.0f && a_now - s_lastLift > std::chrono::milliseconds(750)) {
                s_lastLift = a_now;
                log::warn("EuropiaBridge: dragon too low (z={:.0f}, ground={:.0f}, state={}) - lifting him clear", pos.z, ground, fs);
                d->SetPosition(RE::NiPoint3(pos.x, pos.y, ground + 600.0f), true);
            }
        }
    }

    void Update() {
        auto now = std::chrono::steady_clock::now();
        if (now - s_lastTick < std::chrono::milliseconds(250)) return;
        s_lastTick = now;

        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player || !player->Is3DLoaded()) return;
        if (!s_formsTried) LoadForms();
        if (!s_formsOk) return;

        RE::NiPointer<RE::Actor> mount;
        bool riding = player->GetMount(mount) && mount && IsDragon(mount.get());

        if (riding && !s_active) {
            // wait ~1 s after mounting so the mount animation finishes first
            if (!s_pending) { s_pending = true; s_mountSince = now; return; }
            if (now - s_mountSince < std::chrono::milliseconds(1000)) return;
            s_pending = false;
            Begin(mount.get());
        } else if (!riding) {
            s_pending = false;
            if (s_active) End();
        } else if (s_active && mount.get() != s_dragon.get().get()) {
            End();
        }
        if (s_active) {
            GroundGuard(now);
            if (now - s_lastCamDiag > std::chrono::seconds(3)) {
                s_lastCamDiag = now;
                auto* pc = RE::PlayerCamera::GetSingleton();
                int camId = (pc && pc->currentState) ? static_cast<int>(pc->currentState->id) : -1;
                float fx = 0, fy = 0, cy = 0, cp = 0;
                if (camId == static_cast<int>(RE::CameraState::kDragon)) {
                    auto* st = static_cast<RE::ThirdPersonState*>(pc->currentState.get());
                    fx = st->freeRotation.x; fy = st->freeRotation.y;
                    cy = _ts_SKSEFunctions::GetYaw(st->rotation); cp = _ts_SKSEFunctions::GetPitch(st->rotation);
                }
                auto& cl = CameraLockManager::GetSingleton();
                auto* d = s_dragon.get().get();
                log::info("EuropiaBridge: cam state={} free=({:.2f},{:.2f}) yaw={:.2f} pitch={:.2f} lockOn={} locked={} mouse={} swallowed={} path={} pathYaw={:.2f} pathPitch={:.2f} mode={} dragonYaw={:.2f} turnBack={} planner=[{},{},{},{},{},{}] flyState={}",
                    camId, fx, fy, cy, cp, cl.IsEnabled(), cl.IsCameraLocked(), s_mouseN.exchange(0), s_mouseSwallowed.exchange(0),
                    s_pathN.exchange(0), static_cast<float>(s_pathYaw), static_cast<float>(s_pathPitch),
                    static_cast<int>(FlyingModeManager::GetSingleton().GetFlyingMode()), d ? d->GetAngleZ() : 0.0f, static_cast<bool>(s_turnBack),
                    s_stage[0].exchange(0), s_stage[1].exchange(0), s_stage[2].exchange(0), s_stage[3].exchange(0), s_stage[4].exchange(0), s_stage[5].exchange(0),
                    d ? _ts_SKSEFunctions::GetFlyingState(d) : -1);
            }
        }
    }
}
