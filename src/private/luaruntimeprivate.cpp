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

bool Nuria::LuaRuntimePrivate::checkQObjectOwnership (LuaRuntime *runtime, LuaWrapperUserData *data) {
	const QMetaObject *meta = QMetaType::metaObjectForType (data->meta->pointerMetaTypeId ());
	if (meta && data->ptr) {
		data->owned = (static_cast< QObject * > (data->ptr)->parent () == runtime);
	}
	
	return (meta != nullptr); // Is this a QObject?
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
	
	// Set up ownership
	if (owned && checkQObjectOwnership (this->q_ptr, delegate)) {
		static_cast< QObject * > (object)->setParent (this->q_ptr);
	}
	
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
	if ((this->handlerFlags & o) == 0) {
		return owned;
	}
	
	// 
	try {
		bool destroy = this->objectHandler (o, object, meta);
		return (owned) ? destroy : false;
	} catch (const std::bad_function_call &) {
		return owned;
	}
	
}
