#pragma once

#include <QString>
#include <QPair>
#include <QMap>

class QJsonObject;

typedef QPair<QString,QString> Attribute;

class AttributePool
{
public:
	AttributePool() : nextNum_(0) {}

	Attribute getAttrib(int num) const { return numToAttrib_[num]; }
	QString getAttribKey(int num) const  { return numToAttrib_[num].first; }
	QString getAttribValue(int num) const   { return numToAttrib_[num].second; }

	void fromJSON(const QJsonObject & obj);

private:
	int nextNum_;
	QMap<int, Attribute> numToAttrib_;
};
