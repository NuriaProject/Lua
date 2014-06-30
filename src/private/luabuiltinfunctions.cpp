/* Copyright (c) 2014, The Nuria Project
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *    1. The origin of this software must not be misrepresented; you must not
 *       claim that you wrote the original software. If you use this software
 *       in a product, an acknowledgment in the product documentation would be
 *       appreciated but is not required.
 *    2. Altered source versions must be plainly marked as such, and must not be
 *       misrepresented as being the original software.
 *    3. This notice may not be removed or altered from any source
 *       distribution.
 */

#include "luabuiltinfunctions.hpp"

#include "luametaobjectwrapper.hpp"
#include "luaruntimeprivate.hpp"
#include "../luaruntime.hpp"
#include "luastructures.hpp"
#include <lua.hpp>

int Nuria::LuaBuiltinFunctions::createNuriaTable (Nuria::LuaRuntime *runtime) {
	lua_State *env = (lua_State *)runtime->luaState ();
	lua_createtable (env, 0, 3);
	int ref = luaL_ref (env, LUA_REGISTRYINDEX);
	
	// 
	insertBuiltins (runtime, ref);
	insertNuriaTableIntoEnvironment (runtime, ref);
	
	// 
	return ref;
	
}

void Nuria::LuaBuiltinFunctions::insertBuiltins (Nuria::LuaRuntime *runtime, int tableRef) {
	lua_State *env = (lua_State *)runtime->luaState ();
	
	lua_getref (env, tableRef);
	addNuriaConnectFunction (runtime);
	lua_pop (env, 1);
}

void Nuria::LuaBuiltinFunctions::insertNuriaTableIntoEnvironment (LuaRuntime *runtime, int tableRef) {
	lua_State *env = (lua_State *)runtime->luaState ();
	
	lua_getref (env, tableRef);
	lua_setglobal (env, "Nuria");
	
}

void Nuria::LuaBuiltinFunctions::addNuriaConnectFunction (Nuria::LuaRuntime *runtime) {
	insertFunction ("connect", &LuaBuiltinFunctions::implNuriaConnect, runtime);
}

void Nuria::LuaBuiltinFunctions::insertFunction (const char *name, lua_CFunction func, Nuria::LuaRuntime *runtime) {
	lua_State *env = (lua_State *)runtime->luaState ();
	
	// 
	lua_pushlightuserdata (env, runtime);
	lua_pushcclosure (env, func, 1);
	
	// 
	lua_setfield (env, -2, name);
	
}

int Nuria::LuaBuiltinFunctions::implNuriaConnect (lua_State *env) {
	return luaL_error (env, "Nuria.connect() isn't implemented yet.");
}
