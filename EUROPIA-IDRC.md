# IDRC - Europia multiplayer bridge

Fork of **Intuitive Dragon Ride Control** by staalo18
(https://github.com/staalo18/IntuitiveDragonRideControl, Nexus mod 64679), licensed GPL-3.0-or-later.

Change: `src/EuropiaBridge.cpp` starts IDRC ride control natively when the player mounts any dragon,
so it no longer depends on Skyrim's tame-dragon quest alias (Bend Will), which cannot be filled
under the Skyrim multiplayer client.

Only the SKSE plugin source is here. The IDRC ESP, scripts and interface files still come from the
original Nexus download (not redistributed). Built DLL: `dist/IntuitiveDragonRideControl.dll`.
