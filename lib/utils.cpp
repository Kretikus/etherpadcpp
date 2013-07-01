#include "utils.h"

#include <QMap>

template<>
QString Util::fromJSON(const QJsonValue & v)
{
	return v.toString();
}

template<>
ChatMessage Util::fromJSON(const QJsonValue & v)
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
PadUser Util::fromJSON(const QJsonValue & v)
{
	const QJsonObject obj = v.toObject();
	PadUser user;
	user.colorId   = obj.value("colorId").toString();
	user.name      = obj.value("name").toString();
	user.timeStamp = QDateTime::fromTime_t(obj.value("timeStamp").toDouble());
	user.id        = AuthorID(obj.value("id").toString());
	return user;
}

SessionInfo Util::getSessionInfo(const QJsonObject & payload)
{
	return SessionInfo( AuthorID(payload.value("authorID").toString()),
						GroupID(payload.value("groupID").toString()),
						payload.value("validUntil").toDouble());
}

QMap<SessionID, SessionInfo> Util::getListOfSessionInfos(const QJsonObject & payload)
{
	QMap<SessionID, SessionInfo> sessions;
	Q_FOREACH(const QString & key, payload.keys()) {
		const QJsonObject obj = payload.value(key).toObject();
		sessions.insert(SessionID(key), getSessionInfo(obj));
	}
	return sessions;
}

QByteArray Util::base36enc(quint64 value)
{
	char base36[37] = "0123456789abcdefghijklmnopqrstuvwxyz";
	/* log(2**64) / log(36) = 12.38 => max 13 char + '\0' */
	char buffer[14];
	unsigned int offset = sizeof(buffer);
	
	buffer[--offset] = '\0';
	do {
		buffer[--offset] = base36[value % 36];
	} while (value /= 36);
	
	return QByteArray(&buffer[offset], -1);
}
 
quint64 Util::base36dec(const QByteArray & text)
{
	return strtoul(text.constData(), NULL, 36);
}

