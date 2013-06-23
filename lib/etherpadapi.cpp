#include "etherpadapi.h"

#include "utils.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVector>

const QString apiBaseUrl = "http://localhost:9001/api/1/";
const QString apiKey = "T0bZoBITgHPtzPAeH2t4Cx9JN2kvwRET";

namespace {
namespace ApiCall {
	enum type {
		//Group
		createGroup,
		createGroupIfNotExistsFor,
		deleteGroup,
		listPads,
		createGroupPad,
		listAllGroups,
		//Author
		createAuthor,
		createAuthorIfNotExistsFor,
		listPadsOfAuthor,
		getAuthorName,
		//Session
		createSession,
		deleteSession,
		getSessionInfo,
		listSessionsOfGroup,
		listSessionsOfAuthor,
		//Pad Content
		setText,
		getText,
		getHTML,
		//Chat
		getChatHistory,
		getChatHead,
		//Pad
		createPad,
		getRevisionsCount,
		padUsersCount,
		padUsers,
		deletePad,
		getReadOnlyID,
		setPublicStatus,
		getPublicStatus,
		setPassword,
		isPasswordProtected,
		listAuthorsOfPad,
		getLastEdited,
		sendClientsMessage,
		checkToken
	};
}

}

EtherPadApi::EtherPadApi(QObject *parent)
: QObject(parent)
, networkManager_(new QNetworkAccessManager(this))
{
}

void EtherPadApi::createGroup()
{
	doRequest(apiBaseUrl + "createGroup?apikey="+apiKey, ApiCall::createGroup);
}

void EtherPadApi::createGroupIfNotExistsFor(const QString & groupMapper)
{
	doRequest(apiBaseUrl + "createGroupIfNotExistsFor?apikey="+apiKey
			  +"&groupMapper="+groupMapper, ApiCall::createGroupIfNotExistsFor);
}

void EtherPadApi::deleteGroup(const GroupID & groupID)
{
	doRequest(apiBaseUrl + "deleteGroup?apikey="+apiKey
			  +"&groupID="+groupID, ApiCall::deleteGroup);
}

void EtherPadApi::listPads(const GroupID & groupID)
{
	doRequest(apiBaseUrl + "listPads?apikey="+apiKey
			  +"&groupID="+groupID, ApiCall::listPads);
}

void EtherPadApi::createGroupPad(const GroupID & groupID, const QString & padName, const QString & text)
{
	doRequest(apiBaseUrl + "createGroupPad?apikey="+apiKey
			  +"&groupID="+groupID
			  +"&padName="+padName
			  +"&text="+text, ApiCall::createGroupPad);
}

void EtherPadApi::listAllGroups()
{
	doRequest(apiBaseUrl + "listAllGroups?apikey="+apiKey, ApiCall::listAllGroups);
}

void EtherPadApi::createAuthor(const QString & name)
{
	doRequest(apiBaseUrl + "createAuthor?apikey="+apiKey
			  +"&name="+name, ApiCall::createAuthor);
}

void EtherPadApi::createAuthorIfNotExistsFor(const QString & authorMapper, const QString & name)
{
	doRequest(apiBaseUrl + "createAuthorIfNotExistsFor?apikey="+apiKey
			  +"&authorMapper="+authorMapper
			  +"&name="+name, ApiCall::createAuthorIfNotExistsFor);
}

void EtherPadApi::listPadsOfAuthor(const AuthorID & authorID)
{
	doRequest(apiBaseUrl + "listPadsOfAuthor?apikey="+apiKey
			  +"&authorID="+authorID, ApiCall::listPadsOfAuthor);
}

void EtherPadApi::getAuthorName(const AuthorID & authorID)
{
	doRequest(apiBaseUrl + "getAuthorName?apikey="+apiKey
			  +"&authorID="+authorID, ApiCall::getAuthorName);
}

void EtherPadApi::createSession(const GroupID & groupID, const QString & authorID, const QDateTime & validUntil)
{
	doRequest(apiBaseUrl + "createSession?apikey="+apiKey
			  +"&groupID="+groupID
			  +"&authorID="+authorID
			  +"&validUntil="+QString::number(validUntil.toTime_t()), ApiCall::createSession);
}

void EtherPadApi::deleteSession(const SessionID & sessionID)
{
	doRequest(apiBaseUrl + "deleteSession?apikey="+apiKey
			  +"&sessionID="+sessionID, ApiCall::deleteSession);
}

void EtherPadApi::getSessionInfo(const SessionID & sessionID)
{
	doRequest(apiBaseUrl + "getSessionInfo?apikey="+apiKey
			  +"&sessionID="+sessionID, ApiCall::getSessionInfo);
}

void EtherPadApi::listSessionsOfGroup(const GroupID & groupID)
{
	doRequest(apiBaseUrl + "listSessionsOfGroup?apikey="+apiKey
			  +"&groupID="+groupID, ApiCall::listSessionsOfGroup);
}

void EtherPadApi::listSessionsOfAuthor(const AuthorID & authorID)
{
	doRequest(apiBaseUrl + "listSessionsOfAuthor?apikey="+apiKey
			  +"&authorID="+authorID, ApiCall::listSessionsOfAuthor);
}

void EtherPadApi::setText(const PadID & padID, const QString & text)
{
	doRequest(apiBaseUrl + "setText?apikey="+apiKey
			  +"&padID="+padID
			  +"&text="+text, ApiCall::setText);
}

void EtherPadApi::getText(const PadID & padID, const QString & rev)
{
	doRequest(apiBaseUrl + "getText?apikey="+apiKey
			  +"&padID="+padID
			  +"&rev="+rev, ApiCall::getText);
}

void EtherPadApi::getHTML(const PadID & padID, const QString & rev)
{
	doRequest(apiBaseUrl + "getHTML?apikey="+apiKey
			  +"&padID="+padID
			  +"&rev="+rev, ApiCall::getHTML);
}

void EtherPadApi::getChatHistory(const PadID & padID, const QString & start, const QString & end)
{
	doRequest(apiBaseUrl + "getChatHistory?apikey="+apiKey
			  +"&padID="+padID
			  +"&start="+start
			  +"&end="+end, ApiCall::getChatHistory);
}

void EtherPadApi::getChatHead(const PadID & padID)
{
	doRequest(apiBaseUrl + "getChatHead?apikey="+apiKey
			  +"&padID="+padID, ApiCall::getChatHead);
}

void EtherPadApi::createPad(const PadID & padID, const QString & text)
{
	doRequest(apiBaseUrl + "createPad?apikey="+apiKey
			  +"&padID="+padID
			  +"&text="+text, ApiCall::createPad);
}

void EtherPadApi::getRevisionsCount(const PadID & padID)
{
	doRequest(apiBaseUrl + "getRevisionsCount?apikey="+apiKey
			  +"&padID="+padID, ApiCall::getRevisionsCount);
}

void EtherPadApi::padUsersCount(const PadID & padID)
{
	doRequest(apiBaseUrl + "padUsersCount?apikey="+apiKey
			  +"&padID="+padID, ApiCall::padUsersCount);
}

void EtherPadApi::padUsers(const PadID & padID)
{
	doRequest(apiBaseUrl + "padUsers?apikey="+apiKey
			  +"&padID="+padID, ApiCall::padUsers);
}

void EtherPadApi::deletePad(const PadID & padID)
{
	doRequest(apiBaseUrl + "deletePad?apikey="+apiKey
			  +"&padID="+padID, ApiCall::deletePad);
}

void EtherPadApi::getReadOnlyID(const PadID &padID)
{
	doRequest(apiBaseUrl + "getReadOnlyID?apikey="+apiKey
			  +"&padID="+padID, ApiCall::getReadOnlyID);
}

void EtherPadApi::setPublicStatus(const PadID & padID, bool publicStatus)
{
	doRequest(apiBaseUrl + "setPublicStatus?apikey="+apiKey
			  +"&padID="+padID
			  +"&publicStatus=" + (publicStatus ? "1" : "0"), ApiCall::setPublicStatus);
}

void EtherPadApi::getPublicStatus(const PadID & padID)
{
	doRequest(apiBaseUrl + "getPublicStatus?apikey="+apiKey
			  +"&padID="+padID, ApiCall::getPublicStatus);
}

void EtherPadApi::setPassword(const PadID & padID, const QString & password)
{
	doRequest(apiBaseUrl + "setPassword?apikey="+apiKey
			  +"&padID="+padID
			  +"&password="+password, ApiCall::setPassword);
}

void EtherPadApi::isPasswordProtected(const PadID & padID)
{
	doRequest(apiBaseUrl + "isPasswordProtected?apikey="+apiKey
			  +"&padID="+padID, ApiCall::isPasswordProtected);
}

void EtherPadApi::listAuthorsOfPad(const PadID & padID)
{
	doRequest(apiBaseUrl + "listAuthorsOfPad?apikey="+apiKey
			  +"&padID="+padID, ApiCall::listAuthorsOfPad);
}

void EtherPadApi::getLastEdited(const PadID & padID)
{
	doRequest(apiBaseUrl + "getLastEdited?apikey="+apiKey
			  +"&padID="+padID, ApiCall::getLastEdited);
}

void EtherPadApi::sendClientsMessage(const PadID & padID, const QString & msg)
{
	doRequest(apiBaseUrl + "sendClientsMessage?apikey="+apiKey
			  +"&padID="+padID
			  +"&msg="+msg, ApiCall::sendClientsMessage);
}

void EtherPadApi::checkToken()
{
	doRequest(apiBaseUrl + "checkToken?apikey="+apiKey, ApiCall::checkToken);
}

void EtherPadApi::doRequest(const QString & req, const int apiCallNr)
{
	qDebug("Doing request: %s", qPrintable(req));
	QNetworkRequest request(req);
	request.setAttribute(QNetworkRequest::User, QVariant::fromValue(apiCallNr));
	QNetworkReply * reply = networkManager_->get(request);
	connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(finished()), SLOT(handleNetworkReply()));
}

void EtherPadApi::sslErrors(const QList<QSslError> sslErrors)
{
	qDebug("SSL Error: %s", qPrintable(sslErrors.first().errorString()));
	// TODO
}

void EtherPadApi::error(QNetworkReply::NetworkError error)
{
	qDebug("Network Error: %d", (int)error);
	// TODO
}

void EtherPadApi::handleNetworkReply()
{
	QNetworkReply * reply = qobject_cast<QNetworkReply*>(sender());
	if (!reply) return;

	const QByteArray replyData(reply->readAll());
	qDebug("Network Reply: %s", replyData.data());

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(replyData, &error);
	if (error.error != QJsonParseError::NoError) {
		qDebug("There was an error reading the reply at position %d: %d %s", (int)error.offset, (int)error.error, qPrintable(error.errorString()));
	}

	const QJsonObject jsonObj = doc.object();
	const ApiResponse response(jsonObj.value("code").toDouble(), jsonObj.value("message").toString());
	const QJsonValue dataObj = jsonObj.value("data");
	QJsonObject payload;
	if (dataObj.isObject()) {
		payload = dataObj.toObject();
	}
	const ApiCall::type apiType = static_cast<ApiCall::type>(reply->request().attribute(QNetworkRequest::User).toInt());
	switch(apiType) {
		case ApiCall::createGroup:
		case ApiCall::createGroupIfNotExistsFor:
			emit createGroupResponse(response, GroupID(payload.value("groupID").toString()));
			break;
		case ApiCall::deleteGroup:
			emit deleteGroupResponse(response);
			break;
		case ApiCall::listPads:
			emit listPadsResponse(response, Util::getArrayFromPayload<PadID>(payload, "padIDs") );
			break;
		case ApiCall::createGroupPad:
			emit createGroupPadResponse(response, PadID(payload.value("padID").toString()) );
			break;
		case ApiCall::listAllGroups:
			emit listAllGroupsResponse(response, Util::getArrayFromPayload<GroupID>(payload, "groupIDs"));
			break;
		case ApiCall::createAuthor:
		case ApiCall::createAuthorIfNotExistsFor:
			emit createAuthorResponse(response, AuthorID(payload.value("authorID").toString()));
			break;
		case ApiCall::listPadsOfAuthor:
			emit listPadsOfAuthorResponse(response, Util::getArrayFromPayload<PadID>(payload, "padIDs"));
			break;
		case ApiCall::getAuthorName:
			emit getAuthorNameResponse(response, payload.value("authorName").toString());
			break;
		case ApiCall::createSession:
			emit createSessionResponse(response, SessionID(payload.value("sessionID").toString()));
			break;
		case ApiCall::deleteSession:
			emit deleteSessionResponse(response);
			break;
		case ApiCall::getSessionInfo:
			emit getSessionInfoResponse(response, Util::getSessionInfo(payload));
			break;
		case ApiCall::listSessionsOfGroup:
				emit listSessionsOfGroupResponse(response, Util::getListOfSessionInfos(payload));
			break;
		case ApiCall::listSessionsOfAuthor:
			emit listSessionsOfAuthorResponse(response, Util::getListOfSessionInfos(payload));
			break;
		case ApiCall::setText:
			emit setTextResponse(response);
			break;
		case ApiCall::getText:
			emit getTextResponse(response, payload.value("text").toString());
			break;
		case ApiCall::getHTML:
			emit getTextResponse(response, payload.value("html").toString());
			break;
		case ApiCall::getChatHistory:
			emit getChatHistoryResponse(response, Util::getArrayFromPayload<ChatMessage>(payload, "messages"));
			//{"code":0,"message":"ok","data":{"messages":[{"text":"foo","userId":"a.foo","time":1359199533759,"userName":"test"},{"text":"bar","userId":"a.foo","time":1359199534622,"userName":"test"}]}}
			break;
		case ApiCall::getChatHead:
			emit getChatHeadResponse(response, payload.value("chatHead").toDouble());
			break;
		case ApiCall::createPad:
			emit createPadContentResponse(response);
			break;
		case ApiCall::getRevisionsCount:
			emit getRevisionsCountResponse(response, payload.value("revisions").toDouble());
			break;
		case ApiCall::padUsersCount:
			emit padUsersCountResponse(response, payload.value("padUsersCount").toDouble());
			break;
		case ApiCall::padUsers:
			emit padUsersResponse(response, Util::getArrayFromPayload<PadUser>(payload, "padUsers"));
			//{code: 0, message:"ok", data: {padUsers: [{colorId:"#c1a9d9","name":"username1","timestamp":1345228793126,"id":"a.n4gEeMLsvg12452n"},{"colorId":"#d9a9cd","name":"Hmmm","timestamp":1345228796042,"id":"a.n4gEeMLsvg12452n"}]}}`
			break;
		case ApiCall::deletePad:
			emit deletePadResponse(response);
			break;
		case ApiCall::getReadOnlyID:
			emit getReadOnlyIDResponse(response, ReadOnlyID(payload.value("readOnlyID").toString()));
			break;
		case ApiCall::setPublicStatus:
			emit setPublicStatusResponse(response);
			break;
		case ApiCall::getPublicStatus:
			emit getPublicStatusResponse(response, payload.value("publicStatus").toBool());
			break;
		case ApiCall::setPassword:
			emit setPasswordResponse(response);
			break;
		case ApiCall::isPasswordProtected:
			emit isPasswordProtectedResponse(response, payload.value("passwordProtection").toBool());
			break;
		case ApiCall::listAuthorsOfPad:
			emit listAuthorsOfPadResponse(response, Util::getArrayFromPayload<AuthorID>(payload, "authorIDs"));
			break;
		case ApiCall::getLastEdited:
			emit getLastEditedResponse(response, QDateTime::fromTime_t((uint)payload.value("lastEdited").toDouble()) );
			break;
		case ApiCall::sendClientsMessage:
			emit sendClientsMessageResponse(response);
			break;
		case ApiCall::checkToken:
			emit checkTokenResponse(response);
			break;
	}
}
