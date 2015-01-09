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

#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <nuria/essentials.hpp>
#include <QMetaType>
#include <QString>
#include <QObject>

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

class NURIA_INTROSPECT TestObject : public QObject {
	Q_OBJECT
public:
	
	TestObject () { }
	~TestObject () { qDebug("~TestObject"); }
	
};

struct NURIA_INTROSPECT Complex {
	int id;
	
	~Complex () {
		delete b;
		delete next;
	}
	
	TestStruct a;
	TestStruct *b = nullptr;
	Complex *next = nullptr;
	
};

// Needed for QVariant::fromValue().
Q_DECLARE_METATYPE(TestObject*)
Q_DECLARE_METATYPE(TestStruct*)
Q_DECLARE_METATYPE(TestStruct)

#endif // STRUCTURES_HPP
