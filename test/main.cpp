#include "testutil.h"
#include <utils.h>
#include <log.h>

#include <QApplication>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTest>
#include <QVector>

namespace QTest {
	template<>
	char *toString(const SessionInfo & sessionInfo)
	{
		const QString s = "(" + sessionInfo.groupID
						+ "," + sessionInfo.authorID
						+ "," + QString::number(sessionInfo.validUntil.toTime_t())
						+")";
		return qstrdup(s.toLatin1().data());
	}

	template<>
	char *toString(const QMap<SessionID, SessionInfo> & sessionInfos)
	{
		QString s;
		Q_FOREACH(const SessionID& id, sessionInfos.keys()) {
			s += "ID: " + id + toString(sessionInfos[id]) + " , ";
		}

		return qstrdup(s.toLatin1().data());
	}
}

void testJSONParsing(const QString & data, const ApiResponse & expectedResponse, const GroupID & expectedGroupID )
{
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &error);
	if (error.error != QJsonParseError::NoError) {
		qDebug("There was an error reading the reply at position %d: %d %s", (int)error.offset, (int)error.error, qPrintable(error.errorString()));
		return;
	}
	const QJsonObject jsonObj = doc.object();
	const ApiResponse response(jsonObj.value("code").toDouble(), jsonObj.value("message").toString());
	const QJsonValue dataObj = jsonObj.value("data");
	QJsonObject payload;
	if (dataObj.isObject()) {
		payload = dataObj.toObject();
	}
	if (response != expectedResponse) {
		qFatal("Could not parse JSON! (Response)");
	}
	const PadID padID = PadID(payload.value("padID").toString());
	if (padID != expectedGroupID) {
		qFatal("Could not parse JSON! (PadID)");
	}
}

class BasicTests : public QObject
{
	Q_OBJECT
private Q_SLOTS:

	void base36Test() {
		QCOMPARE(Util::base36enc(1000000000L), QByteArray("gjdgxs"));
		QCOMPARE(Util::base36dec(Util::base36enc(1000000000L)), 1000000000ULL);
	}

	void JSONParsing() {
		testJSONParsing(QString::fromUtf8("{\"code\":0, \"message\":\"ok\", \"data\": {\"padID\": \"g.s8oes9dhwrvt0zif$test\"}}"), ApiResponse(0, "ok"), "g.s8oes9dhwrvt0zif$test");
		testJSONParsing(QString::fromUtf8("{\"code\":1, \"message\":\"pad does already exist\", \"data\": null}"), ApiResponse(1, "pad does already exist"), QString());
	}

	void parseSessionInfoTest() {
		SessionInfo expectedInfo("g.I4celRPygwriFJof", "a.8aXLnNIzoiDLx8M2", 1372093134);
		const QString sessionInfo("{\"groupID\":\""+expectedInfo.groupID+"\",\"authorID\":\""+expectedInfo.authorID+"\",\"validUntil\":"+QString::number(1372093134)+"}");
		QJsonDocument doc = QJsonDocument::fromJson(sessionInfo.toUtf8());
		const QJsonObject jsonObj = doc.object();
		SessionInfo info = Util::getSessionInfo(jsonObj);
		QCOMPARE(info, expectedInfo);
	}

	void parseListOfSessionInfosTest() {
		const int validInfo1 = 1372093134;
		SessionInfo expectedInfo1("g.I4celRPygwriFJof", "a.8aXLnNIzoiDLx8M2", validInfo1);
		const QString sessionInfo1("{\"groupID\":\""+expectedInfo1.groupID+"\",\"authorID\":\""+expectedInfo1.authorID+"\",\"validUntil\":"+QString::number(validInfo1)+"}");
		const int validInfo2 = 1374444444;
		SessionInfo expectedInfo2("g.I4celRPygwriFJof", "a.8aXLnNIzoiDLx8M2", validInfo2);
		const QString sessionInfo2("{\"groupID\":\""+expectedInfo2.groupID+"\",\"authorID\":\""+expectedInfo2.authorID+"\",\"validUntil\":"+QString::number(validInfo2)+"}");
		const SessionID sessionID1 = "s.YTi3iTvKaeSbh5E5";
		const SessionID sessionID2 = "s.LqE6GSNVJyApp49H";
		QMap<SessionID,SessionInfo> expectedInfo;
		expectedInfo.insert(sessionID1, expectedInfo1);
		expectedInfo.insert(sessionID2, expectedInfo2);

		const QString listOfSessionInfos = "{\""+sessionID1+"\":"+sessionInfo1+",\""+sessionID2+"\":"+sessionInfo2+"}";
		QJsonDocument doc = QJsonDocument::fromJson(listOfSessionInfos.toUtf8());
		const QJsonObject jsonObj = doc.object();
		QMap<SessionID,SessionInfo> info = Util::getListOfSessionInfos(jsonObj);
		QCOMPARE(info, expectedInfo);
	}
};

REGISTER_TEST(BasicTests);
#include "main.moc"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	TestUtil::TestRegistry::getInstance()->runTests(argc, argv);
}
