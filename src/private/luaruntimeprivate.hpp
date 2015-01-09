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

#ifndef NURIA_LUARUNTIMEPRIVATE_HPP
#define NURIA_LUARUNTIMEPRIVATE_HPP

#include "luastructures.hpp"
#include "../nuria/luaruntime.hpp"
#include "../nuria/luavalue.hpp"
#include <lua.hpp>

namespace Nuria {

class Q_DECL_HIDDEN LuaRuntimePrivate {
public:
	LuaRuntime *q_ptr;
	
	// 
	LuaMetaObjectWrapper *findWrapper (MetaObject *metaObject);
	LuaMetaObjectWrapper *findOrCreateWrapper (MetaObject *metaObject);
	
	void addObject (void *userData);
	bool hasObject (void *object);
	void *userDataOfObject (void *object);
	void removeObject (void *object);
	
	bool checkQObjectOwnership(LuaRuntime *runtime, LuaWrapperUserData *data);
	void pushOrCreateUserDataObject (void *object, MetaObject *meta,
					 int metaTable, bool owned, int ref = 0);
	void pushWrapperObject (MetaObject *meta, int metaTable);
	
	bool invokeGarbageHandler (bool owned, void *object, MetaObject *meta);	
	
	// Variables
	lua_State *env = nullptr;
	LuaValues lastResults;
	QMap< MetaObject *, LuaMetaObjectWrapper * > wrappers;
	QMap< void *, LuaWrapperUserData * > objects;
	int objectsTable;
	
	// 
	LuaRuntime::ObjectHandler objectHandler;
	LuaRuntime::OwnershipFlags handlerFlags;
	
	// 
	int nuriaTableRef;
	
	
};

} // namespace Nuria

#endif // NURIA_LUARUNTIMEPRIVATE_HPP
