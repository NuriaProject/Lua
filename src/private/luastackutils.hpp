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
