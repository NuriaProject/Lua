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

#include "luaruntimeprivate.hpp"

#include "luametaobjectwrapper.hpp"

Nuria::LuaMetaObjectWrapper *Nuria::LuaRuntimePrivate::findWrapper (Nuria::MetaObject *metaObject) {
	return this->wrappers.value (metaObject);
}

Nuria::LuaMetaObjectWrapper *Nuria::LuaRuntimePrivate::findOrCreateWrapper (Nuria::MetaObject *metaObject) {
	LuaMetaObjectWrapper *wrapper = this->wrappers.value (metaObject);
	if (wrapper) {
		return wrapper;
	}
	
	// Create new one
	wrapper = new LuaMetaObjectWrapper (metaObject, this->q_ptr);
	wrapper->populateMetaTable ();
	this->wrappers.insert (metaObject, wrapper);
	return wrapper;
}

void Nuria::LuaRuntimePrivate::addObject (void *userData) {
	LuaWrapperUserData *data = reinterpret_cast< LuaWrapperUserData * > (userData);
	
	if (data->ptr) {
		this->objects.insert (data->ptr, data);
	}
	
}

bool Nuria::LuaRuntimePrivate::hasObject (void *object) {
	return this->objects.contains (object);
}

void *Nuria::LuaRuntimePrivate::userDataOfObject (void *object) {
	return this->objects.value (object);
}

void Nuria::LuaRuntimePrivate::removeObject (void *object) {
	this->objects.remove (object);
	
	// Remove 'object' out of reference table
	// TODO: LUA probably did this for us. Maybe we can optimize this.
	lua_getref (this->env, this->objectsTable);
	lua_pushlightuserdata (this->env, object);
	lua_pushnil (this->env);
	lua_settable (this->env, -3);
	
}

static Nuria::LuaWrapperUserData *newUserData (lua_State *env, void *object, int ref,
					       Nuria::MetaObject *meta, bool owned) {
	Nuria::LuaWrapperUserData *data;
	data = (Nuria::LuaWrapperUserData *)lua_newuserdata (env, sizeof(Nuria::LuaWrapperUserData));
	
	data->meta = meta;
	data->ptr = object;
	data->owned = owned;
	data->reference = ref;
	
	return data;
}

void Nuria::LuaRuntimePrivate::pushOrCreateUserDataObject (void *object, Nuria::MetaObject *meta,
						    int metaTable, bool owned, int ref) {
	LuaWrapperUserData *delegate = this->objects.value (object);
	
//	nDebug() << "Pushing" << object << "- Already known to LUA:" << (delegate ? "yes" : "no");
	
	// Push objects table and 'object' onto stack
	lua_getref (this->env, this->objectsTable);
	lua_pushlightuserdata (this->env, object);
	
	// Is this object already known?
	if (delegate) {
		
		// Get reference out of the objects table in LUA
		lua_gettable (this->env, -2);
		lua_replace (this->env, -2);
		return;
	}
	
	// Create new delegate structure
	delegate = newUserData (this->env, object, ref, meta, owned);
	this->objects.insert (object, delegate);
	
	// Copy 'delegate' before the table in the stack, we leave it there as return value
	lua_pushvalue (this->env, -1);
	lua_insert (this->env, lua_gettop (this->env) - 3);
	
	// Tell LUA about it. -3 = the table, -2 = lightuserdata -> 'object', -1 = 'delegate'
	lua_settable (this->env, -3);
	
	// Pop the table from the stack and leave the copy of 'delegate' there.
	lua_pop (this->env, 1);
	
	// Push wrapper onto stack and make it the userdatas metatable
	lua_getref (this->env, metaTable);
	lua_setmetatable (this->env, -2);
}

void Nuria::LuaRuntimePrivate::pushWrapperObject (Nuria::MetaObject *meta, int metaTable) {
//	nDebug() << "Pushing wrapper for" << meta->className ();
	
	// Push wrapper
	newUserData (this->env, nullptr, 0, meta, true);
	
	// Set meta-table
	lua_getref (this->env, metaTable);
	lua_setmetatable (this->env, -2);
	
}

bool Nuria::LuaRuntimePrivate::invokeGarbageHandler (bool owned, void *object, MetaObject *meta) {
	LuaRuntime::Ownership o = (owned) ? LuaRuntime::OwnedByLua : LuaRuntime::OwnedByCpp;
	
	try {
		bool destroy = this->objectHandler (o, object, meta);
		return (owned) ? destroy : false;
	} catch (const std::bad_function_call &) {
		return owned;
	}
	
}
