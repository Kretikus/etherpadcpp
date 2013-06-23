#pragma once

#include "datatypes.h"

#include <QObject>
#include <QNetworkReply>
#include <QSslError>

class QNetworkAccessManager;

// async calls to etherpad server
class EtherPadApi : public QObject
{
	Q_OBJECT
public:
	EtherPadApi(QObject * parent = 0);

	//Group functions
	void createGroup(); // creates a new group
	// this functions helps you to map your application group ids to etherpad lite group ids
	void createGroupIfNotExistsFor(const QString & groupMapper);
	void deleteGroup(const GroupID & groupID);
	void listPads(const GroupID & groupID);
	void createGroupPad(const GroupID & groupID, const QString & padName, const QString & text);
	void listAllGroups();

	//Author
	void createAuthor(const QString & name);
	 // this functions helps you to map your application author ids to etherpad lite author ids
	void createAuthorIfNotExistsFor(const QString & authorMapper, const QString & name);
	void listPadsOfAuthor(const AuthorID & authorID);
	void getAuthorName(const AuthorID & authorID);

	//Session
	void createSession(const GroupID & groupID ,const QString & authorID, const QDateTime & validUntil);
	void deleteSession(const SessionID & sessionID);
	void getSessionInfo(const SessionID & sessionID);
	void listSessionsOfGroup(const GroupID & groupID);
	void listSessionsOfAuthor(const AuthorID & authorID);

	//Pad Content
	void setText(const PadID & padID, const QString & text);
	void getText(const PadID & padID, const QString & rev);
	void getHTML(const PadID & padID, const QString & rev);

	//Chat
	 /* returns * a part of the chat history, when `start` and `end` are given or the whole chat histroy, when no extra parameters are given */
	void getChatHistory(const PadID & padID, const QString & start, const QString & end);
	void getChatHead(const PadID & padID);

	//Pad
	// creates a new (non-group) pad. Note that if you need to create a group Pad, you should call createGroupPad.
	void createPad(const PadID & padID, const QString & text);
	void getRevisionsCount(const PadID & padID);
	void padUsersCount(const PadID & padID);
	void padUsers(const PadID & padID);
	void deletePad(const PadID & padID);
	void getReadOnlyID(const PadID & padID);
	void setPublicStatus(const PadID & padID, bool publicStatus);
	void getPublicStatus(const PadID & padID);
	void setPassword(const PadID & padID, const QString & password);
	void isPasswordProtected(const PadID & padID);
	void listAuthorsOfPad(const PadID & padID);
	void getLastEdited(const PadID & padID);
	void sendClientsMessage(const PadID & padID, const QString & msg);
	 // returns ok when the current api token is valid
	void checkToken();

signals:
	// Groups
	void createGroupResponse(const ApiResponse & response, const GroupID & groupID); // also for createGroupIfNotExistsFor
	void deleteGroupResponse(const ApiResponse & response);
	void listPadsResponse(const ApiResponse & response, const QVector<PadID> & groupIDs);
	void createGroupPadResponse(const ApiResponse & response, const PadID & padID);
	void listAllGroupsResponse(const ApiResponse & response, const QVector<GroupID> & groupIDs);
	// Author
	void createAuthorResponse(const ApiResponse & response, const AuthorID & authorID);
	void listPadsOfAuthorResponse(const ApiResponse & response, const QVector<PadID> & padIDs);
	void getAuthorNameResponse(const ApiResponse & response, const QString & authorName);
	//session
	void createSessionResponse(const ApiResponse & response, const SessionID & sessionId);
	void deleteSessionResponse(const ApiResponse & response);
	void getSessionInfoResponse(const ApiResponse & response, const SessionInfo & sessionInfo);
	void listSessionsOfGroupResponse(const ApiResponse & response, const QMap<SessionID, SessionInfo> & sessions);
	void listSessionsOfAuthorResponse(const ApiResponse & response, const QMap<SessionID, SessionInfo> & sessions);
	//pad content
	void setTextResponse(const ApiResponse & response);
	void getTextResponse(const ApiResponse & response, const QString & text);
	void getHtmlResponse(const ApiResponse & response, const QString & html);
	//chat
	void getChatHistoryResponse(const ApiResponse & response, const ChatMessages & messages);
	void getChatHeadResponse(const ApiResponse & response, int chatHead);
	//pad
	void createPadContentResponse(const ApiResponse & response);
	void getRevisionsCountResponse(const ApiResponse & response, int revisions);
	void padUsersCountResponse(const ApiResponse & response, int padUsersCount);
	void padUsersResponse(const ApiResponse & response, const PadUsers & padUsers);
	void deletePadResponse(const ApiResponse & response);
	void getReadOnlyIDResponse(const ApiResponse & response, const ReadOnlyID & readOnlyID);
	void setPublicStatusResponse(const ApiResponse & response);
	void getPublicStatusResponse(const ApiResponse & response, bool publicStatus);
	void setPasswordResponse(const ApiResponse & response);
	void isPasswordProtectedResponse(const ApiResponse & response, bool passwordProtection);
	void listAuthorsOfPadResponse(const ApiResponse & response, const QVector<AuthorID> & authorIDs);
	void getLastEditedResponse(const ApiResponse & response, const QDateTime & lastEdited);
	void sendClientsMessageResponse(const ApiResponse & response);
	void checkTokenResponse(const ApiResponse & response);

private:
	void doRequest(const QString & req, int apiCallNr);

private slots:
	void sslErrors(const QList<QSslError> sslErrors);
	void error(QNetworkReply::NetworkError error);
	void handleNetworkReply();

private:
	QNetworkAccessManager * networkManager_;
};
