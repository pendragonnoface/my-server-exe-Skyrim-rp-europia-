#pragma once
// Europia multiplayer bridge (GPL-3.0-or-later, fork of IDRC by staalo18).
// Starts IDRC ride control natively when the player mounts any dragon,
// without needing Skyrim's DLC2TameDragon quest alias (Bend Will).
namespace IDRC::EuropiaBridge {
    void Update();      // called from the main update hook
    bool IsInUse();     // true once the bridge has taken over at least once
    bool IsActive();    // true while the player is riding under bridge control
    RE::Actor* GetDirectDragon();
}
