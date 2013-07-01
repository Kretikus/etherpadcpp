#include "testutil.h"

#include <log.h>

class LogTest : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void loggingTest()
	{
		int val  = 888;
		int val2 = 777;
		LOG_TRACE("TEST %1%", val);
		LOG_TRACE("Another %1% Test with %2%", val, val2);
		
		QCOMPARE(LogDetails::filenameSubStr("/usr/local/impl.data"), "impl.data");
	}
};

REGISTER_TEST(LogTest);

#include "log_test.moc"
