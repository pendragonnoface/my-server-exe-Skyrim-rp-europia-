#include "EuropiaBridge.h"
#include "Hooks.h"
#include "TargetReticleManager.h"
#include "CameraLockManager.h"
#include "FlyingModeManager.h"
#include "DataManager.h"
#include "CombatManager.h"
#include "FastTravelManager.h"
#include "FlapThrustHandler.h"
#include "IDRCUtils.h"
#include "APIManager.h"
#include "REL/RuntimeDataAccessors.h"

namespace Hooks
{
	void Install()
	{
		log::info("Hooking...");

		MainUpdateHook::Hook();

//		ReadyWeaponHook::Hook();
//		ExtraInteractionHook::Hook();
		LookHook::Hook();
		DragonCameraStateHook::Hook();
//		GetMountHook::Hook();
//		FlightGoalHook::Hook();
		PathingHook::Hook();
		ProjectileLaunchHook::Hook();

		// Patches below have been implemented with Claude Sonnet 5 to fix vanilla crashes
		// IDRC amplifies the occurance of these crashes, as the dragon traverses quickly over 
		// many cells can cause stale pointers to be used in the hooked functions
		// (in particular on slower systems). The hooks below guard against stale vtable pointers
		// and nullify the corresponding parentCell pointers.
		FlushQueuedFormLoadsHook::Hook();
		CheckSaveGameHook::Hook();
// NOTE: CreateSourceTextureResultHook fixes crashing but sometimes the engine will freeze (a few secs) later
// So in many cases a crash is prevented, but some of the prevented crashes end up becoming a freeze...
		CreateSourceTextureResultHook::Hook();
/* UNUSED HOOKS:
		TestHook::Hook();
		DragonFlyLandHook::Hook();
		ExecuteTeleportHook::Hook();
		PapyrusFastTravelHook::Hook();
		UpdateFlyingMountFastTravelHook::Hook();
		CombatHook::Hook();
		VoiceSpellCastHook::Hook();
		VoiceShoutCastHook::Hook();
		StartCastHook::Hook();
		CastSpellImmediateHook::Hook();
		ApplyCastHook::Hook();
		TestCastHook::Hook();
		ProcedureShoutHook::Hook();
		StartVoiceShoutCastHook::Hook();
*/
		log::info("...success");
	}

	void ParseAnimationGraph() {
		auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			log::error("{}: PlayerCharacter is null", __FUNCTION__);
			return;
		}
		log::info("{}: start parsing AnimationGraph", __FUNCTION__);

		RE::BSAnimationGraphManagerPtr animGraph;
		if (player->GetAnimationGraphManager(animGraph)) {
			if (animGraph) {
				auto* mgr = animGraph.get();
				if (mgr) {
					// Iterate all graphs in the manager
					for (auto& graphPtr : mgr->graphs) {
						auto* graph = graphPtr.get();
						if (graph) {
							// Dump all variables in the graph's variableCache
							for (auto& varInfo : mgr->variableCache.variableCache) {
								if (varInfo.variableName.data() && varInfo.variable) {
									// Print as bool/int/float if you know the type, otherwise just print pointer value
//									log::info("AnimVar: {} = [ptr: {}]", varInfo.variableName.c_str(), static_cast<void*>(varInfo.variable));
									if ((varInfo.variableName == "iState" ||
										varInfo.variableName == "iLeftHandType" ||
										varInfo.variableName == "iRightHandType" ||
										varInfo.variableName == "iSyncSprintState")
										&& varInfo.variable) {
										int32_t value = *reinterpret_cast<int32_t*>(varInfo.variable);
										log::info("AnimVar: {} = {}", varInfo.variableName.c_str(), value);
									} else if ((varInfo.variableName == "Speed" ||
										varInfo.variableName == "Direction" ||
										varInfo.variableName == "TurnDelta" ||
										varInfo.variableName == "TDM_VelocityX" ||
										varInfo.variableName == "TDM_VelocityY" ||
										varInfo.variableName == "TDM_Pitch" ||
										varInfo.variableName == "TDM_Roll" ||
										varInfo.variableName == "HorseSpeedSampled" ||
										varInfo.variableName == "SpeedSampled")
										&& varInfo.variable) {
										float value = *reinterpret_cast<float*>(varInfo.variable);
										log::info("AnimVar: {} = {}", varInfo.variableName.c_str(), value);
									} else {								
										bool value = *reinterpret_cast<bool*>(varInfo.variable);
										log::info("AnimVar: {} = {}", varInfo.variableName.c_str(), value ? "true" : "false");
									} 
								} else {
									log::error("{}: Variable name or pointer is null", __FUNCTION__);
								}
							}
						}
						else {
							log::error("{}: GraphPtr is null", __FUNCTION__);
						}
					}
				} else {
					log::error("{}: BSAnimationGraphManagerPtr is null", __FUNCTION__);
				}
			} else {
				log::error("{}: AnimationGraphManagerPtr is null", __FUNCTION__);
			}
		} else {
			log::error("{}: GetAnimationGraphManager failed", __FUNCTION__);
		}
		log::info("{}: Finished parsing AnimationGraph", __FUNCTION__);		
	}

/*****************************************************************************************/

	void MainUpdateHook::Nullsub()
	{
		_Nullsub();

        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return;
        }

		IDRC::EuropiaBridge::Update();
		IDRC::CameraLockManager::GetSingleton().Update();
		IDRC::TargetReticleManager::GetSingleton().Update();
		IDRC::CombatManager::GetSingleton().Update();
		IDRC::FlyingModeManager::GetSingleton().Update();
		IDRC::FastTravelManager::GetSingleton().Update();
//		IDRC::FlapThrustHandler::GetSingleton().Update();

// For debugging
/*
		auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (dragonActor) {
			
			auto combatState = _ts_SKSEFunctions::GetCombatState(dragonActor);
			bool isAllowedToFly = dragonActor->AsActorState()->actorState2.allowFlying;
			int flyingState = _ts_SKSEFunctions::GetFlyingState(dragonActor);
log::info("{}: FastTravelFlag={}, PatrolQueuedFlag={}, combatState={}, AV3= {}, isAllowedToFly= {}, flyingState= {}", __FUNCTION__, 
*g_FastTravelState, *g_PatrolQueuedState, combatState,
dragonActor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kVariable03), isAllowedToFly, flyingState);



if (*g_PatrolQueuedState) {
log::info("{}: ----------------->>>>>>>>>>>>>>>> PATROLQUEUED", __FUNCTION__);
}
bool TS_patrolQueuedState =_ts_SKSEFunctions::IsFlyingMountPatrolQueued(dragonActor);
bool TS_fastTravelState = _ts_SKSEFunctions::IsFlyingMountFastTravelling(dragonActor);
if (TS_patrolQueuedState != static_cast<bool>(*g_PatrolQueuedState) || TS_fastTravelState != static_cast<bool>(*g_FastTravelState)) {
log::info("{}: ----------->>>>>>>>>>>>  Mismatch detected between TS flags and global flags", __FUNCTION__);
log::info("{}: TS Flags - TS_FastTravelState={}, TS_PatrolQueuedState={}", __FUNCTION__, TS_fastTravelState, TS_patrolQueuedState);
log::info("{}: Flags - FastTravelFlag={}, PatrolQueuedFlag={}", __FUNCTION__, *g_FastTravelState, *g_PatrolQueuedState);
}



		}

			auto currentPackage = dragonActor->GetCurrentPackage();
			if (currentPackage) {
				auto packageType = currentPackage->packData.packType;
				auto procedureType = currentPackage->procedureType;
log::info("{}: DragonActor current package: {:0x}, packType: {}, procedureType: {}", __FUNCTION__, currentPackage->GetFormID(), static_cast<uint8_t>(packageType.underlying()), static_cast<uint32_t>(procedureType.underlying()));
			}

			auto storedCombatTarget = IDRC::CombatManager::GetSingleton().GetStoredCombatTarget();
log::info("{}: DragonActor storedCombatTarget: {:0x}", __FUNCTION__, storedCombatTarget ? storedCombatTarget->GetFormID() : 0);

			RE::NiPoint3 testPos = dragonActor->GetPosition();
			int flyingState = _ts_SKSEFunctions::GetFlyingState(dragonActor);
//			testPos.z = _ts_SKSEFunctions::GetLandHeightWithWater(testPos, false);
//			bool isValidForLanding = PathingHook::IsPositionValidForLanding(testPos);
log::info("{}: DragonActor position: {}, flyingState={}", __FUNCTION__, dragonActor->GetPosition(), flyingState);
		}

		auto* playerCamera = RE::PlayerCamera::GetSingleton();
		RE::ThirdPersonState* dragonCameraState = nullptr;

		if (playerCamera && playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kDragon)) {
			dragonCameraState = static_cast<RE::ThirdPersonState*>(playerCamera->currentState.get());
			if (dragonCameraState) {
log::info("{}: AfterMainUpdate: freeRotationY: {}", __FUNCTION__, 180.f/PI * dragonCameraState->freeRotation.y);
			}
		}
/*
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (player) {
			auto playerPackage = player->GetCurrentPackage();
			if (playerPackage) {
				auto playerPackageType = playerPackage->packData.packType;
				auto playerProcedureType = playerPackage->procedureType;
log::info("{}: PlayerCharacter current package: {:0x}, packType: {}, procedureType: {}", __FUNCTION__, playerPackage->GetFormID(), static_cast<uint8_t>(playerPackageType.underlying()), static_cast<uint32_t>(playerProcedureType.underlying()));
			}
		}

		if (player) {
			UpdateRotationMatrixDisplay(player->Get3D());
		}

        if (dragonActor) {
            UpdateRotationMatrixDisplay(dragonActor->Get3D());
        }			
*/
	}


    void MainUpdateHook::UpdateRotationMatrixDisplay(RE:: NiAVObject* m_reference3D) {

        if (m_reference3D) {
            // Extract rotation from body part node
            const RE::NiMatrix3& rotMatrix = m_reference3D->world.rotate;


            auto rotX = rotMatrix.GetVectorX();
            auto rotY = rotMatrix.GetVectorY();
            auto rotZ = rotMatrix.GetVectorZ();

            if (APIs::TrueHUD) {
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate, m_reference3D->world.translate + 20.f * rotX, 0.1f, 0xFF0000FF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 20.f * rotX, m_reference3D->world.translate + 40.f * rotX, 0.1f, 0xFFFFFFFF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 40.f * rotX, m_reference3D->world.translate + 60.f * rotX, 0.1f, 0xFF0000FF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 60.f * rotX, m_reference3D->world.translate + 80.f * rotX, 0.1f, 0xFFFFFFFF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 80.f * rotX, m_reference3D->world.translate + 100.f * rotX, 0.1f, 0xFF0000FF);

                APIs::TrueHUD->DrawLine(m_reference3D->world.translate, m_reference3D->world.translate + 20.f * rotY, 0.1f, 0x00FF00FF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 20.f * rotY, m_reference3D->world.translate + 40.f * rotY, 0.1f, 0xFFFFFFFF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 40.f * rotY, m_reference3D->world.translate + 60.f * rotY, 0.1f, 0x00FF00FF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 60.f * rotY, m_reference3D->world.translate + 80.f * rotY, 0.1f, 0xFFFFFFFF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 80.f * rotY, m_reference3D->world.translate + 100.f * rotY, 0.1f, 0x00FF00FF);

                APIs::TrueHUD->DrawLine(m_reference3D->world.translate, m_reference3D->world.translate + 20.f * rotZ, 0.1f, 0x0000FFFF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 20.f * rotZ, m_reference3D->world.translate + 40.f * rotZ, 0.1f, 0xFFFFFFFF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 40.f * rotZ, m_reference3D->world.translate + 60.f * rotZ, 0.1f, 0x0000FFFF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 60.f * rotZ, m_reference3D->world.translate + 80.f * rotZ, 0.1f, 0xFFFFFFFF);
                APIs::TrueHUD->DrawLine(m_reference3D->world.translate + 80.f * rotZ, m_reference3D->world.translate + 100.f * rotZ, 0.1f, 0x0000FFFF);

            } else {
                log::info("{}: TrueHUD API not available for debug drawing", __FUNCTION__);
            }
        } else {
            log::info("{}: No target point found", __FUNCTION__);
        }
    }

/*****************************************************************************************/

	static bool bTraceGetType = false;

	void ReadyWeaponHook::ProcessButton(RE::ReadyWeaponHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data)
	{
		
		// call original function so other plugins can hook this vfunc properly
		bTraceGetType = true; // reset trace flag
        _ProcessButton(a_this, a_event, a_data);
		bTraceGetType = false; // reset trace flag

		auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			log::error("{}: PlayerCharacter is null", __FUNCTION__);
			return;
		}
/*
		if(!actor) {
			player->lastRiddenHorseHandle = 0;
		} else {
			player->lastRiddenHorseHandle = actor->CreateRefHandle();
		}

		bool wasOnDragon = false;
		bool wasOnMount = false;

		// Save current state
		player->GetGraphVariableBool("IsOnDragon", wasOnDragon);
		player->GetGraphVariableBool("IsOnMount", wasOnMount);
*/
//		auto has_kInteraction = player->extraList.HasType(RE::ExtraDataType::kInteraction);
//		log::info("{}: before: has kInteraction: {}", __FUNCTION__, has_kInteraction ? "true" : "false");
//		player->extraList.RemoveByType(RE::ExtraDataType::kInteraction);
//		has_kInteraction = player->extraList.HasType(RE::ExtraDataType::kInteraction);
//		log::info("{}: after: has kInteraction: {}", __FUNCTION__, has_kInteraction ? "true" : "false");
//		log::info("{}: Player is on Dragon: {}, on Mount: {}", __FUNCTION__, wasOnDragon ? "true" : "false", wasOnMount ? "true" : "false");	
//		player->NotifyAnimationGraph("BeginWeaponDraw");
//ParseAnimationGraph();
log::info("{}: ReadyWeaponHook-ProcessButton called with event IDCode = {}", __FUNCTION__, a_event->GetIDCode());
//		player->DrawWeaponMagicHands(true);
//log::info("{}: DrawWeaponMagicHands called", __FUNCTION__);
//ParseAnimationGraph();

//		log::info("{}: ProcessButton called with event IDCode = {}", __FUNCTION__, a_event->GetIDCode());
	}

		RE::ExtraDataType ExtraInteractionHook::GetType(const RE::ExtraInteraction* a_this)
        {
			if (bTraceGetType) {
				log::info("{}: GetType called, will return {}", __FUNCTION__, static_cast<uint32_t>(_GetType(a_this)));
			}
            return _GetType(a_this);
        }
        bool ExtraInteractionHook::IsNotEqual(const RE::ExtraInteraction* a_this, const RE::BSExtraData* a_rhs)
        {
			log::info("{}: IsNotEqual called, will return {}", __FUNCTION__, _IsNotEqual(a_this, a_rhs) ? "true" : "false");
            return _IsNotEqual(a_this, a_rhs);
        }

/*****************************************************************************************/

	void LookHook::ProcessThumbstick(RE::LookHandler* a_this, RE::ThumbstickEvent* a_event, RE::PlayerControlsData* a_data)
	{
		auto& cameraLockManager = IDRC::CameraLockManager::GetSingleton();
		if (a_event && a_event->IsRight() && cameraLockManager.IsCameraLocked())
		{
			return;
		}
		else
		{
			cameraLockManager.SetUserTurning(true);
			_ProcessThumbstick(a_this, a_event, a_data);
		}
	}

	void LookHook::ProcessMouseMove(RE::LookHandler* a_this, RE::MouseMoveEvent* a_event, RE::PlayerControlsData* a_data)
	{
		auto& cameraLockManager = IDRC::CameraLockManager::GetSingleton();
		if (a_event && cameraLockManager.IsCameraLocked())
		{
			IDRC::EuropiaBridge::NoteMouse(true);
			return;
		}
		else
		{
			IDRC::EuropiaBridge::NoteMouse(false);
			cameraLockManager.SetUserTurning(true);
			_ProcessMouseMove(a_this, a_event, a_data);
		}
	}

/*****************************************************************************************/

	void DragonCameraStateHook::OnEnterState(RE::DragonCameraState* a_this)
	{
		_OnEnterState(a_this);

		RE::Actor* dragon = nullptr;
		dragon = static_cast<RE::Actor*>(a_this->dragonHandle.get().get());

		if (dragon) {
			reinterpret_cast<float&>(a_this->unkEC) = dragon->GetHeading(false); //dragonCurrentDirection

			// TODO: also update pitch? What is the best pitch at the end of the mount animation...
		}
	}

	void DragonCameraStateHook::UpdateRotation(RE::DragonCameraState* a_this)
	{
		float dragonCurrentDirection = reinterpret_cast<float&>(a_this->unkEC);
		float freeRotationX = a_this->freeRotation.x;

		a_this->freeRotationEnabled = true;

		_UpdateRotation(a_this);

		reinterpret_cast<float&>(a_this->unkEC) = dragonCurrentDirection;
		a_this->freeRotation.x = freeRotationX;

		if (a_this->dragonHandle) {
			RE::Actor* dragon = nullptr;
			dragon = static_cast<RE::Actor*>(a_this->dragonHandle.get().get());
			if (dragon) {
				float heading = dragon->GetHeading(false);

				a_this->freeRotation.x += reinterpret_cast<float&>(a_this->unkEC) - heading;

				NiQuaternion_SomeRotationManipulation(a_this->rotation, -a_this->freeRotation.y, 0.f, heading + a_this->freeRotation.x);
				reinterpret_cast<float&>(a_this->unkEC) = heading;
			}
		}
	}

    void DragonCameraStateHook::GetCurrentRotation(RE::DragonCameraState* a_this, RE::NiQuaternion& a_out)
    {
        _GetCurrentRotation(a_this, a_out);

        auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            return;
        }

		float targetRoll = IDRC::Utils::GetDragonRoll();

		const float rollSmoothTime    = IDRC::DataManager::GetSingleton().GetRollSmoothTime();
		const float rollAmplitude = IDRC::DataManager::GetSingleton().GetRollAmplitude();
		static float smoothedRoll   = 0.f;
		static float rollVelocity   = 0.f;  // radians/sec

		if (!RE::UI::GetSingleton()->GameIsPaused()) {
			const float dt = _ts_SKSEFunctions::GetRealTimeDeltaTime();
			const float omega = 2.0f / rollSmoothTime;
			const float e = expf(-omega * dt);
			const float c1 = smoothedRoll - targetRoll;
			const float c2 = rollVelocity + omega * c1;
			smoothedRoll = targetRoll + (c1 + c2 * dt) * e;
			rollVelocity = (c2 - omega * (c1 + c2 * dt)) * e;
		}

		float effectiveRoll = smoothedRoll * rollAmplitude;

		if (effectiveRoll == 0.0f) {
			return;
		}

		const float c = cosf(effectiveRoll * 0.5f);
		const float s = sinf(effectiveRoll * 0.5f);

		// Right-multiply by q_roll = (c, 0, s, 0)  [Y-axis rotation]
		// q_new = a_out * q_roll
		const RE::NiQuaternion q = a_out;
		a_out.w =  q.w * c - q.y * s;
		a_out.x =  q.x * c - q.z * s;
		a_out.y =  q.w * s + q.y * c;
		a_out.z =  q.x * s + q.z * c;
	}

/*****************************************************************************************/

/*
	bool FlightGoalHook::SetGoal(std::uintptr_t* a_this, RE::BSPathingRequest** a_request)
	{
///////////
// NOT USED
///////////
		if (_SetGoal == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}

		// call the original function
		using FuncType = decltype(&SetGoal);
		auto result = reinterpret_cast<FuncType>(_SetGoal)(a_this, a_request);
		auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();

		if (dragonActor) {
			if (IDRC::FlyingModeManager::GetSingleton().GetFlyingMode() == IDRC::FlyingMode::kFlying) {
				if (_ts_SKSEFunctions::GetFlyingState(dragonActor) == 2) {
					if (a_request && *a_request) {
						auto& startPos = (*a_request)->start.location.location;
						float distanceToDragon = (dragonActor->GetPosition()).GetDistance(startPos);
						if (distanceToDragon == 0.f) {
							log::info("{}: Start position: ({}, {}, {}), distance to Dragon: {}", __FUNCTION__, startPos.x, startPos.y, startPos.z, (dragonActor->GetPosition()).GetDistance(startPos));
							auto& goalPos = (*a_request)->goal.targetPoint;
							log::info("{}: Goal position: ({}, {}, {})", __FUNCTION__, goalPos.x, goalPos.y, goalPos.z);
							log::info("{}: Result: {}", __FUNCTION__, result ? "true" : "false");
						}
					} else {
						log::warn("{}: Pathing request pointer is null", __FUNCTION__);
					}
				}
			}
		}

		return result;
	}
*/

	bool PathingHook::IsDragonPathingRequest(std::byte* a_agent) {
		if (!a_agent) {
			log::warn("{}: Agent pointer is null", __FUNCTION__);
			return false;
		}
		auto* actorState = *reinterpret_cast<RE::ActorState**>(a_agent + 0x10);
		if (!actorState) {
			log::warn("{}: ActorState pointer is null", __FUNCTION__);
			return false;
		}

		auto* actor = reinterpret_cast<RE::Actor*>(reinterpret_cast<std::uintptr_t>(actorState) - m_actorOffset);
		auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (dragonActor && actor == dragonActor) {
			return true;
		}

		return false;
	}

/* unused
	void PathingHook::SetFlightPath(std::uintptr_t  a_subPtr, std::uintptr_t* a_newNode, std::uintptr_t* a_newData) {

		if (_SetFlightPath == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}

		// call the original function
		using FuncType = decltype(&SetFlightPath);
		reinterpret_cast<FuncType>(_SetFlightPath)(a_subPtr, a_newNode, a_newData);
log::info("{}: SetFlightPath called", __FUNCTION__);
	}
*/
	void PathingHook::FlightPlannerUpdate(std::uintptr_t a_plannerSubPtr,
									float* a_deltaTime,
									float* a_outMovementIntention,
									void* a_context) {
	// This hook fires for all dragons in flyingstate == 2 (Flying), once per frame.
	// Uses UpdateFlightPathData() to keep mounted dragon on a straight path towards the targeted direction
	
		if (!_FlightPlannerUpdate) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}

		auto agent = reinterpret_cast<std::byte*>(a_plannerSubPtr - 0x18);  // same offset for SE and AE
		IDRC::EuropiaBridge::NoteStage(0);
		UpdateFlightPathData(agent);

		// call the original function
		using FuncType = decltype(&FlightPlannerUpdate);
		reinterpret_cast<FuncType>(_FlightPlannerUpdate)(a_plannerSubPtr, a_deltaTime, a_outMovementIntention, a_context);
	}


	void PathingHook::UpdateFlightPathData(std::byte* a_agent) {
		if (!a_agent) {
			return;
		}

		if (!IsDragonPathingRequest(a_agent)) {
			return;
		}
		IDRC::EuropiaBridge::NoteStage(1);

		auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (!dragonActor) {
			return;
		}

		auto& flyingModeManager = IDRC::FlyingModeManager::GetSingleton();

/* For debugging
		bool skipRePathing = false;
		if (flyingModeManager.GetFlyingMode() == IDRC::FlyingMode::kHovering ||
			flyingModeManager.GetFlyingMode() == IDRC::FlyingMode::kLanded) {
			skipRePathing = true;
		} else 
*/
		if(flyingModeManager.GetFlyingMode() != IDRC::FlyingMode::kFlying)
		{
			return;
		}
		IDRC::EuropiaBridge::NoteStage(2);

		auto currentIndex = *reinterpret_cast<uint32_t*>(a_agent + 0x58);
		auto* pathData = *reinterpret_cast<std::byte**>(a_agent + 0x48);
		if (!pathData) {
			// Each FastTravel (ExecuteTeleport) triggers re-pathing. 
			// In that track, pathData is set to nullptr until the new pathData is generated. 
			// It can take a few frames until the new pathData is generated, 
			// in particular if many pathing requests occur in parallel, eg during combat. 
			// In this case we skip the next FastTravel request to allow engine to catch up with re-pathing. 
			// Flight pathData will not be updated until new pathData is generated, 
			// The dragon will fly straight forward without path data during these frames (typically 1-3)
			
			IDRC::FastTravelManager::GetSingleton().SkipFastTravelRequest(true);
			return;
		}
		// If pathData is valid, allow FastTravel
		IDRC::FastTravelManager::GetSingleton().SkipFastTravelRequest(false);
		IDRC::EuropiaBridge::NoteStage(3);

		// a_pathData + 0x90 = waypointArray_90 (float* base)
		// a_pathData + 0xA0 = waypointCount_A0 (uint32)
		auto* wayPointBase  = *reinterpret_cast<float**>(pathData + 0x90);
		auto  wayPointCount = *reinterpret_cast<std::uint32_t*>(pathData + 0xA0);

		if (!wayPointBase || wayPointCount < 2) {
log::warn("{}: wayPointBase null=? wayPointCount: {}", __FUNCTION__, wayPointCount);
			return;
		}
		IDRC::EuropiaBridge::NoteStage(4);

		auto* playerCamera = RE::PlayerCamera::GetSingleton();
		RE::ThirdPersonState* dragonCameraState = nullptr;

		if (playerCamera && playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kDragon)) {
			dragonCameraState = static_cast<RE::ThirdPersonState*>(playerCamera->currentState.get());
			if (!dragonCameraState) {
				log::warn("{}: Dragon camera state is null", __FUNCTION__);
				return;
			}
		} else {
			log::warn("{}: Player camera state is not DragonCameraState", __FUNCTION__);
			return;
		}

		IDRC::EuropiaBridge::NoteStage(5);
		float targetPitch = 0.0f;
		if(IDRC::CameraLockManager::GetSingleton().IsEnabled() && !flyingModeManager.CheckForHeightChange()) {
			targetPitch = _ts_SKSEFunctions::GetPitch(dragonCameraState->rotation);
		} else {
			targetPitch = flyingModeManager.GetTargetPitch();
			flyingModeManager.SetTargetPitch(0.0f);
		}

		float targetYaw = 0.0f;
		if(IDRC::CameraLockManager::GetSingleton().IsEnabled() && !flyingModeManager.CheckForTurn()) {
			targetYaw = _ts_SKSEFunctions::GetYaw(dragonCameraState->rotation);
		} else {
			targetYaw = flyingModeManager.GetTargetYaw();
			flyingModeManager.SetYawOffset(0.0f);
		}

		// Europia: outside Skyrim's border, fly level back towards the middle of the map
		{
			float europiaYaw = 0.0f;
			if (IDRC::EuropiaBridge::GetTurnBack(europiaYaw)) {
				targetYaw = europiaYaw;
				targetPitch = 0.0f;
			}
		}
		IDRC::EuropiaBridge::NotePath(targetYaw, targetPitch);

		const RE::NiPoint3 dragonPos = dragonActor->GetPosition();
		auto& combatManager = IDRC::CombatManager::GetSingleton();
		auto* shoutTarget = combatManager.GetShoutTarget();

		bool shoutActive = combatManager.IsShoutActive() && shoutTarget;
		float distanceToTarget = 0.0f;
		RE::NiPoint3 pathTargetPos;

		if (shoutActive) {
			pathTargetPos = shoutTarget->GetPosition();
			float dX = pathTargetPos.x - dragonPos.x;
			float dY = pathTargetPos.y - dragonPos.y;
			float yawtoShoutTarget = atan2f(dX, dY);

			pathTargetPos.z += m_shoutHeight; // move path target to shoutHeight units above shout target ...
			// ... and minShoutDistance to left / right of the shout target (depending on GetShoutDirection)
			pathTargetPos.x -= combatManager.GetShoutDirection() * m_minShoutDistance * std::cos(yawtoShoutTarget);
			pathTargetPos.y += combatManager.GetShoutDirection() * m_minShoutDistance * std::sin(yawtoShoutTarget);

			distanceToTarget = pathTargetPos.GetDistance(dragonPos);

			// re-calculate targetYaw to point towards the adjusted pathTargetPos
			dX= pathTargetPos.x - dragonPos.x;
			dY = pathTargetPos.y - dragonPos.y;
			targetYaw = std::atan2f(dX, dY);
		}

		float sinYaw = std::sin(targetYaw);
		float cosYaw = std::cos(targetYaw);
		const float tanPitch = std::tan(targetPitch);
		const float minHeightAboveGround = 500.f;

/* for debugging
		if(skipRePathing) {
			constexpr std::size_t kStride = 0x48 / sizeof(float);  // = 0x12 floats
			int color = 0xFFFF00FF;

			for (std::uint32_t i = 0; i < wayPointCount; ++i) {
				auto& waypointToUpdate = *reinterpret_cast<RE::NiPoint3*>(&wayPointBase[i * kStride]);
				if (APIs::TrueHUD) {
				if (i == wayPointCount - 1) {
					APIs::TrueHUD->DrawPoint(waypointToUpdate, 10.0f, 0.0f, 0x0000FFFF);
				} else {
					APIs::TrueHUD->DrawPoint(waypointToUpdate, 5.0f, 0.0f, color);
				}	
			}
		}
		return;
}
*/		
		// Each entry is 0x48 bytes; XYZ floats at byte offsets +0, +4, +8
		constexpr std::size_t kStride = 0x48 / sizeof(float);  // = 0x12 floats
		for (std::uint32_t i = 0; i < std::min(wayPointCount, currentIndex + 8u); ++i) {
			// #Adjusting the first waypoints to impact the immediate path.
			// Other waypoints do not be adjusted in this frame,
			// because they do not impact the form of the interpolated path close to the dragon.  
			auto& waypointToUpdate = *reinterpret_cast<RE::NiPoint3*>(&wayPointBase[i * kStride]);
			float dx = waypointToUpdate.x - dragonPos.x;
			float dy = waypointToUpdate.y - dragonPos.y;
			float distanceToUpdate = std::sqrt(dx * dx + dy * dy);

			if (i >= wayPointCount - 2) {
				// If re-pathing is late and currentIndex reaches last waypoints, make sure those stay ahead of the dragon
				distanceToUpdate = std::max(distanceToUpdate, 2000.0f);
			}

			float cameraZ = dragonPos.z + distanceToUpdate * tanPitch;
			if (shoutActive) {
				cameraZ = dragonPos.z + distanceToUpdate / distanceToTarget * (pathTargetPos.z - dragonPos.z);
			}

			waypointToUpdate.x = dragonPos.x + distanceToUpdate * sinYaw;
			waypointToUpdate.y = dragonPos.y + distanceToUpdate * cosYaw;

			float landZ = _ts_SKSEFunctions::GetLandHeightWithWater(waypointToUpdate, true) + minHeightAboveGround;
			// Europia: altitude ceiling ~6000 units (about 85 m) above the ground so he can't climb away forever
			waypointToUpdate.z = std::min(std::max(cameraZ, landZ), landZ + 6000.0f);
		}
/* for debugging
		log::info("{}: Updated waypoints for flying dragon. CurrentIndex: {}, WaypointCount: {}", __FUNCTION__, currentIndex, wayPointCount);
		for (std::uint32_t i = 0; i < std::min(wayPointCount - 2, currentIndex + 8u); ++i) {
			auto& waypointToUpdate = *reinterpret_cast<RE::NiPoint3*>(&wayPointBase[i * kStride]);
			if (APIs::TrueHUD) {
				if (i == currentIndex) {
					APIs::TrueHUD->DrawPoint(waypointToUpdate, 10.0f, 0.0f, 0x00FF00FF);
				} else {
					APIs::TrueHUD->DrawPoint(waypointToUpdate, 5.0f, 0.0f, 0xFF0000FF);
				}
			}
		}
*/
	}

/* unused
	void PathingHook::ProcessPathUpdate(std::uintptr_t a_arbiter,
									void* a_newNode,
									void* a_newData,
									void* a_param4) {

		if (_ProcessPathUpdate == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}

log::info("{}: ---------------->>>>>>>>>>>>>> Processing path update.", __FUNCTION__);
		// Call the original ProcessPathUpdate function
		using FuncType = decltype(&ProcessPathUpdate);
		reinterpret_cast<FuncType>(_ProcessPathUpdate)(a_arbiter, a_newNode, a_newData, a_param4);
	}

	void PathingHook::LinearPathToTarget(std::byte** a_pathData, std::uint32_t a_startIndex, const RE::NiPoint3& a_targetPos) {
		if (!a_pathData) {
			return;
		}

		auto* pathData = *a_pathData;
		if (!pathData) {
			return;
		}

		auto dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (!dragonActor) {
			return;
		}

		auto* wayPointBase  = *reinterpret_cast<float**>(pathData + 0x90);
		auto  wayPointCount = *reinterpret_cast<std::uint32_t*>(pathData + 0xA0);

		if (a_startIndex >= wayPointCount) {
			log::warn("{}: Invalid start index: {}, waypoint count: {}", __FUNCTION__, a_startIndex, wayPointCount);
			return;
		}

log::info("{}: LinearPathToTarget called. StartIndex: {}, WaypointCount: {}, TargetPos: ({}, {}, {})", __FUNCTION__, a_startIndex, wayPointCount, a_targetPos.x, a_targetPos.y, a_targetPos.z);
		RE::NiPoint3 dragonPos = dragonActor->GetPosition();
		constexpr std::size_t kStride = 0x48 / sizeof(float);  // = 0x12 floats
		for (std::uint32_t i = a_startIndex; i < wayPointCount; ++i) {
			// linearly interpolate between dragonPos and the target position
			auto& waypointToUpdate = *reinterpret_cast<RE::NiPoint3*>(&wayPointBase[i * kStride]);
			float t = static_cast<float>(i) / static_cast<float>(wayPointCount - 1);
			waypointToUpdate.x = (1.0f - t) * dragonPos.x + t * a_targetPos.x;
			waypointToUpdate.y = (1.0f - t) * dragonPos.y + t * a_targetPos.y;
			waypointToUpdate.z = (1.0f - t) * dragonPos.z + t * a_targetPos.z;
		}
	}
*/

	void PathingHook::SetGroundPath(std::uintptr_t  a_subPtr, std::uintptr_t* a_newNode, std::uintptr_t* a_newData) {
	// This hook fires whenever an actor needs updated pathing,  including dragons in flyingstate == 0 (Landed)
	// Used to modify pathing for mounted dragon in flyingState == 0 (Landed)
	// This is to improve handling of obstacles, and avoid that the dragon deviates from the straight
	// path that has been directed by the player, if possible.

		if (_SetGroundPath == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;

		}
	
		// call the original function
		using FuncType = decltype(&SetGroundPath);
		reinterpret_cast<FuncType>(_SetGroundPath)(a_subPtr, a_newNode, a_newData);

		// modify waypoints for grounded, mounted dragon

		auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (!dragonActor) {
			// no dragon mounted
			return;
		} else if (_ts_SKSEFunctions::GetFlyingState(dragonActor) != 0 || 
					IDRC::FlyingModeManager::GetSingleton().GetFlyingMode() != IDRC::FlyingMode::kLanded) {
			// not in landed state
			return;
		}

		auto agent = reinterpret_cast<std::byte*>(a_subPtr - 0x20);
		auto* actorState = *reinterpret_cast<RE::ActorState**>(agent + 0x10);
		if (!actorState) {
			return;
		}

		auto* actor = reinterpret_cast<RE::Actor*>(reinterpret_cast<std::uintptr_t>(actorState) - m_actorOffset);
		if(actor != dragonActor) {
			// not the mounted dragon
			return;
		}

		// Read waypoints from agent+0x48 (pathData_48), which is now set
		// a_subPtr + 0x28 = agent+0x48 = pathData_48
		auto* pathData = reinterpret_cast<std::byte*>(
		*reinterpret_cast<std::uintptr_t*>(a_subPtr + 0x108));
		if (!pathData) {
			return;
		}

		// pathData + 0x90 = waypointArray_90 (float* base)
		// pathData + 0xA0 = waypointCount_A0 (uint32)
		auto* wayPointBase  = *reinterpret_cast<float**>(pathData + 0x90);
		auto  wayPointCount = *reinterpret_cast<std::uint32_t*>(pathData + 0xA0);


		if (!wayPointBase || wayPointCount == 0) {
			return;
		}

		// Each entry is 0x48 bytes; XYZ floats at byte offsets +0, +4, +8
		constexpr std::size_t kStride = 0x48 / sizeof(float);  // = 0x12 floats

		auto& firstWayPoint = *reinterpret_cast<RE::NiPoint3*>(&wayPointBase[0]);
		auto& lastWayPoint = *reinterpret_cast<RE::NiPoint3*>(&wayPointBase[(wayPointCount - 1) * kStride]);

		for (std::uint32_t i = 0; i < wayPointCount; ++i) {
			// modify waypoints -  linearly interpolate between first and last waypoint
			// then adjust z to be above ground
			const float t = (wayPointCount > 1) ?
				(static_cast<float>(i) / static_cast<float>(wayPointCount - 1)) :
				0.0f;

			RE::NiPoint3 currentWayPoint;
			currentWayPoint.x = firstWayPoint.x + (lastWayPoint.x - firstWayPoint.x) * t;
			currentWayPoint.y = firstWayPoint.y + (lastWayPoint.y - firstWayPoint.y) * t;
			currentWayPoint.z = firstWayPoint.z + (lastWayPoint.z - firstWayPoint.z) * t;

			float landHeight = _ts_SKSEFunctions::GetLandHeightWithWater(currentWayPoint, true);
			currentWayPoint.z = landHeight + 20.f;

			// update waypoint position
			wayPointBase[i * kStride] = currentWayPoint.x;
			wayPointBase[i * kStride + 1] = currentWayPoint.y;
			wayPointBase[i * kStride + 2] = currentWayPoint.z;
		}
	}

/* for debugging:
	void PathingHook::SetupPathingRequest(RE::Actor* _ts_a_actor, void* _ts_a_request, RE::NiPoint3* _ts_a_targetPos, float _ts_a_speed, RE::TESObjectREFR* _ts_a_targetRef)
	{
		if (_SetupPathingRequest == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}

		auto dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (dragonActor && dragonActor == _ts_a_actor) {
			if (APIs::TrueHUD && IDRC::FlyingModeManager::GetSingleton().GetRegisteredForLanding() && _ts_a_targetPos) {
				APIs::TrueHUD->DrawPoint(*_ts_a_targetPos, 10.0f, 20.0f, 0x99FFFFFF);
			}
		}

		reinterpret_cast<decltype(&SetupPathingRequest)>(_SetupPathingRequest)(_ts_a_actor, _ts_a_request, _ts_a_targetPos, _ts_a_speed, _ts_a_targetRef);
	}
*/

	void* PathingHook::GetCurrentPathingLocation (RE::BSPathing* a_pathing,RE::BSPathingLocation* a_loc, RE::Actor* a_actor, std::uintptr_t param4)
	{
		if (_GetCurrentPathingLocation  == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return nullptr;
		}

		if (m_pathingSingleton != a_pathing) {
			if (m_pathingSingleton != nullptr) {
				log::warn("{}: Pathing singleton changed!", __FUNCTION__);
			}

			// initialize the pathing singleton pointer with the first vanilla GetCurrentPathingLocation call
			m_pathingSingleton = a_pathing;
		}

		void* result = reinterpret_cast<decltype(&GetCurrentPathingLocation )>(_GetCurrentPathingLocation )(a_pathing, a_loc, a_actor, param4);
/* for debugging
		auto dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (result && dragonActor) {
			float x = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(result) + 0x0);
			float y = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(result) + 0x4);
			float z = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(result) + 0x8);
			RE::NiPoint3 resultPos(x, y, z);
			if (APIs::TrueHUD) {
				APIs::TrueHUD->DrawPoint(resultPos, 12.0f, 2.0f, 0xFF99FFFF);
				log::info("{}: GetCurrentPathingLocation  returned  ({}, {}, {}), dragonPos=({},{},{}), distance: {}", __FUNCTION__, x, y, z,
					dragonActor->GetPosition().x,
					dragonActor->GetPosition().y,
					dragonActor->GetPosition().z,
					std::sqrt(std::pow(x - dragonActor->GetPosition().x, 2) + std::pow(y - dragonActor->GetPosition().y, 2)));
			}
		} else {
			log::info("{}: GetCurrentPathingLocation  returned nullptr", __FUNCTION__);
		}
*/	
		return result;
	}

/* Hooks for exploration / debugging only:

	RE::TESForm* PathingHook::GetCurrentMountCellOrWorldspaceForm(RE::Actor *_ts_a_actor,void* _ts_a_param2,void* _ts_a_param3, bool _ts_a_param4)
	{
		if (_GetCurrentMountCellOrWorldspaceForm == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return nullptr;
		}

		auto result = reinterpret_cast<decltype(&GetCurrentMountCellOrWorldspaceForm)>(_GetCurrentMountCellOrWorldspaceForm)(_ts_a_actor, _ts_a_param2, _ts_a_param3, _ts_a_param4);
		if (result && result->AsReference()) {
			auto resultRef = result->AsReference();
			RE::NiPoint3 resultPos = resultRef->GetPosition();
			auto baseForm = resultRef->GetBaseObject();
			if (APIs::TrueHUD) {
				APIs::TrueHUD->DrawPoint(resultPos, 5.0f, 20.0f, 0xFF99FFFF);
			}
			log::info("{}: GetCurrentMountCellOrWorldspaceForm called for actor {}, returned {:0x} ({}) at position ({}, {}, {})", __FUNCTION__, _ts_a_actor ? _ts_a_actor->GetName() : "null", result ? result->GetFormID() : 0, baseForm ? baseForm->GetFormEditorID() : "NONE", resultPos.x, resultPos.y, resultPos.z);
		}  else {
			log::info("{}: GetCurrentMountCellOrWorldspaceForm called for actor {}, returned nullptr", __FUNCTION__, _ts_a_actor ? _ts_a_actor->GetName() : "null");
		}
		return result;
	}

	void* PathingHook::TESPackage_sub_140437ac0(void* a_package, void* a_param2, void* a_param3, void* a_param4)
	{
		if (_TESPackage_sub_140437ac0 == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return nullptr;
		}
		void* result = reinterpret_cast<decltype(&TESPackage_sub_140437ac0 )>(_TESPackage_sub_140437ac0 )(a_package, a_param2, a_param3, a_param4);
		float x = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(result) + 0x0);
		float y = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(result) + 0x4);
		float z = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(result) + 0x8);
		RE::NiPoint3 resultPos(x, y, z);
		if (APIs::TrueHUD) {
			APIs::TrueHUD->DrawPoint(resultPos, 12.0f, 0.1f, 0xFF99FFFF);
		}
		log::info("{}: TESPackage_sub_140437ac0 returned  ({}, {}, {})", __FUNCTION__, x, y, z);
		return result;
	}


	bool PathingHook::BuildFlyLandPath(void** a_request, RE::BSPathingSolution* a_result)
	{
		if (_BuildFlyLandPath == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}
		RE::NiPoint3 goal = *reinterpret_cast<RE::NiPoint3*>(reinterpret_cast<std::uintptr_t>(*a_request) + 0x58);
		RE::NiPoint3 targetLocationPos = *reinterpret_cast<RE::NiPoint3*>(reinterpret_cast<std::uintptr_t>(*a_request) + 0x48);
//		*reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(*a_request) + 0x13c) = 50000.f;
		float searchRadius = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(*a_request) + 0x13c);
		bool headingFlag = *reinterpret_cast<bool*>(reinterpret_cast<std::uintptr_t>(*a_request) + 0x140);
		int packageLocationType = *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(*a_request) + 0x138);

log::info("{}: BuildFlyLandPath called with goal=({},{},{}), targetLocationPos=({},{},{})", __FUNCTION__,
	goal.x, goal.y, goal.z,
	targetLocationPos.x, targetLocationPos.y, targetLocationPos.z);
log::info("{}: BuildFlyLandPath called - searchRadius={}, headingFlag={}, packageLocationType={}", __FUNCTION__,
	searchRadius, headingFlag ? "true" : "false", packageLocationType);
	
		bool result = reinterpret_cast<decltype(&BuildFlyLandPath )>(_BuildFlyLandPath )(a_request, a_result);
log::info("{}: BuildFlyLandPath called, result={}", __FUNCTION__, result ? "true" : "---------------->>>>>>>>>>> false");
		return result;
	}


	void PathingHook::SetPathingLocFromPos(RE::BSPathingLocation* a_loc, RE::NiPoint3 a_pos)
	{
		if (_SetPathingLocFromPos == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
//log::info("{}: SetPathingLocFromPos called for position ({}, {}, {})", __FUNCTION__, a_pos.x, a_pos.y, a_pos.z);
		reinterpret_cast<decltype(&SetPathingLocFromPos)>(_SetPathingLocFromPos)(a_loc, a_pos);
	}


	bool PathingHook::FindNavmeshTriangleForLocation(RE::BSPathingLocation* a_loc, RE::FindTriangleForLocationFilter* a_filter)
	{
		// finds the closest navmesh triangle relative to the position in a_loc, and updates a_loc's position to the found triangle position
		// This is useful to determine the closest viable landing spot if the dragon is over an area without navmesh.

		if (_FindNavmeshTriangleForLocation == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}
//log::info("{}: Calling FindNavmeshTriangleForLocation with location ({}, {}, {}), filter:  maxDistAbove={}, maxDistBelow={}", __FUNCTION__, a_loc->location.x, a_loc->location.y, a_loc->location.z, a_filter ? a_filter->maxDistAbove : 0, a_filter ? a_filter->maxDistBelow : 0);
		bool result = reinterpret_cast<decltype(&FindNavmeshTriangleForLocation)>(_FindNavmeshTriangleForLocation)(a_loc, a_filter);
//log::info("{}: FindNavmeshTriangleForLocation called for location ({}, {}, {}), result: {}", __FUNCTION__, a_loc->location.x, a_loc->location.y, a_loc->location.z, result ? "true" : "false");
		return result;
	}

	bool PathingHook::FindNavmeshTriangleForLocation2(RE::BSPathingLocation* a_loc, RE::FindTriangleForLocationFilter* a_filter)
	{
		// finds the closest navmesh triangle relative to the position in a_loc, and updates a_loc's position to the found triangle position
		// This is useful to determine the closest viable landing spot if the dragon is over an area without navmesh.

		if (_FindNavmeshTriangleForLocation2 == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}
//log::info("{}: Calling FindNavmeshTriangleForLocation with location ({}, {}, {}), filter:  maxDistAbove={}, maxDistBelow={}", __FUNCTION__, a_loc->location.x, a_loc->location.y, a_loc->location.z, a_filter ? a_filter->maxDistAbove : 0, a_filter ? a_filter->maxDistBelow : 0);
		bool result = reinterpret_cast<decltype(&FindNavmeshTriangleForLocation2)>(_FindNavmeshTriangleForLocation2)(a_loc, a_filter);
//log::info("{}: FindNavmeshTriangleForLocation called for location ({}, {}, {}), result: {}", __FUNCTION__, a_loc->location.x, a_loc->location.y, a_loc->location.z, result ? "true" : "false");
		return result;
	}
*/

/*****************************************************************************************/

	RE::ProjectileHandle* ProjectileLaunchHook::Launch(RE::ProjectileHandle* a_result, RE::Projectile::LaunchData& a_data) {
		if (_Launch == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return a_result;
		}

		auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (dragonActor && dragonActor == a_data.shooter) {
			// Sometimes the vanilla aiming for ball shouts is not working correctly
			// (ball pitch not aligned with the shout target)
			// Reason is not understood.
			// Patch overwrites projectile's pitch value with the actual direction to target.
			auto* shoutTarget = IDRC::CombatManager::GetSingleton().GetShoutTarget();
			if (shoutTarget) {
				auto shoutOrigin = dragonActor->GetPosition();
				RE::NiAVObject* reference3D = _ts_SKSEFunctions::GetTargetPoint(dragonActor, RE::BGSBodyPartDefs::LIMB_ENUM::kHead).get();
				if (reference3D) {
					shoutOrigin = reference3D->world.translate;
				}

				auto shoutTargetPos = shoutTarget->GetPosition();
				a_data.angleX = -_ts_SKSEFunctions::GetAngleX(shoutOrigin, shoutTargetPos);
			}
		}

		return reinterpret_cast<decltype(&Launch)>(_Launch)(a_result, a_data);	
	}

	void FlushQueuedFormLoadsHook::Hook()
	{
		// All patched instructions are 6-byte indirect calls.
		// write_call<5> replaces bytes 0-4 with a 5-byte relative call stub.
		// Byte 5 (the trailing 0x00) must be explicitly NOP'd to prevent it from
		// executing as 'ADD [rax],al' after hook returns.

		m_imageBase = REL::Module::get().base();
		const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(m_imageBase);
		const auto* nt  = reinterpret_cast<const IMAGE_NT_HEADERS*>(m_imageBase + dos->e_lfanew);
		m_imageEnd = m_imageBase + static_cast<std::uintptr_t>(nt->OptionalHeader.SizeOfImage);

		REL::Relocation<std::uintptr_t> func{ RELOCATION_ID(34645, 35567) };
		auto& trampoline = SKSE::GetTrampoline();

		// SE+0x111 / AE+0x111: 'call [rdx+0x160]' — TESForm::AsReference2 (1st pass)
		const auto patchAddr1 = func.address() + RELOCATION_OFFSET(0x111, 0x111);
		trampoline.write_call<5>(patchAddr1, TESForm_AsReference2_Guard);
		REL::safe_write(patchAddr1 + 5, std::uint8_t{ 0x90 });

		// SE+0x1DF / AE+0x1C1: 'call [rax+0x80]' — TESForm::InitLoadGame (1st pass)
		const auto patchAddr2 = func.address() + RELOCATION_OFFSET(0x1DF, 0x1C1);
		trampoline.write_call<5>(patchAddr2, TESForm_InitLoadGame_Guard);
		REL::safe_write(patchAddr2 + 5, std::uint8_t{ 0x90 });

		// SE+0x304 / AE+0x2E4: 'call [rax+0x88]' — TESForm::FinishLoadGame (2nd pass)
		const auto patchAddr3 = func.address() + RELOCATION_OFFSET(0x304, 0x2E4);
		trampoline.write_call<5>(patchAddr3, TESForm_FinishLoadGame_Guard);
		REL::safe_write(patchAddr3 + 5, std::uint8_t{ 0x90 });

		// SE+0x356 / AE+0x336: 'call [rax+0x160]' — TESForm::AsReference2 (2nd pass, on TESObjectREFR from GetReference)
		const auto patchAddr4 = func.address() + RELOCATION_OFFSET(0x356, 0x336);
		trampoline.write_call<5>(patchAddr4, TESForm_AsReference2_Guard);
		REL::safe_write(patchAddr4 + 5, std::uint8_t{ 0x90 });
	}
	
	RE::TESObjectREFR* FlushQueuedFormLoadsHook::TESForm_AsReference2_Guard(RE::TESForm* a_form)
	{
		// a_form (rcx) = TESForm* from BGSLoadFormData::GetForm — guaranteed non-null by caller.
		// Read the vtable pointer (first 8 bytes of any C++ virtual-dispatch object).
		const auto vtable = *reinterpret_cast<const std::uintptr_t*>(a_form);
		if (vtable < m_imageBase || vtable >= m_imageEnd) {
			// Vtable is outside SkyrimSE.exe image: the form was freed and the memory
			// has been reused (or zeroed). Returning nullptr is safe — the caller checks for nullptr.
			log::info("{}: FlushQueuedFormLoads: stale TESForm at {:#018x} "
			"(vtable={:#018x}) — use-after-free detected, skipping AsReference2", __FUNCTION__, reinterpret_cast<std::uintptr_t>(a_form), vtable);
			return nullptr;
		}

		// Vtable is valid — execute the original TESForm::AsReference2 virtual dispatch.
		using AsReference2_t = RE::TESObjectREFR* (*)(RE::TESForm*);
		auto* ref = (*reinterpret_cast<AsReference2_t*>(vtable + 0x160))(a_form);

		// Both patched call sites use the returned pointer as a raw object `this` with NO
		// further vtable dispatch, so a valid-looking a_form vtable is not enough:
		//   1st pass (SE+0x111/AE+0x111): immediately after this call the function does an
		//   inline 'lock inc [ref+0x28]' refcount bump directly on the returned pointer.
		//   2nd pass (SE+0x356/AE+0x336): the returned pointer is passed straight into
		//   TESObjectREFR::sub_14028a930(ref) as `this`.
		// Neither of those is a virtual call we can guard, so if AsReference2 itself hands
		// back a corrupted/implausible pointer (observed in crash dumps as e.g. ref == 0x4,
		// or ref pointing directly into SkyrimSE.exe's own image instead of the heap) we must
		// catch it here before returning, otherwise the caller crashes on the very next
		// instruction with no guard in between.
		if (ref) {
			const auto refAddr = reinterpret_cast<std::uintptr_t>(ref);
			if (refAddr < 0x10000 || (refAddr >= m_imageBase && refAddr < m_imageEnd)) {
				log::info("{}: FlushQueuedFormLoads: AsReference2 on TESForm at {:#018x} returned "
				"implausible pointer {:#018x} — treating as stale, returning nullptr", __FUNCTION__, reinterpret_cast<std::uintptr_t>(a_form), refAddr);
				return nullptr;
			}
		}

		return ref;
	}

	void FlushQueuedFormLoadsHook::TESForm_InitLoadGame_Guard(RE::TESForm* a_form, void* a_loadBuffer)
	{
		// a_form (rcx) = TESForm*, a_loadBuffer (rdx) = BGSLoadGameBuffer*.
		// rax = *a_form = vtable pointer — garbage when the form has been freed.
		const auto vtable = *reinterpret_cast<const std::uintptr_t*>(a_form);
		if (vtable < m_imageBase || vtable >= m_imageEnd) {
			log::info("{}: FlushQueuedFormLoads: stale TESForm at {:#018x} "
			"(vtable={:#018x}) — use-after-free detected, skipping InitLoadGame", __FUNCTION__, reinterpret_cast<std::uintptr_t>(a_form), vtable);
			return;
		}

		using InitLoadGame_t = void (*)(RE::TESForm*, void*);
		(*reinterpret_cast<InitLoadGame_t*>(vtable + 0x80))(a_form, a_loadBuffer);
	}

	void FlushQueuedFormLoadsHook::TESForm_FinishLoadGame_Guard(RE::TESForm* a_form, void* a_loadBuffer)
	{
		// a_form (rcx) = TESForm*, a_loadBuffer (rdx) = BGSLoadGameBuffer*.
		// rax = *a_form = vtable pointer — garbage when the form has been freed.
		const auto vtable = *reinterpret_cast<const std::uintptr_t*>(a_form);
		if (vtable < m_imageBase || vtable >= m_imageEnd) {
			log::info("{}: FlushQueuedFormLoads: stale TESForm at {:#018x} "
			"(vtable={:#018x}) — use-after-free detected, skipping FinishLoadGame", __FUNCTION__, reinterpret_cast<std::uintptr_t>(a_form), vtable);
			return;
		}

		using FinishLoadGame_t = void (*)(RE::TESForm*, void*);
		(*reinterpret_cast<FinishLoadGame_t*>(vtable + 0x88))(a_form, a_loadBuffer);
	}

/*****************************************************************************************/

	void CheckSaveGameHook::Hook()
	{
		m_imageBase = REL::Module::get().base();
		const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(m_imageBase);
		const auto* nt  = reinterpret_cast<const IMAGE_NT_HEADERS*>(m_imageBase + dos->e_lfanew);
		m_imageEnd = m_imageBase + static_cast<std::uintptr_t>(nt->OptionalHeader.SizeOfImage);

		REL::Relocation<std::uintptr_t> respawnFunc{ RELOCATION_ID(34648, 35570) };
		auto& trampoline = SKSE::GetTrampoline();

		// SE+0x153 / AE+0x1A8: 'mov rcx,rsi; call [rax+0x68]' — CheckSaveGame, 6 bytes.
		// Patch covers MOV RCX,RSI + CALL; JMP thunk restores RCX=RSI before calling guard.
		// Uses absolute (imm64) addressing for the CALL/JMP back out to our own DLL —
		// see CreateSourceTextureResultHook below for why rel32 is unsafe here (ASLR can
		// place the DLL >2GB from the trampoline pool, truncating a rel32 into garbage).
		const auto patchAddr  = respawnFunc.address() + RELOCATION_OFFSET(0x153, 0x1A8);
		const auto resumeAddr = patchAddr + 6;  // TEST AL,AL

		std::uint8_t th[27] = {
			0x48, 0x89, 0xF1,                                     // MOV RCX, RSI
			0x48, 0xB8, 0,0,0,0,0,0,0,0,                          // MOV RAX, <guardFn imm64>
			0xFF, 0xD0,                                           // CALL RAX
			0x48, 0xB8, 0,0,0,0,0,0,0,0,                          // MOV RAX, <resumeAddr imm64>
			0xFF, 0xE0                                            // JMP RAX
		};
		const auto guardFn64    = reinterpret_cast<std::uint64_t>(&CheckSaveGame_Guard);
		const auto resumeAddr64 = static_cast<std::uint64_t>(resumeAddr);
		std::memcpy(th + 5, &guardFn64, sizeof(guardFn64));
		std::memcpy(th + 17, &resumeAddr64, sizeof(resumeAddr64));

		auto* thunkMem = static_cast<std::uint8_t*>(trampoline.allocate(sizeof(th)));
		std::memcpy(thunkMem, th, sizeof(th));

		trampoline.write_branch<5>(patchAddr, reinterpret_cast<std::uintptr_t>(thunkMem));
		REL::safe_write(patchAddr + 5, std::uint8_t{ 0x90 });
	}
	
	std::int8_t CheckSaveGameHook::CheckSaveGame_Guard(RE::TESObjectREFR* a_ref, void* a_saveFormBuffer)
	{
		// a_ref (rcx) = TESObjectREFR*, a_saveFormBuffer (rdx) = BGSSaveFormBuffer*.
		// Called from FUN_14057b120 / FUN_1405ae2d0 (the respawn/unload save helper),
		// which is invoked both from FlushQueuedFormLoads AND from cell activation code
		// (QueuedPromoteLargeReferencesTask path: 19799 -> 35570).

		// Guard 1: vtable range check (use-after-free — form was freed, vtable is garbage).
		const auto vtable = *reinterpret_cast<const std::uintptr_t*>(a_ref);
		if (vtable < m_imageBase || vtable >= m_imageEnd) {
			log::info("{}: respawn helper: stale TESObjectREFR at {:#018x} "
			"(vtable={:#018x}) — use-after-free detected, skipping CheckSaveGame", __FUNCTION__, reinterpret_cast<std::uintptr_t>(a_ref), vtable);
			return 0;
		}

		// Guard 2: null parentCell check (partially-initialized actor).
		// An actor loaded into a cell that was then rapidly unloaded (dragon flight) can have
		// a valid vtable but null parentCell. Calling CheckSaveGame on it crashes inside the
		// implementation at [rax+0x04] where rax is a null sub-object (e.g. currentProcess).
		if (a_ref->parentCell == nullptr) {
			return 0;
		}

		// Both checks passed — dispatch original CheckSaveGame through the valid vtable.
		using CheckSaveGame_t = std::int8_t (*)(RE::TESObjectREFR*, void*);
		return (*reinterpret_cast<CheckSaveGame_t*>(vtable + 0x68))(a_ref, a_saveFormBuffer);
	}

/*****************************************************************************************/
	
	void CreateSourceTextureResultHook::Hook()
	{
		REL::Relocation<std::uintptr_t> func{ RELOCATION_ID(75509, 77301) };
		auto& trampoline = SKSE::GetTrampoline();

		const auto patchAddr  = func.address() + RELOCATION_OFFSET(0x6A, 0x6C);
		const auto resumeAddr = patchAddr + 7;  // 'c7 40 20 01 00 00 00' is 7 bytes

		// Defensive signature check: verify the expected 'mov dword ptr [rax+0x20], 0x1'
		// bytes are still present before patching. Guards against silently corrupting
		// another plugin's patch at this exact address (or a game version mismatch) —
		// if the bytes don't match, skip installing instead of overwriting blindly.
		static constexpr std::uint8_t kExpected[7] = { 0xC7, 0x40, 0x20, 0x01, 0x00, 0x00, 0x00 };
		if (std::memcmp(reinterpret_cast<const void*>(patchAddr), kExpected, sizeof(kExpected)) != 0) {
			log::info("IDRC - CreateSourceTextureResultHook: unexpected bytes at {:#018x} — skipping patch "
			"(already patched by another plugin, or game version mismatch)", patchAddr);
			return;
		}

		std::uint8_t th[27] = {
			0x48, 0x89, 0xC1,                                     // MOV RCX, RAX
			0x48, 0xB8, 0,0,0,0,0,0,0,0,                          // MOV RAX, <guardFn imm64>
			0xFF, 0xD0,                                           // CALL RAX
			0x48, 0xB8, 0,0,0,0,0,0,0,0,                          // MOV RAX, <resumeAddr imm64>
			0xFF, 0xE0                                            // JMP RAX
		};
		// Absolute (imm64) addressing is used here instead of rel32, because a hand-rolled
		// rel32 CALL/JMP is only valid if the target is within +/-2GB of this thunk (which
		// lives in SKSE's trampoline pool, allocated near SkyrimSE.exe). That's guaranteed
		// for jumps INTO the trampoline pool (handled internally by write_branch<5> below),
		// but NOT for a call from the thunk back OUT to a function inside our own DLL —
		// the OS loader/ASLR can place our DLL arbitrarily far away, silently truncating a
		// rel32 offset into a garbage address (this was the actual cause of the AV-on-launch
		// crash from the original rel32-based version of this thunk).
		const auto guardFn64   = reinterpret_cast<std::uint64_t>(&SetResultLoadedFlag_Guard);
		const auto resumeAddr64 = static_cast<std::uint64_t>(resumeAddr);
		std::memcpy(th + 5, &guardFn64, sizeof(guardFn64));
		std::memcpy(th + 17, &resumeAddr64, sizeof(resumeAddr64));

		auto* thunkMem = static_cast<std::uint8_t*>(trampoline.allocate(sizeof(th)));
		std::memcpy(thunkMem, th, sizeof(th));

		trampoline.write_branch<5>(patchAddr, reinterpret_cast<std::uintptr_t>(thunkMem));
		REL::safe_write(patchAddr + 5, std::uint8_t{ 0x90 });
		REL::safe_write(patchAddr + 6, std::uint8_t{ 0x90 });
	}

	void CreateSourceTextureResultHook::SetResultLoadedFlag_Guard(void* a_result)
	{
		// a_result (rcx, moved from rax by the thunk) = result pointer from
		// FUN_140d7dc50/FUN_140dccc30 — null when texture/resource creation failed.
		// Every one of the 10 observed crash dumps had RAX == 0 (write address 0x20), so a
		// plain null check is sufficient here (unlike FlushQueuedFormLoads, this is not a
		// vtable dispatch and there's no "stale but non-null" case to guard against).
		if (!a_result) {
			log::info("{}: texture/resource creation returned null — skipping unconditional "
			"'result->field_0x20 = 1' write to avoid null-pointer crash (SE 75509+0x6A / AE 77301+0x6C)", __FUNCTION__);
			return;
		}

		*reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uintptr_t>(a_result) + 0x20) = 1;
	}

/* UNUSED HOOKS:	

	void CombatHook::StartCombat(RE::Actor* a_this, RE::Actor* a_target, RE::CombatGroup* a_combatGroup) {
		if (_StartCombat == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}

//log::info("{}: StartCombat called for actor {} on target {}", __FUNCTION__, a_this ? a_this->GetName() : "null", a_target ? a_target->GetName() : "null");
		reinterpret_cast<decltype(&StartCombat)>(_StartCombat)(a_this, a_target, a_combatGroup);
	}

	void CombatHook::StopCombat(RE::Actor* a_actor) {
		if (_StopCombat == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}

		reinterpret_cast<decltype(&StopCombat)>(_StopCombat)(a_actor);
//log::info("{}: StopCombat original function returned", __FUNCTION__);
	}

	
	// Both hooked SetAllowFlying functions below perform the following tasks:
	//  - if set to false:
	//		* Determine suitable landing location for the dragon, by calling GetCurrentMountCellOrWorldspaceForm()
	//		* Call InitiateForcedLanding() to put the dragon into kLanding package / procedure
	//		* set dragonActor->AsActorState()->actorState2.allowFlying = false
	//  - if set to true:
	//		* set dragonActor->AsActorState()->actorState2.allowFlying = true

	bool DragonFlyLandHook::SetAllowFlying(void* param_1,void* param_2,RE::Actor* param_3,void* param_4,
          void* param_5,void* param_6,void* param_7,void* param_8)
	{
		if (_SetAllowFlying == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}
log::info("{}: SetAllowFlying called for actor {}", __FUNCTION__, param_3 ? param_3->GetName() : "null", param_4 ? "true" : "false");
		return reinterpret_cast<decltype(&SetAllowFlying)>(_SetAllowFlying)(param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8);
	}

	void DragonFlyLandHook::SetAllowFlyingEx_papyrus(RE::BSScript::IVirtualMachine* a_vm,          // Papyrus VM (unused)
											RE::VMStackID          a_stackID,     // Papyrus call stack (unused)
											RE::Actor*                     a_actor,       // Target actor
											char                       a_bAllow,      // 1 = grant flying, 0 = revoke
											char                       a_searchFlag,  // Forwarded to GetCurrentMountCellOrWorldspaceForm
											void*                 a_pkgFlag       // Forwarded to _ts_InitiateForcedLanding as param_3
			)
	{
		if (_SetAllowFlyingEx_papyrus == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: SetAllowFlyingEx_papyrus called for actor {}", __FUNCTION__, a_actor ? a_actor->GetName() : "null");
		reinterpret_cast<decltype(&SetAllowFlyingEx_papyrus)>(_SetAllowFlyingEx_papyrus)(a_vm, a_stackID, a_actor, a_bAllow, a_searchFlag, a_pkgFlag);
	}

	void DragonFlyLandHook::InitiateForcedLanding(RE::Actor* a_actor, RE::TESForm* a_targetForm, bool a_packageFlag3, bool a_packageFlag4)
	{
		// Called via SetAllowFlying (false) path
		if (_InitiateForcedLanding == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: InitiateForcedLanding called for actor {}, targetFormID = {:0x}, flag3 = {}, flag4 = {}", __FUNCTION__, 
a_actor ? a_actor->GetName() : "null", a_targetForm ? a_targetForm->GetFormID() : 0, a_packageFlag3 ? "true" : "false", a_packageFlag4 ? "true" : "false");
		reinterpret_cast<decltype(&InitiateForcedLanding)>(_InitiateForcedLanding)(a_actor, a_targetForm, a_packageFlag3, a_packageFlag4);
	}

	void DragonFlyLandHook::RunFlyLandPackageProcedure(RE::AIProcess* a_this, RE::Actor* a_actor)
	{
		// Called via SetAllowFlying (false) -> InitiateForcedLanding() path

		// The InitiateForcedLanding() path puts THE DRAGON into kLanding package & procedure
		// RunFlyLandPackageProcedure() is called from the dragon's HighProcessUpdate() once per frame
		// while in kLanding package & procedure

		if (_RunFlyLandPackageProcedure == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
		auto dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (a_actor && dragonActor && a_actor == dragonActor) {
			auto currentProcess = a_actor->GetActorRuntimeData().currentProcess;
			if (currentProcess) {
				auto actorPackage = currentProcess->currentPackage;
				auto currentPackage = actorPackage.package;
				if (currentPackage) {
				
					auto packageSpecificFlags = currentPackage->packData.packageSpecificFlags;
log::info("{}: Dragon actor is in a package with packageSpecificFlags: {:0x}", __FUNCTION__, packageSpecificFlags);
					// bit 11 set?
					if ((packageSpecificFlags >> 0xB & 1) != 0) {
						log::info("{}: Dragon actor is in a package with bit 11 set", __FUNCTION__);
					}
					// bit 12 set?
					if ((packageSpecificFlags >> 0xC & 1) != 0) {
						log::info("{}: Dragon actor is in a package with bit 12 set", __FUNCTION__);
					}

					// set bit 11:
//					packageSpecificFlags |= (1 << 0xB);
//					currentPackage->packData.packageSpecificFlags = packageSpecificFlags;
//log::info("{}: Dragon actor packageSpecificFlags updated to: {:0x}", __FUNCTION__, packageSpecificFlags);
				}
			}
log::info("{}: RunFlyLandPackageProcedure called for dragon actor {}", __FUNCTION__, a_actor ? a_actor->GetName() : "null");
		}
//		float maxDist = *g_maxLandTargetSearchRadius;
//		*g_maxLandTargetSearchRadius = 50000.0f;
		reinterpret_cast<decltype(&RunFlyLandPackageProcedure)>(_RunFlyLandPackageProcedure)(a_this, a_actor);
//		*g_maxLandTargetSearchRadius = maxDist;
	}

	void DragonFlyLandHook::RunFlyLandProcedure(RE::AIProcess* a_this, RE::Actor* a_actor)
	{
		// Called via vanilla dragon-activate -> trigger-landing path

		// The vanilla trigger-landing path puts THE PLAYER into kDismountActor package & procedure
		// RunFlyLandProcedure() is called from the player's HighProcessUpdate() once per frame 
		// while player is in kDismountActor package & procedure

		if (_RunFlyLandProcedure == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: RunFlyLandProcedure called for actor {}", __FUNCTION__, a_actor ? a_actor->GetName() : "null");
		reinterpret_cast<decltype(&RunFlyLandProcedure)>(_RunFlyLandProcedure)(a_this, a_actor);
	}


	void TestHook::HighProcessUpdate(RE::AIProcess* a_this, RE::Actor* a_actor)
	{
		if (_HighProcessUpdate == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}

		reinterpret_cast<decltype(&HighProcessUpdate)>(_HighProcessUpdate)(a_this, a_actor);
		if (a_actor && a_actor == static_cast<RE::Actor*>(RE::PlayerCharacter::GetSingleton())) {
log::info("{}: HighProcessUpdate called for actor {}", __FUNCTION__, a_actor ? a_actor->GetName() : "null");
		}
	}

	std::uint32_t TestHook::GetProcedureIndexRunning(RE::AIProcess* a_this)
	{
		if (_GetProcedureIndexRunning == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return 0;
		}
		std::uint32_t result = reinterpret_cast<decltype(&GetProcedureIndexRunning)>(_GetProcedureIndexRunning)(a_this);

		auto dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
		if (dragonActor && dragonActor->GetActorRuntimeData().currentProcess == a_this) {
//log::info("{}: DragonActor index: {:0x}", __FUNCTION__, result);
		}
		auto player = RE::PlayerCharacter::GetSingleton();
		if (player && player->GetActorRuntimeData().currentProcess == a_this) {
log::info("{}: PlayerCharacter index: {:0x}", __FUNCTION__, result);
		}
		return result;
	}

	void TestHook::PlayerCharacter_Update(RE::PlayerCharacter* a_this, float a_param2)
	{
		if (_PlayerCharacter_Update == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: PlayerCharacter_Update called for actor {} with param2: {}", __FUNCTION__, a_this ? a_this->GetName() : "null", a_param2);
		reinterpret_cast<decltype(&PlayerCharacter_Update)>(_PlayerCharacter_Update)(a_this, a_param2);
	}

	bool TestHook::ObjectRefActivate(void* a_this,void* a_activator,void* a_arg2,void* a_object,void* a_count,
               bool a_defaultProcessingOnly)
	{
		if (_ObjectRefActivate == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}
log::info("{}: --------------->>>>>>>>>>>>>> ObjectRefActivate called", __FUNCTION__);
		return reinterpret_cast<decltype(&ObjectRefActivate)>(_ObjectRefActivate)(a_this, a_activator, a_arg2, a_object, a_count, a_defaultProcessingOnly);
	}

	void TestHook::FlyingMountTriggerLand(void* a_this, void* a_param2, void* a_param3, void* a_param4)
	{
		if (_FlyingMountTriggerLand == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: --------------->>>>>>>>>>>>>> FlyingMountTriggerLand called", __FUNCTION__);
		reinterpret_cast<decltype(&FlyingMountTriggerLand)>(_FlyingMountTriggerLand)(a_this, a_param2, a_param3, a_param4);
	}

	void TestHook::FlyingMountActivate(void* a_this, void* a_param2)
	{
		if (_FlyingMountActivate == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: --------------->>>>>>>>>>>>>> FlyingMountActivate called", __FUNCTION__);
		reinterpret_cast<decltype(&FlyingMountActivate)>(_FlyingMountActivate)(a_this, a_param2);
	}


	bool ExecuteTeleportHook::ExecuteTeleport(
		RE::PlayerCharacter* a_this)
	{
		if (_ExecuteTeleport == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}
//		if (m_blockExecuteTeleport) {
			if (a_this->GetPlayerRuntimeData().queuedTargetLoc.isValid) {
log::info("{}: --------------->>>>>>>>>>>> ExecuteTeleport called - isValid == true", __FUNCTION__);
//				a_this->GetPlayerRuntimeData().queuedTargetLoc.isValid = false;
			}
//		}
		using FuncType = bool(*)(RE::PlayerCharacter*);
log::info("{}: Calling original ExecuteTeleport", __FUNCTION__);
		return reinterpret_cast<FuncType>(_ExecuteTeleport)(a_this);
	}


	void UpdateFlyingMountFastTravelHook::UpdateFastTravel(float a_deltaTime) {
		if (_UpdateFastTravel == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: UpdateFastTravel called, deltaTime: {}", __FUNCTION__, a_deltaTime);
		reinterpret_cast<decltype(&UpdateFastTravel)>(_UpdateFastTravel)(a_deltaTime);
	}

	void UpdateFlyingMountFastTravelHook::UpdateFastTravelState() {
		if (_UpdateFastTravelState == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: UpdateFlyingMountFastTravelState called", __FUNCTION__);
		reinterpret_cast<decltype(&UpdateFastTravelState)>(_UpdateFastTravelState)();
	}

	void UpdateFlyingMountFastTravelHook::UpdatePatrolQueuedState(uint32_t a_mode) {
		if (_UpdatePatrolQueuedState == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: UpdatePatrolQueuedState called for mode {}", __FUNCTION__, a_mode);
		reinterpret_cast<decltype(&UpdatePatrolQueuedState)>(_UpdatePatrolQueuedState)(a_mode);
	}

	void UpdateFlyingMountFastTravelHook::ApproachTarget(RE::NiPoint3* a_targetPos,
									std::uint64_t a_modeRaw,
									std::uint64_t a3,
									std::uint64_t a4) {
		if (_ApproachTarget == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: ApproachTarget called, ({}, {}, {}), modeRaw: {}, a3: {}, a4: {}", __FUNCTION__, a_targetPos->x, a_targetPos->y, a_targetPos->z, a_modeRaw, a3, a4);
		reinterpret_cast<decltype(&ApproachTarget)>(_ApproachTarget)(a_targetPos, a_modeRaw, a3, a4);
	}

	void UpdateFlyingMountFastTravelHook::ExecuteArrival() {
		if (_ExecuteArrival == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: ExecuteArrival called", __FUNCTION__);
		reinterpret_cast<decltype(&ExecuteArrival)>(_ExecuteArrival)();
	}	

	void PapyrusFastTravelHook::FastTravel(RE::BSScript::IVirtualMachine* a_vm,       // Papyrus VM (for error reporting)
											RE::VMStackID                  a_stackID,  // Calling script's stack ID
											RE::StaticFunctionTag*         a_staticTag, // Static function tag placeholder (unused)
											RE::TESObjectREFR*             a_location ) {
		if (_FastTravel == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: FastTravel called ", __FUNCTION__);
		reinterpret_cast<decltype(&FastTravel)>(_FastTravel)(a_vm, a_stackID, a_staticTag, a_location);
	}


	bool VoiceSpellCastHook::HandleVoiceSpellCast(std::uintptr_t  a_this, RE::Actor* a_caster) {
		if (_HandleVoiceSpellCast == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}

log::info("{}: HandleVoiceSpellCast called for caster {}", __FUNCTION__, a_caster ? a_caster->GetName() : "null");

			// call the original function
		auto result = reinterpret_cast<decltype(&HandleVoiceSpellCast)>(_HandleVoiceSpellCast)(a_this, a_caster);

		return result;
	}

	void VoiceShoutCastHook::VoiceShoutCast(RE::Actor* a_caster) {
		if (_VoiceShoutCast == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: VoiceShoutCast called", __FUNCTION__);

		if(a_caster) {
			log::info("{}: Caster is {}", __FUNCTION__, a_caster->GetFormID());
		} else {
			log::warn("{}: Caster is null", __FUNCTION__);
		}
		// call the original function
		reinterpret_cast<decltype(&VoiceShoutCast)>(_VoiceShoutCast)(a_caster);
log::info("{}: VoiceShoutCast original function returned", __FUNCTION__);
	}

	bool StartCastHook::StartCast(RE::Actor* a_caster, RE::MagicSystem::CastingSource a_source) {
		if (_StartCast == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}
log::info("{}: StartCast called ...", __FUNCTION__);
		// call the original function
		auto result = reinterpret_cast<decltype(&StartCast)>(_StartCast)(a_caster, a_source);

		return result;
	}

	void CastSpellImmediateHook::CastSpellImmediate(RE::MagicCaster* _a_magicCaster,
													RE::MagicItem* _a_spell,
													bool _a_loadCast,
													RE::TESObjectREFR* _a_desiredTargetRef,
													float _a_effectivenessMult,
													bool _a_adjustOnlyHostileEffectiveness,
													float _a_magnitudeOverride) {		
		
		if (_CastSpellImmediate == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: CastSpellImmediate called ", __FUNCTION__);
		// call the original function
		reinterpret_cast<decltype(&CastSpellImmediate)>(_CastSpellImmediate)(_a_magicCaster, _a_spell, _a_loadCast, _a_desiredTargetRef, _a_effectivenessMult, _a_adjustOnlyHostileEffectiveness, _a_magnitudeOverride);
log::info("{}: CastSpellImmediate original function returned", __FUNCTION__);
	}

	bool ApplyCastHook::ApplyCast(RE::MagicCaster* _a_magicCaster,
								float _a_effectivenessMult,
								std::uint32_t* _a_targetCount,
								RE::TESBoundObject* _a_source,
								bool _a_loadCast,
								bool _a_adjustOnlyHostileEffectiveness) {
		if (_ApplyCast == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}

if (_a_source) {
	log::info("{}: ApplyCast called with source {} ({}) with min ({}, {}, {}), max ({}, {}, {})", __FUNCTION__, 
		_a_source->GetFormID(), _a_source->GetName(), _a_source->boundData.boundMin.x, _a_source->boundData.boundMin.y, _a_source->boundData.boundMin.z, _a_source->boundData.boundMax.x, _a_source->boundData.boundMax.y, _a_source->boundData.boundMax.z);
} else {
	log::warn("{}: ApplyCast called with null source", __FUNCTION__);
}

bool hasMagicCaster = false;
if (!_a_magicCaster) {
	log::warn("{}: ApplyCast called with null magic caster", __FUNCTION__);
} else {
	log::info("{}: ApplyCast called with magic caster {}", __FUNCTION__, _a_magicCaster->GetCasterAsActor() ? _a_magicCaster->GetCasterAsActor()->GetName() : "null");
	hasMagicCaster = true;
}

		// call the original function
		auto result = reinterpret_cast<decltype(&ApplyCast)>(_ApplyCast)(_a_magicCaster, _a_effectivenessMult, _a_targetCount, _a_source, _a_loadCast, _a_adjustOnlyHostileEffectiveness);
log::info("{}: ApplyCast original function returned", __FUNCTION__);
		return result;
	}

	bool TestCastHook::TestCast(RE::MagicCaster* a_magicCaster,
								RE::MagicItem* a_spell,
								RE::Actor* a_target,
								RE::TESBoundObject* a_source,
								bool a_loadCast) {
		if (_TestCast == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}
log::info("{}: TestCast called ", __FUNCTION__);
		// call the original function
auto actorMagicCaster = static_cast<RE::ActorMagicCaster*>(a_magicCaster);
if (actorMagicCaster) {
	auto node = actorMagicCaster->magicNode;
	if (node) {
auto head = _ts_SKSEFunctions::GetTargetPoint(actorMagicCaster->GetCasterAsActor(),  RE::BGSBodyPartDefs::LIMB_ENUM::kHead);

if (!head) {
    log::error("{}: Error: Could not get head node for dragonActor", __FUNCTION__);
    return false;
}
actorMagicCaster->magicNode = head->AsNode(); // for testing, temporarily set magicNode to head node, to see if that fixes the issue with dragon's magic hands not being visible in the shout animation

		log::info("{}: Magic caster node set to head!!", __FUNCTION__);
	} else {
		log::warn("{}: Magic caster node is null", __FUNCTION__);
	}
} else {
	log::warn("{}: Magic caster is null", __FUNCTION__);
}
		auto result = reinterpret_cast<decltype(&TestCast)>(_TestCast)(a_magicCaster, a_spell, a_target, a_source, a_loadCast);
log::info("{}: TestCast original function returned", __FUNCTION__);
		return result;
	}

	void ProcedureShoutHook::Initiate(std::uint64_t*  a_this,     // param_1: BGSProcedureShout*
    							std::uint64_t*  a_context) {
		if (_Initiate == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: ProcedureShoutHook::Initiate called ", __FUNCTION__);
		// call the original function
		reinterpret_cast<decltype(&Initiate)>(_Initiate)(a_this, a_context);
log::info("{}: ProcedureShoutHook::Initiate original function returned", __FUNCTION__);
	}

	void ProcedureShoutHook::SetupExecState(std::uint64_t*  a_this,     // param_1: BGSProcedureShout*
								std::uint64_t*  a_context) {
		if (_SetupExecState == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return;
		}
log::info("{}: ProcedureShoutHook::SetupExecState called ", __FUNCTION__);
		// call the original function
		reinterpret_cast<decltype(&SetupExecState)>(_SetupExecState)(a_this, a_context);
log::info("{}: ProcedureShoutHook::SetupExecState original function returned", __FUNCTION__);
	}

	bool StartVoiceShoutCastHook::StartVoiceShoutCast(RE::Character* a_caster,
													RE::TESShout* a_shout,
													std::uint32_t a_wordIndex,
													RE::Actor* a_target) {
		if (_StartVoiceShoutCast == 0) {
			log::error("{}: trampoline not initialized!", __FUNCTION__);
			return false;
		}
log::info("{}: StartVoiceShoutCast called, caster: {}, shout: {}, wordIndex: {}, target: {}", __FUNCTION__,
	 a_caster ? a_caster->GetName() : "null", a_shout ? a_shout->GetName() : "null", a_wordIndex, a_target ? a_target->GetName() : "null");
		// call the original function
		auto result = reinterpret_cast<decltype(&StartVoiceShoutCast)>(_StartVoiceShoutCast)(a_caster, a_shout, a_wordIndex, a_target);
log::info("{}: StartVoiceShoutCast original function returned {}", __FUNCTION__, result ? "true" : "false");
		return result; 
	}
*/
} // namespace Hooks
