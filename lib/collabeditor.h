#pragma once

#include <QPlainTextEdit>

class CollabEditor : public QPlainTextEdit {
	Q_OBJECT
public:
	CollabEditor(QWidget * parent = 0);

private Q_SLOTS:
	void createChangeset();

private:
	QString lastContent_;
};
