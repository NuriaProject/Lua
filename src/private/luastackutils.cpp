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

#include "luastackutils.hpp"

#include "luacallbacktrampoline.hpp"
#include "../nuria/luaruntime.hpp"
#include "luaruntimeprivate.hpp"
#include <nuria/callback.hpp>

Nuria::LuaValues Nuria::LuaStackUtils::popResultsFromStack (LuaRuntime *runtime, int oldTop) {
	
	// Count of results
	int newTop = lua_gettop ((lua_State *)runtime->luaState ());
	int resultCount = newTop - oldTop;
	return popFromStackMulti (runtime, resultCount);
	
}

Nuria::LuaValues Nuria::LuaStackUtils::popFromStackMulti (LuaRuntime *runtime, int count) {
	if (count == 0) {
		return { };
	} else if (count == 1) {
		return { LuaValue::fromStack (runtime, -1) };
	}
	
	return popFromStackMultiInternal (runtime, count);
}

Nuria::LuaValues Nuria::LuaStackUtils::popFromStackMultiInternal (LuaRuntime *runtime, int count) {
	LuaValues list = readValuesFromStack (runtime, count);
	
	// Pop stack
	lua_pop ((lua_State *)runtime->luaState (), count);
	return list;
}

Nuria::LuaValues Nuria::LuaStackUtils::readValuesFromStack (LuaRuntime *runtime, int count) {
	LuaValues list;
	list.reserve (count);
	
	// Put the last 'count' stack values in reverse order into list.
	for (int i = count - 1; i >= 0; i--) {
		list.append (LuaValue::fromStack (runtime, -i-1));
	}
	
	return list;
}

void Nuria::LuaStackUtils::pushManyVariantsOnStack (LuaRuntime *runtime, const QVariantList &list) {
	for (const QVariant &cur : list) {
		pushVariantOnStack (runtime, cur);
	}
	
}

void Nuria::LuaStackUtils::pushVariantOnStack (LuaRuntime *runtime, const QVariant &variant) {
	lua_State *env = (lua_State *)runtime->luaState ();
	int type = variant.userType ();
	
	switch (type) {
	default:
		if (type == qMetaTypeId< Callback > ()) {
			LuaCallbackTrampoline::pushCallbackOnStack (runtime, variant.value< Callback > ());
			return;
		} else if (type == qMetaTypeId< LuaObject > ()) {
			variant.value< LuaObject > ().pushOnStack ();
		}
		
		// 
		pushCObjectOnStack (runtime, variant);
		break;
		
	case QMetaType::Int:
	case QMetaType::Float:
	case QMetaType::Double:
		lua_pushnumber (env, variant.toDouble ());
		break;
		
	case QMetaType::Bool:
		lua_pushboolean (env, variant.toBool ());
		break;
		
	case QMetaType::QString:
		lua_pushstring (env, variant.toString ().toUtf8 ().constData ());
		break;
		
	case QMetaType::QByteArray: {
		QByteArray data = variant.toByteArray ();
		lua_pushlstring (env, data.constData (), data.length ());
	} break;
		
	case QMetaType::QVariantList:
		pushVariantListOnStack (runtime, variant.toList ());
		break;
		
	case QMetaType::QVariantMap:
		pushVariantMapOnStack (runtime, variant.toMap ());
		break;
		
	}
	
}

void Nuria::LuaStackUtils::pushVariantMapOnStack (LuaRuntime *runtime, const QVariantMap &map) {
	lua_State *env = (lua_State *)runtime->luaState ();
	
	// Create LUA table with space for 'map'
	lua_createtable (env, 0, map.count ());
	
	auto it = map.constBegin ();
	auto end = map.constEnd ();
	for (; it != end; ++it) {
		
		// Push key
		QByteArray key = it.key ().toUtf8 ();
		lua_pushstring (env, key.constData ()); // Key at -2
		
		// Push value and create table element
		pushVariantOnStack (runtime, it.value ()); // Value at -1
		lua_rawset (env, -3); // The table is now at -3
	}
	
}

void Nuria::LuaStackUtils::pushVariantListOnStack (LuaRuntime *runtime, const QVariantList &list) {
	lua_State *env = (lua_State *)runtime->luaState ();
	
	// Create LUA table with space for 'list'
	lua_createtable (env, list.length (), 0);
	for (int i = 0; i < list.length (); i++) {
		pushVariantOnStack (runtime, list.at (i)); // Value at -1
		lua_rawseti (env, -2, i + 1); // The table is now at -2
	}
	
}

void Nuria::LuaStackUtils::pushCObjectOnStack (LuaRuntime *runtime, const QVariant &variant) {
	LuaObject obj = LuaValue (runtime, variant).object ();
	
	if (obj.isValid ()) {
		obj.pushOnStack ();
	} else {
		lua_pushnil ((lua_State *)runtime->luaState ());
	}
	
}

QVariant Nuria::LuaStackUtils::luaValuesToVariant (const Nuria::LuaValues &values) {
	
	if (values.isEmpty ()) {
		return QVariant ();
	}
	
	// Single result?
	if (values.length () == 1) {
		return values.first ().toVariant ();
	}
	
	// Many results
	QVariantList list;
	for (const LuaValue &cur : values) {
		list.append (cur.toVariant ());
	}
	
	return list;
}

#include <nuria/debug.hpp>
QVariant Nuria::LuaStackUtils::variantFromStack (LuaRuntime *runtime, int idx, bool takeOwnership) {
	lua_State *env = (lua_State *)runtime->luaState ();
	int type = lua_type (env, idx);
	
	switch (type) {
	case LUA_TNIL: return QVariant ();
	case LUA_TNUMBER: return lua_tonumber (env, idx);
	case LUA_TBOOLEAN: return bool (lua_toboolean (env, idx));
	case LUA_TSTRING: {
		size_t len = 0;
		const char *str = lua_tolstring (env, idx, &len);
		return (str) ? QString::fromUtf8 (str, len) : QVariant ();
	}
	case LUA_TTABLE: return tableFromStack (runtime, idx, takeOwnership);
	case LUA_TFUNCTION: return LuaCallbackTrampoline::functionFromStack (runtime, idx);
	case LUA_TUSERDATA: {
		void *ptr = const_cast< void * > (lua_topointer (env, idx));
		LuaWrapperUserData *data = (LuaWrapperUserData *)ptr;
		if (takeOwnership) { data->owned = false; }
		return QVariant (data->meta->pointerMetaTypeId (), &data->ptr);
	}
		// TODO: Can we make something useful out of a Lua thread?
	case LUA_TTHREAD: return QVariant ();
	case LUA_TLIGHTUSERDATA:
		return QVariant::fromValue (const_cast< void * > (lua_topointer (env, idx)));
	}
	
	return QVariant ();
}

QVariant Nuria::LuaStackUtils::tableFromStack (LuaRuntime *runtime, int idx, bool takeOwnership) {
	QVariantList list;
	QVariantMap map;
	
	traverseTable (runtime, map, list, idx, takeOwnership);
	
	// Is it a map?
	if (!map.isEmpty ()) {
		mergeTable (map, list);
		return map;
	}
	
	// It's a list
	return list;
}

void Nuria::LuaStackUtils::traverseTable (LuaRuntime *runtime, QVariantMap &map, QVariantList &list, int idx,
                                          bool takeOwnership) {
	lua_State *env = (lua_State *)runtime->luaState ();
	double num = 0.f;
	
	// 
	lua_pushvalue (env, idx); // -2 = the table
	lua_pushnil (env); // -1 = first key
	while (lua_next (env, -2) != 0) {
		
		// -1 = Value
		// -2 = Key
		// -3 = Table
		// Type of the key. If it's the next index, append it to 'list'.
		if (lua_isnumber (env, -2) && (num = lua_tonumber (env, -2)) == list.length () + 1) {
			list.append (variantFromStack (runtime, -1, takeOwnership));
		} else {
			QString key = variantFromStack (runtime, -2, takeOwnership).toString ();
			map.insert (key, variantFromStack (runtime, -1, takeOwnership));
		}
		
		// Remove the value from stack
		lua_pop (env, 1);
	}
	
	// 
	lua_pop (env, 1);
	
}

void Nuria::LuaStackUtils::mergeTable (QVariantMap &map, const QVariantList &list) {
	auto it = map.constBegin ();
	
	for (int i = 0; i < list.length (); i++) {
		it = map.insert (it, QString::number (i), list.at (i));
	}
	
}
