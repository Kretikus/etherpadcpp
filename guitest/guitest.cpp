#include "../test/testutil.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	TestUtil::TestRegistry::getInstance()->runTests(argc, argv);
}
