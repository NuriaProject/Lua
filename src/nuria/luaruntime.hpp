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

#ifndef NURIA_LUARUNTIME_HPP
#define NURIA_LUARUNTIME_HPP

#include <functional>
#include <QObject>

#include <nuria/metaobject.hpp>
#include "lua_global.hpp"
#include "luavalue.hpp"

class QIODevice;

namespace Nuria {

namespace Internal { class Delegate; }
class LuaCallbackTrampoline;
class LuaMetaObjectWrapper;
class LuaBuiltinFunctions;
class LuaRuntimePrivate;
class LuaMetaObject;
class Callback;

/**
 * \brief Runtime for the LUA scripting language.
 * 
 * This class is a C++/Qt wrapper for LuaJit, a JITing implementation of the LUA
 * VM. You can use it if you need scripting support in your application easily.
 * 
 * \par Usage
 * It's really easy to get started! First, instantiate a LuaRuntime. You can
 * then export specific C++ classes to LUA using registerMetaObject() or run
 * scripts using execute() or executeStream().
 * 
 * Currently, the only exposed API to Lua provided by LuaRuntime is
 * "Nuria.connect(object, slotName, function)", which works like
 * QObject::connect().
 * 
 * \par Garbage collection and ownership
 * Garbage collection is taken care of by Lua itself. What's more important is
 * ownership of instances of C++ classes and structures. Generally, a class
 * instance is owned by the world it was created in (So C++ or LUA) and thus
 * may or may not be subject to garbage collection. This means in plain words:
 * 
 * - A object created inside LUA belongs to LUA and is subject to the GC.
 * - A object created outside of LUA is \b not subject to the GC.
 * 
 * You can control this more fine-grained if you want to though.
 * LuaObject::fromStructure has a \a ownership argument for this. To change
 * ownership later, you can use setObjectOwnership() and objectOwnership() to
 * set or get the current ownership respectively. You can also register a
 * handler using setObjectHandler() to install a runtime-wide function which is
 * called whenever LUA collected a C++ object.
 * 
 * \par Class interoperability
 * Using Nuria::MetaObject, it's possible to use C++ classes inside Lua. You can
 * pass C++ objects (That is, a void pointer with a MetaObject) to Lua or expose
 * MetaObjects directly to Lua, which enable Lua scripts to construct instances
 * at will.
 * 
 */
class NURIA_LUA_EXPORT LuaRuntime : public QObject {
	Q_OBJECT
public:
	
	/** Built-in LUA libraries. */
	enum LuaLib {
		Base = 0x01,
		Math = 0x02,
		String = 0x04,
		Table = 0x08,
		InputOutput = 0x10,
		OperatinSystem = 0x20,
		Package = 0x40,
		Debug = 0x80,
		Bit = 0x100,
		Jit = 0x200,
		Ffi = 0x400,
		
		/** All LUA libraries. This is the default. */
		AllLibraries = Base | Math | String | Table | InputOutput |
		OperatinSystem | Package | Debug | Bit | Jit | Ffi
	};
	
	Q_DECLARE_FLAGS(LuaLibs, LuaLib)
	
	/** Ownership of objects inside LUA. */
	enum Ownership {
		
		/**
		 * The object is owned by LUA and thus is subject to its
		 * garbage collection.
		 */
		OwnedByLua = 0x1,
		
		/**
		 * The object is owned by C++ and thus is \b not garbage
		 * collected by LUA, even if it's no longer used in the
		 * runtime anywhere.
		 */
		OwnedByCpp = 0x2,
		
	};
	
	Q_DECLARE_FLAGS(OwnershipFlags, Ownership)
	
	/**
	 * Constructor.
	 * Loads all \a libraries into the environment (These are provided by
	 * the internal Lua implementation).
	 */
	explicit LuaRuntime (LuaLibs libraries, QObject *parent = 0);
	
	/** Destructor. */
	~LuaRuntime () override;
	
	/**
	 * Executes \a script in the runtime. Returns \c true on success.
	 * Returns \c false if an error occured, like a syntax error.
	 * 
	 * If the call failed, lastResult() will return a string containing a
	 * human-readable error message.
	 * 
	 * After \a script has returned, lastResult() will return the result of
	 * it. If there were multiple results, then lastResult() will return
	 * a QVariantList containing all results.
	 * 
	 * \sa lastResult
	 */
	bool execute (const QByteArray &script);
	
	/**
	 * Reads all available bytes from \a device and executes it as script.
	 * \a device must be open and readable. Result is the same as execute().
	 * \sa lastResult execute
	 */
	bool executeStream (QIODevice *device);
	
	/**
	 * Returns the result of the last call to execute().
	 * If multiple results were returned only the first one is returend.
	 * \sa allResults
	 */
	LuaValue lastResult () const;
	
	/**
	 * Like lastResult(), but instead returns all results of the last
	 * execute() call. If no results were returned from the LUA script,
	 * an empty list is returned.
	 */
	LuaValues allResults () const;
	
	/** Returns global variable \a name. */
	LuaValue global (const QString &name);
	
	/** Sets a global variable called \a name to \a value. */
	void setGlobal (const QString &name, const QVariant &value);
	
	/** \a overload */
	void setGlobal (const QString &name, const LuaValue &value);
	
	/** Returns \c true if there's a global variable called \a name. */
	bool hasGlobal (const QString &name);
	
	/**
	 * Registers \a metaObject for usage in the runtime. After this, the
	 * class can be used in LUA. The type will be stored as userdata inside
	 * LUA. Its name will be what MetaObject::className() returns.
	 * Namespaces will be interpreted as tables, meaning that the class
	 * "Something::Foo::Bar" will end up as "Something.Foo.Bar". \a prefix
	 * is prepended to the class name verbatim. Thus, if you want to put
	 * the meta object inside another table, you prefix to end with "::".
	 * 
	 * \note Calling this multiple times on the same \a metaObject is
	 * harmless.
	 */
	void registerMetaObject (MetaObject *metaObject, const QByteArray &prefix = QByteArray ());
	
	/**
	 * Instructs the LUA runtime to run the garbage collector, which will
	 * free memory no longer used.
	 * If you're doing this to not only destroy structures but also really
	 * free their memory now, you should call this method twice as the LUA
	 * garbage collector will only destroy instances at first.
	 */
	void collectGarbage ();
	
	/**
	 * Returns the ownership of \a object.
	 * If \a object does not exist, \c OwnedByLua is returned.
	 */
	Ownership objectOwnership (void *object);
	
	/**
	 * Changes the ownership of \a object to \a ownership.
	 * If \a object is not known to LUA, this function has no effect.
	 */
	void setObjectOwnership (void *object, Ownership ownership);
	
	/**
	 * Handler method for to-be-collected objects. This method is called
	 * when LUA is about to collect an object. The prototype looks as
	 * follows:
	 * \code
	 * bool (Ownership ownership, void *object, MetaObject *metaObject);
	 * \endcode
	 * 
	 * \a ownership indicates who owns the object, \a object is the affected
	 * object with its \a metaObject. If \a ownership is \c OwnedByLua, the
	 * \a object will be destroyed if the handler returns \c true. In any
	 * other case \a object is not touched.
	 */
	typedef std::function< bool(Ownership, void *, MetaObject *) > ObjectHandler;
	
	/**
	 * Installs a object handler called whenever a object is about to be
	 * collected by Luas garbage collector. You can control when \a handler
	 * should be called using \a flags, e.g. to be only notified when an
	 * object owned by Lua is collected.
	 * 
	 * \sa ObjectHandler
	 */
	void setObjectHandler (ObjectHandler handler, OwnershipFlags flags = OwnershipFlags (OwnedByLua | OwnedByCpp));
	
	/**
	 * Returns the internal LUA runtime. The returned pointer is of type
	 * lua_State*. To make use of it, you'll need to #include "lua.hpp".
	 * 
	 * \warning The Lua integration doesn't expect sudden changes to the
	 * environment. Be careful with this function.
	 */
	void *luaState ();
	
private:
	friend class LuaCallbackTrampoline;
	friend class LuaMetaObjectWrapper;
	friend class LuaBuiltinFunctions;
	friend class Internal::Delegate;
	friend class LuaMetaObject;
	friend class LuaObject;
	
	// 
	void createObjectsReferenceTable ();
	
	bool pcall (int argCount, LuaValues &results);
	
	void setLastResultError (LuaValues &values, const QString &message);
	static QString luaErrorToString (int error);
	void createLuaInstance ();
	void openLuaLibraries (LuaLibs libraries);
	
	// 
	LuaRuntimePrivate *d_ptr;
	
};

}

#endif // NURIA_LUARUNTIME_HPP
