/* Copyright (c) 2014-2015, The Nuria Project
 * The NuriaProject Framework is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * The NuriaProject Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with The NuriaProject Framework.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "luabuiltinfunctions.hpp"

#include "luametaobjectwrapper.hpp"
#include "luaruntimeprivate.hpp"
#include "../nuria/luaruntime.hpp"
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
