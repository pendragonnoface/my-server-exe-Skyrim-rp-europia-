#pragma once

#include "API/TrueDirectionalMovementAPI.h"
#include "API/TrueHUDAPI.h"

struct APIs
{
	static inline TDM_API::IVTDM1* TrueDirectionalMovementV1 = nullptr;
	static inline TDM_API::IVTDM4* TrueDirectionalMovementV4 = nullptr;
    static inline TRUEHUD_API::IVTrueHUD3* TrueHUD = nullptr;

	static bool CheckTDMVersion();
	static void RequestAPIs();
};
