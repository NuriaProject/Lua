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
