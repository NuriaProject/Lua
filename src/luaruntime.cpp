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

#include "nuria/luaruntime.hpp"

#include <nuria/metaobject.hpp>
#include <nuria/callback.hpp>
#include <nuria/logger.hpp>
#include <QIODevice>
#include <QPointer>
#include <QVariant>
#include <lua.hpp>

#include "private/luacallbacktrampoline.hpp"
#include "private/luametaobjectwrapper.hpp"
#include "private/luabuiltinfunctions.hpp"
#include "private/luaruntimeprivate.hpp"
#include "private/luastackutils.hpp"
#include "private/luastructures.hpp"

Nuria::LuaRuntime::LuaRuntime (LuaLibs libraries, QObject *parent)
	: QObject (parent), d_ptr (new LuaRuntimePrivate)
{
	this->d_ptr->q_ptr = this;
	
	createLuaInstance ();
	openLuaLibraries (libraries);
	createObjectsReferenceTable ();
	
	LuaCallbackTrampoline::registerMetaTable (this->d_ptr->env);
	this->d_ptr->nuriaTableRef = LuaBuiltinFunctions::createNuriaTable (this);
	
}

Nuria::LuaRuntime::~LuaRuntime () {
	lua_close (this->d_ptr->env);
	qDeleteAll (this->d_ptr->wrappers);
	this->d_ptr->env = nullptr;
	delete this->d_ptr;
}

bool Nuria::LuaRuntime::execute (const QByteArray &script) {
	
	// Parse script. Pushes the compiled chunk onto the stack.
	int r = luaL_loadstring (this->d_ptr->env, script.constData ());
	if (r != 0) {
		setLastResultError (this->d_ptr->lastResults, luaErrorToString (r));
		return false;
	}
	
	// Call
	return pcall (0, this->d_ptr->lastResults);
}

bool Nuria::LuaRuntime::executeStream (QIODevice *device) {
	if (!device->isOpen () || !device->isReadable ()) {
		setLastResultError (this->d_ptr->lastResults, luaErrorToString (LUA_ERRFILE));
		return false;
	}
	
	return execute (device->readAll ());
}

Nuria::LuaValues Nuria::LuaRuntime::allResults () const {
	return this->d_ptr->lastResults;
}

Nuria::LuaValue Nuria::LuaRuntime::lastResult () const {
	if (this->d_ptr->lastResults.isEmpty ()) {
		return LuaValue ();
	}
	
	return this->d_ptr->lastResults.first ();
}

Nuria::LuaValue Nuria::LuaRuntime::global (const QString &name) {
	lua_getfield (this->d_ptr->env, LUA_GLOBALSINDEX, qPrintable(name));
	LuaValue value = LuaValue::fromStack (this, -1);
	lua_pop (this->d_ptr->env, 1);
	
	return value;
}

void Nuria::LuaRuntime::setGlobal (const QString &name, const QVariant &value) {
	LuaStackUtils::pushVariantOnStack (this, value);
	lua_setfield (this->d_ptr->env, LUA_GLOBALSINDEX, qPrintable(name));
}

void Nuria::LuaRuntime::setGlobal (const QString &name, const Nuria::LuaValue &value) {
	if (value.type () != LuaValue::Table) {
		LuaStackUtils::pushVariantOnStack (this, value.toVariant ());
	}
	
	lua_setfield (this->d_ptr->env, LUA_GLOBALSINDEX, qPrintable(name));
	
}

bool Nuria::LuaRuntime::hasGlobal (const QString &name) {
	lua_getfield (this->d_ptr->env, LUA_GLOBALSINDEX, qPrintable(name));
	bool exists = !lua_isnil (this->d_ptr->env, -1);
	lua_pop (this->d_ptr->env, 1);
	
	return exists;
}

static bool buildOrFindTablePath (lua_State *env, const QList< QByteArray > &path) {
	int count = path.length () - 1;
	
	// Traverse
	lua_getglobal (env, "_G");
	for (int i = 0; i < count; i++) {
		
		// Find item
		lua_pushstring (env, path.at (i).constData ());
		lua_gettable (env, -2);
		
		// Type check
		int type = lua_type (env, -1);
		if (type == LUA_TNIL) {
			
			// Insert a new table
			lua_pop (env, 1); // Throw away the 'nil' value
			lua_createtable (env, 0, 1); // The new table = -3
			lua_pushstring (env, path.at (i).constData ()); // Key = -2
			lua_pushvalue (env, -2); // Value = -1
			lua_settable (env, -4); // The parent is now at -4
			
			// Remove the parent table, which is now at -2.
			lua_remove (env, -2);
			
		} else if (type != LUA_TTABLE) {
			nWarn() << "Failed to traverse path" << path << "until position" << i
				<< "- The element at this position is neither nil nor a table!";
			return false;
		} else {
		
			// Replace the parent table with the current table on the stack
			lua_replace (env, -2);
			
		}
		
	}
	
	// Done.
	return true;
}

void Nuria::LuaRuntime::registerMetaObject (Nuria::MetaObject *metaObject, const QByteArray &prefix) {
	LuaMetaObjectWrapper *wrapper = this->d_ptr->wrappers.value (metaObject);
	if (wrapper && wrapper->isRegistered ()) {
		return;
	}
	
	// Create wrapper if it doesn't exist already
	if (!wrapper) {
		wrapper = new LuaMetaObjectWrapper (metaObject, this);
		wrapper->populateMetaTable ();
		this->d_ptr->wrappers.insert (metaObject, wrapper);
	}
	
	// Find (or build) path of tables to the class
	QByteArray fullPath = prefix + metaObject->className ();
	QList< QByteArray > path = fullPath.replace ("::", ".").split ('.');
	
	if (!buildOrFindTablePath (this->d_ptr->env, path)) {
		return;
	}
	
	// The table in which we insert the wrapper is now on the top of the stack.
	LuaObject staticObj = LuaObject::fromStructure (nullptr, metaObject, this, true);
	lua_pushstring (this->d_ptr->env, path.last ().constData ());
	lua_getref (this->d_ptr->env, staticObj.reference ());
	lua_settable (this->d_ptr->env, -3);
	
	// Get rid of the table again.
	lua_pop (this->d_ptr->env, 1);
	
}

void Nuria::LuaRuntime::collectGarbage () {
	lua_gc (this->d_ptr->env, LUA_GCCOLLECT, 0);
}

Nuria::LuaRuntime::Ownership Nuria::LuaRuntime::objectOwnership (void *object) {
	LuaWrapperUserData *data = this->d_ptr->objects.value (object);
	if (!data) {
		return OwnedByLua;
	}
	
	return (data->owned) ? OwnedByLua : OwnedByCpp;
}

void Nuria::LuaRuntime::setObjectOwnership (void *object, Ownership ownership) {
	LuaWrapperUserData *data = this->d_ptr->objects.value (object);
	if (data) {
		data->owned = (ownership == OwnedByLua);
	}
	
}

void Nuria::LuaRuntime::setObjectHandler (ObjectHandler handler, OwnershipFlags flags) {
	this->d_ptr->objectHandler = handler;
	this->d_ptr->handlerFlags = flags;
}

void *Nuria::LuaRuntime::luaState () {
	return this->d_ptr->env;
}

void Nuria::LuaRuntime::createObjectsReferenceTable () {
	
	// Object table
	lua_createtable (this->d_ptr->env, 0, 0);
	
	// Create meta-table with __mode=v
	lua_createtable (this->d_ptr->env, 0, 1);
	lua_pushliteral (this->d_ptr->env, "v");
	lua_setfield (this->d_ptr->env, -2, "__mode");
	
	// Set meta-table
	lua_setmetatable (this->d_ptr->env, -2);
	
	// Store reference
	this->d_ptr->objectsTable = luaL_ref (this->d_ptr->env, LUA_REGISTRYINDEX);
	
}

bool Nuria::LuaRuntime::pcall (int argCount, Nuria::LuaValues &results) {
	int oldTop = lua_gettop (this->d_ptr->env) - 1 - argCount;
	
	// Execute
	int r = lua_pcall (this->d_ptr->env, argCount, LUA_MULTRET, 0);
	if (r != 0) {
		const char *message = lua_tostring (this->d_ptr->env, -1);
		lua_pop (this->d_ptr->env, 1);
		
		setLastResultError (results, luaErrorToString (r) + QStringLiteral(": ") + message);
		return false;
	}
	
	// Return results
	results = LuaStackUtils::popResultsFromStack (this, oldTop);
	return true;
}

void Nuria::LuaRuntime::setLastResultError (LuaValues &values, const QString &message) {
#ifdef QT_DEBUG
	nWarn() << "Lua execution error:" << message;
#endif
	
	values.clear ();
	values.append (LuaValue (this, message));
}

QString Nuria::LuaRuntime::luaErrorToString (int error) {
	switch (error) {
	case LUA_ERRRUN: return QStringLiteral("Runtime error");
	case LUA_ERRSYNTAX: return QStringLiteral("Syntax error");
	case LUA_ERRMEM: return QStringLiteral("Memory allocation failed");
	case LUA_ERRERR: return QStringLiteral("Unknown error");
	case LUA_ERRFILE: return QStringLiteral("Bad file or stream");
	}
	
	return QString ();
}

void Nuria::LuaRuntime::createLuaInstance () {
	this->d_ptr->env = lua_open ();
}

void Nuria::LuaRuntime::openLuaLibraries (LuaLibs libraries) {
	
	// Same order as in enum LuaLib
	luaL_reg libs[] = {
		{ "", luaopen_base }, { "math", luaopen_math },
		{ "string", luaopen_string }, { "table", luaopen_table },
		{ "io", luaopen_io }, { "os", luaopen_os },
		{ "package", luaopen_package }, { "debug", luaopen_debug },
		{ "bit", luaopen_bit }, { "jit", luaopen_jit },
		{ "ffi", luaopen_ffi }
	};
	
	// 
	quint32 mask = libraries;
	for (quint32 i = 0; i < sizeof (libs) / sizeof(*libs); i++) {
		if (mask & (1 << i)) {
			lua_pushcfunction (this->d_ptr->env, libs[i].func);
			lua_pushstring (this->d_ptr->env, libs[i].name);
			lua_call (this->d_ptr->env, 1, 0);
		}
		
	}
}
