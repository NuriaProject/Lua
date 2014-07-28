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
