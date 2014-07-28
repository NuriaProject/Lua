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

#include "nuria/luavalue.hpp"

#include <QSharedData>
#include <QPointer>
#include <lua.hpp>

#include "private/luastackutils.hpp"
#include "nuria/luaruntime.hpp"
#include "nuria/luaobject.hpp"

namespace Nuria {
class Q_DECL_HIDDEN LuaValuePrivate : public QSharedData {
public:
	
	QPointer< LuaRuntime > runtime = nullptr;
	LuaValue::Type type = LuaValue::Nil;
	QVariant value;
	LuaObject object;
	
};
}

Nuria::LuaValue::LuaValue ()
	: d (new LuaValuePrivate)
{
	
}

Nuria::LuaValue::LuaValue (const LuaValue &other)
	: d (other.d)
{
	
}

Nuria::LuaValue::LuaValue (const Nuria::LuaObject &table)
	: d (new LuaValuePrivate)
{
	this->d->runtime = table.runtime ();
	this->d->object = table;
	this->d->type = Table;
	
}

Nuria::LuaValue::LuaValue (LuaRuntime *runtime, const QVariant &variant)
	: d (new LuaValuePrivate)
{
	
	this->d->runtime = runtime;
	
	if (variant.userType () == qMetaTypeId< LuaObject > ()) {
		this->d->object = variant.value< LuaObject > ();
		this->d->type = Table;
	} else {
		
		LuaObject obj = LuaObject::fromVariant (variant, runtime);
		if (obj.isValid ()) {
			this->d->object = obj;
			this->d->type = Table;
		} else {
			this->d->type = qtTypeToLua (variant.userType ());
			this->d->value = variant;
		}
		
	}
	
}

Nuria::LuaValue::~LuaValue () {
	
}

Nuria::LuaValue &Nuria::LuaValue::operator= (const LuaValue &other) {
	this->d = other.d;
	return *this;
}

Nuria::LuaValue::Type Nuria::LuaValue::type () const {
	return this->d->type;
}

Nuria::LuaRuntime *Nuria::LuaValue::runtime () const {
	return this->d->runtime;
}

bool Nuria::LuaValue::isValid () const {
	return (this->d->runtime && this->d->type != Nil);
}

QVariant Nuria::LuaValue::toVariant () const {
	if (this->d->value.isValid ()) {
		return this->d->value;
	}
	
	// 
	return QVariant::fromValue (this->d->object);
	
}

Nuria::LuaObject Nuria::LuaValue::object () const {
	return this->d->object;
}

Nuria::LuaValue::Type Nuria::LuaValue::qtTypeToLua (int type) {
	switch (type) {
	case QMetaType::Int:
	case QMetaType::Float:
	case QMetaType::Double:
		return Number;
	case QMetaType::Bool:
		return Boolean;
	case QMetaType::QString:
	case QMetaType::QByteArray:
		return String;
	case QMetaType::QVariantList:
	case QMetaType::QVariantMap:
		return Table;
	}
	
	return Nil;
}

Nuria::LuaValue Nuria::LuaValue::fromStack (Nuria::LuaRuntime *runtime, int idx) {
	LuaValue value;
	
	value.d->runtime = runtime;
	value.initValue (idx);
	
	return value;
}

void Nuria::LuaValue::initValue (int idx) {
	lua_State *env = (lua_State *)this->d->runtime->luaState ();
	
	// 
	this->d->type = Type (lua_type (env, idx));
	if (this->d->type != UserData || !LuaObject::findMetaObjectOfData (this->d->runtime, idx)) {
		this->d->value = LuaStackUtils::variantFromStack (this->d->runtime, idx);
	} else {
		
		// This is a table with a MetaObject
		setLuaObject (idx);
	}
	
}

void Nuria::LuaValue::setLuaObject (int idx) {
	lua_State *env = (lua_State *)this->d->runtime->luaState ();
	
	// Get a LUA reference to the table
	lua_pushvalue (env, idx);
	int ref = luaL_ref (env, LUA_REGISTRYINDEX);
	
	// 
	this->d->object = LuaObject (this->d->runtime, ref);
}
