#pragma once

#include <QDateTime>
#include <QString>

typedef QString AuthorID;   // a.16RANDOMCHARS, for example a.s8oes9dhwrvt0zif
typedef QString GroupID;    // g.16RANDOMCHARS, for example g.s8oes9dhwrvt0zif
typedef QString PadID;      // GROUPID$PADNAME, for example the pad test of group g.s8oes9dhwrvt0zif has padID g.s8oes9dhwrvt0zif$test
typedef QString ReadOnlyID; // r.16RANDOMCHARS, for example r.s8oes9dhwrvt0zif
typedef QString SessionID;  // s.16RANDOMCHARS, for example s.s8oes9dhwrvt0zif

class ApiResponse {
public:
	ApiResponse(int code, const QString & message) : code(code), message(message) {}

	bool ok() const { return code == 0; }
	bool operator==(const ApiResponse& rhs) const { return code == rhs.code && message == rhs.message; }
	bool operator!=(const ApiResponse& rhs) const { return !operator==(rhs); }

	int code;
	QString message;
};

class SessionInfo {
public:
	SessionInfo() {}
	SessionInfo(const AuthorID & authorID, const GroupID & groupID, const uint validUntil)
		: authorID(authorID), groupID(groupID), validUntil(QDateTime::fromTime_t(validUntil)) {}

	bool operator==(const SessionInfo& rhs) const {
		return authorID == rhs.authorID
				&& groupID == rhs.groupID
				&& validUntil == rhs.validUntil;
	}
	bool operator!=(const SessionInfo& rhs) const { return !operator==(rhs); }

	AuthorID  authorID;
	GroupID   groupID;
	QDateTime validUntil;
};

class ChatMessage
{
public:
	QString   text;
	AuthorID  userId; // really the AuthorID?
	QDateTime time;
	QString   userName;
};

typedef QVector<ChatMessage> ChatMessages;

class PadUser
{
public:
	QString   colorId;
	QString   name;
	QDateTime timeStamp;
	AuthorID  id;
};

typedef QVector<PadUser> PadUsers;
