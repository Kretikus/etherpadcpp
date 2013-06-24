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

template<> ChatMessage fromJSON(const QJsonValue & v);
template<> QString     fromJSON(const QJsonValue & v);
template<> PadUser     fromJSON(const QJsonValue & v);

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

SessionInfo getSessionInfo(const QJsonObject & payload);
QMap<SessionID, SessionInfo> getListOfSessionInfos(const QJsonObject & payload);

QByteArray base36enc(quint64 value);
quint64 base36dec(const QByteArray & text);

}
