# Set CMake minimum version and CMake policy required by UrhoCommon module
cmake_minimum_required (VERSION 3.10.2)
if (COMMAND cmake_policy)
    # Libraries linked via full path no longer produce linker search paths
    cmake_policy (SET CMP0003 NEW)
    # INTERFACE_LINK_LIBRARIES defines the link interface
    cmake_policy (SET CMP0022 NEW)
    # Disallow use of the LOCATION target property - so we set to OLD as we still need it
    cmake_policy (SET CMP0026 OLD)
    # MACOSX_RPATH is enabled by default
    cmake_policy (SET CMP0042 NEW)
    # Honor the visibility properties for SHARED target types only
    cmake_policy (SET CMP0063 OLD)
endif ()

# Set project name
project (Archive)

# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

# Include UrhoCommon.cmake module after setting project name
include (UrhoCommon)

# Define target name
set (TARGET_NAME archive-test)

# Turn on C++14 (Learned this from UrhoX) 
set(CMAKE_CXX_STANDARD 17)

include_directories(
#    Urhox
#    Urhox/ThirdParty
    Archival
)

# Turn on C++14
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -DURHO3D_FMT" )

# Add UrhoX
# Your-external "UrhoX", add GLOBAL if the imported library is located in directories above the current.
#add_library( UrhoX STATIC IMPORTED )
# You can define two import-locations: one for debug and one for release.
#set_target_properties( UrhoX PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libUrhox.a )

# Define source files
define_source_files (GLOB_CPP_PATTERNS *.cpp virtualGizmo3D/*.cpp *.h virtualGizmo3D/*.h implot/*.cpp implot/*.h *.inl Archival/*cpp Archival/*.h Archival/*/*.h Archival/*/*.cpp)

# disable grouping (maybe) NEL
#set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCE_FILES})

# Setup target with resource copying
setup_main_executable ()

#TARGET_LINK_LIBRARIES(${TARGET_NAME} UrhoX)

# Setup test cases
if (URHO3D_ANGELSCRIPT)
    setup_test (NAME ExternalLibAS OPTIONS Scripts/12_PhysicsStressTest.as -w)
endif ()
if (URHO3D_LUA)
    setup_test (NAME ExternalLibLua OPTIONS LuaScripts/12_PhysicsStressTest.lua -w)
endif ()

# Urho3D documentation
# add_subdirectory (Docs)
