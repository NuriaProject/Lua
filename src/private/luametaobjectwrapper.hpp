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

#ifndef NURIA_LUAMETAOBJECTWRAPPER_HPP
#define NURIA_LUAMETAOBJECTWRAPPER_HPP

#include <QObject>

namespace Nuria {

class LuaMetaObjectWrapperPrivate;
class LuaRuntime;
class MetaObject;

namespace Internal { class Delegate; }

/**
 * \brief Internal class for LuaRuntime for MetaObject <-> LUA delegation
 * 
 * The LuaMetaObjectWrapper delegates accesses to MetaObject's from LUA code.
 * It does this by registering itself into a meta table, thus intercepting
 * access to these tables.
 * 
 */
class Q_DECL_HIDDEN LuaMetaObjectWrapper : public QObject {
	Q_OBJECT
public:
	
	/** */
	LuaMetaObjectWrapper (MetaObject *metaObject, LuaRuntime *runtime);
	
	/** Destructor. */
	~LuaMetaObjectWrapper ();
	
	/** Returns the wrapped MetaObject. */
	MetaObject *nuriaMetaObject () const;
	
	/** Returns the associated runtime. */
	LuaRuntime *runtime () const;
	
	/**
	 * Inserts the meta methods into the table which is on the top of the
	 * stack.
	 */
	void populateMetaTable ();
	
	/** Returns the LUA reference to the meta table. */
	int reference () const;
	
	/** Returns \c true if this wrapper is registered by name to LUA. */
	bool isRegistered () const;
	
	/** Sets whether this wrapper is known directly in LUA. */
	void setRegistered (bool registered);
	
private:
	friend class Internal::Delegate;
	
	static void pushFieldOnStack (LuaRuntime *runtime, void *inst, const char *name);
	static void setFieldFromStack (LuaRuntime *runtime, void *inst, const char *name);
	
	static bool pushMethod (LuaRuntime *runtime, void *inst, const QByteArray &name);
	static bool pushField (LuaRuntime *runtime, void *inst, const QByteArray &name);
	
	static int invokeMethod (void *state);
	static void pushInvocationResult (LuaRuntime *runtime, MetaObject *meta,
					  int funcIdx, QVariant &result);
	static bool findPossibleMethodsByArgCount (MetaObject *meta, int count, int &begin, int &end);
	static int chooseMethod (LuaRuntime *runtime, MetaObject *meta, int count, int begin, int end,
				 bool staticCall);
	static int findMethodWhere (MetaObject *meta, int begin, int end, bool isMember);
	
	// 
	LuaMetaObjectWrapperPrivate *d_ptr;
	
};

} // namespace Nuria

#endif // NURIA_LUAMETAOBJECTWRAPPER_HPP
