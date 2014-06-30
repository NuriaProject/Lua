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

#ifndef NURIA_LUABUILTINFUNCTIONS_HPP
#define NURIA_LUABUILTINFUNCTIONS_HPP

#include <QByteArray>
#include <lua.hpp>

namespace Nuria {

class LuaWrapperUserData;
class LuaRuntime;

/* internal class exposing the Nuria.* methods to Lua */
class Q_DECL_HIDDEN LuaBuiltinFunctions {
public:
	
	static int createNuriaTable (LuaRuntime *runtime);
	static void insertBuiltins (LuaRuntime *runtime, int tableRef);
	static void insertNuriaTableIntoEnvironment (LuaRuntime *runtime, int tableRef);
	static void addNuriaConnectFunction (LuaRuntime *runtime);
	static void insertFunction (const char *name, lua_CFunction func, LuaRuntime *runtime);
	
private:
	
	static int implNuriaConnect (lua_State *env);
	
};

}

#endif // NURIA_LUABUILTINFUNCTIONS_HPP
