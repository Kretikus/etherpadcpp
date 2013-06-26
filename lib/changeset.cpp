#include "changeset.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

void AttributePool::fromJSON(const QJsonObject & obj)
{
	if (!obj.contains("nextNum") || !obj.contains("numToAttrib")) {
		qDebug("Could not parse AttributePool");
		return;
	}
	nextNum_ = obj.value("nextNum").toDouble();

	numToAttrib_.clear();
	const QJsonObject numToAtribObj =  obj.value("numToAttrib").toObject();
	Q_FOREACH(const QString & key, numToAtribObj.keys()) {
		const int keyAsInt = key.toInt();
		const QJsonArray arr = numToAtribObj.value(key).toArray();
		if (arr.size() != 2) {
			qDebug("No key value pair in key: %s", qPrintable(key));
			continue;
		}
		numToAttrib_[keyAsInt] = qMakePair(arr[0].toString(), arr[1].toString());
	}
}
