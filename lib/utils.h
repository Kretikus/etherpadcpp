#pragma once

#include "datatypes.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

namespace Util {

template<typename T>
T fromJSON(const QJsonValue&)
{
	T::create_a_specialisation();
}

template<>
inline QString fromJSON(const QJsonValue & v)
{
	return v.toString();
}

template<>
inline ChatMessage fromJSON(const QJsonValue & v)
{
	const QJsonObject obj = v.toObject();
	ChatMessage msg;
	msg.text     = obj.value("text").toString();
	msg.userId   = AuthorID(obj.value("userId").toString());
	msg.time     = QDateTime::fromTime_t(obj.value("time").toDouble());
	msg.userName = obj.value("userName").toString();
	return msg;
}

template<>
inline PadUser fromJSON(const QJsonValue & v)
{
	const QJsonObject obj = v.toObject();
	PadUser user;
	user.colorId   = obj.value("colorId").toString();
	user.name      = obj.value("name").toString();
	user.timeStamp = QDateTime::fromTime_t(obj.value("timeStamp").toDouble());
	user.id        = AuthorID(obj.value("id").toString());
	return user;
}

template<typename T>
QVector<T> getArrayFromPayload(const QJsonObject & payload, const QString & key)
{
	QVector<T> array;
	if (!payload.isEmpty()) {
		const QJsonArray ja = payload.value(key).toArray();
		array.reserve(ja.size());
		Q_FOREACH(const QJsonValue & v, ja) {
			array.push_back(fromJSON<T>(v));
		}
	}
	return array;
}

inline SessionInfo getSessionInfo(const QJsonObject & payload)
{
	return SessionInfo( AuthorID(payload.value("authorID").toString()),
						GroupID(payload.value("groupID").toString()),
						payload.value("validUntil").toDouble());
}

inline QMap<SessionID, SessionInfo> getListOfSessionInfos(const QJsonObject & payload)
{
	QMap<SessionID, SessionInfo> sessions;
	Q_FOREACH(const QString & key, payload.keys()) {
		const QJsonObject obj = payload.value(key).toObject();
		sessions.insert(SessionID(key), getSessionInfo(obj));
	}
	return sessions;
}

}
