#pragma once

namespace IDRC {   

    // GENERAL NOTES ON DRAGON FAST TRAVEL: 
    ///////////////////////////////////////
    //
    // This mod uses the game engine's dragon-FastTravel logic to provide directed flying.
    //
    // The game engine uses a special mode during dragon-FastTravel, with different dragon AI
    // behavior. For example, the dragon ignores combat while in this mode.
    // The game engine dragon-FastTravel mode consists of two states: a PatrolQueuedState
    // and the actual FastTravelState. See Hooks.h for a detailed RE'ed callstack of the FastTravel-related vanilla functions,
    // and for a definition of the PatrolQueued states used in the game.
    //
    // Whenever the engine decides to put the dragon into the dragon-FastTravel mode, 
    // the dragon first enters the PatrolQueuedState. 
    // The purpose of the PatrolQueuedState is to patrol the dragon towards the entry point of the FastTravelState. 
    // Once the dragon's orientation relative to the FastTravel target is good, the engine switches the dragon 
    // into the FastTravelState, and the dragon starts to fly on a straight line to the target.
    // The engine can also switch to PatrolQueuedState when the dragon exits FastTravelState to approach the target. 
    //
    // The engine switches into dragon-FastTravel mode when
    // 1) a FastTravel is initiated (eg via papyrus or map-triggered FastTravel, COC, etc)
    // 2) the dragon is put into an orbit package, with the orbit target far enough away from the dragon.
    //
    // The engine switches the dragon out of dragon-FastTravel mode when the dragon is IN AN ORBIT PACKAGE,
    // and the orbit target is close enough to the dragon.
    // If the dragon is not in an orbit package, the engine will not switch the dragon out of dragon-FastTravel mode, 
    // even if the dragon is close to the target! 
    //
    // Important for our purpose of controlling the dragon's direction during flight is that each time a FastTravel is initiated
    // with a new target, this triggers an update of the dragon's flight path.
    // This flight path update is hooked via Hooks::PathingHook::SetFlightPath() 
    // and Hooks::PathingHook::FlightPlannerUpdate() hooks.
    // In these hooks IDRC modifies the dragon's flight path waypoints to keep the dragon 
    // flying in the direction that the player has requested.
    // By re-triggering FastTravels during flight, IDRC can adjust the dragon's flight path continuously.
    // If FastTravel is not re-triggered while the dragon is in flight, 
    // no SetPath and FlightPlannerUpdate calls are triggered by the engine,
    // and therefore the pathing cannot be adjusted (or will be ignored by the engine).
    // Probably there are other ways to force the engine to re-evalute the flight pathing, but I haven't found them.
    //
    // The facts that 
    //  - the engine auto-switches to fastTravel for longer dragon flight-paths 
    //    (and I found no easy way to disable this behavior completely) ,
    //  - that we can trigger continuous flight-path updates by re-triggering FastTravel, 
    // is the the reason IDRC'a flight control is based on the existing dragon-FastTravel system.
    //
    // Apart from modifying the dragon's flight path in the SKSE plugin via hooks, the second
    // part of the flight control is to modify the dragon's package stack to control behavior while in dragon-FastTravel mode.
    //
    // While the dragon is in dragon-FastTravel mode, it is in one of the following packages (top of package stack):
    //  * DLC2TameDragonPatrolPlayerRiding (while PatrolQueuedState == true) (vanilla package)
    //  * _ts_DR_TameDragonFastTravelPlayerRiding
    //      -> This package checks for FastTravelState == true, and variable03 == 2. 
    //      -> It is an orbit package with OrbitMarker as target.
    //      -> the OrbitMarker is placed outside the borders of the worldspace, in the target direction.
    //  * _ts_DR_TameDragonStopFastTravel
    //      -> This package checks for FastTravelState == true, and variable03 != 2. 
    //      -> It is an orbit package with the dragon as target.
    //      -> By setting variable03 to 0 during FastTravel, the dragon will switch into this package 
    //      -> and stop FastTravel because the target is the dragon itself, and therefore 
    //      -> close enough that the engine switches the dragon out of dragon-FastTravel mode.
    //      -> Once out of dragon-FastTravel mode (ie StopFastTravel package conditions no longer met),
    //      -> the dragon will switch into the regular orbit package in the dragon's package stack.
    //
    // Note that even while the engine has put the dragon into dragon-FastTravel mode,
    // you can still make the dragon land via SetAllowFlying(false), 
    // or put it into any non-FastTravel package (eg Hover) if you define the package stack accordingly.
    // In that case the dragon will land or hover, but still will be in dragon-FastTravel mode from 
    // the game engine's perspective. 
    // Ie, the game engine assumes that the dragon is in an Orbit package whenever the dragon is in the 
    // dragon-FastTravel mode. The dragon's behavior can be derailled if this engine assumption is not met. 
     

    class FastTravelManager {
    public:
        static FastTravelManager& GetSingleton() {
            static FastTravelManager instance;
            return instance;
        }
        FastTravelManager(const FastTravelManager&) = delete;
        FastTravelManager& operator=(const FastTravelManager&) = delete;

        void Update();

        void InitializeData(RE::BGSListForm* a_list);
            
        void FastTravel(const RE::TESObjectREFR* a_fastTravelTarget);
        
        void SkipFastTravelRequest(bool a_skip) { m_skipFastTravelRequest = a_skip; }
    
    private:
        FastTravelManager() = default;
        ~FastTravelManager() = default;

        bool m_skipFastTravelRequest = false; 
        bool m_lastFastTravelState = false;
    }; // class FastTravelManager
} // namespace IDRC
