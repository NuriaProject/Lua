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

#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <nuria/essentials.hpp>
#include <QMetaType>
#include <QString>

struct NURIA_INTROSPECT TestStruct {
	int a;
	int b;
	QString c;
	
	TestStruct () {}
	TestStruct (int A, int B) : a (A), b (B) {
		qDebug("ctor %i %i", A, B);
	}
	
	static int sum (int a, int b) {
		qDebug("static");
		return a + b;
	}
	
	int sum () {
		qDebug("member");
		this->c = QString ("%1 + %2 = %3").arg (a).arg (b).arg (a + b);
		return a + b;
	}
	
};

// Needed for QVariant::fromValue().
Q_DECLARE_METATYPE(TestStruct*)
Q_DECLARE_METATYPE(TestStruct)

#endif // STRUCTURES_HPP
