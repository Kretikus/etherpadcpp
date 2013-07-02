#include "log.h"

#include <QFile>
#include <QDateTime>
#include <QString>

void LogDetails::output(QByteArray & msg, int val) {
	msg.append( QString::number(val) );
}

void LogDetails::output(QByteArray & msg, const QString & val) {
	msg.append( val );
}

const char * LogDetails::filenameSubStr(const char * filePath)
{
	// strategy: Find last / or
	const size_t size = ::strlen(filePath);
	const char * pos = filePath + size;
	for(;;--pos) {
		if (pos == filePath) return pos;
		if (*pos == '/' || *pos == '\\') return pos+1;
	}
	return pos;
}

inline void LogDetails::writePadded(char ** dest, uint number, int width, char pad)
{
	*dest += width;
	char * pos = *dest;
	for (int i = 0 ; i < width ; ++i) {
			if (number == 0 && i > 0) {
					*(--pos) = pad;
			} else {
					*(--pos) = '0' + number % 10;
					number /= 10;
			}
	}
}

class LogImpl
{
public:
	LogImpl() : logFile_("debug.log")
	{
		logFile_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
	}

	~LogImpl() {
		logFile_.close();
	}

	static LogImpl* instance() {
		static LogImpl* impl = new LogImpl;
		return impl;
	}

	QFile logFile_;
};

Log::Log(int logLevel, const char * filename, int lineNo, bool halt)
	: logLevel_(logLevel), filename_(filename), lineNo_(lineNo), halt_(halt)
{
}

void Log::log(int level, const char * file, int line, bool halt, const QByteArray & msgContent)
{
	QByteArray msg;
	msg.reserve(256);
	msg.resize(52);
	char * pos = (char *)msg.constData(); // constData for performance

	// timestamp
	const QDateTime now = QDateTime::currentDateTime();
	LogDetails::writePadded(&pos, now.date().year(), 4);
	*(pos++) = '-';
	LogDetails::writePadded(&pos, now.date().month(), 2, '0');
	*(pos++) = '-';
	LogDetails::writePadded(&pos, now.date().day(), 2, '0');
	*(pos++) = ' ';
	LogDetails::writePadded(&pos, now.time().hour(), 2, '0');
	*(pos++) = ':';
	LogDetails::writePadded(&pos, now.time().minute(), 2, '0');
	*(pos++) = ':';
	LogDetails::writePadded(&pos, now.time().second(), 2, '0');
	*(pos++) = '.';
	LogDetails::writePadded(&pos, now.time().msec(), 3, '0');
	*(pos++) = ' ';
	LogDetails::writePadded(&pos, level, 1);
	*(pos++) = ' ';

	const char * filename = ::LogDetails::filenameSubStr(file);
	for (int i = 0 ; i < 20 ; ++i) *(pos++) = *filename ? *(filename++) : ' ';
	LogDetails::writePadded(&pos, line, 5);
	*(pos++) = ' ';

	msg.append(msgContent.constData(), msgContent.size());
	LogImpl::instance()->logFile_.write(msg);
	LogImpl::instance()->logFile_.write("\n");
	LogImpl::instance()->logFile_.flush();

	if(halt) {
		Q_ASSERT(false);
	}
}
