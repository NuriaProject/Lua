# CMake file for NuriaLua
# Source: https://github.com/NuriaProject/Lua
cmake_minimum_required(VERSION 2.8.8)

# Base configuration
PROJECT(NuriaLua)

cmake_policy(SET CMP0020 NEW)

# Enable moc
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Dependencies
FIND_PACKAGE(Qt5Core REQUIRED)
if (NOT TARGET NuriaCore)
  FIND_PACKAGE(NuriaCore REQUIRED)
endif()

# 
SET(NuriaLua_SRC
    src/nuria/lua_global.hpp
    src/luaobject.cpp
    src/nuria/luaobject.hpp
    src/luaruntime.cpp
    src/nuria/luaruntime.hpp
    src/luavalue.cpp
    src/nuria/luavalue.hpp
    src/private/luabuiltinfunctions.cpp
    src/private/luabuiltinfunctions.hpp
    src/private/luacallbacktrampoline.cpp
    src/private/luacallbacktrampoline.hpp
    src/private/luametaobjectwrapper.cpp
    src/private/luametaobjectwrapper.hpp
    src/private/luaruntimeprivate.cpp
    src/private/luaruntimeprivate.hpp
    src/private/luastackutils.cpp
    src/private/luastackutils.hpp
    src/private/luastructures.hpp
)

# Create build target
ADD_LIBRARY(NuriaLua SHARED ${NuriaLua_SRC})
target_include_directories(NuriaLua PRIVATE ${LUAJIT_INCLUDE_DIR})
target_link_libraries(NuriaLua NuriaCore LuaJit)
QT5_USE_MODULES(NuriaLua Core)

if(UNIX)
  target_link_libraries(NuriaLua -ldl)
endif()

# 
set_target_properties(
    NuriaLua PROPERTIES
    VERSION ${NURIA_VERSION}
    SOVERSION ${NURIA_SOVERSION}
)

# Add public include directories to target
target_include_directories(NuriaLua PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
)

# Install target
INSTALL(TARGETS NuriaLua EXPORT NuriaLuaConfig
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

INSTALL(DIRECTORY src/nuria/ DESTINATION include/nuria
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "private" EXCLUDE
)

INSTALL(EXPORT NuriaLuaConfig DESTINATION lib/cmake/NuriaLua)

export(TARGETS NuriaLua FILE "${NURIA_CMAKE_PREFIX}/NuriaLuaConfig.cmake")

# Add Tests
enable_testing()
add_unittest(NAME tst_luaruntime NURIA NuriaLua SOURCES structures.hpp)
