#include "testutil.h"

#include <QList>
#include <QPair>
#include <QString>

namespace TestUtil {

class TestRegistry::Impl
{
public:
	typedef QPair<QString, QObject*> Name2Test;
	QList<Name2Test> tests_;
};

TestRegistry::TestRegistry()
	: impl_(new Impl)
{
}

TestRegistry::~TestRegistry()
{
	delete impl_;
}

TestRegistry* TestRegistry::getInstance() {
	static TestRegistry registry;
	return &registry;
}

void TestRegistry::registerTest(QString const& name, QObject* test) {
	impl_->tests_ << qMakePair(name, test);
}

void TestRegistry::printSuits()
{
	for( QList<Impl::Name2Test>::const_iterator it = impl_->tests_.begin(); it != impl_->tests_.end(); ++it)
	{
		printf( "%s", qPrintable(it->first) );
		if (it == impl_->tests_.begin()) {
			printf(" (Default)");
		}
		printf("\n");
	}
}

namespace {
void additionalOptions() {
	printf(" -suits              : Returns a list of current test suits\n");
	printf(" -s name             : Select a specific suite\n");
}
}

int TestRegistry::runTests(int argc, char* argv[])
{
	QString singleSuite;

	QVector<char*> args;
	for( int i = 0; i < argc; ++i ) {
		if( strcmp(argv[i], "-help") == 0 ) {
				atexit(&additionalOptions);
				QTest::qExec(new QObject(), argc, argv);
				return 0;
		} else if( strcmp(argv[i], "-suits") == 0 ) {
				printSuits();
				return 0;
		} else if( strcmp(argv[i], "-s") == 0 ) {
			if( i+1 < argc ) {
				++i;
				singleSuite = QString(argv[i]);
			}
		}
		else {
			args.push_back(argv[i]);
		}
	}
	argc = args.size();

	int result = 0;
	for (QList<Impl::Name2Test>::const_iterator it = impl_->tests_.begin(); it != impl_->tests_.end(); ++it) {
		if (singleSuite.isEmpty() || it->first == singleSuite) {
				result |= QTest::qExec(it->second, argc, args.data());
		}
	}
	return result;
}

}
