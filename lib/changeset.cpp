#include "changeset.h"
#include "utils.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

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

static QString packNum(int num) {
	return Util::base36enc(num);
}

static QChar getOperationChar(Changeset::Operation op) {
	switch (op) {
		case Changeset::InsertChars:   return '+';
		case Changeset::SkipOverChars: return '-';
		case Changeset::KeepChars:     return '=';
	}
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
		if (opData.newlines != -1) {
			ret += QChar('|') + packNum(opData.newlines);
		}
		ret += getOperationChar(it->first) + packNum(opData.opLength);
	}
	ret += "$";
	ret += bank_;
	return ret;
}


int detail::getMaxPrefix(const QStringRef & oldText, const QStringRef & newText)
{
	int offset = 0;
	for ( ; offset < oldText.length() && offset < newText.length() && oldText.at(offset) == newText.at(offset); ++offset);
	return offset;
}

int detail::getCharDeleteCount(const QStringRef & oldText, const QStringRef & newText)
{
	int deleteChars = 0;
	for (int o = 0; o < oldText.length(); ++o) {
		if (newText.contains(oldText.at(o))) break;
		++deleteChars;
	}
	return deleteChars;
}

Changeset detail::optimizeChangeset(const Changeset & changeset, const QString & oldText, const QString & newText)
{
	Changeset cs = changeset;
	// optimization

	int oOff = 0;
	int nOff = 0;
	for(int i = 0; i < cs.ops_.size(); ++i) {
		const Changeset::Op & op = cs.ops_[i];
		if (op.first == Changeset::KeepChars) { oOff += op.second.opLength; nOff += op.second.opLength; }
		else if(op.first == Changeset::SkipOverChars) nOff += op.second.opLength;
		else if(op.first == Changeset::InsertChars) {
			if ( (i-2) >= 0 && cs.ops_[i-2].first == Changeset::KeepChars) {
				qDebug("OldStr: %s", qPrintable(oldText.mid(oOff-cs.ops_[i-2].second.opLength, oOff)));
				qDebug("New: %s", qPrintable(newText.mid(nOff, op.second.opLength)));
			}
		}
	}

	bool removeAllOps = true;
	foreach(const Changeset::Op & op, cs.ops_) {
		if (op.second.opLength != 0) removeAllOps = false;
	}
	if (removeAllOps) cs.ops_ = Changeset::Ops();

	return cs;
}


Changeset createChangeset(const QString & oldText, const QString & newText)
{
	if (oldText.isEmpty() && newText.isEmpty()) return Changeset();

	qDebug("Getting changeset of : '%s' and '%s'", qPrintable(oldText), qPrintable(newText));

	Changeset::Ops ops;
	QString bank;

	bool addedAttrib = false;

	int oldOffset = 0;
	int newOffset = 0;

	if (oldText.isEmpty() && !newText.isEmpty()) {
		int attrib = -1;
		if (!addedAttrib) { attrib = 0; addedAttrib = true; }
		ops.append(qMakePair(Changeset::InsertChars, Changeset::OperationData(newText.length(), -1, attrib)));
		 bank += newText;
	}

	// Strategy:
	// 1. get maximum common prefix
	// 2. check if there are chars in the sourcetext that needs deleting
	// 3. add the maximum amount of new chars
	// start from the beginning....

	while (oldOffset < oldText.length() && newOffset < newText.length())
	{
		const int keep = detail::getMaxPrefix(QStringRef(&oldText, oldOffset, oldText.length() - oldOffset), QStringRef(&newText, newOffset, newText.length() - newOffset));
//		qDebug("keep: %d", keep);
		if (keep > 0) {
			oldOffset += keep;
			newOffset += keep;
			if (oldOffset < oldText.length() || newOffset < newText.length()) {
				ops.append(qMakePair(Changeset::KeepChars, Changeset::OperationData(keep, -1, -1)));
			}
		}

		const int deleteCount = detail::getCharDeleteCount(QStringRef(&oldText, oldOffset, oldText.length() - oldOffset), QStringRef(&newText, newOffset, newText.length() - newOffset));
		if (deleteCount > 0) {
			newOffset += deleteCount;
			ops.append(qMakePair(Changeset::SkipOverChars, Changeset::OperationData(deleteCount, -1, -1)));
		}

//		qDebug("OldOffset: %d", oldOffset);
		if (oldOffset >= oldText.length()) {
			if (newText.length() - newOffset > 0) {
				int attrib = -1;
				if (!addedAttrib) { attrib = 0; addedAttrib = true; }
				ops.append(qMakePair(Changeset::InsertChars, Changeset::OperationData(newText.length() - newOffset, -1, attrib)));
				const QString appendedText = newText.mid(newOffset);
				bank += appendedText;
				newOffset += appendedText.length();
			}
		} else {
			int insertCount = 0;
			const QChar c = oldText[oldOffset];
			while(newOffset+insertCount < newText.length() && newText[newOffset+insertCount] != c) {
				++insertCount;
			}
			if (insertCount > 0) {
				int attrib = -1;
				if (!addedAttrib) { attrib = 0; addedAttrib = true; }
				ops.append(qMakePair(Changeset::InsertChars,  Changeset::OperationData(insertCount, -1, attrib)));
				bank += newText.mid(newOffset, insertCount);
				newOffset += insertCount;
			}
		}
		++oldOffset;
		++newOffset;
	}

	Changeset changeset(oldText.length(), newText.length(), ops, bank);

	return detail::optimizeChangeset(changeset, oldText, newText);
}


class Rows {
public:
	Rows() : o(0) {}
	QVector<int> rows;
	void * o;
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

Changeset JS::optimizeChangeset(const QString & oldText, const Changeset & changeset) {

//  var append_part = function(changeset, part) {
//    var packNum = function(num) { return num.toString(36).toLowerCase(); };
//    for (var i = 0; i < part.attribs.length; i++) {
//      changeset += "*" + part.attribs[i];
//    }
//    if (part.newlines > 0)
//      changeset += "|" + packNum(part.newlines);
//    changeset += part.op + packNum(part.len);
//    return changeset
//  };

//  var compareAttribs = function(attribs1, attribs2) {
//    attribs1 = attribs1.slice(0);
//    attribs2 = attribs2.slice(0);
//    if (attribs1.length != attribs2.length)
//      return false;
//    for (var i = 0; i < attribs1.length; i++)
//      if (attribs1[i] != attribs2[i])
//        return false;
//    return true;
//  };

//  var collapse = function(changeset) {
//    parsed = parseChangeset(changeset);
//    collapsed = parsed.prefix;
//    var prevPart = null;
//    for (var part = parsed.ops.next(); part != null; part = parsed.ops.next()) {
//      if (prevPart && prevPart.op == part.op && compareAttribs(prevPart.attribs, part.attribs)) {
//        prevPart.len += part.len;
//        prevPart.newlines += part.newlines;
//      } else {
//        if (prevPart) {
//          collapsed = append_part(collapsed, prevPart);
//          prevPart = null;
//        }
//        prevPart = part;
//      }
//    }
//    if (prevPart && prevPart.op != "=")
//      collapsed = append_part(collapsed, prevPart);
//    collapsed += "$" + parsed.bank;

//    return collapsed;
	return Changeset();
  };


Changeset JS::createChangeset(const QString & oldText, const QString & newText)
{
	if (oldText.isEmpty() && newText.isEmpty()) return Changeset();

	QPair<JS::DiffOut,JS::DiffOut> out = JS::diff(oldText.split(" "), newText.split(" "));

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
	
	// Iterate over tokens in new text
	for (int i=0; i<outN.size(); ++i) {

		Changeset::Ops potentialOps;
		
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
				potentialOps.append(qMakePair(Changeset::KeepChars,  Changeset::OperationData(currentText.length(), JS::newLines(currentText), -1)));
			} else {
				ops += potentialOps;
				potentialOps.clear();
				ops.append(qMakePair(Changeset::KeepChars,  Changeset::OperationData(currentText.length(), JS::newLines(currentText), -1)));
				ops += dels;
			}
		}
	}

//	result = optimizeChangeset(oldText, str);

//	if (applyChangeset(oldText, result) != newText) {
//		func = alert;
//		//func = console.log;
//		func("Changeset Generation Failed! Application yields '" + 
//			  applyChangeset(oldText, result) + "' instead of '" + newText + "'");
//	}

	return Changeset(oldText.length(), newText.length(), ops, bank);
}

/**

function generateChangeset(oldText, newText){

  if (newText == null || oldText == null)
    alert("Null text: ");

    var str = 'Z:' + packNum(oldText.length);
    str += newText.length >= oldText.length 
        ? '>' + packNum(newText.length - oldText.length) 
        : '<' + packNum(oldText.length - newText.length); 
    
    // Contains two sequences of tokens representing the substrings with relations to each other
    var out = _diff(oldText == '' ? [] : oldText.split(/ /), newText == '' ? [] : newText.split(/ /));
    var pot = '';
    var potentialStr = '';
    var currentText;
    var oSpace = oldText.match(/ /g);
    var nSpace = newText.match(/ /g);

    if (oSpace == null) {
        oSpace=[];
    }

    if (nSpace == null) {
        nSpace=[];
    }
    
    // Deletion from the beginning of the string
    if (out.n[0].row != 0) {
        for(n=0; n<out.o.length && out.o[n].text==null; n++) {
            currentText = out.o[n] + (n >= oSpace.length ? '' : oSpace[n]);
            str += _newlines(currentText) + '-' + packNum(currentText.length);
        }
    }

    // Iterate over tokens in new text
    for (var i=0; i<out.n.length; i++) {

        // Addition (token in new text does not exist in old)
        if (out.n[i].text == null) {
            currentText = out.n[i] + (i >= nSpace.length ? '' : nSpace[i]);
            str += potentialStr;
            str += '*0' + _newlines(currentText) + '+' + packNum(currentText.length);
            potentialStr = '';
            pot += currentText;
        
        // Skip, but may also be followed by deletions
        } else {
            var dels = '';
            var nextWordInOldPos = out.n[i].row + 1;

            // Deletion Check

            // 
            // If the next word has been deleted from the old text, check to see
            // if we're also missing the space following this word
            //
            if (nextWordInOldPos < out.o.length && 
                    out.o[nextWordInOldPos].text == null && i 
                    >= nSpace.length) {
                dels += '-1';
            }

            //
            // Check old text tokens starting with the one corresponding to the position
            // after our current word, and for each of them that's deleted, append
            // the deletion operator to a temporary variable that we'll dump in a moment
            //
            for (n = nextWordInOldPos; n < out.o.length && out.o[n].text == null; n++) {
                currentText = out.o[n] + (n >= oSpace.length ? '' : oSpace[n]);
                dels += _newlines(currentText) + '-' + packNum(currentText.length);
            }

            // Writing Operators

            //
            // Add the skip operator to our holding variable for skips, unless we've
            // got deletions from the previous step, in which case dump all the skip
            // operators into the changeset, followed by the deletion operators
            //
            currentText = out.n[i].text;
            if (i + 1 < out.n.length && out.n[i + 1].text == null && 
                    out.n[i].row >= oSpace.length) {
                dels = '*0+1' + dels;
                pot += ' ';
            } else {
                currentText += (i >= nSpace.length ? '' : nSpace[i]);
            }
            if (dels == '') {
                potentialStr += _newlines(currentText) + '=' + packNum(currentText.length);
            } else {
                str += potentialStr;
                str += _newlines(currentText) + '=' + packNum(currentText.length) + dels;
                potentialStr = '';
            }
        }
    }

    str = str + '$' + pot;

    result = optimizeChangeset(oldText, str);

    if (applyChangeset(oldText, result) != newText) {
      func = alert;
      //func = console.log;
        func("Changeset Generation Failed! Application yields '" + 
              applyChangeset(oldText, result) + "' instead of '" + newText + "'");
    }

    return result;
}
*/

