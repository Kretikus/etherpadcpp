#include <changeset.h>
#include <log.h>

#include "testutil.h"

#include <QMetaType>
#include <QJsonObject>

class ChangesetApplyTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void applyChangesetTest_data() {
		QTest::addColumn<QString  >("oldText");
		QTest::addColumn<QString>("changeset");
		QTest::addColumn<QString>("result");

		QTest::newRow("foobaz") << "foobaz" << "Z:6>3=3*0+3$bar" << "foobarbaz";
		QTest::newRow("<br />") << "<br />" << "Z:6>d=1-4*0+h$p>Hello world!</p" << "<p>Hello world!</p>";
		QTest::newRow("<p>Hello world!</p><p>Goodbye world!</p>") << "<p>Hello world!</p><p>Goodbye world!</p>" << "Z:14>m=f*0+m$</p><p>This is a test." << "<p>Hello world!</p><p>This is a test.</p><p>Goodbye world!</p>";
		QTest::newRow("<br />") << "<br />" << "Z:6>4k=1-4*0+4o$table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table" << "<table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table>";
		QTest::newRow("<table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table>") << "<table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td></td><td></td></tr><tr><td></td><td></td></tr><tr><td></td><td></td></tr></tbody></table>" << "Z:4q>m=29-1r*0+2d$one</td><td>two</td></tr><tr><td>three</td><td>four</td></tr><tr><td>five</td><td>six" << "<table summary=\"Just another table\"><caption>A new table</caption><tbody><tr><td>one</td><td>two</td></tr><tr><td>three</td><td>four</td></tr><tr><td>five</td><td>six</td></tr></tbody></table>";
		QTest::newRow("<p>a\nb\nc\nd</p>") << "<p>a\nb\nc\nd</p>" << "Z:e>2|2=8*0|1+2$\nq" << "<p>a\nb\nc\nq\nd</p>";
		QTest::newRow("<p>a</p>\n<p>b</p\n<p>c</p>\n") << "<p>a</p>\n<p>b</p\n<p>c</p>\n" << "Z:q>9|1=g*0|1+9$\n<p>q</p>" << "<p>a</p>\n<p>b</p\n<p>q</p>\n<p>c</p>\n";
		QTest::newRow("<p>a \n b</p>") << "<p>a \n b</p>" << "Z:c>2=5*0|1+2$\nc" << "<p>a \nc\n b</p>";
		QTest::newRow("<p>Hello</p>") << "<p>Hello</p>" << "Z:c>c=8*0+c$</p><p>There" << "<p>Hello</p><p>There</p>";
		QTest::newRow("<p>a</p>\n\n<table>\n\n<tbody>\n<tr>\n<td></td></tr>\n<tr>\n<td></td></tr></tbody></table>") << "<p>a</p>\n\n<table>\n\n<tbody>\n<tr>\n<td></td></tr>\n<tr>\n<td></td></tr></tbody></table>\n\n" << "Z:2c>1|6=10*0+1$a" << "<p>a</p>\n\n<table>\n\n<tbody>\n<tr>\n<td>a</td></tr>\n<tr>\n<td></td></tr></tbody></table>\n\n";
		QTest::newRow("foobar") << "foobar" << "Z:6<3=3-3$" << "foo";
		QTest::newRow("foobarbaz") << "foobarbaz" << "Z:9<5=3-5$" << "fooz";
		QTest::newRow("foobarbaz") << "foobarbaz" << "Z:9<3-4*0+1$b" << "barbaz";
		QTest::newRow("Hello World") << "Hello World" << "Z:b<6-6$" << "World";
		QTest::newRow("Hello World") << "Hello World" << "Z:b<6=5-6$" << "Hello";
		QTest::newRow("<p>Hello, World!</p>") << "<p>Hello, World!</p>" << "Z:k<7=8-7$" << "<p>Hello!</p>";
		QTest::newRow("<p>Hello!</p><p>World!</p><p>Foobar</p>") << "<p>Hello!</p><p>World!</p><p>Foobar</p>" << "Z:13<d=8-d$" << "<p>Hello!</p><p>Foobar</p>";
		QTest::newRow("<table><tbody><tr><td></td></tr></tbody></table>") << "<table><tbody><tr><td></td></tr></tbody></table>\n\n" << "Z:1e<16=1-1a*0+4$br /" << "<br />\n\n";
		QTest::newRow("foobar") << "foobar" << "Z:6>0=5-1*0+1$z" << "foobaz";
		QTest::newRow("foobar") << "foobar" << "Z:6>0-3*0+3$zzz" << "zzzbar";
		QTest::newRow("foobar") << "foobar" << "Z:6>0=3-3*0+3$zzz" << "foozzz";
		QTest::newRow("foobar") << "foobar" << "Z:6>6=3-3*0+9$aaaabbbbb" << "fooaaaabbbbb";
		QTest::newRow("foobar") << "foobar" << "Z:6>k-6*0+q$abcdefghijklmnopqrstuvwxyz" << "abcdefghijklmnopqrstuvwxyz";
		QTest::newRow("this is sentence") << "this is sentence" << "Z:g>8-1*0+1=f*0+8$T of mine" << "This is sentence of mine";
		QTest::newRow("<p>Hello world!</p>") << "<p>Hello world!</p>" << "Z:j>0=9-5*0+5$there" << "<p>Hello there!</p>";
		QTest::newRow("<p>Hello!</p><p>World!</p>") << "<p>Hello!</p><p>World!</p>" << "Z:q>j=g-6*0+p$There!</p><p>How are you?" << "<p>Hello!</p><p>There!</p><p>How are you?</p>";
		QTest::newRow("<p>Hello my World!</p>") << "<p>Hello my World!</p>\n\n" << "Z:o<5=3|2-l*0+g$Initial HTML</p>" << "<p>Initial HTML</p>";
		QTest::newRow("foobar") << "foobar" << "Z:6>0$" << "foobar";
		QTest::newRow("foobar") << "foobar" << "Z:6>1+1$z" << "zfoobar";
		QTest::newRow("foobar") << "foobar" << "Z:6>1=3+1$z" << "foozbar";
		QTest::newRow("foobar") << "foobar" << "Z:6>1=6+1$z" << "foobarz";
		QTest::newRow("zfoobar") << "zfoobar" << "Z:7<1-1$" << "foobar";
		QTest::newRow("foozbar") << "foozbar" << "Z:7<1=3-1$" << "foobar";
		QTest::newRow("foobarz") << "foobarz" << "Z:7<1=6-1$" << "foobar";
		QTest::newRow("foobar") << "foobar" << "Z:6>3+3$baz" << "bazfoobar";
		QTest::newRow("foobar") << "foobar" << "Z:6>3=3+3$baz" << "foobazbar";
		QTest::newRow("foobar") << "foobar" << "Z:6>3=6+3$baz" << "foobarbaz";
		QTest::newRow("bazfoobar") << "bazfoobar" << "Z:9<3-3$" << "foobar";
		QTest::newRow("foobazbar") << "foobazbar" << "Z:9<3=3-3$" << "foobar";
		QTest::newRow("foobarbaz") << "foobarbaz" << "Z:9<3=6-3$" << "foobar";
		QTest::newRow("foobar") << "foobar" << "Z:6>1*1+1$z" << "zfoobar";
		QTest::newRow("foobar") << "foobar" << "Z:6>1*0=3*1+1$z" << "foozbar";
		QTest::newRow("foobar") << "foobar" << "Z:6>1*0=6*1+1$z" << "foobarz";
		QTest::newRow("zfoobar") << "zfoobar" << "Z:7<1-1$" << "foobar";
		QTest::newRow("foozbar") << "foozbar" << "Z:7<1*0=3-1$" << "foobar";
		QTest::newRow("foobarz") << "foobarz" << "Z:7<1*0=6-1$" << "foobar";
		QTest::newRow("foobar") << "foobar" << "Z:6>3*1+3$baz" << "bazfoobar";
		QTest::newRow("foobar") << "foobar" << "Z:6>3*0=3*1+3$baz" << "foobazbar";
		QTest::newRow("foobar") << "foobar" << "Z:6>3*0=6*1+3$baz" << "foobarbaz";
		QTest::newRow("bazfoobar") << "bazfoobar" << "Z:9<3-3$" << "foobar";
		QTest::newRow("foobazbar") << "foobazbar" << "Z:9<3*0=3-3$" << "foobar";
		QTest::newRow("foobarbaz") << "foobarbaz" << "Z:9<3*0=3*2=3-3$" << "foobar";
		QTest::newRow("foo\nbar") << "foo\nbar" << "Z:7>1*1+1$z" << "zfoo\nbar";
		QTest::newRow("foo\nbar") << "foo\nbar" << "Z:7>1*0|1=4*1+1$z" << "foo\nzbar";
		QTest::newRow("foo\nbar") << "foo\nbar" << "Z:7>1*0|1=7*1+1$z" << "foo\nbarz";
		QTest::newRow("zfoo\nbar") << "zfoo\nbar" << "Z:8<1-1$" << "foo\nbar";
		QTest::newRow("foo\nzbar") << "foo\nzbar" << "Z:8<1*0|1=4-1$" << "foo\nbar";
		QTest::newRow("foo\nbarz") << "foo\nbarz" << "Z:8<1*0|1=7-1$" << "foo\nbar";
		QTest::newRow("foo\nbar") << "foo\nbar" << "Z:7>3*1+3$baz" << "bazfoo\nbar";
		QTest::newRow("foo\nbar") << "foo\nbar" << "Z:7>3*0|1=4*1+3$baz" << "foo\nbazbar";
		QTest::newRow("foo\nbar") << "foo\nbar" << "Z:7>3*0|1=7*1+3$baz" << "foo\nbarbaz";
		QTest::newRow("bazfoo\nbar") << "bazfoo\nbar" << "Z:a<3-3$" << "foo\nbar";
		QTest::newRow("foo\nbazbar") << "foo\nbazbar" << "Z:a<3*0|1=4-3$" << "foo\nbar";
		QTest::newRow("foo\nbarbaz") << "foo\nbarbaz" << "Z:a<3*0|1=4*2=3-3$" << "foo\nbar";
		QTest::newRow("foobar") << "foobar" << "Z:6>3*1+1*0=3*1+1*3=3*1+1$zzz" << "zfoozbarz";
		QTest::newRow("foobar") << "foobar" << "Z:6>9*1+3*0=3*1+3*3=3*1+3$bazbazbaz" << "bazfoobazbarbaz";
		QTest::newRow("zfoozbarz") << "zfoozbarz" << "Z:9<3-1*0=3-1*0=3-1$" << "foobar";
		QTest::newRow("bazfoobazbarbaz") << "bazfoobazbarbaz" << "Z:f<9-3*0=3-3*0=3-3$" << "foobar";
		QTest::newRow("hello world") << "hello world" << "Z:b>0-1*0+1=5-1*0+1$HW" << "Hello World";
		QTest::newRow("hELLO wORLD") << "hELLO wORLD" << "Z:b>0-5*0+5=1-5*0+5$HelloWorld" << "Hello World";
		QTest::newRow("This is sentence") << "This is sentence" << "Z:g>4=8*0+4$a a " << "This is a a sentence";
		QTest::newRow("Append to first") << "Append to first" << "Z:f>5=6*0+5$inged" << "Appendinged to first";
		QTest::newRow("word") << "word" << "Z:4>7=4*0+7$addmore" << "wordaddmore";
		QTest::newRow("word") << "word" << "Z:4>7=4*0+7$moreadd" << "wordmoreadd";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>" << "Z:i>4=9*0+3=5*0+1$My !" << "<p>Hello My World!</p>";
		QTest::newRow("This is a simple sentence.") << "This is a simple sentence." << "Z:q<7=a-7$" << "This is a sentence.";
		QTest::newRow("This s a sentence.") << "This s a sentence." << "Z:i>9=5*0+2=4*0+7$wasimple " << "This was a simple sentence.";
		QTest::newRow("b hello world b") << "b hello world b" << "Z:f>1=1*0+7=7-7*0+1$ prefixb" << "b prefix hello b";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>2=1*0+1=4*0+1$st" << "xsxxtxtx\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>a=9*0+a$Brave New " << "<p>Hello Brave New World</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>1=e*0+1$!" << "<p>Hello World!</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>3=9*0+3$My " << "<p>Hello My World</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>1=e*0+1$!" << "<p>Hello World!</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>4=9*0+3=5*0+1$My !" << "<p>Hello My World!</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>3=9*0+3$My " << "<p>Hello My World</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>1=e*0+1$!" << "<p>Hello World!</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>3=9*0+3$My " << "<p>Hello My World</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>1=e*0+1$!" << "<p>Hello World!</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>3=9*0+3$My " << "<p>Hello My World</p>\n\n";
		QTest::newRow("<p>Hello World</p>") << "<p>Hello World</p>\n\n" << "Z:k>4=9*0+3=5*0+1$My !" << "<p>Hello My World!</p>\n\n";
		QTest::newRow("<p>Hello World!</p>") << "<p>Hello World!</p>\n\n" << "Z:l>3=9*0+3$My " << "<p>Hello My World!</p>\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=1*0+1$s" << "xsxxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=3*0+1$t" << "xxxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=3*0+1$t" << "xxxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>3=1-1*0+3=1*0+1$sxlt" << "xsxlxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1*0+1=2*0+1$st" << "xsxxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>1=1*0+1$s" << "xsxxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "xsxlxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>1=1*0+1$s" << "xsxxtx\n\n";
		QTest::newRow("xsxxtx") << "xsxxtx\n\n" << "Z:8>1=3*0+1$l" << "xsxlxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=1*0+1$s" << "xsxxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=3*0+1$t" << "xxxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=3*0+1$t" << "xxxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>3=1-1*0+3=1*0+1$sxlt" << "xsxlxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1*0+1=2*0+1$st" << "xsxxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>1=1*0+1$s" << "xsxxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "xsxlxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>1=1*0+1$s" << "xsxxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7<1=3-1$" << "xxxx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "xsxlxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7<1=3-1$" << "xxxx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "xsxlxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>1=1-1*0+3=1-1$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>0=1*0+1=2-1$s" << "xsxxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=1*0+1$s" << "xsxxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=1*0+1$s" << "xsxxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=3*0+1$t" << "xxxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>1=3*0+1$t" << "xxxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1-1*0+3$sxl" << "xsxlxx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>3=1-1*0+3=1*0+1$sxlt" << "xsxlxtx\n\n";
		QTest::newRow("xxxx") << "xxxx\n\n" << "Z:6>2=1*0+1=2*0+1$st" << "xsxxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>1=1*0+1$s" << "xsxxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>2=1-1*0+3$sxl" << "xsxlxtx\n\n";
		QTest::newRow("xxxtx") << "xxxtx\n\n" << "Z:7>1=1*0+1$s" << "xsxxtx\n\n";
		QTest::newRow("xsxxtx") << "xsxxtx\n\n" << "Z:8>1=3*0+1$l" << "xsxlxtx\n\n";
		QTest::newRow("xxxxx") << "xxxxx\n\n" << "Z:7>1=1*0+1$s" << "xsxxxx\n\n";
		QTest::newRow("xxxxx") << "xxxxx\n\n" << "Z:7>2=1-1*0+3$sxl" << "xsxlxxx\n\n";
		QTest::newRow("xxxxx") << "xxxxx\n\n" << "Z:7>1=3*0+1$t" << "xxxtxx\n\n";
		QTest::newRow("xxxxx") << "xxxxx\n\n" << "Z:7>2=1-1*0+3$sxl" << "xsxlxxx\n\n";
		QTest::newRow("xxxxx") << "xxxxx\n\n" << "Z:7>1=3*0+1$t" << "xxxtxx\n\n";
		QTest::newRow("xxxxx") << "xxxxx\n\n" << "Z:7>2=1-1*0+3$sxl" << "xsxlxxx\n\n";
		QTest::newRow("xxxxx") << "xxxxx\n\n" << "Z:7>3=1-1*0+3=1*0+1$sxlt" << "xsxlxtxx\n\n";
		QTest::newRow("xxxxx") << "xxxxx\n\n" << "Z:7>2=1*0+1=2*0+1$st" << "xsxxtxx\n\n";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>1=1*0+1$s" << "xsxxtxx\n\n";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>2=1-1*0+3$sxl" << "xsxlxtxx\n\n";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>1=1*0+1$s" << "xsxxtxx\n\n";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>1=5*0+1$t" << "xxxtxtx\n\n";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>2=1-1*0+3$sxl" << "xsxlxtxx\n\n";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>1=5*0+1$t" << "xxxtxtx\n\n";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>2=1-1*0+3$sxl" << "xsxlxtxx\n\n";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>3=1-1*0+3=3*0+1$sxlt" << "xsxlxtxtx\n\n";
		QTest::newRow("xxxtxx") << "xxxtxx\n\n" << "Z:8>2=1*0+1=4*0+1$st" << "xsxxtxtx\n\n";
		QTest::newRow("xxxtxtx") << "xxxtxtx\n\n" << "Z:9>1=1*0+1$s" << "xsxxtxtx\n\n";
		QTest::newRow("xxxtxtx") << "xxxtxtx\n\n" << "Z:9>2=1-1*0+3$sxl" << "xsxlxtxtx\n\n";
		QTest::newRow("xxxtxtx") << "xxxtxtx\n\n" << "Z:9>1=1*0+1$s" << "xsxxtxtx\n\n";
		QTest::newRow("xsxxtxtx") << "xsxxtxtx\n\n" << "Z:a>1=3*0+1$l" << "xsxlxtxtx\n\n";
	}

	void applyChangesetTest() {
		QFETCH(QString,   oldText);
		QFETCH(QString, changeset);
		QFETCH(QString,   result);

		QCOMPARE(applyChangeset(oldText, Changeset::fromString(changeset)), result);
	}
};


REGISTER_TEST(ChangesetApplyTest)

#include "changeset_apply_test.moc"

