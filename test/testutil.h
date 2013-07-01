#pragma once

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

#include <QTest>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace TestUtil {

class TestRegistry {
public:
	/**
	 * Retrieve the single instance of the registry.
	 */
	static TestRegistry* getInstance();

	/**
	 * Register a QtTest test. 
	 * This method is called  by QTTESTUTIL_REGISTER_TEST, and you should 
	 * not use this method directly.
	 */
	void registerTest(QString const& name, QObject*);

	/**
	 * Run all registered tests using QTest::qExec()
	 */
	int runTests(int argc, char* argv[]);

private:
	TestRegistry();
	~TestRegistry();
	void printSuits();

private:
	class Impl;
	Impl* impl_;
};

template<typename TestClass>
class TestRegistration {
public:
	TestRegistration(char const* n ) {
			test_ = new TestClass();
			TestRegistry::getInstance()->registerTest(n, test_);
	}

	~TestRegistration() {
			delete test_;
	}

private:
	TestClass* test_;
};

}

#define REGISTER_TEST(TestClass) \
	static TestUtil::TestRegistration<TestClass> TestClass##Registration(#TestClass);
