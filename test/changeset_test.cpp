#include <changeset.h>
#include <log.h>

#include "testutil.h"

#include <QMetaType>
#include <QJsonObject>

Q_DECLARE_METATYPE(Changeset)

namespace QTest {
template<>
char *toString(const Attribute & attr)
{
	const QString s = "(" + attr.first
					+ "," + attr.second
					+")";
	return qstrdup(s.toLatin1().data());
}

template<>
char *toString(const JS::DiffOut & d)
{
	QString s;
	foreach (const JS::DiffOutData & data, d) {
		s += "[";
		s += data.text + ", " + QString::number(data.row) + "]";
	}
	return qstrdup(s.toLatin1().data());
}
}

QJsonObject getObjectFromString(const QString & data); // defined in main.cpp

class ChangesetTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void parseJSONAttributeTest() {
		QString data = "{'nextNum': '2', 'numToAttrib': {'0': ['foo','bar'], '1': ['Wurst','Ja']}}";
		data.replace("'", "\"");
		AttributePool pool;
		QJsonObject obj = getObjectFromString(data);
		QVERIFY(obj != QJsonObject());
		pool.fromJSON(obj);
		QCOMPARE(pool.getAttrib(0), qMakePair(QString("foo"),   QString("bar")));
		QCOMPARE(pool.getAttrib(1), qMakePair(QString("Wurst"), QString("Ja")));
	}

	void generateChangesetTest_data() {
		QTest::addColumn<QString>("original");
		QTest::addColumn<QString>("result");
		QTest::addColumn<QString>("changeset");

		QTest::newRow("Identity (no change)")          << "foo" << "foo" << "Z:3>0$";
		QTest::newRow("Single letter append")          << "fo" << "foo" << "Z:2>1=1*0+1$o";
//		QTest::newRow("Three letter append")           << "foo" << "foobar" << "Z:3>3=3*0+3$bar";
//		QTest::newRow("Insert at beginning")           << "barbaz" << "foobarbaz" << "Z:6>3*0+3$foo";
//		QTest::newRow("Three letter insert to center") << "foobaz " << "foobarbaz" << "Z:6>3=3*0+3$bar";
//		QTest::newRow("Many letter append")            << "foo" << "foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar" << "Z:3>1r=3*0+1r$barfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar";
//		QTest::newRow("Empty string append")           << "" << "foobar" << "Z:0>6*0+6$foobar";
//		QTest::newRow("Word append")                   << "foo bar baz" << "foo extra bar baz" << "Z:b>6=4*0+6$extra ";
//		QTest::newRow("Word append on new line")       << "foo\nbar\nbaz\n" << "foo\nbar\nbing\nbaz\n" << "Z:c>5|1=7*0|1+5$\nbing";
//		QTest::newRow("Multiple whitespace characters ") << "foo\n bar " << "foo\n baz\nbar " << "Z:8>4|1=5*0|1+4$baz\n";
	}

	void generateChangesetTest() {
		QFETCH(QString, original);
		QFETCH(QString, result);
		QFETCH(QString, changeset);

		QCOMPARE(JS::createChangeset(original, result).toString(), changeset);
	}

#define OPS Changeset::Ops()
#define INSERT(num, n, a)  << qMakePair(Changeset::InsertChars, Changeset::OperationData(num, n, a))
#define KEEP(num)          << qMakePair(Changeset::KeepChars,   Changeset::OperationData(num, -1, -1))
#define KEEPNL(num, n)     << qMakePair(Changeset::KeepChars,   Changeset::OperationData(num, n, -1))

	void changesetToStringTest_data() {
		QTest::addColumn<QString  >("changesetString");
		QTest::addColumn<Changeset>("changeset");

		QTest::newRow("Identity (no change)")            << "Z:3>0$"          << Changeset(3, 3, OPS, "");
		QTest::newRow("Single letter append")            << "Z:2>1=1*0+1$o"   << Changeset(2, 3, OPS KEEP(1) INSERT(1, -1 , 0), "o");
		QTest::newRow("Three letter append")             << "Z:3>3=3*0+3$bar" << Changeset(3, 6, OPS KEEP(3) INSERT(3, -1, 0), "bar");
		QTest::newRow("Insert at beginning")             << "Z:6>3*0+3$foo"   << Changeset(6, 9, OPS INSERT(3, -1, 0), "foo");
		QTest::newRow("Three letter insert to center")   << "Z:6>3=3*0+3$bar" << Changeset(6, 9, OPS KEEP(3) INSERT(3, -1, 0), "bar");
		QTest::newRow("Many letter append")              << "Z:3>1r=3*0+1r$barfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar"
														 << Changeset(3, 66, OPS KEEP(3) INSERT(63, -1, 0), "barfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar");
		QTest::newRow("Empty string append")             << "Z:0>6*0+6$foobar" << Changeset(0, 6, OPS INSERT(6, -1, 0), "foobar");
		QTest::newRow("Word append")                     << "Z:b>6=4*0+6$extra" << Changeset(11, 17, OPS KEEP(4) INSERT(6, -1, 0), "extra");
		QTest::newRow("Word append on new line")         << "Z:c>5|1=7*0|1+5$\nbing" << Changeset(12, 17, OPS KEEPNL(7, 1) INSERT(5, 1, 0), "\nbing");
		QTest::newRow("Multiple whitespace characters ") << "Z:8>4|1=5*0|1+4$baz\n"  << Changeset(8, 12, OPS KEEPNL(5, 1) INSERT(4, 1, 0), "baz\n");
		QTest::newRow("Multiple whitespace characters ") << "Z:9k<4o|1=5*0|1+4$baz\n"  << Changeset(344, 176, OPS KEEPNL(5, 1) INSERT(4, 1, 0), "baz\n");
	}
	
	void changesetToStringTest() {
		QFETCH(QString,   changesetString);
		QFETCH(Changeset, changeset);

		QCOMPARE(changeset.toString(), changesetString);
		QCOMPARE(Changeset::fromString(changesetString).toString(), changeset.toString());
	}

	void miscStringToolTest() {
		{
			QString l("foo");
			QString r("fobingo");
			QCOMPARE(detail::getMaxPrefix(&l, &r), 2);
		}
		
	}

	void JS_collapseTest_data()
	{
	
	}

	void JS_collapseTest()
	{
	
	}

	void JS_diffTest()
	{
		{
			QStringList o = {"foo"};
			QStringList n = {"foo"};
			JS::DiffOut oDo;
			oDo.append(JS::DiffOutData("foo", 0));
			QCOMPARE(JS::diff(o, n).first, oDo);
			QCOMPARE(JS::diff(o, n).second, oDo);
		}

		{
			QStringList o = {"<p>Hello","World</p>\n\n"};
			QStringList n = {"<p>Hello","My","World</p>\n\n"};
			JS::DiffOut oDo;
			oDo.append( JS::DiffOutData("<p>Hello", 0));
			oDo.append( JS::DiffOutData("World</p>\n\n", 2));
			QCOMPARE(JS::diff(o, n).first, oDo);
			
			JS::DiffOut nDo;
			nDo.append(JS::DiffOutData("<p>Hello",0));
			nDo.append(JS::DiffOutData("My"));
			nDo.append(JS::DiffOutData("World</p>\n\n",1));
			QCOMPARE(JS::diff(o, n).second, nDo);
		}
	}
};

REGISTER_TEST(ChangesetTest)

#include "changeset_test.moc"
