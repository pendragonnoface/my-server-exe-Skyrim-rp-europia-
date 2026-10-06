#pragma once
// Europia multiplayer bridge (GPL-3.0-or-later, fork of IDRC by staalo18).
// Starts IDRC ride control natively when the player mounts any dragon,
// without needing Skyrim's DLC2TameDragon quest alias (Bend Will).
namespace IDRC::EuropiaBridge {
    void Update();      // called from the main update hook
    bool IsInUse();     // true once the bridge has taken over at least once
    bool IsActive();    // true while the player is riding under bridge control
    RE::Actor* GetDirectDragon();
    // Europia: edge-of-Skyrim turn-back (instead of IDRC's hover loop) and diagnostics
    void SetTurnBack(bool a_on, float a_yaw);
    bool GetTurnBack(float& a_yaw);
    void NoteMouse(bool a_swallowed);
    void NotePath(float a_yaw, float a_pitch);
    void NoteStage(int a_stage);  // 0 planner call,1 our dragon,2 flying mode,3 path data,4 waypoints,5 dragon camera
}
