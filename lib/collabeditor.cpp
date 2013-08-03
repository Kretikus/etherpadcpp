#include "collabeditor.h"

#include <changeset.h>

#include <QTimer>

CollabEditor::CollabEditor(QWidget *parent)
	: QPlainTextEdit(parent)
{
	QTimer * timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(createChangeset()));
	timer->start(1000);
}

void CollabEditor::createChangeset()
{
	const QString newContent = toPlainText();
	if (newContent != lastContent_) {
		const Changeset set = ::createChangeset(lastContent_, newContent);
		qDebug("CS: %s", qPrintable(set.toString()));
		lastContent_ = newContent;
	}
}
