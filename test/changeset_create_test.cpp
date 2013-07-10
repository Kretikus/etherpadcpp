#include <changeset.h>
#include <log.h>

#include "testutil.h"

#include <QMetaType>
#include <QJsonObject>


namespace QTest {
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

class ChangesetCreateTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void JS_optimizeTest_data() {
		QTest::addColumn<QString>("oldtext");
		QTest::addColumn<QString>("input");
		QTest::addColumn<QString>("output");

		QTest::newRow("foo1") << "foo" << "Z:3>0$" << "Z:3>0$"; 
		QTest::newRow("fo1") << "fo" << "Z:2>1-2*0+3$foo" << "Z:2>1=1*0+1=1$o"; 
		QTest::newRow("fo2") << "fo" << "Z:2>1=1*0+1$o" << "Z:2>1=1*0+1$o"; 
		QTest::newRow("foo2") << "foo" << "Z:3>3-3*0+6$foobar" << "Z:3>3=3*0+3$bar"; 
		QTest::newRow("foo3") << "foo" << "Z:3>3=3*0+3$bar" << "Z:3>3=3*0+3$bar"; 
		QTest::newRow("barbaz1") << "barbaz" << "Z:6>3-6*0+9$foobarbaz" << "Z:6>3*0+3=6$foo"; 
		QTest::newRow("barbaz2") << "barbaz" << "Z:6>3*0+3$foo" << "Z:6>3*0+3$foo"; 
		QTest::newRow("foobaz3") << "foobaz" << "Z:6>3-6*0+9$foobarbaz" << "Z:6>3=3*0+3=3$bar"; 
		QTest::newRow("foobaz4") << "foobaz" << "Z:6>3=3*0+3$bar" << "Z:6>3=3*0+3$bar"; 
		QTest::newRow("foo4") << "foo" << "Z:3>1r-3*0+1u$foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar" << "Z:3>1r=3*0+1r$barfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar"; 
		QTest::newRow("foo5") << "foo" << "Z:3>1r=3*0+1r$barfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar" << "Z:3>1r=3*0+1r$barfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar"; 
		QTest::newRow("empty") << "" << "Z:0>6*0+6$foobar" << "Z:0>6*0+6$foobar"; 
		QTest::newRow("foo bar baz") << "foo bar baz" << "Z:b>6=4*0+6$extra " << "Z:b>6=4*0+6$extra "; 
		QTest::newRow("foo\nbar\nbaz\n") << "foo\nbar\nbaz\n" << "Z:c>5|3-c*0|4+h$foo\nbar\nbing\nbaz\n" << "Z:c>5|1=7*0|1+5|2=5$\nbing"; 
		QTest::newRow("foo\nbar\nbaz\n") << "foo\nbar\nbaz\n" << "Z:c>5|1=7*0|1+5$\nbing" << "Z:c>5|1=7*0|1+5$\nbing"; 
		QTest::newRow("foo\n bar") << "foo\n bar" << "Z:8>4|1=5-3*0|1+7$baz\nbar" << "Z:8>4|1=5*0|1+4=3$baz\n"; 
		QTest::newRow("foo\n bar") << "foo\n bar" << "Z:8>4|1=5*0|1+4$baz\n" << "Z:8>4|1=5*0|1+4$baz\n"; 
		QTest::newRow("<br />") << "<br />" << "Z:6>d-6*0+j$<p>Hello world!</p>" << "Z:6>d=1-4*0+h=1$p>Hello world!</p"; 
		QTest::newRow("<br />") << "<br />" << "Z:6>d=1-4*0+h$p>Hello world!</p" << "Z:6>d=1-4*0+h$p>Hello world!</p"; 
		QTest::newRow("<p>Hello world!</p><p>Goodbye world!</p>") << "<p>Hello world!</p><p>Goodbye world!</p>" << "Z:14>m=9-l*0+17$world!</p><p>This is a test.</p><p>Goodbye " << "Z:14>m=9=6*0+m=f$</p><p>This is a test."; 
		QTest::newRow("<p>Hello world!</p><p>Goodbye world!</p>") << "<p>Hello world!</p><p>Goodbye world!</p>" << "Z:14>m=f*0+m$</p><p>This is a test." << "Z:14>m=f*0+m$</p><p>This is a test."; 
		QTest::newRow("<br />") << "<br />" << "Z:6>4k-6*0+4q$<table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table>" << "Z:6>4k=1-4*0+4o=1$table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table"; 
		QTest::newRow("<br />") << "<br />" << "Z:6>4k=1-4*0+4o$table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table" << "Z:6>4k=1-4*0+4o$table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table"; 
		QTest::newRow("<table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table>") << "<table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table>" << "Z:4q>m=1f-3b*0+3x$table</caption><tbody><tr><td>one</td><td>two</td></tr><tr><td>three</td><td>four</td></tr><tr><td>five</td><td>six</td></tr></tbody></table>" << "Z:4q>m=1f=u-1r*0+2d=q$one</td><td>two</td></tr><tr><td>three</td><td>four</td></tr><tr><td>five</td><td>six"; 
		QTest::newRow("<table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table>") << "<table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table>" << "Z:4q>m=29-1r*0+2d$one</td><td>two</td></tr><tr><td>three</td><td>four</td></tr><tr><td>five</td><td>six" << "Z:4q>m=29-1r*0+2d$one</td><td>two</td></tr><tr><td>three</td><td>four</td></tr><tr><td>five</td><td>six"; 
		QTest::newRow("<p>a\nb\nc\nd</p>") << "<p>a\nb\nc\nd</p>" << "Z:e>2|3-e*0|4+g$<p>a\nb\nc\nq\nd</p>" << "Z:e>2|2=8*0|1+2|1=6$\nq"; 
		QTest::newRow("<p>a\nb\nc\nd</p>") << "<p>a\nb\nc\nd</p>" << "Z:e>2|2=8*0|1+2$\nq" << "Z:e>2|2=8*0|1+2$\nq"; 
		QTest::newRow("<p>a</p>\n<p>b</p\n<p>c</p>\n") << "<p>a</p>\n<p>b</p\n<p>c</p>\n" << "Z:q>9|3-q*0|4+z$<p>a</p>\n<p>b</p\n<p>q</p>\n<p>c</p>\n" << "Z:q>9|1=g*0|1+9|2=a$\n<p>q</p>"; 
		QTest::newRow("<p>a</p>\n<p>b</p\n<p>c</p>\n") << "<p>a</p>\n<p>b</p\n<p>c</p>\n" << "Z:q>9|1=g*0|1+9$\n<p>q</p>" << "Z:q>9|1=g*0|1+9$\n<p>q</p>"; 
		QTest::newRow("<p>a \n b</p>") << "<p>a \n b</p>" << "Z:c>2=5|1-2*0|2+4$\nc\n " << "Z:c>2=5*0|1+2|1=2$\nc"; 
		QTest::newRow("<p>a \n b</p>") << "<p>a \n b</p>" << "Z:c>2=5*0|1+2$\nc" << "Z:c>2=5*0|1+2$\nc"; 
		QTest::newRow("<p>Hello</p>") << "<p>Hello</p>" << "Z:c>c-c*0+o$<p>Hello</p><p>There</p>" << "Z:c>c=8*0+c=4$</p><p>There"; 
		QTest::newRow("<p>Hello</p>") << "<p>Hello</p>" << "Z:c>c=8*0+c$</p><p>There" << "Z:c>c=8*0+c$</p><p>There"; 
		QTest::newRow("<p>a</p>\n\n<table>\n\n<tbody>\n<tr>\n<td></td></tr>\n<tr>\n<td></td></tr></tbody></table>\n\n") << "<p>a</p>\n\n<table>\n\n<tbody>\n<tr>\n<td></td></tr>\n<tr>\n<td></td></tr></tbody></table>\n\n" << "Z:2c>1|a-2c*0|a+2d$<p>a</p>\n\n<table>\n\n<tbody>\n<tr>\n<td>a</td></tr>\n<tr>\n<td></td></tr></tbody></table>\n\n" << "Z:2c>1|6=10*0+1|4=1c$a"; 
		QTest::newRow("<p>a</p>\n\n<table>\n\n<tbody>\n<tr>\n<td></td></tr>\n<tr>\n<td></td></tr></tbody></table>\n\n") << "<p>a</p>\n\n<table>\n\n<tbody>\n<tr>\n<td></td></tr>\n<tr>\n<td></td></tr></tbody></table>\n\n" << "Z:2c>1|6=10*0+1$a" << "Z:2c>1|6=10*0+1$a"; 
		QTest::newRow("foobar") << "foobar" << "Z:6<3-6*0+3$foo" << "Z:6<3=3-3$"; 
		QTest::newRow("foobar") << "foobar" << "Z:6<3=3-3$" << "Z:6<3=3-3$"; 
		QTest::newRow("foobarbaz") << "foobarbaz" << "Z:9<5-9*0+4$fooz" << "Z:9<5=3-5=1$"; 
		QTest::newRow("foobarbaz") << "foobarbaz" << "Z:9<5=3-5$" << "Z:9<5=3-5$"; 
		QTest::newRow("foobarbaz") << "foobarbaz" << "Z:9<3-9*0+6$barbaz" << "Z:9<3-4*0+1=5$b"; 
		QTest::newRow("foobarbaz") << "foobarbaz" << "Z:9<3-4*0+1$b" << "Z:9<3-4*0+1$b"; 
		QTest::newRow("Hello World") << "Hello World" << "Z:b<6-6$" << "Z:b<6-6$"; 
		QTest::newRow("Hello World") << "Hello World" << "Z:b<6=5-6$" << "Z:b<6=5-6$"; 
		QTest::newRow("Hello World") << "Hello World" << "Z:b<6=5-6$" << "Z:b<6=5-6$"; 
		QTest::newRow("<p>Hello, World!</p>") << "<p>Hello, World!</p>" << "Z:k<7-k*0+d$<p>Hello!</p>" << "Z:k<7=8-7=5$"; 
		QTest::newRow("<p>Hello, World!</p>") << "<p>Hello, World!</p>" << "Z:k<7=8-7$" << "Z:k<7=8-7$"; 
		QTest::newRow("<p>Hello!</p><p>World!</p><p>Foobar</p>") << "<p>Hello!</p><p>World!</p><p>Foobar</p>" << "Z:13<d-13*0+q$<p>Hello!</p><p>Foobar</p>" << "Z:13<d=8-d=i$"; 
		QTest::newRow("<p>Hello!</p><p>World!</p><p>Foobar</p>") << "<p>Hello!</p><p>World!</p><p>Foobar</p>" << "Z:13<d=8-d$" << "Z:13<d=8-d$"; 
		QTest::newRow("<table><tbody><tr><td></td></tr></tbody></table>\n\n") << "<table><tbody><tr><td></td></tr></tbody></table>\n\n" << "Z:1e<16|2-1e*0|2+8$<br />\n\n" << "Z:1e<16=1-1a*0+4|2=3$br /"; 
		QTest::newRow("<table><tbody><tr><td></td></tr></tbody></table>\n\n") << "<table><tbody><tr><td></td></tr></tbody></table>\n\n" << "Z:1e<16=1-1a*0+4$br /" << "Z:1e<16=1-1a*0+4$br /"; 
		QTest::newRow("foobar") << "foobar" << "Z:6>0-6*0+6$foobaz" << "Z:6>0=5-1*0+1$z"; 
		QTest::newRow("foobar") << "foobar" << "Z:6>0=5-1*0+1$z" << "Z:6>0=5-1*0+1$z"; 
		QTest::newRow("foobar") << "foobar" << "Z:6>0-6*0+6$zzzbar" << "Z:6>0-3*0+3=3$zzz"; 
		QTest::newRow("foobar") << "foobar" << "Z:6>0-3*0+3$zzz" << "Z:6>0-3*0+3$zzz"; 
		QTest::newRow("foobar") << "foobar" << "Z:6>0-6*0+6$foozzz" << "Z:6>0=3-3*0+3$zzz"; 
		QTest::newRow("foobar") << "foobar" << "Z:6>0=3-3*0+3$zzz" << "Z:6>0=3-3*0+3$zzz"; 
		QTest::newRow("foobar") << "foobar" << "Z:6>6-6*0+c$fooaaaabbbbb" << "Z:6>6=3-3*0+9$aaaabbbbb"; 
		QTest::newRow("foobar") << "foobar" << "Z:6>6=3-3*0+9$aaaabbbbb" << "Z:6>6=3-3*0+9$aaaabbbbb"; 
		QTest::newRow("foobar") << "foobar" << "Z:6>k-6*0+q$abcdefghijklmnopqrstuvwxyz" << "Z:6>k-6*0+q$abcdefghijklmnopqrstuvwxyz"; 
		QTest::newRow("this is sentence") << "this is sentence" << "Z:g>8-5*0+5=b*0+8$This  of mine" << "Z:g>8-1*0+1=4=b*0+8$T of mine"; 
		QTest::newRow("this is sentence") << "this is sentence" << "Z:g>8-1*0+1=f*0+8$T of mine" << "Z:g>8-1*0+1=f*0+8$T of mine"; 
		QTest::newRow("<p>Hello world!</p>") << "<p>Hello world!</p>" << "Z:j>0=9-a*0+a$there!</p>" << "Z:j>0=9-5*0+5=5$there"; 
		QTest::newRow("<p>Hello world!</p>") << "<p>Hello world!</p>" << "Z:j>0=9-5*0+5$there" << "Z:j>0=9-5*0+5$there"; 
		QTest::newRow("<p>Hello!</p><p>World!</p>") << "<p>Hello!</p><p>World!</p>" << "Z:q>j-q*0+19$<p>Hello!</p><p>There!</p><p>How are you?</p>" << "Z:q>j=g-6*0+p=4$There!</p><p>How are you?"; 
		QTest::newRow("<p>Hello!</p><p>World!</p>") << "<p>Hello!</p><p>World!</p>" << "Z:q>j=g-6*0+p$There!</p><p>How are you?" << "Z:q>j=g-6*0+p$There!</p><p>How are you?"; 
		QTest::newRow("<p>Hello my World!</p>\n\n") << "<p>Hello my World!</p>\n\n" << "Z:o<5|2-o*0+j$<p>Initial HTML</p>" << "Z:o<5=3|2-l*0+g$Initial HTML</p>"; 
		QTest::newRow("<p>Hello my World!</p>\n\n") << "<p>Hello my World!</p>\n\n" << "Z:o<5=3|2-l*0+g$Initial HTML</p>" << "Z:o<5=3|2-l*0+g$Initial HTML</p>"; 
		QTest::newRow("hello world") << "hello world" << "Z:b>0-1*0+1=5-1*0+1$HW" << "Z:b>0-1*0+1=5-1*0+1$HW"; 
		QTest::newRow("jacob") << "jacob" << "Z:5<2=1-1=1-1$" << "Z:5<2=1-1=1-1$"; 
		QTest::newRow("eeasstterr eeeeggg") << "eeasstterr eeeeggg" << "Z:i<8=1-1=2-1=1-1=2-1=2-3=2-1$" << "Z:i<8=1-1=2-1=1-1=2-1=2-3=2-1$"; 
		QTest::newRow("new york") << "new york" << "Z:8>0-1*0+1=3-1*0+1$NY" << "Z:8>0-1*0+1=3-1*0+1$NY"; 
		QTest::newRow("Need mnay test") << "Need mnay test" << "Z:e>1=6-1=1*0+1=6*0+1$ns" << "Z:e>1=6-1=1*0+1=6*0+1$ns"; 
		QTest::newRow("hELLO wORLD") << "hELLO wORLD" << "Z:b>0-5*0+5=1-5*0+5$HelloWorld" << "Z:b>0-5*0+5=1-5*0+5$HelloWorld"; 
		QTest::newRow("This is sentence") << "This is sentence" << "Z:g>4=8*0+4$a a " << "Z:g>4=8*0+4$a a "; 
		QTest::newRow("This is sentence") << "This is sentence" << "Z:g>4=8*0+4$a a " << "Z:g>4=8*0+4$a a "; 
		QTest::newRow("Append to first") << "Append to first" << "Z:f>5=6*0+5$inged" << "Z:f>5=6*0+5$inged"; 
		QTest::newRow("Append to first") << "Append to first" << "Z:f>5=6*0+5$inged" << "Z:f>5=6*0+5$inged"; 
		QTest::newRow("word") << "word" << "Z:4>7=4*0+7$addmore" << "Z:4>7=4*0+7$addmore"; 
		QTest::newRow("word") << "word" << "Z:4>7=4*0+7$addmore" << "Z:4>7=4*0+7$addmore"; 
		QTest::newRow("word") << "word" << "Z:4>7=4*0+7$moreadd" << "Z:4>7=4*0+7$moreadd"; 
		QTest::newRow("word") << "word" << "Z:4>7=4*0+7$moreadd" << "Z:4>7=4*0+7$moreadd"; 
		QTest::newRow("word") << "word" << "Z:4>7=4*0+7$moreadd" << "Z:4>7=4*0+7$moreadd"; 
		QTest::newRow("word") << "word" << "Z:4>7=4*0+7$moreadd" << "Z:4>7=4*0+7$moreadd"; 
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>" << "Z:i>4=9*0+3=4-1*0+2$My d!" << "Z:i>4=9*0+3=4=1*0+1$My !"; 
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>" << "Z:i>4=9*0+3=5*0+1$My !" << "Z:i>4=9*0+3=5*0+1$My !"; 
		QTest::newRow("This is a simple sentence.") << "This is a simple sentence." << "Z:q<7=a-7$" << "Z:q<7=a-7$"; 
		QTest::newRow("This is a simple sentence.") << "This is a simple sentence." << "Z:q<7=a-7$" << "Z:q<7=a-7$"; 
		QTest::newRow("This is a simple sentence.") << "This is a simple sentence." << "Z:q<7=a-7$" << "Z:q<7=a-7$"; 
		QTest::newRow("This s a sentence.") << "This s a sentence." << "Z:i>9=5-1*0+3=3*0+7$wassimple " << "Z:i>9=5*0+2=1=3*0+7$wasimple "; 
		QTest::newRow("This s a sentence.") << "This s a sentence." << "Z:i>9=5*0+2=4*0+7$wasimple " << "Z:i>9=5*0+2=4*0+7$wasimple "; 
		QTest::newRow("b hello world b") << "b hello world b" << "Z:f>1-1*0+8=7-7*0+1$b prefixb" << "Z:f>1=1*0+7=7-7*0+1$ prefixb"; 
		QTest::newRow("b hello world b") << "b hello world b" << "Z:f>1=1*0+7=7-7*0+1$ prefixb" << "Z:f>1=1*0+7=7-7*0+1$ prefixb"; 
		QTest::newRow("xxxtxx\n\n") << "xxxtxx\n\n" << "Z:8>2-1*0+2=3-1*0+2$xsxt" << "Z:8>2=1*0+1=3=1*0+1$st"; 
		QTest::newRow("xxxtxx\n\n") << "xxxtxx\n\n" << "Z:8>2=1*0+1=4*0+1$st" << "Z:8>2=1*0+1=4*0+1$st"; 
		QTest::newRow("") << "" << "Z:0>e*0+e$nulls are cool" << "Z:0>e*0+e$nulls are cool"; 
		QTest::newRow("") << "" << "Z:0>e*0+e$nulls are cool" << "Z:0>e*0+e$nulls are cool"; 
		QTest::newRow("dificcult") << "dificcult" << "Z:9>1=3*0+2=2-1$ff" << "Z:9>1=3*0+2=2-1$ff"; 
		QTest::newRow("dificcult") << "dificcult" << "Z:9>1=3*0+2=2-1$ff" << "Z:9>1=3*0+2=2-1$ff"; 
		QTest::newRow("a") << "a" << "Z:1>1-1*0+2$bc" << "Z:1>1-1*0+2$bc"; 
		QTest::newRow("a") << "a" << "Z:1>1-1*0+2$bc" << "Z:1>1-1*0+2$bc"; 
		QTest::newRow("helloworld") << "helloworld" << "Z:a>2-1*0+1=4-1*0+3$H\nW" << "Z:a>2-1*0+1=4-1*0+3$H\nW"; 
		QTest::newRow("helloworld") << "helloworld" << "Z:a>2-1*0+1=4-1*0+3$H\nW" << "Z:a>2-1*0+1=4-1*0+3$H\nW"; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>a=9*0+a$Brave New " << "Z:k>a=9*0+a$Brave New "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>a=9*0+a$Brave New " << "Z:k>a=9*0+a$Brave New "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>a=9*0+a$Brave New " << "Z:k>a=9*0+a$Brave New "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>a=9*0+a$Brave New " << "Z:k>a=9*0+a$Brave New "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>a=9*0+a$Brave New " << "Z:k>a=9*0+a$Brave New "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>a=9*0+a$Brave New " << "Z:k>a=9*0+a$Brave New "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>1=9|2-b*0|2+c$World!</p>\n\n" << "Z:k>1=9=5*0+1|2=6$!"; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>1=e*0+1$!" << "Z:k>1=e*0+1$!"; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>3=9*0+3$My " << "Z:k>3=9*0+3$My "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>4=9*0+3=5*0+1$My !" << "Z:k>4=9*0+3=5*0+1$My !"; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>3=9*0+3$My " << "Z:k>3=9*0+3$My "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>3=9*0+3$My " << "Z:k>3=9*0+3$My "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>1=9|2-b*0|2+c$World!</p>\n\n" << "Z:k>1=9=5*0+1|2=6$!"; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>1=e*0+1$!" << "Z:k>1=e*0+1$!"; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>3=9*0+3$My " << "Z:k>3=9*0+3$My "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>3=9*0+3$My " << "Z:k>3=9*0+3$My "; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>4=9*0+3=5*0+1$My !" << "Z:k>4=9*0+3=5*0+1$My !"; 
		QTest::newRow("<p>Hello World</p>\n\n") << "<p>Hello World</p>\n\n" << "Z:k>4=9*0+3=5*0+1$My !" << "Z:k>4=9*0+3=5*0+1$My !"; 
		QTest::newRow("<p>Hello World!</p>\n\n") << "<p>Hello World!</p>\n\n" << "Z:l>3=9*0+3$My " << "Z:l>3=9*0+3$My "; 
		QTest::newRow("<p>Hello World!</p>\n\n") << "<p>Hello World!</p>\n\n" << "Z:l>3=9*0+3$My " << "Z:l>3=9*0+3$My "; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1|2-6*0|2+7$xsxxx\n\n" << "Z:6>1=1*0+1|2=5$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1=1*0+1$s" << "Z:6>1=1*0+1$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1|2-6*0|2+7$xsxxx\n\n" << "Z:6>1=1*0+1|2=5$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1=1*0+1$s" << "Z:6>1=1*0+1$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2|2-6*0|2+8$xsxlxx\n\n" << "Z:6>2=1-1*0+3|2=4$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "Z:6>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1|2-6*0|2+7$xxxtx\n\n" << "Z:6>1=3*0+1|2=3$t"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1=3*0+1$t" << "Z:6>1=3*0+1$t"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2|2-6*0|2+8$xsxlxx\n\n" << "Z:6>2=1-1*0+3|2=4$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "Z:6>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2|2-6*0|2+8$xsxlxx\n\n" << "Z:6>2=1-1*0+3|2=4$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "Z:6>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>3=1-1*0+3=1*0+1$sxlt" << "Z:6>3=1-1*0+3=1*0+1$sxlt"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1*0+1=2*0+1$st" << "Z:6>2=1*0+1=2*0+1$st"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>1|2-7*0|2+8$xsxxtx\n\n" << "Z:7>1=1*0+1|2=6$s"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>1=1*0+1$s" << "Z:7>1=1*0+1$s"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2|2-7*0|2+9$xsxlxtx\n\n" << "Z:7>2=1-1*0+3|2=5$sxl"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "Z:7>2=1-1*0+3$sxl"; 
		QTest::newRow("xsxxtx\n\n") << "xsxxtx\n\n" << "Z:8>1|2-8*0|2+9$xsxlxtx\n\n" << "Z:8>1=3*0+1|2=5$l"; 
		QTest::newRow("xsxxtx\n\n") << "xsxxtx\n\n" << "Z:8>1=3*0+1$l" << "Z:8>1=3*0+1$l"; 
		QTest::newRow("xsxxtx\n\n") << "xsxxtx\n\n" << "Z:8>1|2-8*0|2+9$xsxlxtx\n\n" << "Z:8>1=3*0+1|2=5$l"; 
		QTest::newRow("xsxxtx\n\n") << "xsxxtx\n\n" << "Z:8>1=3*0+1$l" << "Z:8>1=3*0+1$l"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1|2-6*0|2+7$xsxxx\n\n" << "Z:6>1=1*0+1|2=5$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1=1*0+1$s" << "Z:6>1=1*0+1$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1|2-6*0|2+7$xsxxx\n\n" << "Z:6>1=1*0+1|2=5$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1=1*0+1$s" << "Z:6>1=1*0+1$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2|2-6*0|2+8$xsxlxx\n\n" << "Z:6>2=1-1*0+3|2=4$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "Z:6>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1|2-6*0|2+7$xxxtx\n\n" << "Z:6>1=3*0+1|2=3$t"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1=3*0+1$t" << "Z:6>1=3*0+1$t"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2|2-6*0|2+8$xsxlxx\n\n" << "Z:6>2=1-1*0+3|2=4$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "Z:6>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2|2-6*0|2+8$xsxlxx\n\n" << "Z:6>2=1-1*0+3|2=4$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "Z:6>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>3=1-1*0+3=1*0+1$sxlt" << "Z:6>3=1-1*0+3=1*0+1$sxlt"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1*0+1=2*0+1$st" << "Z:6>2=1*0+1=2*0+1$st"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>1|2-7*0|2+8$xsxxtx\n\n" << "Z:7>1=1*0+1|2=6$s"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>1=1*0+1$s" << "Z:7>1=1*0+1$s"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2|2-7*0|2+9$xsxlxtx\n\n" << "Z:7>2=1-1*0+3|2=5$sxl"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "Z:7>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7<1|2-7*0|2+6$xxxx\n\n" << "Z:7<1=3-1|2=3$"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7<1=3-1$" << "Z:7<1=3-1$"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2|2-7*0|2+9$xsxlxtx\n\n" << "Z:7>2=1-1*0+3|2=5$sxl"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "Z:7>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2|2-7*0|2+9$xsxlxtx\n\n" << "Z:7>2=1-1*0+3|2=5$sxl"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "Z:7>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>1=1-1*0+3=1-1$sxl" << "Z:7>1=1-1*0+3=1-1$sxl"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>0=1*0+1=2-1$s" << "Z:7>0=1*0+1=2-1$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1|2-6*0|2+7$xsxxx\n\n" << "Z:6>1=1*0+1|2=5$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1=1*0+1$s" << "Z:6>1=1*0+1$s"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2|2-6*0|2+8$xsxlxx\n\n" << "Z:6>2=1-1*0+3|2=4$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "Z:6>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1|2-6*0|2+7$xxxtx\n\n" << "Z:6>1=3*0+1|2=3$t"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>1=3*0+1$t" << "Z:6>1=3*0+1$t"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2|2-6*0|2+8$xsxlxx\n\n" << "Z:6>2=1-1*0+3|2=4$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "Z:6>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2|2-6*0|2+8$xsxlxx\n\n" << "Z:6>2=1-1*0+3|2=4$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "Z:6>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>3=1-1*0+3=1*0+1$sxlt" << "Z:6>3=1-1*0+3=1*0+1$sxlt"; 
		QTest::newRow("xxxx\n\n") << "xxxx\n\n" << "Z:6>2=1*0+1=2*0+1$st" << "Z:6>2=1*0+1=2*0+1$st"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>1|2-7*0|2+8$xsxxtx\n\n" << "Z:7>1=1*0+1|2=6$s"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>1=1*0+1$s" << "Z:7>1=1*0+1$s"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2|2-7*0|2+9$xsxlxtx\n\n" << "Z:7>2=1-1*0+3|2=5$sxl"; 
		QTest::newRow("xxxtx\n\n") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "Z:7>2=1-1*0+3$sxl"; 
		QTest::newRow("xsxxtx\n\n") << "xsxxtx\n\n" << "Z:8>1|2-8*0|2+9$xsxlxtx\n\n" << "Z:8>1=3*0+1|2=5$l"; 
		QTest::newRow("xsxxtx\n\n") << "xsxxtx\n\n" << "Z:8>1=3*0+1$l" << "Z:8>1=3*0+1$l"; 
		QTest::newRow("xsxxtx\n\n") << "xsxxtx\n\n" << "Z:8>1|2-8*0|2+9$xsxlxtx\n\n" << "Z:8>1=3*0+1|2=5$l"; 
		QTest::newRow("xsxxtx\n\n") << "xsxxtx\n\n" << "Z:8>1=3*0+1$l" << "Z:8>1=3*0+1$l"; 
		QTest::newRow("xxxxx1") << "xxxxx\n\n" << "Z:7>1|2-7*0|2+8$xsxxxx\n\n" << "Z:7>1=1*0+1|2=6$s"; 
		QTest::newRow("xxxxx2") << "xxxxx\n\n" << "Z:7>1=1*0+1$s" << "Z:7>1=1*0+1$s"; 
		QTest::newRow("xxxxx3") << "xxxxx\n\n" << "Z:7>1|2-7*0|2+8$xsxxxx\n\n" << "Z:7>1=1*0+1|2=6$s"; 
		QTest::newRow("xxxxx4") << "xxxxx\n\n" << "Z:7>1=1*0+1$s" << "Z:7>1=1*0+1$s"; 
		QTest::newRow("xxxxx5") << "xxxxx\n\n" << "Z:7>2|2-7*0|2+9$xsxlxxx\n\n" << "Z:7>2=1-1*0+3|2=5$sxl"; 
		QTest::newRow("xxxxx6") << "xxxxx\n\n" << "Z:7>2=1-1*0+3$sxl" << "Z:7>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxxx7") << "xxxxx\n\n" << "Z:7>1|2-7*0|2+8$xxxtxx\n\n" << "Z:7>1=3*0+1|2=4$t"; 
		QTest::newRow("xxxxx8") << "xxxxx\n\n" << "Z:7>1=3*0+1$t" << "Z:7>1=3*0+1$t"; 
		QTest::newRow("xxxxx9") << "xxxxx\n\n" << "Z:7>2|2-7*0|2+9$xsxlxxx\n\n" << "Z:7>2=1-1*0+3|2=5$sxl"; 
		QTest::newRow("xxxxx0") << "xxxxx\n\n" << "Z:7>2=1-1*0+3$sxl" << "Z:7>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxxx10") << "xxxxx\n\n" << "Z:7>2|2-7*0|2+9$xsxlxxx\n\n" << "Z:7>2=1-1*0+3|2=5$sxl"; 
		QTest::newRow("xxxxx11") << "xxxxx\n\n" << "Z:7>2=1-1*0+3$sxl" << "Z:7>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxxx12") << "xxxxx\n\n" << "Z:7>3=1-1*0+3=1*0+1$sxlt" << "Z:7>3=1-1*0+3=1*0+1$sxlt"; 
		QTest::newRow("xxxxx13") << "xxxxx\n\n" << "Z:7>2=1*0+1=2*0+1$st" << "Z:7>2=1*0+1=2*0+1$st"; 
		QTest::newRow("xxxtxx1") << "xxxtxx\n\n" << "Z:8>1|2-8*0|2+9$xsxxtxx\n\n" << "Z:8>1=1*0+1|2=7$s"; 
		QTest::newRow("xxxtxx2") << "xxxtxx\n\n" << "Z:8>1=1*0+1$s" << "Z:8>1=1*0+1$s"; 
		QTest::newRow("xxxtxx3") << "xxxtxx\n\n" << "Z:8>2|2-8*0|2+a$xsxlxtxx\n\n" << "Z:8>2=1-1*0+3|2=6$sxl"; 
		QTest::newRow("xxxtxx4") << "xxxtxx\n\n" << "Z:8>2=1-1*0+3$sxl" << "Z:8>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxtxx5") << "xxxtxx\n\n" << "Z:8>1|2-8*0|2+9$xxxtxtx\n\n" << "Z:8>1=5*0+1|2=3$t"; 
		QTest::newRow("xxxtxx6") << "xxxtxx\n\n" << "Z:8>1=5*0+1$t" << "Z:8>1=5*0+1$t"; 
		QTest::newRow("xxxtxx7") << "xxxtxx\n\n" << "Z:8>2|2-8*0|2+a$xsxlxtxx\n\n" << "Z:8>2=1-1*0+3|2=6$sxl"; 
		QTest::newRow("xxxtxx8") << "xxxtxx\n\n" << "Z:8>2=1-1*0+3$sxl" << "Z:8>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxtxx9") << "xxxtxx\n\n" << "Z:8>2|2-8*0|2+a$xsxlxtxx\n\n" << "Z:8>2=1-1*0+3|2=6$sxl"; 
		QTest::newRow("xxxtxx10") << "xxxtxx\n\n" << "Z:8>2=1-1*0+3$sxl" << "Z:8>2=1-1*0+3$sxl"; 
		QTest::newRow("xxxtxx11") << "xxxtxx\n\n" << "Z:8>3=1-1*0+3=3*0+1$sxlt" << "Z:8>3=1-1*0+3=3*0+1$sxlt"; 
		QTest::newRow("xxxtxx12") << "xxxtxx\n\n" << "Z:8>2=1*0+1=4*0+1$st" << "Z:8>2=1*0+1=4*0+1$st"; 
		QTest::newRow("xxxtxtx1") << "xxxtxtx\n\n" << "Z:9>1|2-9*0|2+a$xsxxtxtx\n\n" << "Z:9>1=1*0+1|2=8$s"; 
		QTest::newRow("xxxtxtx2") << "xxxtxtx\n\n" << "Z:9>1=1*0+1$s" << "Z:9>1=1*0+1$s"; 
		QTest::newRow("xxxtxtx3") << "xxxtxtx\n\n" << "Z:9>2|2-9*0|2+b$xsxlxtxtx\n\n" << "Z:9>2=1-1*0+3|2=7$sxl"; 
		QTest::newRow("xxxtxtx4") << "xxxtxtx\n\n" << "Z:9>2=1-1*0+3$sxl" << "Z:9>2=1-1*0+3$sxl"; 
		QTest::newRow("xsxxtxtx1") << "xsxxtxtx\n\n" << "Z:a>1|2-a*0|2+b$xsxlxtxtx\n\n" << "Z:a>1=3*0+1|2=7$l"; 
		QTest::newRow("xsxxtxtx2") << "xsxxtxtx\n\n" << "Z:a>1=3*0+1$l" << "Z:a>1=3*0+1$l"; 
		QTest::newRow("xsxxtxtx3") << "xsxxtxtx\n\n" << "Z:a>1|2-a*0|2+b$xsxlxtxtx\n\n" << "Z:a>1=3*0+1|2=7$l"; 
		QTest::newRow("xsxxtxtx4") << "xsxxtxtx\n\n" << "Z:a>1=3*0+1$l" << "Z:a>1=3*0+1$l";
	}

	void JS_optimizeTest() {
		QFETCH(QString, oldtext);
		QFETCH(QString, input);
		QFETCH(QString, output);
		QCOMPARE(JS::optimize(Changeset::fromString(input), oldtext).toString(), output);
	}

	void JS_collapseTest_data()
	{
		QTest::addColumn<QString  >("before");
		QTest::addColumn<QString>("after");

		QTest::newRow("Simple Test")  << "Z:3>3=3*0+1+1$bar"   << "Z:3>3=3*0+2$bar";
		QTest::newRow("Newline Test") << "Z:3>3=3*0|1+1+1$bar" << "Z:3>3=3*0|1+2$bar";
		QTest::newRow("Skip and Newline Test") << "Z:3>3=3*0|1-1-1$bar" << "Z:3>3=3*0|1-2$bar";
		QTest::newRow("Z:7>2=1-1*0+3|2=5$sxl") << "Z:7>2=1-1*0+3|2=5$sxl" << "Z:7>2=1-1*0+3$sxl";
		QTest::newRow("Z:6>3=3*0+3=3$bar") << "Z:6>3=3*0+3=3$bar" << "Z:6>3=3*0+3$bar";
	}

	void JS_collapseTest()
	{
		QFETCH(QString, before);
		QFETCH(QString, after);

		QCOMPARE(JS::collapse(Changeset::fromString(before)).toString(), after);
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

	void createUnoptimizedChangeset_data() {
		QTest::addColumn<QString>("oldText");
		QTest::addColumn<QString>("newText");
		QTest::addColumn<QString>("changeset");

		QTest::newRow("foo -> foo") << "foo" << "foo" << "Z:3>0$";
		QTest::newRow("fo -> foo") << "fo" << "foo" << "Z:2>1-2*0+3$foo";
		QTest::newRow("foo -> foobar") << "foo" << "foobar" << "Z:3>3-3*0+6$foobar";
		QTest::newRow("barbaz -> foobarbaz") << "barbaz" << "foobarbaz" << "Z:6>3-6*0+9$foobarbaz";
		QTest::newRow("foobaz -> foobarbaz") << "foobaz" << "foobarbaz" << "Z:6>3-6*0+9$foobarbaz";
		QTest::newRow("foo -> foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar") << "foo" << "foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar" << "Z:3>1r-3*0+1u$foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar";
		QTest::newRow(" -> foobar") << "" << "foobar" << "Z:0>6*0+6$foobar";
		QTest::newRow("foo bar baz -> foo extra bar baz") << "foo bar baz" << "foo extra bar baz" << "Z:b>6=4*0+6$extra ";
		QTest::newRow("foo bar baz -> foo bar bing baz ") << "foo\nbar\nbaz\n" << "foo\nbar\nbing\nbaz\n" << "Z:c>5|3-c*0|4+h$foo\nbar\nbing\nbaz\n";
		QTest::newRow("foo bar -> foo baz bar") << "foo\n bar" << "foo\n baz\nbar" << "Z:8>4|1=5-3*0|1+7$baz\nbar"; 
	}
	void createUnoptimizedChangeset() {
		QFETCH(QString, oldText);
		QFETCH(QString, newText);
		QFETCH(QString, changeset);

		QCOMPARE(JS::createChangeset(oldText, newText).toString(), changeset);
	}
};

REGISTER_TEST(ChangesetCreateTest)

#include "changeset_create_test.moc"
