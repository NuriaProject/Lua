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

#include "nuria/luaobject.hpp"

#include <QSharedData>
#include <QPointer>
#include <lua.hpp>
#include <QVector>

#include "private/luametaobjectwrapper.hpp"
#include "private/luaruntimeprivate.hpp"
#include "private/luastructures.hpp"
#include <nuria/metaobject.hpp>
#include "nuria/luaruntime.hpp"

namespace Nuria {

class Q_DECL_HIDDEN LuaObjectPrivate : public QSharedData {
public:
	~LuaObjectPrivate () {
		releaseReference ();
	}
	
	void releaseReference () {
		if (reference == 0 || runtime.isNull ())
			return;
		
		lua_State *env = (lua_State *)runtime->luaState ();
		if (env)
			lua_unref (env, reference);
		
	}
	
	// 
	QPointer< LuaRuntime > runtime;
	int reference;
	
};

}

Nuria::LuaObject::LuaObject ()
	: d (new LuaObjectPrivate)
{
	
}

Nuria::LuaObject::LuaObject (const LuaObject &other)
	: d (other.d)
{
	
}

Nuria::LuaObject::LuaObject (Nuria::LuaRuntime *runtime, int userDataWrapperRef)
	: d (new LuaObjectPrivate)
{
	
	this->d->runtime = runtime;
	this->d->reference = userDataWrapperRef;
	
}

Nuria::LuaObject &Nuria::LuaObject::operator= (const LuaObject &other) {
	this->d = other.d;
	return *this;
}

Nuria::LuaObject::~LuaObject () {
	
}

bool Nuria::LuaObject::isValid () const {
	return (!this->d->runtime.isNull () && this->d->reference > 0);
}

Nuria::LuaRuntime *Nuria::LuaObject::runtime () const {
	return this->d->runtime;
}

int Nuria::LuaObject::reference () const {
	return this->d->reference;
}

static Nuria::LuaWrapperUserData *userData (const Nuria::LuaRuntime *runtime, int ref) {
	using namespace Nuria;
	
	lua_State *env = (lua_State *)const_cast< Nuria::LuaRuntime * > (runtime)->luaState ();
	lua_rawgeti (env, LUA_REGISTRYINDEX, ref);
	LuaWrapperUserData *ptr = (LuaWrapperUserData *)lua_touserdata (env, -1);
	lua_pop (env, 1);
	
	return ptr;
}

Nuria::MetaObject *Nuria::LuaObject::metaObject () const {
	if (this->d->runtime.isNull () || this->d->reference < 1) {
		return nullptr;
	}
	
	// 
	return userData (this->d->runtime, this->d->reference)->meta;
}

void *Nuria::LuaObject::object () const {
	if (this->d->runtime.isNull () || this->d->reference < 1) {
		return nullptr;
	}
	
	// 
	LuaWrapperUserData *data = userData (this->d->runtime, this->d->reference);
	if (data->reference > 0) {
		return data;
	}
	
	return data->ptr;
}

QVariant Nuria::LuaObject::toVariant () const {
	void *ptr = object ();
	MetaObject *meta = metaObject ();
	
	if (!ptr || !meta) {
		return QVariant ();
	}
	
	// 
	return QVariant (meta->pointerMetaTypeId (), &ptr, true);
}

QVariant Nuria::LuaObject::copy () const {
	void *ptr = object ();
	MetaObject *meta = metaObject ();
	
	if (!ptr || !meta) {
		return QVariant ();
	}
	
	// 
	return QVariant (meta->metaTypeId (), ptr);
}

Nuria::LuaObject Nuria::LuaObject::fromStructure (void *object, Nuria::MetaObject *metaObject,
						  Nuria::LuaRuntime *runtime, bool takeOwnership) {
	lua_State *env = (lua_State *)runtime->luaState ();
	LuaMetaObjectWrapper *wrapper = runtime->d_ptr->findOrCreateWrapper (metaObject);
	
	// Build delegate user data
	if (object) {
		runtime->d_ptr->pushOrCreateUserDataObject (object, metaObject, wrapper->reference (), takeOwnership);
	} else {
		runtime->d_ptr->pushWrapperObject (metaObject, wrapper->reference ());
	}
	
	// 
	int ref = luaL_ref (env, LUA_REGISTRYINDEX);
	return LuaObject (runtime, ref);
}

static bool variantContainsPointer (const QVariant &variant) {
	const char *typeName = variant.typeName ();
	
	int len = typeName ? qstrlen (typeName) : 0;
	return (len < 1) ? false : typeName[len - 1] == '*';
}

static Nuria::MetaObject *metaObjectOfVariant (const QVariant &variant, bool isPointer) {
	QByteArray name (variant.typeName ());
	
	if (isPointer) {
		name.chop (1);
	}
	
	// 
	return Nuria::MetaObject::byName (name);
}

static void *copyVariant (const QVariant &variant, Nuria::MetaObject *meta) {
	
	// Find copy ctor
	Nuria::MetaMethod copy = meta->method ({ QByteArray (), meta->className () });
	
	// Invoke
	QVariant v = copy.callback () (variant);
	return Nuria::Variant::stealPointer (v);
}

Nuria::LuaObject Nuria::LuaObject::fromVariant (const QVariant &variant, Nuria::LuaRuntime *runtime,
						bool takeOwnership) {
	bool isPointer = variantContainsPointer (variant);
	MetaObject *meta = metaObjectOfVariant (variant, isPointer);
	
	// No meta object found?
	if (!meta) {
		return LuaObject ();
	}
	
	// 
	void *ptr = nullptr;
	if (isPointer) {
		QVariant v = variant;
		ptr = Variant::stealPointer (v);
	} else {
		ptr = copyVariant (variant, meta);
		takeOwnership = true;
	}
	
	// 
	return fromStructure (ptr, meta, runtime, takeOwnership);
}

Nuria::MetaObject *Nuria::LuaObject::findMetaObjectOfData (Nuria::LuaRuntime *runtime, int idx) {
	lua_State *env = (lua_State *)runtime->luaState ();
	
	// Find meta table of the table at 'idx'
	if (lua_getmetatable (env, idx) == 0) {
		return nullptr;
	}
	
	// It has a meta table. Look for "_nuria_metaobject"
	lua_pushliteral (env, "_nuria_metaobject");
	lua_gettable (env, -2);
	
	// Get meta object if it's a 'light user data'
	MetaObject *meta = nullptr;
	if (lua_type (env, -1) == LUA_TLIGHTUSERDATA) {
		meta = (MetaObject *)lua_topointer (env, -1);
	}
	
	// Clean stack
	lua_pop (env, 2);
	return meta;
}

void Nuria::LuaObject::pushOnStack () {
	lua_State *env = (lua_State *)this->d->runtime->luaState ();
	
	if (this->d->reference < 1) {
		lua_pushnil (env);
		return;
	}
	
	// 
	lua_getref (env, this->d->reference);
}

