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

#ifndef NURIA_LUAOBJECT_HPP
#define NURIA_LUAOBJECT_HPP

#include <QSharedDataPointer>
#include "lua_global.hpp"
#include "luavalue.hpp"

namespace Nuria {

class LuaObjectPrivate;
class LuaStackUtils;
class LuaRuntime;
class MetaObject;
class LuaValue;
class MetaEnum;

/**
 * \brief Stores a C++ object with an associated Nuria::MetaObject.
 * 
 * LuaObject encapsulates a Lua "user data" item, exposing a C++ object
 * through Lua's meta table with the help of MetaObject.
 * 
 * All accesses to it from Lua are redirected to the C++ object.
 */
class NURIA_LUA_EXPORT LuaObject {
public:
	
	/** Constructor for a invalid instance. */
	LuaObject ();
	
	/** Copy constructor. */
	LuaObject (const LuaObject &other);
	
	/** Assignment operator. */
	LuaObject &operator= (const LuaObject &other);
	
	/** Destructor */
	~LuaObject ();
	
	/** Returns \c true if this instance is valid. */
	bool isValid () const;
	
	/** Returns the associated LuaRuntime. */
	LuaRuntime *runtime () const;
	
	/** Returns the internal LUA reference. */
	int reference () const;
	
	/** Returns the associated MetaObject. */
	MetaObject *metaObject () const;
	
	/**
	 * Returns the wrapped C++ structure. Returns \c nullptr if there's no
	 * associated object.
	 * 
	 * \sa toVariant(), copy()
	 */
	void *object () const;
	
	/**
	 * Puts the result of object() into a QVariant and returns it.
	 * If object() returned \c nullptr, the resulting QVariant is invalid.
	 * 
	 * \sa copy()
	 */
	QVariant toVariant () const;
	
	/**
	 * Like toVariant(), but the referenced object is copied using its copy
	 * constructor and the resulting pointer put into a QVariant.
	 * 
	 * If copy'ing fails, the resulting QVariant is invalid.
	 * 
	 * \sa toVariant()
	 */
	QVariant copy () const;
	
	/**
	 * Creates a object from \a object which is defiend by \a metaObject.
	 * The resulting LuaObject is bound to \a runtime.
	 * 
	 * If \a takeOwnership is \c true, then ownership of \a object is
	 * transferred to the LUA runtime.
	 */
	static LuaObject fromStructure (void *object, MetaObject *metaObject, LuaRuntime *runtime,
					bool takeOwnership = false);
	
	/**
	 * Creates a object from \a variant. If conversion is not possible, the
	 * resulting LuaObject will be invalid.
         * The resulting LuaObject is bound to \a runtime.
         * 
         * If \a variant contains a pointer type and \a takeOwnership is \c true
         * then ownership of the object inside \a variant is transferred to
         * the LUA runtime.
         * 
         * \sa LuaValue(LuaRuntime *runtime, const QVariant &variant) isValid
	 */
	static LuaObject fromVariant (const QVariant &variant, LuaRuntime *runtime,
				      bool takeOwnership = false);
	
private:
	friend class LuaMetaObjectWrapper;
	friend class LuaStackUtils;
	friend class LuaRuntime;
	friend class LuaValue;
	
	LuaObject (LuaRuntime *runtime, int userDataWrapperRef);
	
	static MetaObject *findMetaObjectOfData (LuaRuntime *runtime, int idx);
	void pushOnStack ();
	
	// 
	QSharedDataPointer< LuaObjectPrivate > d;
};

}

Q_DECLARE_METATYPE(Nuria::LuaObject)

#endif // NURIA_LUAOBJECT_HPP
