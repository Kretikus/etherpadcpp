#include "editor_test.h"
#include "../test/testutil.h"

#include <QVBoxLayout>
#include <QTextBrowser>

#include <collabeditor.h>
#include <changeset.h>


void EditorTest::editTest()
{
	QWidget wdg;
	QVBoxLayout* boxLayout = new QVBoxLayout(&wdg);
	CollabEditor* editor = new CollabEditor;
	QTextBrowser* browser = new QTextBrowser;
	boxLayout->addWidget(editor);
	boxLayout->addWidget(browser);
	TestController controller(editor, browser);
	wdg.show();
	qApp->exec();
}


REGISTER_TEST(EditorTest)

TestController::TestController(CollabEditor *editor, QTextBrowser *browser)
	: editor_(editor), browser_(browser)
{
	connect(editor, SIGNAL(newChangeset(Changeset)), SLOT(newChangeset(Changeset)));
}

void TestController::newChangeset(const Changeset & cs)
{
	text_ = applyChangeset(text_, cs);
	browser_->setPlainText(text_);
}


