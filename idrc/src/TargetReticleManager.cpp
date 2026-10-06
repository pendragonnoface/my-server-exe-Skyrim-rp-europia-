#include "TargetReticleManager.h"
#include "APIManager.h"
#include "Offsets.h"
#include "_ts_SKSEFunctions.h"
#include "DataManager.h"
#include "ControlsManager.h"
#include "CombatManager.h"
#include "IDRCUtils.h"

namespace IDRC {
    void TargetReticleManager::Initialize()
    {
        m_isReticleLocked = false;
        m_isWidgetActive = false;
        m_combatState = 0;
        m_reticleTarget = RE::ActorHandle{};
        m_currentTargetMode = TargetMode::kNone;
        
        if (m_isInitialized) {
            log::info("{}: CombatTargetReticle already initialized", __FUNCTION__);
            return;
        }

        if (APIs::TrueHUD) {
            APIs::TrueHUD->LoadCustomWidgets(SKSE::GetPluginHandle(), "IntuitiveDragonRideControl/IDRC_Widgets.swf"sv, [this](TRUEHUD_API::APIResult a_apiResult) {
                if (a_apiResult == TRUEHUD_API::APIResult::OK) {
                    log::info("IDRC - TrueHUD API: CombatTargetReticle loaded successfully.");
                    APIs::TrueHUD->RegisterNewWidgetType(SKSE::GetPluginHandle(), 'IDRC');
                    this->m_isInitialized = true;
                }
            });
        }
    }

    void TargetReticleManager::SetPrimaryTargetMode(TargetMode a_targetMode) {
        m_primaryTargetMode = a_targetMode;
    }

    void TargetReticleManager::SetMaxTargetDistance(float a_maxReticleDistance) {
        m_maxReticleDistance = a_maxReticleDistance;
    }

    void TargetReticleManager::SetDistanceMultiplierSmall(float a_distanceMultiplierSmall) {
        m_distanceMultiplierSmall = a_distanceMultiplierSmall;
    }

    void TargetReticleManager::SetDistanceMultiplierLarge(float a_distanceMultiplierLarge) {
        m_distanceMultiplierLarge = a_distanceMultiplierLarge;
    }

    void TargetReticleManager::SetDistanceMultiplierExtraLarge(float a_distanceMultiplierExtraLarge) {
        m_distanceMultiplierExtraLarge = a_distanceMultiplierExtraLarge;
    }
    void TargetReticleManager::SetMaxTargetScanAngle(float a_maxTargetScanAngle) {
        m_maxTargetScanAngle = a_maxTargetScanAngle;
    }

    void TargetReticleManager::Update()
    {
        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return;
        }

        if (!APIs::TrueHUD) {
            return;
        }

        if (!DataManager::GetSingleton().GetDragonActor()) {
            return;
        }

        if (!m_isInitialized) {
            log::warn("{}: CombatTargetReticle not initialized", __FUNCTION__);
            return;
        }

        UpdateReticle();
    }

    void TargetReticleManager::UpdateReticle() {
        if (!APIs::TrueHUD) {
            log::info("{}: TrueHUD API not available", __FUNCTION__);
            return;
        }

        if (!m_isInitialized) {
            return;
        }

        if (m_reticleMode == ReticleMode::kOff) {
            DisposeReticle();
            return;
        }

        RE::Actor* newTarget = nullptr;
        bool hasTDMTarget = false;

        if (IsTDMLocked()) {
            if (APIs::TrueDirectionalMovementV1) {
                auto targetHandle = APIs::TrueDirectionalMovementV1->GetCurrentTarget();
                if (targetHandle) {
                    newTarget = targetHandle.get().get();
                } else {
                    newTarget = nullptr;
                }
                hasTDMTarget = true;
            }
        }

        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::warn("{}: No dragon actor found", __FUNCTION__);
            DisposeReticle();
            return;
        }

        int newCombatState = GetCombatState();

        if (m_isReticleLocked) {
            auto* lockedActor = m_reticleTarget ? m_reticleTarget.get().get() : nullptr;
            if (lockedActor && lockedActor->GetDistance(dragonActor) <= m_maxReticleDistance * GetDistanceRaceSizeMultiplier(lockedActor->GetRace()) && !lockedActor->IsDead()) {
                if (!m_isWidgetActive || m_combatState != newCombatState) {
                    // re-enable reticle in case it was disposed during TDM target lock
                    m_combatState = newCombatState;
                    SetReticleTarget();
                }
                return;
            } else {
                ToggleLockReticle();
            }
        }

        auto* selectedActor = GetSelectedActor();        
        auto combatHandle = GetCombatTarget();
        auto* combatTarget = combatHandle ? combatHandle.get().get() : nullptr;
        
        // Get new target mode after updates
        TargetMode newTargetMode = GetTargetMode((selectedActor != nullptr), (bool)combatHandle);
        
        // Determine appropriate target for the new target mode
        if (!hasTDMTarget) {
            if (newTargetMode == TargetMode::kCombatTarget) {
                newTarget = combatTarget;
            } else if (newTargetMode == TargetMode::kSelectedActor) {
                newTarget = selectedActor;
            } else {
                if (m_reticleTarget) {
                    DisposeReticle();
                }
                return;
            }
        }

        if ((!m_isWidgetActive && newTarget) ||
             m_currentTargetMode != newTargetMode || 
             m_reticleTarget.get().get() != newTarget ||
             m_combatState != newCombatState)
        {
            m_currentTargetMode = newTargetMode;
            m_reticleTarget = newTarget ? newTarget->GetHandle() : RE::ActorHandle{};
            m_combatState = newCombatState;
            SetReticleTarget();
        }

        UpdateReticleState();
    }

    int TargetReticleManager::GetCombatState() {
        int combatState = 0;
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            log::warn("{}: No dragon actor found", __FUNCTION__);
            return combatState;
        }
        
        if (m_reticleTarget == GetCombatTarget()) {    
            if (Utils::IsFastTravelActive()) {
                combatState = CombatManager::GetSingleton().GetStoredCombatTargetState();
            } else {
                combatState = _ts_SKSEFunctions::GetCombatState(dragonActor);
            }
        }

        return combatState;
    }

    bool TargetReticleManager::IsTDMLocked() {
        bool isTDMLocked = false;
        if (APIs::TrueDirectionalMovementV1 && APIs::TrueDirectionalMovementV1->GetTargetLockState()) {
            isTDMLocked = true;
        }
        return isTDMLocked;
    }

    RE::ActorHandle TargetReticleManager::GetCurrentTarget() const {
        if (m_reticleTarget) {
            return m_reticleTarget;
        }

        // In case reticle logic is not active (eg TrueHUD not installed), fall back to selected/combat targets
        auto* selectedActor = GetSelectedActor();
        auto* combatTarget = GetCombatTarget().get().get();
        TargetMode targetMode = GetTargetMode((selectedActor != nullptr), (combatTarget != nullptr));

        RE::Actor* currentTarget = nullptr;

        // Use primary target
        if (targetMode == TargetMode::kCombatTarget) {
            currentTarget = combatTarget;
        }
        else if (targetMode == TargetMode::kSelectedActor) {
            currentTarget = selectedActor;
        }

        // If no primary target, use secondary target
        if (!currentTarget) {
            if (targetMode == TargetMode::kCombatTarget) {
                currentTarget = selectedActor;
            }
            else if (targetMode == TargetMode::kSelectedActor) {
                currentTarget = combatTarget;
            } else if (selectedActor) {  
                // in case m_reticleMode == ReticleMode::kOnlyCombatTarget
                // GetTargetMode() will never return  TargetMode::kSelectedActor
                // make sure selected actor is used in that case - if available
                currentTarget = selectedActor;
            }
        }

        return currentTarget ? currentTarget->GetHandle() : RE::ActorHandle{}; 
    }

    bool TargetReticleManager::GetUseTarget() const {
        // This API function can be used to inform other mods that they should use IDRC's current combat target
        // Currently in use in TrueDirectionalMovement: 
        //     if GetUseTarget() returns true, the TDMLock will lock on the dragon's target when activated.
        return m_useTarget;
    }

    void TargetReticleManager::SetUseTarget(bool a_useTarget) {
        m_useTarget = a_useTarget;
    }

    RE::Actor* TargetReticleManager::GetSelectedActor() const {
        auto* playerActor = RE::PlayerCharacter::GetSingleton();
        auto* processLists = RE::ProcessLists::GetSingleton();
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();

        if (!playerActor) {
            log::error("{}: PlayerActor is null", __FUNCTION__);
            return nullptr;
        }
        if (!processLists) {
            log::error("{}: ProcessLists is null", __FUNCTION__);
            return nullptr;
        }
        if (!playerCamera) {
            log::error("{}: PlayerCamera is null", __FUNCTION__);
            return nullptr;
        }
        if (!dragonActor) {
            log::error("{}: DragonActor is null", __FUNCTION__);
            return nullptr;
        }

        auto cameraPos = playerCamera->cameraRoot->world.translate;
        auto playerPos = playerActor->GetPosition();

        auto root = playerCamera->cameraRoot;
        const auto& worldTransform = root->world; // RE::NiTransform

        // The forward vector is the third column of the rotation matrix
        RE::NiPoint3 cameraForward = worldTransform.rotate * RE::NiPoint3{ 0.0f, 1.0f, 0.0f };
        float cameraToPlayerDistance = playerPos.GetDistance(cameraPos);

        std::vector<RE::Actor*> excludeActors;
        excludeActors.push_back(dragonActor);

        RE::Actor* selectedActor = nullptr;
        for (auto handle : processLists->highActorHandles) {
            auto actor = handle.get().get();
            if (actor
                && std::find(excludeActors.begin(), excludeActors.end(), actor) == excludeActors.end() 
                && actor->Get3D() 
                && !actor->IsDead()
//                && !(actor->GetFactionReaction(playerActor) == RE::FIGHT_REACTION::kAlly)
                && (m_maxReticleDistance <= 0.0f || actor->GetPosition().GetDistance(playerPos) <= m_maxReticleDistance * GetDistanceRaceSizeMultiplier(actor->GetRace()))) {

                RE::NiPoint3 actorPos = actor->GetPosition();
                float fAngleForward = _ts_SKSEFunctions::GetAngleBetweenVectors(actorPos - cameraPos, cameraForward);
                float fAngleBackwards = _ts_SKSEFunctions::GetAngleBetweenVectors(playerPos - actorPos, -cameraForward);
                float distanceToPlayer = actorPos.GetDistance(playerPos);
                
                // forward cone: starting at camerapos, opening towards playerpos with angle m_maxTargetScanAngle
                bool inForwardCone = (fAngleForward <= m_maxTargetScanAngle);
                // backwards cone: starting at player, opening towards camerapos with angle m_maxTargetScanAngle
                bool inBackwardCone = (fAngleBackwards >= 180.f - m_maxTargetScanAngle) && (distanceToPlayer < cameraToPlayerDistance);
                if (inForwardCone || inBackwardCone || m_maxTargetScanAngle <= 0.0f) {
                    if (selectedActor) {
                        if (actor->GetPosition().GetDistance(playerPos) < selectedActor->GetPosition().GetDistance(playerPos)) {
                            selectedActor = actor;
                        }
                    } else {
                        selectedActor = actor;
                    }
                }
            }
        }

        return selectedActor;
    }

    RE::ActorHandle TargetReticleManager::GetCombatTarget() const {
        auto* dragonActor = DataManager::GetSingleton().GetDragonActor();
        if (!dragonActor) {
            return RE::ActorHandle{};
        }
        auto handle = dragonActor->GetActorRuntimeData().currentCombatTarget;

        if (!(handle && handle.get().get()) && (Utils::IsFastTravelActive())) {
           handle = CombatManager::GetSingleton().GetStoredCombatTargetHandle();
        }
        
        return handle;
    }

    TargetReticleManager::TargetMode TargetReticleManager::GetTargetMode(bool a_hasSelectedActor, bool a_hasCombatTarget) const {
        TargetMode targetMode = TargetMode::kNone;

        // check for primary target, as defined by m_primaryTargetMode
        if (m_primaryTargetMode == TargetMode::kCombatTarget || m_reticleMode == ReticleMode::kOnlyCombatTarget) {
            targetMode = a_hasCombatTarget ? TargetMode::kCombatTarget : TargetMode::kNone;
        }
        else if (m_primaryTargetMode == TargetMode::kSelectedActor) {
            targetMode = a_hasSelectedActor ? TargetMode::kSelectedActor : TargetMode::kNone;
        }

        // If no primary target found, check for secondary target
        if (targetMode == TargetMode::kNone && m_reticleMode != ReticleMode::kOnlyCombatTarget) {
            if (m_primaryTargetMode == TargetMode::kCombatTarget) {
                targetMode = a_hasSelectedActor ? TargetMode::kSelectedActor : TargetMode::kNone;
            }
            else if (m_primaryTargetMode == TargetMode::kSelectedActor) {
                targetMode = a_hasCombatTarget ? TargetMode::kCombatTarget : TargetMode::kNone;
            }
        }

        return targetMode;
    }

    bool TargetReticleManager::IsReticleLocked() const {
        return m_isReticleLocked;
    }

    void TargetReticleManager::ToggleLockReticle() {
        if (!APIs::TrueHUD) {
            log::info("{}: TrueHUD API not available", __FUNCTION__);
            return;
        }

        auto* lockedActor = m_reticleTarget ? m_reticleTarget.get().get() : nullptr;
        if (lockedActor || m_isReticleLocked) {
            m_isReticleLocked = !m_isReticleLocked;
            std::string sMessage = "Target reticle " + std::string(m_isReticleLocked && lockedActor ? "locked on " + std::string(lockedActor->GetName()) + "." : "unlocked.");
            RE::SendHUDMessage::ShowHUDMessage(sMessage.c_str());
        } else {
            std::string sMessage = "No target to lock target reticle on.";
            RE::SendHUDMessage::ShowHUDMessage(sMessage.c_str());
        }

        UpdateReticleState();
    }

    void TargetReticleManager::UpdateReticleState() {

        if (!m_reticleTarget) {
            DisposeReticle();
            return;
        }
        
        auto widget = m_TargetReticle.lock();
        if (!widget) {
            return;
        }
        
        widget->UpdateState(m_isReticleLocked, IsTDMLocked(), m_combatState);
    }

    void TargetReticleManager::TogglePrimaryTargetMode() {
        if (m_primaryTargetMode == TargetMode::kCombatTarget) {
            m_primaryTargetMode = TargetMode::kSelectedActor;
            RE::SendHUDMessage::ShowHUDMessage("Primary target: picked from screen center.");
        } else if (m_primaryTargetMode == TargetMode::kSelectedActor) {
            m_primaryTargetMode = TargetMode::kCombatTarget;
            RE::SendHUDMessage::ShowHUDMessage("Primary target: the dragon's current combat target.");
        } else {
            m_primaryTargetMode = TargetMode::kSelectedActor;
        }
    }    

    void TargetReticleManager::DisposeReticle() {
        if (!APIs::TrueHUD) {
            log::info("{}: TrueHUD API not available", __FUNCTION__);
            return;
        }

        m_reticleTarget = RE::ActorHandle{};

        auto widget = m_TargetReticle.lock();
        if (!widget || !m_isWidgetActive) {
            return;
        }

        widget->WidgetReadyToRemove();

        APIs::TrueHUD->RemoveWidget(SKSE::GetPluginHandle(), 'IDRC', 0, TRUEHUD_API::WidgetRemovalMode::Normal);
        m_isWidgetActive = false;
    }

    void TargetReticleManager::SetReticleLockAnimationStyle(int a_style) {
        m_reticleLockAnimationStyle = a_style;
        auto widget = m_TargetReticle.lock();
        if (widget) {
            widget->SetReticleLockAnimationStyle(m_reticleLockAnimationStyle);
        }
    }

    void TargetReticleManager::SetReticleTarget() {
        if (!APIs::TrueHUD) {
            log::info("{}: TrueHUD API not available", __FUNCTION__);
            return;
        }

        if (!m_reticleTarget) {
            DisposeReticle();
            return;
        }

        auto actorHandle = m_reticleTarget;
        auto* reticleActor = actorHandle.get().get();
        if (!reticleActor) {
            log::warn("{}: Actor handle is invalid", __FUNCTION__);
            DisposeReticle();
            return;
        }

        auto targetPoint = GetTargetPoint(actorHandle);
        if (!targetPoint) {
            log::warn("{}: Target point is nullptr", __FUNCTION__);
            DisposeReticle();
            return;
        }

        auto widget = m_TargetReticle.lock();
        if (widget) {
            widget->ChangeTarget(actorHandle, targetPoint);
        } else {
            widget = std::make_shared<CombatTargetReticle>(actorHandle.native_handle(), actorHandle, targetPoint,
                                                            m_reticleLockAnimationStyle);
            if (!widget) {
                log::warn("{}: Failed to create CombatTargetReticle widget", __FUNCTION__);
                return;
            }
            
            m_TargetReticle = widget;
            APIs::TrueHUD->AddWidget(SKSE::GetPluginHandle(), 'IDRC', 0, "IDRC_TargetReticle", widget);
        }
        m_isWidgetActive = true;
        
        UpdateReticleState();
    }

    RE::NiPointer<RE::NiAVObject> TargetReticleManager::GetTargetPoint(RE::ActorHandle a_actorHandle) const {
        RE::NiPointer<RE::NiAVObject> targetPoint = nullptr;

        auto* a_actor = a_actorHandle.get().get();
        if (!a_actor) {
            return nullptr;
        }

        auto race = a_actor->GetRace();
        if (!race) {
            return nullptr;
        }

        RE::BGSBodyPartData* bodyPartData = race->bodyPartData;
        if (!bodyPartData) {
            return nullptr;
        }

        auto actor3D = a_actor->Get3D2();
        if (!actor3D) {
            return nullptr;
        }
    
        RE::BGSBodyPart* bodyPart = bodyPartData->parts[RE::BGSBodyPartDefs::LIMB_ENUM::kTorso];
        if (!bodyPart) {
            bodyPart = bodyPartData->parts[RE::BGSBodyPartDefs::LIMB_ENUM::kTotal];
        }
        if (!bodyPart) {
            bodyPart = bodyPartData->parts[RE::BGSBodyPartDefs::LIMB_ENUM::kHead];
        }
        if (bodyPart) {
            targetPoint = RE::NiPointer<RE::NiAVObject>(NiAVObject_LookupBoneNodeByName(actor3D, bodyPart->targetName, true));
        }

        return targetPoint;
    }

    float TargetReticleManager::GetMaxTargetDistance(RE::Actor* a_actor) const {
        if (!a_actor) {
            return m_maxReticleDistance;
        }
        
         return  m_maxReticleDistance * GetDistanceRaceSizeMultiplier(a_actor->GetRace()); 
    }

    // GetDistanceRaceSizeMultiplier() provides consistency with True Directional Movement's settings
    // Corresponding TDM function is DirectionalMovementHandler::GetTargetLockDistanceRaceSizeMultiplier()
    // see https://github.com/ersh1/TrueDirectionalMovement
    float TargetReticleManager::GetDistanceRaceSizeMultiplier(RE::TESRace* a_race) const {
        if (a_race) {
            switch (a_race->data.raceSize.get())
            {
            case RE::RACE_SIZE::kMedium:
            default:
                return 1.f;
            case RE::RACE_SIZE::kSmall:
                return m_distanceMultiplierSmall;
            case RE::RACE_SIZE::kLarge:
                return m_distanceMultiplierLarge;
            case RE::RACE_SIZE::kExtraLarge:
                return m_distanceMultiplierExtraLarge;
            }
        }

        return 1.f;
    }
}  // namespace IDRC
