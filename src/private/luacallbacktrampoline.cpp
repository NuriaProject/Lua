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

#include "luacallbacktrampoline.hpp"

#include "luastackutils.hpp"
#include "../luaruntime.hpp"
#include "../luavalue.hpp"
#include <debug.hpp>
#include <QPointer>

namespace Nuria {

// Helper class. This acts like a guard for LUA references. The given LUA
// reference is destroyed when this shared class is destroyed. We use this
// when creating a Nuria::Callback out of a LUA function. We can't directly
// reference a LUA function, so we store a reference. The lambda will be
// eventually destroyed (When the Callback gets destroyed), which will make
// the reference count go to zero and destroy the LUA reference for us.
class Q_DECL_HIDDEN Trampoline {
public:
	
	Trampoline () : d (new Data) {}
	
	struct Data : public QSharedData {
		int luaRef;
		QPointer< LuaRuntime > runtime;
		
		~Data () {
			if (!runtime.isNull ()) {
				lua_State *state = (lua_State *)runtime->luaState ();
				
				if (state) {
					luaL_unref (state, LUA_REGISTRYINDEX, luaRef);
				}
				
			}
			
		}
		
	};
	
	QExplicitlySharedDataPointer< Data > d;
};

// Helper class to store a Nuria::Callback inside a LUA function as C Closure.
class Q_DECL_HIDDEN Caller {
public:
	
	static void registerMetaTable (lua_State *env) {
		luaL_newmetatable (env, "_Nuria_CallbackDeallocator");
		lua_pushstring (env, "__gc");
		lua_pushcclosure (env, &destroyCallbackUserData, 0);
		lua_settable (env, -3);
	}
	
	static int destroyCallbackUserData (lua_State *env) {
		Nuria::Callback *callback = (Nuria::Callback *)lua_touserdata (env, 1);
		callback->~Callback ();
		return 0;
	}
	
	// TODO: Isn't this logic somewhere else already used? Can it be merged?
	static QVariant objectToVariant (LuaObject object, int targetType) {
		const char *name = QMetaType::typeName (targetType);
		if (name && name[qstrlen (name) - 1] == '*') {
			return object.toVariant ();
		}
		
		return object.copy ();
		
	}
	
	static QVariantList valuesToList (const LuaValues &values, const QList< int > &types) {
		QVariantList arguments;
		
		for (int i = 0; i < values.length (); i++) {
			const LuaValue &cur = values.at (i);
			if (cur.object ().isValid ()) {
				int type = (i < types.length ()) ? types.at (i) : 0;
				arguments.append (objectToVariant (cur.object (), type));
			} else {
				arguments.append (cur.toVariant ());
			}
			
		}
		
		return arguments;
	}
	
	static int invokeCallback (lua_State *env) {
		Nuria::Callback &callback = *(Nuria::Callback *)lua_touserdata(env, lua_upvalueindex(1));
		Nuria::LuaRuntime *runtime = (Nuria::LuaRuntime *)lua_touserdata(env, lua_upvalueindex(2));
		
		// Arguments LUA -> C++
		// Sanity check
		int count = lua_gettop (env);
		if (!callback.isVariadic () && count != callback.argumentTypes ().length ()) {
			lua_pushfstring (env, "Failed to invoke function, expected %i arguments, but got %i.",
					 count, callback.argumentTypes ().length ());
			return lua_error (env);
		}
		
		// Argument types
		QList< int > types;
		if (!callback.isVariadic ()) {
			types = callback.argumentTypes ();
		}
		
		// Read arguments
		LuaValues args = LuaStackUtils::readValuesFromStack (runtime, count);
		QVariantList arguments = valuesToList (args, types);
		
		// Invoke ...
		QVariant result = callback.invoke (arguments);
		
		// Push result if there is one
		if (!result.isValid ()) {
			return 0;
		}
		
		// 
		LuaStackUtils::pushVariantOnStack (runtime, result);
		return 1;
	}

};

}

QVariant Nuria::LuaCallbackTrampoline::trampolineInvoke (LuaRuntime *runtime, int ref, const QVariantList &arguments) {
	lua_State *env = (lua_State *)runtime->luaState ();
	int oldTop = lua_gettop (env);
	
	// Push function onto stack
	lua_rawgeti (env, LUA_REGISTRYINDEX, ref);
	
	// Push arguments
	LuaStackUtils::pushManyVariantsOnStack (runtime, arguments);
	
	// Call TODO: Use LuaRuntime::pcall()
	int r = lua_pcall (env, arguments.length (), LUA_MULTRET, 0);
	if (r != 0) {
		nError() << "Failed to invoke LUA function:" << LuaRuntime::luaErrorToString (r);
		return QVariant ();
	}
	
	// Return result
	LuaValues results = LuaStackUtils::popResultsFromStack (runtime, oldTop);
	return LuaStackUtils::luaValuesToVariant (results);
}

void Nuria::LuaCallbackTrampoline::registerMetaTable (lua_State *env) {
	luaL_newmetatable (env, "_Nuria_CallbackDeallocator");
	lua_pushstring (env, "__gc");
	lua_pushcclosure (env, &destroyCallbackUserData, 0);
	lua_settable (env, -3);
}

int Nuria::LuaCallbackTrampoline::destroyCallbackUserData (lua_State *env) {
	Nuria::Callback *callback = (Nuria::Callback *)lua_touserdata (env, 1);
	callback->~Callback ();
	return 0;
}

QVariant Nuria::LuaCallbackTrampoline::objectToVariant (const LuaObject &object, int targetType) {
	const char *name = QMetaType::typeName (targetType);
	if (name && name[qstrlen (name) - 1] == '*') {
		return object.toVariant ();
	}
	
	return object.copy ();
	
}

QVariantList Nuria::LuaCallbackTrampoline::valuesToList (const Nuria::LuaValues &values, const QList< int > &types) {
	
	QVariantList arguments;
	
	for (int i = 0; i < values.length (); i++) {
		const LuaValue &cur = values.at (i);
		if (cur.object ().isValid ()) {
			int type = (i < types.length ()) ? types.at (i) : 0;
			arguments.append (objectToVariant (cur.object (), type));
		} else {
			arguments.append (cur.toVariant ());
		}
		
	}
	
	return arguments;
}

int Nuria::LuaCallbackTrampoline::invokeCallback (lua_State *env) {
	Nuria::Callback &callback = *(Nuria::Callback *)lua_touserdata(env, lua_upvalueindex(1));
	Nuria::LuaRuntime *runtime = (Nuria::LuaRuntime *)lua_touserdata(env, lua_upvalueindex(2));
	
	// Arguments LUA -> C++
	// Sanity check
	int count = lua_gettop (env);
	if (!callback.isVariadic () && count != callback.argumentTypes ().length ()) {
		lua_pushfstring (env, "Failed to invoke function, expected %i arguments, but got %i.",
				 count, callback.argumentTypes ().length ());
		return lua_error (env);
	}
	
	// Argument types
	QList< int > types;
	if (!callback.isVariadic ()) {
		types = callback.argumentTypes ();
	}
	
	// Read arguments
	LuaValues args = LuaStackUtils::readValuesFromStack (runtime, count);
	QVariantList arguments = valuesToList (args, types);
	
	// Invoke ...
	QVariant result = callback.invoke (arguments);
	
	// Push result if there is one
	if (!result.isValid ()) {
		return 0;
	}
	
	// 
	LuaStackUtils::pushVariantOnStack (runtime, result);
	return 1;
}

QVariant Nuria::LuaCallbackTrampoline::functionFromStack (LuaRuntime *runtime, int idx) {
	lua_State *env = (lua_State *)runtime->luaState ();
	
	// Copy the function to the top of the stack
	lua_pushvalue (env, idx);
	
	// Grab a reference to this function
	int index = luaL_ref (env, LUA_REGISTRYINDEX);
	
	// Create the trampoline structure. It uses reference counting and
	// destroys the LUA reference if it's no longer needed.
	Trampoline trampoline;
	trampoline.d->runtime = runtime;
	trampoline.d->luaRef = index;
	
	// 
	auto lambda = [runtime, trampoline](const QVariantList &args) {
		return trampolineInvoke (runtime, trampoline.d->luaRef, args);
	};
	
	Callback cb = Callback::fromLambda (lambda, true);
	return QVariant::fromValue (cb);
}

void Nuria::LuaCallbackTrampoline::pushCallbackOnStack (LuaRuntime *runtime, const Callback &callback) {
	
	// We need store a handle to the callback inside LUA in a way we can
	// access it later AND so we get notified when it's garbage collected.
	// All this while having LUA function ("C Closure Function").
	// Solution: Create a userdata object which stores the callback and give
	// a metatable to it which offers a '__gc' method. Use this as upvalue
	// for a C closure and we can call Nuria::Callbacks in LUA.
	
	lua_State *env = (lua_State *)runtime->luaState ();
	void *ptr = lua_newuserdata (env, sizeof(Callback));
	new (ptr) Callback (callback); // Copy callback into the user data
	
	// Set the helper-meta table on the user data - See Caller
	luaL_getmetatable (env, "_Nuria_CallbackDeallocator");
	lua_setmetatable (env, -2);
	
	// Push the runtime as another upvalue
	lua_pushlightuserdata (env, runtime);
	
	// Push trampoline
	lua_pushcclosure (env, &Caller::invokeCallback, 2);
}
