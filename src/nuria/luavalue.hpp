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

#ifndef NURIA_LUAVALUE_HPP
#define NURIA_LUAVALUE_HPP

#include <QSharedDataPointer>
#include <QVariant>
#include <QVector>

#include "lua_global.hpp"
#include "luaobject.hpp"

namespace Nuria {

class LuaMetaObjectWrapper;
class LuaValuePrivate;
class LuaMetaObject;
class LuaStackUtils;
class LuaRuntime;
class MetaObject;
class LuaObject;

/**
 * \brief Stores a value from LUA
 */
class NURIA_LUA_EXPORT LuaValue {
public:
	
	/** LUA types. */
	enum Type {
		Nil = 0,
		Boolean = 1,
		LightUserData = 2,
		Number = 3,
		String = 4,
		Table = 5,
		Function = 6,
		UserData = 7,
		Thread = 8
	};
	
	/** Constructs a invalid instance. */
	LuaValue ();
	
	/** Copy constructor. */
	LuaValue (const LuaValue &other);
	
	/** Constructs a value out of a LuaObject. */
	LuaValue (const LuaObject &object);
	
	/** Constructs a LuaValue out of a QVariant. */
	LuaValue (LuaRuntime *runtime, const QVariant &variant);
	/** Destructor. */
	~LuaValue ();
	
	/** Assignment operator. */
	LuaValue &operator= (const LuaValue &other);
	
	/** Returns the LUA type of this instance. */
	Type type () const;
	
	/** Returns the associated LUA runtime. */
	LuaRuntime *runtime () const;
	
	/** Returns \c true if this instance is valid. */
	bool isValid () const;
	
	/** Converts the value into a QVariant. */
	QVariant toVariant () const;
	
	/**
	 * Returns the LuaObject if this value is a LUA user-data pointing to
	 * a type with a Nuria::MetaObject. If this isn't an object, the
	 * returned instance is invalid.
	 */
	LuaObject object () const;
	
	/**
	 * Returns the LUA type that'd be used to represent \a type.
	 * If there's no known way to store \a type in LUA, \c Nil is returned.
	 */
	static Type qtTypeToLua (int type);
	
	/**
	 * Constructs a instance from the value at stack \a idx of \a runtime.
	 * \note This method is intended for internal use.
	 */
	static LuaValue fromStack (LuaRuntime *runtime, int idx);
	
	
private:
	friend class LuaMetaObjectWrapper;
	friend class LuaMetaObject;
	friend class LuaStackUtils;
	friend class LuaRuntime;
	friend class LuaObject;
	
	// 
	void initValue (int idx);
	void setLuaObject (int idx);
	
	// 
	QExplicitlySharedDataPointer< LuaValuePrivate > d;
	
};

/** List of LuaValues. */
typedef QVector< LuaValue > LuaValues;

}

#endif // NURIA_LUAVALUE_HPP
