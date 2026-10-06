#include "ModAPI.h"
#include "DataManager.h"
#include "_ts_SKSEFunctions.h"
#include "TargetReticleManager.h"

Messaging::IDRCInterface::IDRCInterface() noexcept {
	apiTID = GetCurrentThreadId();
}

Messaging::IDRCInterface::~IDRCInterface() noexcept {}

unsigned long Messaging::IDRCInterface::GetIDRCThreadId() const noexcept {
	return apiTID;
}

RE::ActorHandle Messaging::IDRCInterface::GetCurrentTarget() const noexcept  {
    auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
    if (dragonActor) {
        return IDRC::TargetReticleManager::GetSingleton().GetCurrentTarget();
    }

	return RE::ActorHandle();
}

bool Messaging::IDRCInterface::UseTarget() const noexcept  {
    auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
    if (dragonActor) {
        return IDRC::TargetReticleManager::GetSingleton().GetUseTarget();
    }

	return false;
}

RE::ActorHandle Messaging::IDRCInterface::GetDragon() const noexcept  {
    auto* dragonActor = IDRC::DataManager::GetSingleton().GetDragonActor();
    if (dragonActor) {
        return dragonActor->GetHandle();
 	}

	return RE::ActorHandle();
}

