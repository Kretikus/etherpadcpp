#pragma once

#include <QByteArray>

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_HALT  3

#define LOG_TRACE Log(LOG_LEVEL_TRACE, __FILE__, __LINE__)
#define LOG_INFO  Log(LOG_LEVEL_WARN,  __FILE__, __LINE__)
#define LOG_WARN  Log(LOG_LEVEL_HALT,  __FILE__, __LINE__)

#define LOG_HALT  Log(LOG_LEVEL_HALT,  __FILE__, __LINE__, true)

namespace LogDetails {
	void output(QByteArray & msg, int val);
	void output(QByteArray & msg, const QString & val);
	const char * filenameSubStr(const char * filePath);
	void writePadded(char ** dest, uint number, int width, char pad = ' ');
}

class Log
{
public:
	Log(int logLevel, const char* filename, int lineNo, bool halt = false);

	void operator()(const char * str) const {
		logImpl(QByteArray(str));
	}

	template<typename T1>
	void operator()(const char * str, const T1 & t1) const {
		QByteArray msg; msg.reserve(128);
		const int strLen = strlen(str);
		for(int i = 0; i < strLen; ++i) {
			const char * pStr = str + i;
			if (*pStr == '%' && strLen >= i+3 && *(pStr+1) == '1' && *(pStr+2) == '%') {
				LogDetails::output(msg, t1);
				i += 2;
			} else {
				msg.append(*pStr);
			}
		}
		logImpl(msg);
	}

	template<typename T1, typename T2>
	void operator()(const char * str, const T1 & t1, const T2 & t2) const {
		QByteArray msg; msg.reserve(128);
		const int strLen = strlen(str);
		for(int i = 0; i < strLen; ++i) {
			const char * pStr = str + i;
			if (*pStr == '%' && strLen >= i+3 && *(pStr+2) == '%') {
				const char placeHolder = *(pStr+1);
				if (placeHolder == '1') { LogDetails::output(msg, t1); }
				else if (placeHolder == '2') { LogDetails::output(msg, t2); }
				else qDebug("Unexpected placeholder '%c'", placeHolder);
				i += 2;
			} else {
				msg.append(*pStr);
			}
		}
		logImpl(msg);
	}

private:
		void logImpl(const QByteArray & msg) const { log(logLevel_, filename_, lineNo_, halt_, msg); }
		static void log(int level, const char * file, int line, bool halt, const QByteArray & msgContent);
private:
	int logLevel_;
	const char* filename_;
	int lineNo_;
	bool halt_;
};
