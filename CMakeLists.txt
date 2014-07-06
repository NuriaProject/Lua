# CMake file for NuriaLua
# Source: https://github.com/NuriaProject/Lua
cmake_minimum_required(VERSION 2.8.8)

# Base configuration
PROJECT(NuriaLua)

# Enable moc
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Dependencies
FIND_PACKAGE(Qt5Core REQUIRED)

# Build LuaJit
add_custom_target(build_luajit ALL
                  COMMAND ${CMAKE_MAKE_PROGRAM} TARGET_CFLAGS=-fPIC BUILDMODE=static
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src
                  COMMENT "LuaJit library")

# 
SET(LUA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src)
include_directories(${LUA_INCLUDE_DIR})

SET(LUAJIT_LIB ${LUA_INCLUDE_DIR}/libluajit.a)


# 
SET(NuriaLua_SRC
    src/lua_global.hpp
    src/luaobject.cpp
    src/luaobject.hpp
    src/luaruntime.cpp
    src/luaruntime.hpp
    src/luavalue.cpp
    src/luavalue.hpp
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

# C++11
LIST(APPEND CMAKE_CXX_FLAGS "-std=c++11")
add_definitions(-DNURIA_LUA_LIBRARY)

# Create build target
ADD_LIBRARY(LuaJit IMPORTED STATIC)
ADD_LIBRARY(NuriaLua SHARED ${NuriaLua_SRC})
set_property(TARGET LuaJit APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(LuaJit PROPERTIES IMPORTED_LOCATION_NOCONFIG ${LUAJIT_LIB})
target_link_libraries(NuriaLua NuriaCore)
target_link_libraries(NuriaLua LuaJit)
add_dependencies(LuaJit build_luajit)
QT5_USE_MODULES(NuriaLua Core)

# 
set_target_properties(
    NuriaLua PROPERTIES
    VERSION ${NURIA_VERSION}
    SOVERSION ${NURIA_SOVERSION}
)

# Install target
INSTALL(TARGETS NuriaLua 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION lib
)

INSTALL(DIRECTORY src/ DESTINATION include/nuria
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "private" EXCLUDE
)
