#include <qwidget.h>

class CollabEditor;
class QTextBrowser;
class Changeset;

class EditorTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void editTest();
};


class TestController : public QObject
{
	Q_OBJECT

public:
	TestController(CollabEditor* editor, QTextBrowser* browser);

public Q_SLOTS:
	void newChangeset(const Changeset&);

private:
	CollabEditor* editor_;
	QTextBrowser* browser_;
	QString text_;
};
