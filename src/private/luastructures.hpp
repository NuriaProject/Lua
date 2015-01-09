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

#ifndef NURIA_LUASTRUCTURES_HPP
#define NURIA_LUASTRUCTURES_HPP

#include <nuria/metaobject.hpp>

namespace Nuria {

class Q_DECL_HIDDEN LuaWrapperUserData {
public:
	
	bool owned; // Does Lua own the object?
	int reference = 0; // Lua table which extends upon 'ptr'
	void *ptr = nullptr; // The object
	MetaObject *meta; // MetaObject of 'ptr'
	
};

}

#endif // NURIA_LUASTRUCTURES_HPP
