#pragma once

#include <QPlainTextEdit>

class Changeset;

class CollabEditor : public QPlainTextEdit {
	Q_OBJECT
public:
	CollabEditor(QWidget * parent = 0);

Q_SIGNALS:
	void newChangeset(const Changeset& cs);

private Q_SLOTS:
	void createChangeset();

private:
	QString lastContent_;
};
