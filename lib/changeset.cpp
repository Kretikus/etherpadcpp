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
		case Changeset::NewLine:       return '|';
		case Changeset::Attrib:        return '*';
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
		ret += getOperationChar(it->first) + packNum(it->second);
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
		if (op.first == Changeset::KeepChars) { oOff += op.second; nOff += op.second; }
		else if(op.first == Changeset::SkipOverChars) nOff += op.second;
		else if(op.first == Changeset::InsertChars) {
			if ( (i-2) >= 0 && cs.ops_[i-2].first == Changeset::KeepChars) {
				qDebug("OldStr: %s", qPrintable(oldText.mid(oOff-cs.ops_[i-2].second, oOff)));
				qDebug("New: %s", qPrintable(newText.mid(nOff, op.second)));
			}
		}
	}

	bool removeAllOps = true;
	foreach(const Changeset::Op & op, cs.ops_) {
		if (op.second != 0) removeAllOps = false;
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
		if (!addedAttrib) { ops.append(qMakePair(Changeset::Attrib, 0)); addedAttrib = true; }
		ops.append(qMakePair(Changeset::InsertChars, newText.length()));
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
			if (oldOffset < oldText.length() || newOffset < newText.length()) ops.append(qMakePair(Changeset::KeepChars, keep));
		}

		const int deleteCount = detail::getCharDeleteCount(QStringRef(&oldText, oldOffset, oldText.length() - oldOffset), QStringRef(&newText, newOffset, newText.length() - newOffset));
		if (deleteCount > 0) {
			newOffset += deleteCount;
			ops.append(qMakePair(Changeset::SkipOverChars, deleteCount));
		}

//		qDebug("OldOffset: %d", oldOffset);
		if (oldOffset >= oldText.length()) {
			if (newText.length() - newOffset > 0) {
				if (!addedAttrib) { ops.append(qMakePair(Changeset::Attrib, 0)); addedAttrib = true; }
				ops.append(qMakePair(Changeset::InsertChars, newText.length() - newOffset));
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
				if (!addedAttrib) { ops.append(qMakePair(Changeset::Attrib, 0)); addedAttrib = true; }
				ops.append(qMakePair(Changeset::InsertChars, insertCount));
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

/**
	function _newlines(t) {
        var newlines = t.match(/\n/g);
        if (newlines == null) {
            return '';
        }
        return '|' + packNum(newlines.length);
    }

	function _diff(o, n){
        var ns = {};
        var os = {};
        var i;
        var x = null;
        for (i=0; i<n.length; i++) {
            if (ns[n[i]] == x) {
                ns[n[i]] = {rows:[], o:x};
            }
            ns[n[i]].rows.push(i)
        }
        for (i=0; i<o.length; i++) {
            if(os[o[i]] == x) {
                os[o[i]] = {rows:[], n:x};
            }
            os[o[i]].rows.push(i);
        }
        for (i in ns){
            if (ns[i].rows.length == 1 && typeof(os[i]) != 'undefined' && os[i].rows.length == 1) {
                n[ns[i].rows[0]] = {text:n[ns[i].rows[0]], row:os[i].rows[0]};
                o[os[i].rows[0]] = {text:o[os[i].rows[0]], row:ns[i].rows[0]};
            }
        }
        for (i=0; i<n.length-1; i++){
            if (n[i].text != x && n[i+1].text == x && n[i].row + 1 < o.length
                && o[n[i].row+1].text == x && n[i+1]==o[n[i].row+1]) {
                n[i+1] = {text:n[i+1], row:n[i].row+1};
                o[n[i].row+1] = {text:o[n[i].row+1], row:i+1};
            }
        }
        for(i=n.length-1; i>0; i--){
            if(n[i].text!=x && n[i-1].text==x && n[i].row>0 && o[n[i].row-1].text==x &&
            n[i-1] == o[n[i].row-1]) {
                n[i-1] = {text:n[i-1], row:n[i].row - 1};
                o[n[i].row-1] = {text:o[n[i].row-1], row:i - 1};
            }
        }
        return {o:o, n:n}
    }

var packNum = function(num) { return num.toString(36).toLowerCase(); };


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
