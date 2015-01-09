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

#ifndef NURIA_LUAMETAOBJECTWRAPPER_HPP
#define NURIA_LUAMETAOBJECTWRAPPER_HPP

#include <QObject>

struct lua_State;

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
	static int declarativeCreate (LuaRuntime *runtime, lua_State *env, MetaObject *meta);
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
