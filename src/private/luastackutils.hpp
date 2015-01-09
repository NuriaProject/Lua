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

#ifndef NURIA_LUASTACKUTILS_HPP
#define NURIA_LUASTACKUTILS_HPP

#include <nuria/callback.hpp>
#include "../nuria/luavalue.hpp"
#include <lua.hpp>

namespace Nuria {

class LuaRuntime;

class Q_DECL_HIDDEN LuaStackUtils {
public:
	
	static LuaValues popResultsFromStack (LuaRuntime *runtime, int oldTop);
	static LuaValues popFromStackMulti (LuaRuntime *runtime, int count);
	static LuaValues popFromStackMultiInternal (LuaRuntime *runtime, int count);
	static LuaValues readValuesFromStack (LuaRuntime *runtime, int count);
	
	static void pushManyVariantsOnStack (LuaRuntime *runtime, const QVariantList &list);
	static void pushVariantOnStack (LuaRuntime *runtime, const QVariant &variant);
	static void pushVariantMapOnStack (LuaRuntime *runtime, const QVariantMap &map);
	static void pushVariantListOnStack (LuaRuntime *runtime, const QVariantList &list);
	static void pushCObjectOnStack (LuaRuntime *runtime, const QVariant &variant);
	
	static QVariant luaValuesToVariant (const LuaValues &values);
	
	static QVariant variantFromStack (LuaRuntime *runtime, int idx, bool takeOwnership = false);
	static QVariant tableFromStack (LuaRuntime *runtime, int idx, bool takeOwnership);
	static void traverseTable (LuaRuntime *runtime, QVariantMap &map, QVariantList &list, int idx,
	                           bool takeOwnership);
	static void mergeTable (QVariantMap &map, const QVariantList &list);
		
	
};

}

#endif // NURIA_LUASTACKUTILS_HPP
