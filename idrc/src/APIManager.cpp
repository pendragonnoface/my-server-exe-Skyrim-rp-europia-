#include "APIManager.h"

void APIs::RequestAPIs()
{
	if (!TrueDirectionalMovementV1) {
		TrueDirectionalMovementV1 = reinterpret_cast<TDM_API::IVTDM1*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V1));
		if (TrueDirectionalMovementV1) {
			log::info("Obtained TrueDirectionalMovement API (V1) - {0:x}", reinterpret_cast<uintptr_t>(TrueDirectionalMovementV1));
		}
	}

	if (!TrueDirectionalMovementV4) {
		TrueDirectionalMovementV4 = reinterpret_cast<TDM_API::IVTDM4*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V4));
		if (TrueDirectionalMovementV4) {
			log::info("Obtained TrueDirectionalMovement API (V4) - {0:x}", reinterpret_cast<uintptr_t>(TrueDirectionalMovementV4));
		}
	}

	if (!TrueDirectionalMovementV4 && !TrueDirectionalMovementV1) {
		log::info("Failed to obtain TrueDirectionalMovement API");
	}

    if (!TrueHUD) {
		TrueHUD = reinterpret_cast<TRUEHUD_API::IVTrueHUD3*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V3));
		if (TrueHUD) {
			log::info("Obtained TrueHUD API - {0:x}", reinterpret_cast<uintptr_t>(TrueHUD));
		} else {
			log::info("Failed to obtain TrueHUD API");
		}
	}	
}

bool APIs::CheckTDMVersion() {
	if (!reinterpret_cast<TDM_API::IVTDM4*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V4))
		&& reinterpret_cast<TDM_API::IVTDM1*>(TDM_API::RequestPluginAPI(TDM_API::InterfaceVersion::V1))) {
		return false;
	}
	return true;
}
