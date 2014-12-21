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

#include <QtTest/QtTest>
#include <QObject>

#include <nuria/luaruntime.hpp>
#include <nuria/metaobject.hpp>
#include <nuria/debug.hpp>
#include "structures.hpp"

using namespace Nuria;

class LuaRuntimeTest : public QObject {
	Q_OBJECT
private slots:
	
	void initTestCase ();
	
	// Basic types
	void returnInt ();
	void returnBool ();
	void returnString ();
	void returnList ();
	void returnMap ();
	void returnFunction ();
	
	void passInt ();
	void passBool ();
	void passString ();
	void passList ();
	void passMap ();
	void passFunction ();
	
	void returnMultipleValues ();
	
	// Global access
	void verifyGlobals ();
	void globalToCode ();
	void globalFromCode ();
	
	// Complex types (C++ structures -> LUA)
	void structureToLua ();
	void pointerToLua ();
	void constructClassInLua ();
	void invokeMemberMethod ();
	void invokeStaticMethod ();
	void globalTestStruct ();
	void returnTestStruct ();
	void passTestStructToCpp ();
	void passTestStructToCppAsPointer ();
	void verifyStructureWrapperExistsOnlyOnce ();
	void createInstanceDeclarative ();
	void createComplexInstanceDeclarative ();
	
	// Ownership
	void verifyObjectHandlerBehaviour_data ();
	void verifyObjectHandlerBehaviour ();
	void destroyOwnedQObject ();
	void leaveUnownedQObjectAlone ();
	void reownedQObjectNotDestroyed ();
	
private:
	bool hasTria = false;
	
};

#define NEEDS_TRIA \
	if (!this->hasTria) QSKIP("Tria is needed for this test-case.")

void LuaRuntimeTest::initTestCase () {
	this->hasTria = (MetaObject::byName ("TestStruct") != nullptr);
}

void LuaRuntimeTest::returnInt() {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return 4 + 5"));
	QCOMPARE(runtime.allResults ().length (), 1);
	QCOMPARE(runtime.lastResult ().toVariant ().toInt (), 9);
}

void LuaRuntimeTest::returnBool() {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return true"));
	QCOMPARE(runtime.allResults ().length (), 1);
	QCOMPARE(runtime.lastResult ().toVariant ().toBool (), true);
}

void LuaRuntimeTest::returnString() {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return \"works\""));
	QCOMPARE(runtime.allResults ().length (), 1);
	QCOMPARE(runtime.lastResult ().toVariant ().toString (), QString ("works"));
}

void LuaRuntimeTest::returnList() {
	QVariantList expected { 1, 2, 3 };
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return { 1, 2, 3 }"));
	QCOMPARE(runtime.allResults ().length (), 1);
	QCOMPARE(runtime.lastResult ().type (), LuaValue::Table);
	QCOMPARE(runtime.lastResult ().toVariant ().toList (), expected);
}

void LuaRuntimeTest::returnMap () {
	QVariantMap expected { { "a", "foo" }, { "b", 123 } };
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return { a = \"foo\", b = 123 }"));
	QCOMPARE(runtime.allResults ().length (), 1);
	QCOMPARE(runtime.lastResult ().type (), LuaValue::Table);
	QCOMPARE(runtime.lastResult ().toVariant ().toMap (), expected);
}

void LuaRuntimeTest::returnFunction () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	QVERIFY(runtime.execute ("return function(a,b) return a+b end"));
	QCOMPARE(runtime.allResults ().length (), 1);
	Callback cb = runtime.lastResult ().toVariant ().value< Callback > ();
	
	QVERIFY(cb.isValid ());
	QCOMPARE(cb (3, 4).toInt (), 7);
}

void LuaRuntimeTest::passInt () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return function(a) return a+1 end"));
	
	Callback cb = runtime.lastResult ().toVariant ().value< Callback > ();
	QCOMPARE(cb (3).toInt (), 4);
}

void LuaRuntimeTest::passBool () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return function(a) return (not a) end"));
	
	Callback cb = runtime.lastResult ().toVariant ().value< Callback > ();
	QCOMPARE(cb (false).toBool (), true);
	
}

void LuaRuntimeTest::passString () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return function(a) return a .. \"bar\" end"));
	
	Callback cb = runtime.lastResult ().toVariant ().value< Callback > ();
	QCOMPARE(cb (QString ("foo")).toString (), QString ("foobar"));
	
}

void LuaRuntimeTest::passList () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return function(a) return { a[3], a[2], a[1] } end"));
	
	Callback cb = runtime.lastResult ().toVariant ().value< Callback > ();
	QCOMPARE(cb (QVariantList ({ 3, 2, 1 })).toList (), QVariantList ({ 1, 2, 3 }));
}

void LuaRuntimeTest::passMap () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return function(a) return { foo = a.foo + 1, bar = a.bar + 1 } end"));
	
	Callback cb = runtime.lastResult ().toVariant ().value< Callback > ();
	QVariantMap input { { "foo", 1 }, { "bar", 2 } };
	QVariantMap expected { { "foo", 2 }, { "bar", 3 } };
	
	QCOMPARE(cb (input).toMap (), expected);
	
}

void LuaRuntimeTest::passFunction () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return function(a) return a(2) end, function(b) return b*b end"));
	
	// LUA function -> Callback -> LUA -> Returns the result of the LUA function inside the Callback.
	Callback a = runtime.allResults ().last ().toVariant ().value< Callback > ();
	Callback cb = runtime.lastResult ().toVariant ().value< Callback > ();
	
	QCOMPARE(cb (a).toInt (), 4);
}

void LuaRuntimeTest::returnMultipleValues () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	QVERIFY(runtime.execute ("return 1, 2, 3"));
	
	LuaValues results = runtime.allResults ();
	QCOMPARE(results.length (), 3);
	QCOMPARE(results.at (0).toVariant ().toInt (), 1);
	QCOMPARE(results.at (1).toVariant ().toInt (), 2);
	QCOMPARE(results.at (2).toVariant ().toInt (), 3);
}

void LuaRuntimeTest::verifyGlobals () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	QVERIFY(!runtime.hasGlobal ("foo"));
	
	runtime.setGlobal ("foo", 123);
	QVERIFY(runtime.hasGlobal ("foo"));
	QCOMPARE(runtime.global ("foo").toVariant ().toInt (), 123);
	
}

void LuaRuntimeTest::globalToCode () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	runtime.setGlobal ("foo", "bar");
	QVERIFY(runtime.execute ("return foo"));
	QCOMPARE(runtime.lastResult ().toVariant ().toString (), QString ("bar"));
}

void LuaRuntimeTest::globalFromCode () {
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	runtime.setGlobal ("foo", "bar");
	QVERIFY(runtime.execute ("foo = 42"));
	QVERIFY(runtime.hasGlobal ("foo"));
	QCOMPARE(runtime.global ("foo").toVariant ().toInt (), 42);
	
}

void LuaRuntimeTest::structureToLua () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	TestStruct f;
	f.a = 123;
	f.b = 456;
	f.c = "Hello";
	
	runtime.setGlobal ("foo", QVariant::fromValue (f));
	runtime.execute ("a = foo.a; b = foo.b; c = foo.c\n" // Read from the structure
			 "foo.a = 1337"); // Assign a value to the copy
	
	QCOMPARE(f.a, 123);
	QCOMPARE(f.b, 456);
	QCOMPARE(f.c, QString ("Hello"));
	QCOMPARE(runtime.global ("a").toVariant ().toInt (), 123);
	QCOMPARE(runtime.global ("b").toVariant ().toInt (), 456);
	QCOMPARE(runtime.global ("c").toVariant ().toString (), QString ("Hello"));
}

void LuaRuntimeTest::pointerToLua () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	TestStruct *f = new TestStruct;
	f->a = 123;
	f->b = 456;
	f->c = "Hello";
	
	runtime.setGlobal ("foo", QVariant::fromValue (f));
	runtime.execute ("a = foo.a; b = foo.b; c = foo.c\n" // Read from the structure
			 "foo.a = 1337"); // Assign a value to the reference
	
	QCOMPARE(f->a, 1337); // Changed through LUA
	QCOMPARE(f->b, 456);
	QCOMPARE(f->c, QString ("Hello"));
	QCOMPARE(runtime.global ("a").toVariant ().toInt (), 123);
	QCOMPARE(runtime.global ("b").toVariant ().toInt (), 456);
	QCOMPARE(runtime.global ("c").toVariant ().toString (), QString ("Hello"));
	
	delete f;
}

void LuaRuntimeTest::constructClassInLua () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	// Registers 'TestStruct' in 'Test' inside LUA
	runtime.registerMetaObject (MetaObject::byName ("TestStruct"), "Test::");
	
	// 
	QTest::ignoreMessage (QtDebugMsg, "ctor 2 3");
	QTest::ignoreMessage (QtDebugMsg, "member");
	runtime.execute ("inst = Test.TestStruct.new (2, 3)\n"
			 "return inst:sum ()");
	
	// 
	QCOMPARE(runtime.lastResult ().toVariant ().toInt (), 5);
	
}

void LuaRuntimeTest::invokeMemberMethod () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	TestStruct f;
	f.a = 4;
	f.b = 3;
	
	QTest::ignoreMessage (QtDebugMsg, "member");
	runtime.setGlobal ("foo", QVariant::fromValue (&f));
	runtime.execute ("bar = foo:sum ()");
	
	QCOMPARE(runtime.global ("bar").toVariant ().toInt (), 7);
	QCOMPARE(f.c, QString ("4 + 3 = 7"));
	
}

void LuaRuntimeTest::invokeStaticMethod () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	TestStruct f;
	
	QTest::ignoreMessage (QtDebugMsg, "static");
	runtime.setGlobal ("foo", QVariant::fromValue (f));
	runtime.execute ("bar = foo.sum (4,3)");
	
	QCOMPARE(runtime.global ("bar").toVariant ().toInt (), 7);
	
}

void LuaRuntimeTest::globalTestStruct () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	MetaObject *meta = MetaObject::byName ("TestStruct");
	runtime.registerMetaObject (meta, "Test::");
	
	// 
	QTest::ignoreMessage (QtDebugMsg, "ctor 2 3");
	runtime.execute ("foo = Test.TestStruct.new (2,3)\n"
			 "foo.c = \"Hi from LUA\"");
	
	// 
	LuaObject obj = runtime.global ("foo").object ();
	QVERIFY(obj.isValid ());
	
	QCOMPARE(obj.metaObject (), meta);
	TestStruct *f = (TestStruct *)obj.object ();
	
	QVERIFY(f);
	QCOMPARE(f->a, 2);
	QCOMPARE(f->b, 3);
	QCOMPARE(f->c, QString ("Hi from LUA"));
}

void LuaRuntimeTest::returnTestStruct () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	MetaObject *meta = MetaObject::byName ("TestStruct");
	runtime.registerMetaObject (meta, "Test::");
	
	// 
	QTest::ignoreMessage (QtDebugMsg, "ctor 2 3");
	runtime.execute ("local foo = Test.TestStruct.new (2,3)\n"
			 "foo.c = \"Hi from LUA\"\n"
			 "return foo");
	
	// 
	LuaObject obj = runtime.lastResult ().object ();
	QVERIFY(obj.isValid ());
	
	QCOMPARE(obj.metaObject (), meta);
	TestStruct *f = (TestStruct *)obj.object ();
	
	QVERIFY(f);
	QCOMPARE(f->a, 2);
	QCOMPARE(f->b, 3);
	QCOMPARE(f->c, QString ("Hi from LUA"));
}

void LuaRuntimeTest::passTestStructToCpp () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	MetaObject *meta = MetaObject::byName ("TestStruct");
	runtime.registerMetaObject (meta, "Test::");
	
	// 
	TestStruct f;
	Callback cb = Callback::fromLambda ([&f](TestStruct a) { f = a; });
	runtime.setGlobal ("someFunc", QVariant::fromValue (cb));
	
	// 
	QTest::ignoreMessage (QtDebugMsg, "ctor 2 3");
	runtime.execute ("someFunc (Test.TestStruct.new (2,3))");
	
	// 
	QCOMPARE(f.a, 2);
	QCOMPARE(f.b, 3);
}

void LuaRuntimeTest::passTestStructToCppAsPointer () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	MetaObject *meta = MetaObject::byName ("TestStruct");
	runtime.registerMetaObject (meta, "Test::");
	
	// 
	TestStruct f;
	Callback cb = Callback::fromLambda ([&f](TestStruct *a) { f = *a; a->a = 4; });
	runtime.setGlobal ("someFunc", QVariant::fromValue (cb));
	
	// 
	QTest::ignoreMessage (QtDebugMsg, "ctor 2 3");
	runtime.execute ("local f = Test.TestStruct.new (2,3)\n"
			 "someFunc (f)\n"
			 "return f:sum()");
	
	// 
	QCOMPARE(runtime.lastResult ().toVariant ().toInt (), 7);
	QCOMPARE(f.a, 2);
	QCOMPARE(f.b, 3);
}

void LuaRuntimeTest::verifyStructureWrapperExistsOnlyOnce () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	
	MetaObject *meta = MetaObject::byName ("TestStruct");
	runtime.registerMetaObject (meta, "Test::");
	
	// 
	QTest::ignoreMessage (QtDebugMsg, "ctor 2 4");
	runtime.execute ("a = Test.TestStruct.new (2,4)\n"
			 "return a, function(b) return a == b end");
	
	// 
	void *ptr = runtime.allResults ().first ().object ().object ();
	TestStruct *f = (TestStruct *)ptr;
	
	Callback func = runtime.allResults ().last ().toVariant ().value< Callback > ();
	
	// 
	QVERIFY(func (f).toBool ());
	
}

void LuaRuntimeTest::createInstanceDeclarative () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	MetaObject *meta = MetaObject::byName ("TestStruct");
	runtime.registerMetaObject (meta, "Test::");
	
	runtime.execute ("a = Test.TestStruct { a = 123, b = 456, c = 'declarative' }");
	LuaObject obj = runtime.global ("a").object ();
	
	QVERIFY(obj.isValid ());
	TestStruct *test = (TestStruct *)obj.object ();
	QVERIFY(test);
	
	QCOMPARE(test->a, 123);
	QCOMPARE(test->b, 456);
	QCOMPARE(test->c, QString ("declarative"));
	
}

void LuaRuntimeTest::createComplexInstanceDeclarative () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	runtime.registerMetaObject (MetaObject::byName ("Complex"), "Test::");
	runtime.registerMetaObject (MetaObject::byName ("TestStruct"), "Test::");
	
	runtime.execute ("a = Test.Complex {"
	                 "  id = 1,"
	                 "  a = Test.TestStruct { a = 123, b = 456, c = 'stack' },"
	                 "  b = Test.TestStruct { a = 147, b = 258, c = 'heap' },"
	                 "  next = Test.Complex {"
	                 "    id = 2,"
	                 "    a = Test.TestStruct { a = 321, b = 654, c = 'foo' },"
	                 "    b = Test.TestStruct { a = 963, b = 852, c = 'bar' },"
	                 "    next = Test.Complex { id = 3 }"
	                 "  }"
	                 "}");
	LuaObject obj = runtime.global ("a").object ();
	
	QVERIFY(obj.isValid ());
	Complex *root = (Complex *)obj.object ();
	
	QVERIFY(root);
	QCOMPARE(root->id, 1);
	QCOMPARE(root->a.a, 123);
	QCOMPARE(root->a.b, 456);
	QCOMPARE(root->a.c, QString ("stack"));
	QVERIFY(root->b);
	QCOMPARE(root->b->a, 147);
	QCOMPARE(root->b->b, 258);
	QCOMPARE(root->b->c, QString ("heap"));
	QVERIFY(root->next);
	QCOMPARE(root->next->a.a, 321);
	QCOMPARE(root->next->a.b, 654);
	QCOMPARE(root->next->a.c, QString ("foo"));
	QVERIFY(root->next->b);
	QCOMPARE(root->next->b->a, 963);
	QCOMPARE(root->next->b->b, 852);
	QCOMPARE(root->next->b->c, QString ("bar"));
	QVERIFY(root->next->next);
	QCOMPARE(root->next->next->id, 3);
}

Q_DECLARE_METATYPE(Nuria::LuaRuntime::Ownership)
Q_DECLARE_METATYPE(Nuria::LuaRuntime::OwnershipFlags)

void LuaRuntimeTest::verifyObjectHandlerBehaviour_data () {
	QTest::addColumn< LuaRuntime::Ownership > ("owner"); // Ownership of the object
	QTest::addColumn< LuaRuntime::OwnershipFlags > ("mask"); // Handler mask
	QTest::addColumn< bool > ("called"); // If the handler should be called
	QTest::addColumn< bool > ("destroy"); // What the handler returns
	
	LuaRuntime::OwnershipFlags flagBoth = LuaRuntime::OwnershipFlags (LuaRuntime::OwnedByCpp |
	                                                                  LuaRuntime::OwnedByLua);
	LuaRuntime::OwnershipFlags flagCpp = LuaRuntime::OwnedByCpp;
	LuaRuntime::OwnershipFlags flagLua = LuaRuntime::OwnedByLua;
	
	// 
	QTest::newRow ("cpp-keep") << LuaRuntime::OwnedByCpp << flagBoth << true << false;
	QTest::newRow ("cpp-destroy") << LuaRuntime::OwnedByCpp << flagBoth << true << true;
	QTest::newRow ("lua-keep") << LuaRuntime::OwnedByLua << flagBoth << true << false;
	QTest::newRow ("lua-destroy") << LuaRuntime::OwnedByLua << flagBoth << true << true;
	
	QTest::newRow ("cpp-keep") << LuaRuntime::OwnedByCpp << flagCpp << true << false;
	QTest::newRow ("cpp-destroy") << LuaRuntime::OwnedByCpp << flagCpp << true << true;
	QTest::newRow ("lua-keep-not-called") << LuaRuntime::OwnedByLua << flagCpp << false << false;
	QTest::newRow ("lua-destroy-not-called") << LuaRuntime::OwnedByLua << flagCpp << false << true;
	
	
	QTest::newRow ("cpp-keep-not-called") << LuaRuntime::OwnedByCpp << flagLua << false << false;
	QTest::newRow ("cpp-destroy-not-called") << LuaRuntime::OwnedByCpp << flagLua << false << true;
	QTest::newRow ("lua-keep") << LuaRuntime::OwnedByLua << flagLua << true << false;
	QTest::newRow ("lua-destroy") << LuaRuntime::OwnedByLua << flagLua << true << true;
	
}

void LuaRuntimeTest::verifyObjectHandlerBehaviour () {
	NEEDS_TRIA;
	
	QFETCH(Nuria::LuaRuntime::Ownership, owner);
	QFETCH(Nuria::LuaRuntime::OwnershipFlags, mask);
	QFETCH(bool, called);
	QFETCH(bool, destroy);
	
	// 
	LuaRuntime::Ownership reportedOwner;
	void *reportedObject = nullptr;
	MetaObject *reportedMeta = nullptr;
	bool handlerCalled = false;
	auto handler = [&](LuaRuntime::Ownership o, void *obj, MetaObject *meta) {
		reportedOwner = o;
		reportedObject = obj;
		reportedMeta = meta;
		handlerCalled = true;
		return destroy;
	};
	
	// 
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	MetaObject *meta = MetaObject::byName ("TestObject");
	TestObject *object = new TestObject;
	
	// 
	bool takeOwnership = (owner == LuaRuntime::OwnedByLua);
	runtime.setObjectHandler (handler, mask);
	runtime.setGlobal ("foo", LuaObject::fromStructure (object, meta, &runtime, takeOwnership));
	
	if (takeOwnership) {
		QTest::ignoreMessage (QtDebugMsg, "~TestObject");
		QCOMPARE(object->parent (), &runtime);
	} else {
		QCOMPARE(object->parent (), (QObject *)nullptr);
	}
	
	// 
	QVERIFY(runtime.execute ("foo = nil"));
	runtime.collectGarbage ();
	runtime.collectGarbage ();
	
	// 
	QCOMPARE(handlerCalled, called);
	
	if (handlerCalled) {
		QCOMPARE(reportedOwner, owner);
		QCOMPARE(reportedObject, object);
		QCOMPARE(reportedMeta, meta);
	}
	
	// 
	if (!takeOwnership) {
		QTest::ignoreMessage (QtDebugMsg, "~TestObject");
		delete object;
	}
	
}

void LuaRuntimeTest::destroyOwnedQObject () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	MetaObject *meta = MetaObject::byName ("TestObject");
	TestObject *object = new TestObject;
	QSignalSpy spy (object, SIGNAL(destroyed()));
	
	// 
	QTest::ignoreMessage (QtDebugMsg, "~TestObject");
	runtime.setGlobal ("foo", LuaObject::fromStructure (object, meta, &runtime, true));
	QCOMPARE(object->parent (), &runtime);
	
	QVERIFY(runtime.execute ("foo = nil"));
	runtime.collectGarbage ();
	runtime.collectGarbage ();
	
	// 
	QCOMPARE(spy.length (), 1);
}

void LuaRuntimeTest::leaveUnownedQObjectAlone () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	MetaObject *meta = MetaObject::byName ("TestObject");
	TestObject *object = new TestObject;
	QSignalSpy spy (object, SIGNAL(destroyed()));
	
	// 
	runtime.setGlobal ("foo", LuaObject::fromStructure (object, meta, &runtime, false));
	QCOMPARE(object->parent (), (QObject *)nullptr);
	
	QVERIFY(runtime.execute ("foo = nil"));
	runtime.collectGarbage ();
	runtime.collectGarbage ();
	
	// 
	QCOMPARE(spy.length (), 0);
	QTest::ignoreMessage (QtDebugMsg, "~TestObject");
	delete object;
}

void LuaRuntimeTest::reownedQObjectNotDestroyed () {
	NEEDS_TRIA;
	
	LuaRuntime runtime (LuaRuntime::AllLibraries);
	MetaObject *meta = MetaObject::byName ("TestObject");
	TestObject *object = new TestObject;
	QSignalSpy spy (object, SIGNAL(destroyed()));
	
	// 
	runtime.setGlobal ("foo", LuaObject::fromStructure (object, meta, &runtime, true));
	QCOMPARE(object->parent (), &runtime);
	object->setParent (qApp); // Move ownership to another QObject
	
	QVERIFY(runtime.execute ("foo = nil"));
	runtime.collectGarbage ();
	runtime.collectGarbage ();
	
	// 
	QCOMPARE(spy.length (), 0);
}

QTEST_MAIN(LuaRuntimeTest)
#include "tst_luaruntime.moc"
