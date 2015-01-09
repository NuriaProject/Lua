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

#ifndef NURIA_LUACALLBACKTRAMPOLINE_HPP
#define NURIA_LUACALLBACKTRAMPOLINE_HPP

#include "../nuria/luavalue.hpp"
#include <QVariant>
#include <lua.hpp>

namespace Nuria {

class LuaRuntime;
class LuaObject;
class Callback;

class Q_DECL_HIDDEN LuaCallbackTrampoline {
public:
	
	static QVariant trampolineInvoke (LuaRuntime *runtime, int ref, const QVariantList &arguments);
	
	static void registerMetaTable (lua_State *env);
	static int destroyCallbackUserData (lua_State *env);
	static QVariant objectToVariant (const LuaObject &object, int targetType);
	static QVariantList valuesToList (const LuaValues &values, const QList< int > &types);
	static int invokeCallback (lua_State *env);
	
	static QVariant functionFromStack (LuaRuntime *runtime, int idx);
	static void pushCallbackOnStack (LuaRuntime *runtime, const Callback &callback);
	
};

}

#endif // NURIA_LUACALLBACKTRAMPOLINE_HPP
