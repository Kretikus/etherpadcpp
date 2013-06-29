#pragma once

#include <QString>
#include <QPair>
#include <QMap>
#include <Qvector>

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


class Changeset
{
public:
	enum Operation {
		InsertChars,    // +
		SkipOverChars,  // -
		KeepChars,      // =
		NewLine,        // |
		Attrib          // *
	};
	typedef QPair<Operation, int> Op;
	typedef QVector<Op> Ops;

	Changeset() : oldLength_(), newLength_() {}
	Changeset(int oldLength, int newLength, const Ops & ops, const QString & bank)
		: oldLength_(oldLength), newLength_(newLength)
		, ops_(ops), bank_(bank)
	{}

	QString toString() const;

	static const QString prefix;
	int oldLength_;
	int newLength_;
	Ops ops_;
	QString bank_;
};

namespace detail {
int getMaxPrefix(const QStringRef & oldText, const QStringRef & newText);
int getCharDeleteCount(const QStringRef & oldText, const QStringRef & newText);
Changeset optimizeChangeset(const Changeset & changeset, const QString & oldText, const QString & newText);
}


Changeset createChangeset(const QString & oldText, const QString & newText);
