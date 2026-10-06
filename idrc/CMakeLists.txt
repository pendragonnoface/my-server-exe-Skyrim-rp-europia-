# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.21)

# Set your project name. This will be the name of your SKSE .dll file.
set(NAME "IntuitiveDragonRideControl" CACHE STRING "")
set(VERSION 8.2.0 CACHE STRING "")

project(
	${NAME}
	VERSION ${VERSION}
	LANGUAGES CXX
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Plugin.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Plugin.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/plugin.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/plugin.rc
	@ONLY
)

list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/plugin.rc)

# OUTPUT_FOLDER removed for CI build (Europia fork)

option(ENABLE_SKYRIM_SE "Enable support for Skyrim SE in the dynamic runtime feature." ON)
option(ENABLE_SKYRIM_AE "Enable support for Skyrim AE in the dynamic runtime feature." ON)
option(ENABLE_SKYRIM_VR "Enable support for Skyrim VR in the dynamic runtime feature." ON)
set(BUILD_TESTS OFF)

# Get all source files from src/ and include/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h include/*.h)

# Add the include directory for TS_SKSEFunctions
set(TSSKSEFUNCTIONS_INCLUDE_DIR "../TS_SKSEFunctions-main/include")
file(GLOB_RECURSE TSSKSEFUNCTIONS_SOURCES CONFIGURE_DEPENDS "../TS_SKSEFunctions-main/src/*.cpp" "../TS_SKSEFunctions-main/include/*.h")

# Combine the source files
list(APPEND SOURCES ${TSSKSEFUNCTIONS_SOURCES})

# --- CommonLibSSE: Use local path from environment variable ---

set(COMMONLIBSSE_PATH $ENV{CommonLibSSEPath_NG})

if(NOT COMMONLIBSSE_PATH)
    message(FATAL_ERROR "CommonLibSSEPath_NG environment variable is not set!")
endif()

add_subdirectory("${COMMONLIBSSE_PATH}" CommonLibSSE EXCLUDE_FROM_ALL)

# --- ClibUtils: Use local path from environment variable ---
set(CLIBUTILS_PATH $ENV{CLibUtilsPath})

if(NOT CLIBUTILS_PATH)
    message(FATAL_ERROR "CLibUtilsPath environment variable is not set!")
endif()

add_subdirectory("$ENV{CLibUtilsPath}" ClibUtils EXCLUDE_FROM_ALL)

# Add your SKSE plugin target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(${PROJECT_NAME} PRIVATE include/PCH.h) # <--- PCH.h is required!

# Generate PDB files for all build types (Release uses /Zi + /DEBUG to keep optimizations)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/Zi>)
target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)

target_include_directories(${PROJECT_NAME} PRIVATE include ${TSSKSEFUNCTIONS_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

# Link to CommonLibSSE (adjust the library name if needed)
target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)

# Link to CLibUtil (header-only interface library)
target_link_libraries(${PROJECT_NAME} PRIVATE CLibUtil::CLibUtil)

# Include inih for .ini files
find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")
target_include_directories(${PROJECT_NAME} PUBLIC ${SIMPLEINI_INCLUDE_DIRS})

# When your SKSE .dll is compiled, this will automatically copy the .dll into your mods folder.
# Only works if you configure DEPLOY_ROOT above (or set the SKYRIM_MODS_FOLDER environment variable)
if(DEFINED OUTPUT_FOLDER)
    # If you specify an <OUTPUT_FOLDER> (including via environment variables)
    # then we'll copy your mod files into Skyrim or a mod manager for you!

    # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    # Copy .pdb file (debug symbols) for all build types
    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    # Copy compiled Papyrus scripts (.pex files) from Scripts/ to output Scripts/ folder
    set(SCRIPTS_FOLDER "${OUTPUT_FOLDER}/Scripts")
    message(STATUS "Papyrus scripts output folder: ${SCRIPTS_FOLDER}")

    file(GLOB_RECURSE PAPYRUS_PEX_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/*.pex"
    )

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SCRIPTS_FOLDER}"
        VERBATIM
    )

    if(PAPYRUS_PEX_FILES)
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${PAPYRUS_PEX_FILES} "${SCRIPTS_FOLDER}"
            VERBATIM
        )
    endif()

    # Copy source Papyrus scripts (.psc files) from Source/Scripts/ to output Source/Scripts/ folder
    set(SOURCE_SCRIPTS_FOLDER "${OUTPUT_FOLDER}/Source/Scripts")
    message(STATUS "Papyrus source scripts output folder: ${SOURCE_SCRIPTS_FOLDER}")

    file(GLOB_RECURSE PAPYRUS_PSC_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Scripts/*.psc"
    )

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SOURCE_SCRIPTS_FOLDER}"
        VERBATIM
    )

    if(PAPYRUS_PSC_FILES)
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${PAPYRUS_PSC_FILES} "${SOURCE_SCRIPTS_FOLDER}"
            VERBATIM
        )
    endif()

    # Copy ESP files to output folder
    message(STATUS "ESP output folder: ${OUTPUT_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${OUTPUT_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ESP/IntuitiveDragonRideControl.esp" "${OUTPUT_FOLDER}/IntuitiveDragonRideControl.esp"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ESP/IntuitiveDragonRideControl.ini" "${DLL_FOLDER}/IntuitiveDragonRideControl.ini"
        VERBATIM
    )

    # Copy SWF files from swf/ to Interface/IntuitiveDragonRideControl/ folder
    set(INTERFACE_FOLDER "${OUTPUT_FOLDER}/Interface/IntuitiveDragonRideControl")
    message(STATUS "SWF output folder: ${INTERFACE_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${INTERFACE_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/swf/IDRC_Assets0.swf" "${INTERFACE_FOLDER}/IDRC_Assets0.swf"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/swf/IDRC_Widgets.swf" "${INTERFACE_FOLDER}/IDRC_Widgets.swf"
        VERBATIM
    )


endif()
