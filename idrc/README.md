## Requirements
* [CommonLibSSE-NG](https://github.com/alandtse/CommonLibSSE-NG)
	* Add the environment variable `COMMONLIBSSE_PATH` with the value as the path to the folder containing CommonLibSSE-NG
* [openvr](https://github.com/valvesoftware/openvr)
	* Extract openvr sources into extern/openvr folder
	* Alternatively set ENABLE_SKYRIM_VR OFF in CMakeLists.txt
* [_ts_SKSEFunctions](https://github.com/staalo18/TS_SKSEFunctions)
	* Install this into a directory parallel to the IDRC project directory
* Change OUTPUT_FOLDER variable in CMakeLists.txt to point to your local path for where the generated DLL should be copied to.

## License

[GPL-3.0-or-later](COPYING).

> [!IMPORTANT]
> This is a breaking change from the previous MIT license. Originally based on code under [MIT](Licenses/LICENSE-MIT).
