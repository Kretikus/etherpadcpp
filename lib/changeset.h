#pragma once

#include <QString>
#include <QPair>
#include <QMap>
#include <QVector>

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

	class OperationData
	{
	public:
		OperationData() : opLength(-1), newlines(-1), attrib(-1) {}
		OperationData(int opLength, int newLines, int attrib)
			: opLength(opLength), newlines(newLines), attrib(attrib) {}
		int opLength;
		int newlines;
		int attrib;
	};

	enum Operation {
		InsertChars,    // +
		SkipOverChars,  // -
		KeepChars       // =
	};
	typedef QPair<Operation, OperationData> Op;
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


namespace JS {
int newLines(const QString & text);

class DiffOutData {
public:
	DiffOutData() : row(-1) {}
	DiffOutData(const QString & t) : text(t), row(-1) {}
	DiffOutData(const QString & t, int r) : text(t), row(r) {}

	bool operator ==(const DiffOutData & rhs) const {
		return text == rhs.text && row == rhs.row;
	}

	bool isValid() const { return row != 1; }

	QString text;
	int row;
};
typedef QVector<DiffOutData> DiffOut;

QPair<DiffOut, DiffOut> diff(const QStringList & oldText, const QStringList & newText);
Changeset createChangeset(const QString & oldText, const QString & newText);
Changeset optimizeChangeset(const QString & oldText, const Changeset & changeset);
}

