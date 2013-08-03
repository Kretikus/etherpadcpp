#include "changeset.h"
#include "utils.h"
#include "log.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#include <QDebug>

void AttributePool::fromJSON(const QJsonObject & obj)
{
	if (!obj.contains("nextNum") || !obj.contains("numToAttrib")) {
		qDebug("Could not parse AttributePool");
		return;
	}
	nextNum_ = obj.value("nextNum").toDouble();

	numToAttrib_.clear();
	const QJsonObject numToAtribObj =  obj.value("numToAttrib").toObject();
	Q_FOREACH(const QString & key, numToAtribObj.keys()) {
		const int keyAsInt = key.toInt();
		const QJsonArray arr = numToAtribObj.value(key).toArray();
		if (arr.size() != 2) {
			qDebug("No key value pair in key: %s", qPrintable(key));
			continue;
		}
		numToAttrib_[keyAsInt] = qMakePair(arr[0].toString(), arr[1].toString());
	}
}

QString Changeset::OperationData::toString() const
{
	QString ret;
	ret += "l: " + QString::number(opLength) +  " n: " + QString::number(newlines) +  " a: " + QString::number(attrib);
	return ret;
}

static QString packNum(int num) {
	return Util::base36enc(num);
}

static QChar getOperationChar(Changeset::Operation op) {
	switch (op) {
		case Changeset::InsertChars:   return '+';
		case Changeset::SkipOverChars: return '-';
		case Changeset::KeepChars:     return '=';
	}
	LOG_HALT("Called with unknown Changeset::Operation %1%", (int)op);
	return QChar();
}

const QString Changeset::prefix("Z:");

QString Changeset::toString() const
{
	QString ret = prefix;
	ret += packNum(oldLength_);
	ret += (newLength_ >= oldLength_)
				? '>' + packNum(newLength_ - oldLength_)
				: '<' + packNum(oldLength_ - newLength_);
	for (Ops::ConstIterator it = ops_.begin(); it != ops_.end(); ++it) {
		const OperationData & opData = it->second;
		if (opData.attrib != -1) {
			ret += QChar('*') + packNum(opData.attrib);
		}
		if (opData.newlines > 0) {
			ret += QChar('|') + packNum(opData.newlines);
		}
		ret += getOperationChar(it->first) + packNum(opData.opLength);
	}
	ret += "$";
	ret += bank_;
	return ret;
}

Changeset Changeset::fromString(const QString & str)
{
	Changeset changeset;
	if (!str.startsWith("Z:")) return changeset;

	QRegExp sizesExpression("^Z:(\\w+)([><])(\\w+)");
	int pos = sizesExpression.indexIn(str);
	if (pos != 0) return changeset;
	const QStringList captured = sizesExpression.capturedTexts();
	if (sizesExpression.captureCount() != 3) return changeset;

	changeset.oldLength_ = Util::base36dec(captured[1].toLatin1());
	const int sizeDiff = Util::base36dec(captured[3].toLatin1());
	changeset.newLength_ = changeset.oldLength_;
	if (captured[2] == ">") {
		changeset.newLength_ += sizeDiff;
	} else {
		changeset.newLength_ -= sizeDiff;
	}
	pos += sizesExpression.matchedLength();

	QRegExp opsMatch("([=\\+\\-\\*\\|]\\w+)");
	int newLines = -1;
	int attrib = -1;
	while (opsMatch.indexIn(str, pos) != -1) {
		QString op = opsMatch.capturedTexts().at(1);
		QChar c = op[0];
		const int opLength = Util::base36dec(op.mid(1).toLatin1());
		if (c == '|') {
			if(newLines == -1) newLines = 0;
			newLines += opLength;
		} else if (c == '*') {
			attrib = opLength;
		} else if (c == '=') {
			changeset.ops_.append(qMakePair(Changeset::KeepChars, Changeset::OperationData(opLength, newLines, attrib)));
			newLines = -1; attrib = -1;
		} else if (c == '+') {
			changeset.ops_.append(qMakePair(Changeset::InsertChars, Changeset::OperationData(opLength, newLines, attrib)));
			newLines = -1; attrib = -1;
		} else if (c == '-') {
			changeset.ops_.append(qMakePair(Changeset::SkipOverChars, Changeset::OperationData(opLength, newLines, attrib)));
			newLines = -1; attrib = -1;
		}
		pos += opsMatch.matchedLength();
	}
	changeset.bank_ = str.mid(pos+1);

	return changeset;
}

Changeset createChangeset(const QString & oldText, const QString & newText)
{
	Changeset result = JS::createChangeset(oldText, newText);
	result = JS::optimizeChangeset(oldText, result);

	//	if (applyChangeset(oldText, result) != newText) {
	//		func = alert;
	//		//func = console.log;
	//		func("Changeset Generation Failed! Application yields '" +
	//			  applyChangeset(oldText, result) + "' instead of '" + newText + "'");
	//	}
	return result;
}

class Rows {
public:
	Rows()  {}
	QVector<int> rows;
};

static bool isValid(JS::DiffOut & o, int i)
{
	if (i<0) return false;
	return i < o.size();
}

QPair<JS::DiffOut,JS::DiffOut> JS::diff(const QStringList & oldTextLines, const QStringList & newTextLines)
{
	typedef QMap<QString, Rows> String2Row;
	String2Row ns;
	String2Row os;

	DiffOut oDo;
	std::copy(oldTextLines.begin(), oldTextLines.end(), std::back_inserter(oDo));
	DiffOut nDo;
	std::copy(newTextLines.begin(), newTextLines.end(), std::back_inserter(nDo));

	for (int i=0; i<newTextLines.size(); ++i) {
		ns[newTextLines[i]].rows.append(i);
	}

	for  (int i=0; i<oldTextLines.size(); ++i) {
		os[oldTextLines[i]].rows.append(i);
	}

	Q_FOREACH (const QString & sin, ns.keys()){
		if (ns[sin].rows.size() == 1 && os.contains(sin) && os[sin].rows.size() == 1) {
			const int nsIdx = ns[sin].rows[0];
			const int osIdx = os[sin].rows[0];
			nDo[nsIdx] = DiffOutData(newTextLines[nsIdx], osIdx);
			oDo[osIdx] = DiffOutData(oldTextLines[osIdx], nsIdx);
		}
	}
	for (int i=0; i<nDo.size()-1; ++i){
		if (isValid(nDo, i) && isValid(nDo, i+1) && nDo[i].row + 1 < oDo.size()
				&& oDo[nDo[i].row+1].isValid() && nDo[i+1] == oDo[nDo[i].row+1])
		{
			nDo[i+1]          = DiffOutData(newTextLines[i+1], nDo[i].row+1);
			oDo[nDo[i].row+1] = DiffOutData(oldTextLines[nDo[i].row+1], i+1);
		}
	}
	for(int i=nDo.size(); i>0; --i){
		if(isValid(nDo, i) && isValid(nDo, i-1) && nDo[i].row > 0 &&
		   isValid(oDo, nDo[i].row-1) && nDo[i-1] == oDo[nDo[i].row-1])
		{
			nDo[i-1] = DiffOutData(newTextLines[i-1], nDo[i].row-1);
			oDo[nDo[i].row-1] = DiffOutData(oldTextLines[nDo[i].row-1], i-1);
		}
	}
	return qMakePair(oDo, nDo);
}

int JS::newLines(const QString & text)
{
	const int count = text.count('\n');
	if (count == 0) return -1;
	return count;
}

Changeset JS::collapse(const Changeset & changeset)
{
	Changeset collapsed = changeset;
	collapsed.ops_.clear();
	for (auto it = changeset.ops_.begin(); it != changeset.ops_.end(); ++it)
	{
		const auto nextIt = it + 1;

		if (nextIt != changeset.ops_.end() && it->first == nextIt->first) {
			Changeset::Op collapsedOp = *it;
			collapsedOp.second.opLength += nextIt->second.opLength;
			if(nextIt->second.newlines != -1) {
				collapsedOp.second.newlines += nextIt->second.newlines;
			}
			collapsed.ops_.append(collapsedOp);
			++it; // skip 2
		} else {
			if (nextIt != changeset.ops_.end() || it->first != Changeset::KeepChars) {
				collapsed.ops_.append(*it);
			}
		}
	}
	return collapsed;
}

Changeset JS::optimizeChangeset(const QString & oldText, const Changeset & changeset) {
	Changeset newChangeset = changeset;
	Changeset beforeOptimizing;
	do {
		beforeOptimizing = newChangeset;
		newChangeset = JS::collapse(newChangeset);
		newChangeset = JS::optimize(newChangeset, oldText);
		newChangeset = JS::collapse(newChangeset);
	} while (newChangeset != beforeOptimizing);
	return newChangeset;
}

Changeset JS::createChangeset(const QString & oldText, const QString & newText)
{
	if (oldText.isEmpty() && newText.isEmpty()) return Changeset();

	if (newText.isEmpty()) {
		return Changeset(oldText.length(), 0, Changeset::Ops() << qMakePair(Changeset::SkipOverChars, Changeset::OperationData(oldText.length(), -1, -1)), QString());
	}

	QStringList oldList = oldText.isEmpty() ? QStringList() : oldText.split(" ");
	QStringList newList = newText.isEmpty() ? QStringList() : newText.split(" ");

	QPair<JS::DiffOut, JS::DiffOut> out = JS::diff(oldList, newList);

	Changeset::Ops ops;

	int oSpace = oldText.count(" ");
	int nSpace = newText.count(" ");
	JS::DiffOut & outO = out.first;
	JS::DiffOut & outN = out.second;

	const QString sp = " ";

	// Deletion from the beginning of the string
	if (outN[0].row != 0) {
		for(int n=0; n<outO.size() && outO[n].row == -1; ++n) {
			QString currentText = outO[n].text + (n >= oSpace ? "" : sp);
			ops.append(qMakePair(Changeset::SkipOverChars, Changeset::OperationData(currentText.length(), JS::newLines(currentText), -1)));
		}
	}

	QString bank;

	Changeset::Ops potentialOps;

	// Iterate over tokens in new text
	for (int i=0; i<outN.size(); ++i) {

		// Addition (token in new text does not exist in old)
		if (outN[i].row == -1) {
			const QString currentText = outN[i].text + (i >= nSpace ? "" : sp);
			ops += potentialOps;
			potentialOps.clear();
			ops.append(qMakePair(Changeset::InsertChars, Changeset::OperationData(currentText.length(), JS::newLines(currentText), 0)));
			bank += currentText;
		} else {
			Changeset::Ops dels;
			int nextWordInOldPos = outN[i].row + 1;

			// Deletion Check
			//
			// If the next word has been deleted from the old text, check to see
			// if we're also missing the space following this word
			//
			if (nextWordInOldPos < outO.size() && outO[nextWordInOldPos].row == -1 &&
				i >= nSpace) {
				dels.append(qMakePair(Changeset::SkipOverChars, Changeset::OperationData(1, -1, -1)));;
			}

			//
			// Check old text tokens starting with the one corresponding to the position
			// after our current word, and for each of them that's deleted, append
			// the deletion operator to a temporary variable that we'll dump in a moment
			//
			for (int n = nextWordInOldPos; n < outO.size() && outO[n].row == -1; ++n) {
				const QString currentText = outO[n].text + (n >= oSpace ? "" : sp);
				dels.append(qMakePair(Changeset::SkipOverChars, Changeset::OperationData(currentText.length(), JS::newLines(currentText), -1)));
			}

			// Writing Operators

			//
			// Add the skip operator to our holding variable for skips, unless we've
			// got deletions from the previous step, in which case dump all the skip
			// operators into the changeset, followed by the deletion operators
			//
			QString currentText = outN[i].text;
			if ( (i + 1) < outN.size() && outN[i + 1].row == -1 &&
					outN[i].row >= oSpace) {
				//dels = '*0+1' + dels;
				dels.prepend(qMakePair(Changeset::InsertChars, Changeset::OperationData(1, -1, 0)));
				bank += " ";
			} else {
				currentText += (i >= nSpace) ? "" : sp;
			}
			if (dels.isEmpty()) {
				potentialOps.append(qMakePair(Changeset::KeepChars, Changeset::OperationData(currentText.length(), JS::newLines(currentText), -1)));
			} else {
				ops += potentialOps;
				potentialOps.clear();
				ops.append(qMakePair(Changeset::KeepChars,  Changeset::OperationData(currentText.length(), JS::newLines(currentText), -1)));
				ops += dels;
			}
		}
	}

	return Changeset(oldText.length(), newText.length(), ops, bank);
}

Changeset JS::optimize(const Changeset & changeset, const QString & oldText)
{
	QString changesetBank = changeset.bank_;

	QString text = oldText;

	Changeset optimized(changeset.oldLength_, changeset.newLength_, Changeset::Ops(), QString());

	const Changeset::Ops::ConstIterator opsEnd = changeset.ops_.end();
	Changeset::Ops::ConstIterator prevPart = opsEnd;

	for (auto it = changeset.ops_.begin(); it != opsEnd; ++it) {
		if (prevPart != opsEnd && it->first == Changeset::InsertChars) {
			QString textPart = text.mid(0, prevPart->second.opLength);
			QString potPart = changesetBank.mid(0, it->second.opLength);
			int i = textPart.length()-1;
			int j = potPart.length()-1;
			int  newlines = 0;
			while ((i >= 0) && (j > 0) && (textPart[i] == potPart[j])) {
				if (textPart[i] == '\n') {
					newlines++;
				}
				--i;
				--j;
			}
			const int len = textPart.length() - 1 - i;
//			qDebug() << "i:" << i << " j: " << j << " len: " << len;

			Changeset::Op prevOp = *prevPart;
			prevOp.second.opLength -= len;
			prevOp.second.newlines -= newlines;

			Changeset::Op curOp  = *it;
			curOp.second.opLength -= len;
			curOp.second.newlines -= newlines;

//			qDebug() << "PrevOp " << prevOp.first << ", " << prevOp.second.toString();
//			qDebug() << "CurOp " << curOp.first << ", " << curOp.second.toString();

			const Changeset::Op newOpPost(qMakePair(Changeset::KeepChars, Changeset::OperationData(len, newlines, -1)));

//			qDebug() << "NewOp " << newOpPost.first << ", " << newOpPost.second.toString();
			textPart = textPart.mid(0, i+1);
			potPart  = potPart.mid(0, j+1);
			i = 0;
			newlines = 0;
			for (i = 0; (i < textPart.length()) && (i < potPart.length()) && textPart[i] == potPart[i] ; ++i) {
				if (textPart[i] == '\n')
					++newlines;
			}
			if (i > 0) {
//				qDebug("BigI");
				prevOp.second.opLength -= i;
				prevOp.second.newlines -= newlines;
				curOp.second.opLength -= i;
				curOp.second.newlines -= newlines;
				const Changeset::Op newOp(qMakePair(Changeset::KeepChars, Changeset::OperationData(i, newlines, -1)));

				optimized.ops_.push_back(newOp);
				text = text.mid(i);
			}

			if (prevOp.second.opLength > 0) {
				optimized.ops_.push_back(prevOp);
				text = text.mid(prevOp.second.opLength);
			}
			if (curOp.second.opLength > 0) {
				optimized.ops_.push_back(curOp);
			}
			if (newOpPost.second.opLength > 0) {
				optimized.ops_.push_back(newOpPost);
			}

			optimized.bank_ += changesetBank.mid(i, curOp.second.opLength);
			changesetBank = changesetBank.mid(i + curOp.second.opLength + len);
			prevPart = opsEnd;
		} else {
			if (prevPart != opsEnd) {
				//unoptimized '-' op
				text = text.mid(prevPart->second.opLength);
				optimized.ops_.push_back(*prevPart);
				prevPart = opsEnd;
			}
			switch (it->first) {
				case Changeset::KeepChars:
					text = text.mid(it->second.opLength);
					optimized.ops_.push_back(*it);
					break;
				case Changeset::InsertChars:
					optimized.bank_ += changesetBank.mid(0, it->second.opLength);
					changesetBank = changesetBank.mid(it->second.opLength);
					optimized.ops_.push_back(*it);
					break;
				case Changeset::SkipOverChars:
					prevPart = it;
					break;
				}
			}
		}
		if (prevPart != opsEnd) {
			optimized.ops_.push_back(*prevPart);
		}
	return optimized;
}

QString applyChangeset(const QString & oldText, const Changeset & changeset)
{
	QString ret = oldText;

	int pos = 0;
	int bankPos = 0;
	Q_FOREACH(const Changeset::Op & op, changeset.ops_) {
		if (op.first == Changeset::KeepChars) {
			pos += op.second.opLength;
		} else if (op.first == Changeset::InsertChars) {
			const int len = op.second.opLength;
			const QString insStr = changeset.bank_.mid(bankPos, len);
			ret.insert(pos, insStr);
			pos += len;
			bankPos += len;
		} else if (op.first == Changeset::SkipOverChars) {
			ret.remove(pos, op.second.opLength);
		}
	}

	return ret;
}


/*
// %CLIENT FILE ENDS HERE%
exports.follow = function (cs1, cs2, reverseInsertOrder, pool) {
  var unpacked1 = exports.unpack(cs1);
  var unpacked2 = exports.unpack(cs2);
  var len1 = unpacked1.oldLen;
  var len2 = unpacked2.oldLen;
  exports.assert(len1 == len2, "mismatched follow");
  var chars1 = exports.stringIterator(unpacked1.charBank);
  var chars2 = exports.stringIterator(unpacked2.charBank);

  var oldLen = unpacked1.newLen;
  var oldPos = 0;
  var newLen = 0;

  var hasInsertFirst = exports.attributeTester(['insertorder', 'first'], pool);

  var newOps = exports.applyZip(unpacked1.ops, 0, unpacked2.ops, 0, function (op1, op2, opOut) {
	if (op1.opcode == '+' || op2.opcode == '+') {
	  var whichToDo;
	  if (op2.opcode != '+') {
		whichToDo = 1;
	  } else if (op1.opcode != '+') {
		whichToDo = 2;
	  } else {
		// both +
		var firstChar1 = chars1.peek(1);
		var firstChar2 = chars2.peek(1);
		var insertFirst1 = hasInsertFirst(op1.attribs);
		var insertFirst2 = hasInsertFirst(op2.attribs);
		if (insertFirst1 && !insertFirst2) {
		  whichToDo = 1;
		} else if (insertFirst2 && !insertFirst1) {
		  whichToDo = 2;
		}
		// insert string that doesn't start with a newline first so as not to break up lines
		else if (firstChar1 == '\n' && firstChar2 != '\n') {
		  whichToDo = 2;
		} else if (firstChar1 != '\n' && firstChar2 == '\n') {
		  whichToDo = 1;
		}
		// break symmetry:
		else if (reverseInsertOrder) {
		  whichToDo = 2;
		} else {
		  whichToDo = 1;
		}
	  }
	  if (whichToDo == 1) {
		chars1.skip(op1.chars);
		opOut.opcode = '=';
		opOut.lines = op1.lines;
		opOut.chars = op1.chars;
		opOut.attribs = '';
		op1.opcode = '';
	  } else {
		// whichToDo == 2
		chars2.skip(op2.chars);
		exports.copyOp(op2, opOut);
		op2.opcode = '';
	  }
	} else if (op1.opcode == '-') {
	  if (!op2.opcode) {
		op1.opcode = '';
	  } else {
		if (op1.chars <= op2.chars) {
		  op2.chars -= op1.chars;
		  op2.lines -= op1.lines;
		  op1.opcode = '';
		  if (!op2.chars) {
			op2.opcode = '';
		  }
		} else {
		  op1.chars -= op2.chars;
		  op1.lines -= op2.lines;
		  op2.opcode = '';
		}
	  }
	} else if (op2.opcode == '-') {
	  exports.copyOp(op2, opOut);
	  if (!op1.opcode) {
		op2.opcode = '';
	  } else if (op2.chars <= op1.chars) {
		// delete part or all of a keep
		op1.chars -= op2.chars;
		op1.lines -= op2.lines;
		op2.opcode = '';
		if (!op1.chars) {
		  op1.opcode = '';
		}
	  } else {
		// delete all of a keep, and keep going
		opOut.lines = op1.lines;
		opOut.chars = op1.chars;
		op2.lines -= op1.lines;
		op2.chars -= op1.chars;
		op1.opcode = '';
	  }
	} else if (!op1.opcode) {
	  exports.copyOp(op2, opOut);
	  op2.opcode = '';
	} else if (!op2.opcode) {
	  // @NOTE: Critical bugfix for EPL issue #1625. We do not copy op1 here
	  // in order to prevent attributes from leaking into result changesets.
	  // exports.copyOp(op1, opOut);
	  op1.opcode = '';
	} else {
	  // both keeps
	  opOut.opcode = '=';
	  opOut.attribs = exports.followAttributes(op1.attribs, op2.attribs, pool);
	  if (op1.chars <= op2.chars) {
		opOut.chars = op1.chars;
		opOut.lines = op1.lines;
		op2.chars -= op1.chars;
		op2.lines -= op1.lines;
		op1.opcode = '';
		if (!op2.chars) {
		  op2.opcode = '';
		}
	  } else {
		opOut.chars = op2.chars;
		opOut.lines = op2.lines;
		op1.chars -= op2.chars;
		op1.lines -= op2.lines;
		op2.opcode = '';
	  }
	}
	switch (opOut.opcode) {
	case '=':
	  oldPos += opOut.chars;
	  newLen += opOut.chars;
	  break;
	case '-':
	  oldPos += opOut.chars;
	  break;
	case '+':
	  newLen += opOut.chars;
	  break;
	}
  });
  newLen += oldLen - oldPos;

  return exports.pack(oldLen, newLen, newOps, unpacked2.charBank);
};
*/

