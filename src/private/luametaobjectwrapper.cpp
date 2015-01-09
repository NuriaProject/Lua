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

#include "luametaobjectwrapper.hpp"

#include "../nuria/luaruntime.hpp"
#include "luaruntimeprivate.hpp"
#include <nuria/metaobject.hpp>
#include <nuria/serializer.hpp>
#include "luastackutils.hpp"
#include "luastructures.hpp"
#include <nuria/debug.hpp>
#include <QVariant>
#include <lua.hpp>
#include <QSet>

namespace Nuria {
class LuaMetaObjectWrapperPrivate {
public:
	
	MetaObject *metaObject;
	LuaRuntime *runtime;
	
	// Reference to the metatable
	int metaRef = 0;
	bool registered = false;
	
};

}

Nuria::LuaMetaObjectWrapper::LuaMetaObjectWrapper (MetaObject *metaObject, LuaRuntime *runtime)
	: QObject (runtime), d_ptr (new LuaMetaObjectWrapperPrivate)
{
	
	this->d_ptr->metaObject = metaObject;
	this->d_ptr->runtime = runtime;
	
}

Nuria::LuaMetaObjectWrapper::~LuaMetaObjectWrapper () {
	
	
	delete this->d_ptr;
}

Nuria::MetaObject *Nuria::LuaMetaObjectWrapper::nuriaMetaObject () const {
	return this->d_ptr->metaObject;
}

Nuria::LuaRuntime *Nuria::LuaMetaObjectWrapper::runtime () const {
	return this->d_ptr->runtime;
}

namespace Nuria {
namespace Internal {
class Delegate {
public:
	
	// Delegate for __index (Read access)
	static int delegateRead (lua_State *env) {
		Nuria::LuaRuntime *runtime = (Nuria::LuaRuntime *)lua_touserdata (env, lua_upvalueindex(1));
		void *inst = lua_touserdata (env, -2);
		const char *field = luaL_checkstring (env, -1);
		
		LuaMetaObjectWrapper::pushFieldOnStack (runtime, inst, field);
		return 1;
	}
	
	// Delegate for __newindex (Write access)
	static int delegateWrite (lua_State *env) {
		Nuria::LuaRuntime *runtime = (Nuria::LuaRuntime *)lua_touserdata (env, lua_upvalueindex(1));
		void *inst = lua_touserdata (env, -3);
		const char *field = luaL_checkstring (env, -2);
		// -1 = The new value
		
		LuaMetaObjectWrapper::setFieldFromStack (runtime, inst, field);
		return 0;
	}
	
	static void destroyRecursive (lua_State *env, LuaRuntime *runtime, LuaWrapperUserData *data) {
		runtime->d_ptr->removeObject (data);
		runtime->d_ptr->checkQObjectOwnership (runtime, data);
		
		if (data->ptr && runtime->d_ptr->invokeGarbageHandler (data->owned, data->ptr, data->meta)) {
			if (data->reference > 0) { // Is this a sub-class?
				LuaWrapperUserData *parent = (LuaWrapperUserData *)data->ptr;
				destroyRecursive (env, runtime, parent);
				delete parent;
			} else {
				data->meta->destroyInstance (data->ptr);
			}
			
		}
		
		if (data->reference > 0) {
			lua_unref (env, data->reference);
		}
		
	}
	
	// Delegate for __gc (Garbage collected)
	static int delegateDestroy (lua_State *env) {
		LuaRuntime *runtime = (LuaRuntime *)lua_touserdata (env, lua_upvalueindex(1));
		void *inst = lua_touserdata (env, -1);
		LuaWrapperUserData *data = (LuaWrapperUserData *)inst;
		
		// 
		destroyRecursive (env, runtime, data);
		return 0;
	}
	
	// Delegate for method invocations
	static int methodDelegate (lua_State *env) {
		return LuaMetaObjectWrapper::invokeMethod (env);
		
	}
	
	static int delegateDeclarativeCreation (lua_State *env) {
		LuaRuntime *runtime = (LuaRuntime *)lua_touserdata (env, lua_upvalueindex(1));
		void *inst = lua_touserdata (env, 1);
		LuaWrapperUserData *data = (LuaWrapperUserData *)inst;
		
		return LuaMetaObjectWrapper::declarativeCreate (runtime, env, data->meta);
	}
	
};
}
}
	
static void pushClosureIntoTable (lua_State *env, const char *literalName, lua_CFunction func,
				  Nuria::LuaRuntime *runtime) {
	
	// Push key
	lua_pushstring (env, literalName);
	
	// Push value (Closure)
	lua_pushlightuserdata (env, runtime);
	lua_pushcclosure (env, func, 1);

	// Insert
	lua_settable (env, -3);
}

void Nuria::LuaMetaObjectWrapper::populateMetaTable () {
	lua_State *env = (lua_State *)this->d_ptr->runtime->luaState ();
	MetaObject *metaObject = this->d_ptr->metaObject;
	LuaRuntime *runtime = this->d_ptr->runtime;
	
	// Create table
	lua_createtable (env, 0, 3);
	
	// Insert the MetaObject
	lua_pushliteral (env, "_nuria_metaobject");
	lua_pushlightuserdata (env, metaObject);
	lua_settable (env, -3);
	
	// Insert the meta methods
	pushClosureIntoTable (env, "__index", &Internal::Delegate::delegateRead, runtime);
	pushClosureIntoTable (env, "__newindex", &Internal::Delegate::delegateWrite, runtime);
	pushClosureIntoTable (env, "__gc", &Internal::Delegate::delegateDestroy, runtime);
	pushClosureIntoTable (env, "__call", &Internal::Delegate::delegateDeclarativeCreation, runtime);
	
	// Store reference to the meta table
	this->d_ptr->metaRef = luaL_ref (env, LUA_REGISTRYINDEX);
	
}

int Nuria::LuaMetaObjectWrapper::reference () const {
	return this->d_ptr->metaRef;
}

bool Nuria::LuaMetaObjectWrapper::isRegistered () const {
	return this->d_ptr->registered;
}

void Nuria::LuaMetaObjectWrapper::setRegistered (bool registered) {
	this->d_ptr->registered = registered;
}

void Nuria::LuaMetaObjectWrapper::pushFieldOnStack (LuaRuntime *runtime, void *inst, const char *name) {
	QByteArray n = QByteArray::fromRawData (name, qstrlen (name));
	if (!pushMethod (runtime, inst, n) && !pushField (runtime, inst, n)) {
		lua_pushnil ((lua_State *)runtime->luaState ());
	}
	
}

void Nuria::LuaMetaObjectWrapper::setFieldFromStack (LuaRuntime *runtime, void *inst, const char *name) {
	LuaWrapperUserData *data = (LuaWrapperUserData *)inst;
	
	// 
	QByteArray n = QByteArray::fromRawData (name, qstrlen (name));
	MetaField field = data->meta->fieldByName (n);
	
	if (field.isValid ()) {
		QVariant value = LuaValue::fromStack (runtime, -1).toVariant ();
		field.write (data->ptr, value);
	}
	
}

bool Nuria::LuaMetaObjectWrapper::pushMethod (LuaRuntime *runtime, void *inst, const QByteArray &name) {
	lua_State *env = (lua_State *)runtime->luaState ();
	LuaWrapperUserData *data = (LuaWrapperUserData *)inst;
	
	// Special handling for constructors
	if (name == "new") {
		return pushMethod (runtime, inst, QByteArray ());
	}
	
	// Find method
	int begin = data->meta->methodLowerBound (name);
	int end = data->meta->methodUpperBound (name);
	
	if (begin < 0) {
		return false;
	}
	
	// Push upvalues
	lua_pushlightuserdata (env, runtime);
	lua_pushvalue (env, -3); // The userdata
	lua_pushinteger (env, begin); // Range
	lua_pushinteger (env, end);
	
	// Return c closure
	lua_pushcclosure (env, &Internal::Delegate::methodDelegate, 4);
	return true;
}

bool Nuria::LuaMetaObjectWrapper::pushField (LuaRuntime *runtime, void *inst, const QByteArray &name) {
	LuaWrapperUserData *data = (LuaWrapperUserData *)inst;
	
	// Find field
	MetaField field = data->meta->fieldByName (name);
	if (!field.isValid ()) {
		return false;
	}
	
	// Push value
	void *ptr = (data->ptr) ? data->ptr : data;
	LuaStackUtils::pushVariantOnStack (runtime, field.read (ptr));
	return true;
}

static bool isStaticCall (lua_State *env, Nuria::LuaWrapperUserData *data) {
	if (lua_gettop (env) < 1) {
		return true;
	}
	
	// 
	Nuria::LuaWrapperUserData *ptr = (Nuria::LuaWrapperUserData *)lua_touserdata (env, 1);
	if (!ptr || ptr == data) {
		return (ptr != data);
	}
	
	// 
	return (ptr->ptr != data->ptr);
	
}

int Nuria::LuaMetaObjectWrapper::invokeMethod (void *state) {
	lua_State *env = (lua_State *)state;
	Nuria::LuaRuntime *runtime = (Nuria::LuaRuntime *)lua_touserdata(env, lua_upvalueindex(1));
	Nuria::LuaWrapperUserData *data = (Nuria::LuaWrapperUserData *)lua_touserdata(env, lua_upvalueindex(2));
	int begin = lua_tointeger (env, lua_upvalueindex(3));
	int end = lua_tointeger (env, lua_upvalueindex(4));
	
	// 
	int count = lua_gettop (env);
	bool isStatic = isStaticCall (env, data);
	bool forceStatic = !data;
	
	// If there's no instance for this, then only static calls are allowed.
	if (forceStatic && !isStatic) {
		lua_pushliteral (env, "You can only call static methods on this instance!");
		return lua_error (env);
	}
	
	// Find method
	int idx = chooseMethod (runtime, data->meta, count, begin, end, isStatic);
	if (idx < 0) {
		lua_pushfstring (env, "No method '%s' with %i arguments found.",
				 data->meta->method (begin).name ().constData (), count);
		return lua_error (env);
	}
	
	// Use the first one available.
	// FIXME: Choose the best one, not the first one
	MetaMethod method = data->meta->method (idx);
	LuaValues args = LuaStackUtils::readValuesFromStack (runtime, count);
	QVariantList arguments;
	
	// Skip first argument if it's a member method call
	for (int i = !isStatic; i < args.length (); i++) {
		arguments.append (args.at (i).toVariant ());
	}
	
	// Invoke callback
	Nuria::Callback cb = method.callback (data->ptr);
	QVariant result = cb.invoke (arguments);
	
	// Return result
	pushInvocationResult (runtime, data->meta, idx, result);
	return 1;
	
}

int Nuria::LuaMetaObjectWrapper::declarativeCreate (LuaRuntime *runtime, lua_State *env, MetaObject *meta) {
	// Lua calls this function as func(self, argument), where argument has
	// to be a table in our case.
	if (!lua_istable(env, 2)) {
		return luaL_error (env, "Declarative creation expects a table as only argument.");
	}
	
	// Convert table, taking ownership of all objects inside.
	QVariant argData = LuaStackUtils::tableFromStack (runtime, 2, true);
	if (argData.userType () == QMetaType::QVariantList) {
		return luaL_error (env, "The passed table must be associative data, not an array.");
	}
	
	// Use Nuria::Serializer
	Serializer serializer;
	serializer.setRecursionDepth (Serializer::InfiniteRecursion);
	QVariantMap map = argData.toMap ();
	void *inst = serializer.deserialize (map, meta);
	
	// Sanity check
	if (!inst) {
		return luaL_error (env, "Failed to construct instance of type %s",
		                   meta->className ().constData ());
	}
	
	// Push the instance on to the stack while transferring ownership to the environment.
	LuaObject::fromStructure (inst, meta, runtime, true).pushOnStack ();
	return 1;
}

void Nuria::LuaMetaObjectWrapper::pushInvocationResult (Nuria::LuaRuntime *runtime, Nuria::MetaObject *meta,
							int funcIdx, QVariant &result) {
	if (meta->method (funcIdx).type () != MetaMethod::Constructor) {
		LuaStackUtils::pushVariantOnStack (runtime, result);
		return;
	}
	
	// That was a constructor. Take ownership of the data!
	void *ptr = Variant::stealPointer (result);
	LuaObject::fromStructure (ptr, meta, runtime, true).pushOnStack ();
	
}

bool Nuria::LuaMetaObjectWrapper::findPossibleMethodsByArgCount (Nuria::MetaObject *meta, int count, int &begin, int &end) {
	for (; begin < end && meta->method (begin).argumentTypes ().length () < count; begin++);
	for (; end > begin && meta->method (end).argumentTypes ().length () > count; end--);
	
	if (begin == end && meta->method (begin).argumentTypes ().length () != count) {
		return false;
	}
	
	return true;
}

int Nuria::LuaMetaObjectWrapper::chooseMethod (Nuria::LuaRuntime *runtime, Nuria::MetaObject *meta, int count,
					       int begin, int end, bool staticCall) {
	Q_UNUSED(runtime)
	
	// Limit range to those methods which take 'count' arguments
	if (!findPossibleMethodsByArgCount (meta, count - !staticCall, begin, end)) {
		return -1;
	}
	
	// Find method which is (not) a member method as indicated by 'staticCall'
	int idx = findMethodWhere (meta, begin, end, !staticCall);
	return idx;
}

int Nuria::LuaMetaObjectWrapper::findMethodWhere (Nuria::MetaObject *meta, int begin, int end, bool isMember) {
	for (int idx = begin; idx <= end; idx++) {
		MetaMethod::Type type = meta->method (idx).type ();
		
		if ((isMember && type == MetaMethod::Method) ||
		    (!isMember && type != MetaMethod::Method)) {
			return idx;
		}
		
		
	}
	
	return -1;
}
