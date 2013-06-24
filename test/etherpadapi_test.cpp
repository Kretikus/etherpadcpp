#include "testutil.h"

#include <etherpadapi.h>
#include <log.h>

class TestObj : public QObject
{
Q_OBJECT

public Q_SLOTS:
	void groupResponse(const ApiResponse &, const GroupID id) { gId = id; }
	void sessionResponse(const ApiResponse &, const SessionID id) { sessions.push_back(id); }
	void createAuthorResponse(const ApiResponse &, const AuthorID & id) {authors.push_back(id); }

public:
	GroupID gId;
	QVector<AuthorID> authors;
	QVector<SessionID> sessions;
};

#define WAIT_UNTIL(g, maxTime) \
{ \
	QElapsedTimer timer; \
	timer.start(); \
	while(timer.elapsed() < maxTime) { \
		qApp->processEvents(); \
		if (g) break; \
	} \
} \

class EtherPadApiTest : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void listSessionsOfGroupTest()
	{
		EtherPadApi api;
		TestObj testObj;
		testObj.connect(&api, SIGNAL(createGroupResponse(ApiResponse,GroupID)), &testObj, SLOT(groupResponse(ApiResponse,GroupID)));
		testObj.connect(&api, SIGNAL(createSessionResponse(ApiResponse,SessionID)), &testObj, SLOT(sessionResponse(ApiResponse,SessionID)));
		testObj.connect(&api, SIGNAL(createAuthorResponse(ApiResponse,AuthorID)), &testObj, SLOT(createAuthorResponse(ApiResponse,AuthorID)));
		api.createGroup();

		WAIT_UNTIL(!testObj.gId.isEmpty(), 1000);
		QVERIFY(!testObj.gId.isEmpty());
		api.createAuthor("Author1");
		api.createAuthor("Author2");
		WAIT_UNTIL(testObj.authors.size() == 2, 1000);
		QCOMPARE(testObj.authors.size(), 2);

		const QDateTime validity = QDateTime::currentDateTime().addDays(1);
		api.createSession(testObj.gId, testObj.authors[0], validity);
		api.createSession(testObj.gId, testObj.authors[1], validity);
		WAIT_UNTIL(testObj.sessions.size() == 2, 1000);
		QCOMPARE(testObj.sessions.size(), 2);

		api.listSessionsOfGroup(testObj.gId);
	}
};

REGISTER_TEST(EtherPadApiTest);

#include "etherpadapi_test.moc"
