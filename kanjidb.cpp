#include "kanjidb.h"
#include "kanji.h"
#include "readingmeaninggroup.h"
#include <QRegExp>

const QString KanjiDB::ucsKey = QString("ucs=");
const QString KanjiDB::gradeKey = QString("grade=");
const QString KanjiDB::jlptKey = QString("jlpt=");
const QString KanjiDB::strokesKey = QString("strokes=");
const QString KanjiDB::strokesMoreKey = QString("strokes>");
const QString KanjiDB::strokesLessKey = QString("strokes<");
const QString KanjiDB::jis208Key = QString("jis208=");
const QString KanjiDB::jis212Key = QString("jis212=");
const QString KanjiDB::jis213Key = QString("jis213=");

KanjiDB::KanjiDB()
{
    //empty list returned when no match found
    kanjisByStroke[0] = new QSet<Kanji *>();
    maxStrokes = 0;
    minStrokes = 255;
}

bool KanjiDB::read(QIODevice *device)
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(device, true, &errorStr, &errorLine,
                                &errorColumn))
    {
        //TODO raise log error
        return false;
    }

    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "kanjidic2")
    {
        //TODO raise not kanjidic2 file error
        return false;
    }

    // kanjidic has 1 header node and pairs of comment and character nodes
    long count = ( root.childNodes().count() - 1 ) / 2;

    QDomElement child = root.firstChildElement("character");
    while (!child.isNull())
    {
        ++count;
        parseCharacterElement(child);
        child = child.nextSiblingElement("character");
    }

    return true;
}

void KanjiDB::parseCharacterElement(const QDomElement &element){
    QString title = element.firstChildElement("literal").text();
    Q_ASSERT(title.length() > 0);
    Kanji *k = new Kanji(title);

    bool ok;

    QDomElement codepoint= element.firstChildElement("codepoint");
    QDomElement child = codepoint.firstChildElement("cp_value");
    while (!child.isNull())
    {
        if(QString::compare(child.attribute("cp_type", ""), "ucs") == 0)
        {
            int unicode = child.text().toInt(&ok, 16);
            k->setUnicode(unicode);
            Q_ASSERT(unicode > 0);
            kanjis[unicode] = k;
        } else if(QString::compare(child.attribute("cp_type", ""), "jis208") == 0)
        {
            QString text = child.text();
            k->setJis208(text);
            kanjisJIS208[text] = k;
        }
        else if(QString::compare(child.attribute("cp_type", ""), "jis212") == 0)
        {
            QString text = child.text();
            k->setJis212(text);
            kanjisJIS212[text] = k;
        }
        else if(QString::compare(child.attribute("cp_type", ""), "jis213") == 0)
        {
            QString text = child.text();
            k->setJis213(text);
            kanjisJIS213[text] = k;
        }
        child = child.nextSiblingElement("cp_value");
    }

    QDomElement radical = element.firstChildElement("radical");
    child = radical.firstChildElement("rad_value");
    while (!child.isNull())
    {
        if(QString::compare(child.attribute("rad_type", ""), "classical") == 0)
        {
            int cRad = child.text().toInt(&ok, 10);
            k->setClassicalRadical(cRad);
            QSet<Kanji *> *set = kanjisByRadical[cRad];
            if(set == 0)
            {
                set = new QSet<Kanji *>();
                kanjisByRadical[cRad] = set;
            }
            set->insert(k);
        }
        else if(QString::compare(child.attribute("rad_type", ""), "nelson_c") == 0)
            k->setNelsonRadical(child.text().toInt(&ok, 10));
        child = child.nextSiblingElement("rad_value");
    }

    //parsing misc element of character
    QDomElement misc = element.firstChildElement("misc");

    child = misc.firstChildElement("grade");
    if(!child.isNull())
    {
        int grade = child.text().toInt(&ok, 10);
        k->setGrade(grade);
        QSet<Kanji *> *set = kanjisByGrade[grade];
        if(set == 0)
        {
            set = new QSet<Kanji *>();
            kanjisByGrade[grade] = set;
        }
        set->insert(k);
    }

    child = misc.firstChildElement("variant");
    while (!child.isNull())
    {
        if(QString::compare(child.attribute("var_type", ""), "ucs") == 0)
            k->addUnicodeVariant(child.text().toInt(&ok, 16));
        else if(QString::compare(child.attribute("var_type", ""), "jis208") == 0)
            k->addJis208Variant(child.text());
        else if(QString::compare(child.attribute("var_type", ""), "jis212") == 0)
            k->addJis212Variant(child.text());
        else if(QString::compare(child.attribute("var_type", ""), "jis213") == 0)
            k->addJis213Variant(child.text());
        child = child.nextSiblingElement("variant");
    }

    child = misc.firstChildElement("freq");
    if(!child.isNull())
        k->setFrequency(child.text().toInt(&ok, 10));

    child = misc.firstChildElement("rad_name");
    while (!child.isNull())
    {
        k->addNameAsRadical(child.text());
        child = child.nextSiblingElement("rad_name");
    }

    child = misc.firstChildElement("jlpt");
    if(!child.isNull())
    {
        int jlpt = child.text().toInt(&ok, 10);
        k->setJLPT(jlpt);
        QSet<Kanji *> *set = kanjisByJLPT[jlpt];
        if(set == 0)
        {
            set = new QSet<Kanji *>();
            kanjisByJLPT[jlpt] = set;
        }
        set->insert(k);
    }

    int strokeCount = misc.firstChildElement("stroke_count").text().toInt(&ok, 10);

    k->setStrokeCount(strokeCount);
    QSet<Kanji *> *set = kanjisByStroke[strokeCount];
    if(set == 0)
    {
        set = new QSet<Kanji *>();
        kanjisByStroke[strokeCount] = set;
        if(strokeCount < minStrokes)
            minStrokes = strokeCount;
        else if(strokeCount > maxStrokes)
            maxStrokes = strokeCount;
    }
    set->insert(k);

    // end of misc parsing

    // parsing reading_meaning element of character
    QDomElement reamea = element.firstChildElement("reading_meaning");
    if(reamea.isNull())
        return;
    QDomElement rmgroupElement = reamea.firstChildElement("rmgroup");
    while(!rmgroupElement.isNull())
    {
        ReadingMeaningGroup *rmGroup = new ReadingMeaningGroup;
        child = rmgroupElement.firstChildElement("reading");
        while (!child.isNull())
        {
            if(QString::compare(child.attribute("r_type", ""), "ja_on") == 0)
                rmGroup->addOnReading(child.text());
            else if(QString::compare(child.attribute("r_type", ""), "ja_kun") == 0)
                rmGroup->addKunReading(child.text());
            child = child.nextSiblingElement("reading");
        }
        child = rmgroupElement.firstChildElement("meaning");
        while (!child.isNull())
        {
            if(!child.hasAttribute("m_lang") || QString::compare(child.attribute("m_lang", ""), "en") == 0)
                rmGroup->addEnglishMeaning(child.text());
            else if(QString::compare(child.attribute("m_lang", ""), "fr") == 0)
                rmGroup->addFrenchMeaning(child.text());
            child = child.nextSiblingElement("meaning");
        }
        k->addReadingMeaningGroup(rmGroup);
        rmgroupElement = rmgroupElement.nextSiblingElement("rmgroup");
    }
    // nanori readings
    child = reamea.firstChildElement("nanori");
    while (!child.isNull())
    {
        k->addNanoriReading(child.text());
        child = child.nextSiblingElement("nanori");
    }
}

void KanjiDB::search(const QString &s, QSet<Kanji *> &set) const
{
    unsigned int size = s.size();
    if(size < 1)
        set.clear();
    else if(size == 1)
        searchByUnicode(s.at(0).unicode(), set, true);
    else
    {
        // todo: regexp check?
        // here the request is parsed
        // we try to match keyword and process the request one keyword group at a time
        // ie: 'strokes=1,jlpt=4' is cut as 'strokes=1,' and 'jlpt=4'
        // first all kanjis with 1 stroke will be stored in the result set
        // ',' indicates the next set of result must be united to the previous one
        // so all the kanji of the jlpt will be united to the result set

        // unite reads the end of the current keywordgroup but indicates what to do with the next keyword group
        // previousUnite tells whether to unite or intersect current keygroup results with the global result set
        bool unite;
        bool previousUnite = true;
        QString copy = s;
        while(!copy.isEmpty())
        {
            if(copy.startsWith(ucsKey))
            {
                QString ucsValue = parseKey(copy, ucsKey, unite);
                bool ok;
                int ucs = ucsValue.toInt(&ok, 16);
                if(ok)
                    searchByUnicode(ucs, set, previousUnite);
                else if(!previousUnite)
                    set.clear();
            } else if(copy.startsWith(jis208Key))
            {
                searchByStringIndex(parseKey(copy, jis208Key, unite), kanjisJIS208, set, previousUnite);
            } else if(copy.startsWith(jis212Key))
            {
                searchByStringIndex(parseKey(copy, jis212Key, unite), kanjisJIS212, set, previousUnite);
            } else if(copy.startsWith(jis213Key))
            {
                searchByStringIndex(parseKey(copy, jis213Key, unite), kanjisJIS213, set, previousUnite);
            } else if(copy.startsWith(jlptKey))
            {
                bool ok;
                int jlpt = parseKey(copy, jlptKey, unite).toInt(&ok, 10);
                if(ok)
                    searchByIntIndex(jlpt, kanjisByJLPT, set, previousUnite);
                else if(!previousUnite)
                    set.clear();
            } else if(copy.startsWith(gradeKey))
            {
                bool ok;
                int grade = parseKey(copy, gradeKey, unite).toInt(&ok, 10);
                if(ok)
                    searchByIntIndex(grade, kanjisByGrade, set, previousUnite);
                else if(!previousUnite)
                    set.clear();
            } else if(copy.startsWith(strokesKey))
            {
                bool ok;
                int strokes = parseKey(copy, strokesKey, unite).toInt(&ok, 10);
                if(ok)
                    searchByIntIndex(strokes, kanjisByStroke, set, previousUnite);
                else if(!previousUnite)
                    set.clear();
            } else if(copy.startsWith(strokesLessKey))
            {
                bool ok;
                int strokes = parseKey(copy, strokesLessKey, unite).toInt(&ok, 10);
                if(ok)
                {
                    QSet<Kanji *> tmpSet;
                    for(int i = minStrokes; i < strokes; ++i)
                        searchByIntIndex(i, kanjisByStroke, tmpSet, true);
                    if(previousUnite)
                        set.unite(tmpSet);
                    else
                        set.intersect(tmpSet);
                } else if(!previousUnite)
                    set.clear();
            } else if(copy.startsWith(strokesMoreKey))
            {
                bool ok;
                int strokes = parseKey(copy, strokesMoreKey, unite).toInt(&ok, 10);
                if(ok)
                {
                    QSet<Kanji *> tmpSet;
                    for(int i = maxStrokes; i > strokes ; --i)
                        searchByIntIndex(i, kanjisByStroke, tmpSet, true);
                    if(previousUnite)
                        set.unite(tmpSet);
                    else
                        set.intersect(tmpSet);
                } else if(!previousUnite)
                    set.clear();
            } else
            {
                // only kanji supported yet -> multiple characters & no keywords = no result
                set.clear();
                copy = QString();
            }
            previousUnite = unite;
        }
        // keywords (ucs=, jis208=, jis212=, jis213=, jlpt=, strokes[<>=], grade=, ',', ' ')
    }
}

QString KanjiDB::parseKey(QString &parsedString, const QString &key, bool &unite) const
{
    QString result;
    parsedString = parsedString.mid(key.size(), -1);
    int index = parsedString.indexOf(QRegExp("[, ]"));
    // no more ' ' or ',' means it the last keygroup, unite is meaningless at this point
    if(index == -1)
    {
        result = parsedString;
        parsedString = QString();
    } else
    {
        unite = parsedString.at(index) == QChar(',');
        result = parsedString.mid(0, index);
        parsedString = parsedString.mid(index+1, -1);
    }
    return result;
}

void KanjiDB::searchByIntIndex(int index, const QMap<int, QSet<Kanji *> *> &searchedMap, QSet<Kanji *> &setToFill, bool unite) const
{
    if(index > 0 && searchedMap.contains(index))
    {
        if(unite)
            setToFill.unite(*searchedMap[index]);
        else
            setToFill.intersect(*searchedMap[index]);
    } else if(!unite)
        setToFill.clear();
}

void KanjiDB::searchByStringIndex(const QString &indexString, const QMap<QString, Kanji *> &searchedMap, QSet<Kanji *> &setToFill, bool unite) const
{
    if(indexString.size() > 0 && searchedMap.contains(indexString))
    {
        if(unite)
            setToFill.insert(searchedMap[indexString]);
        else
        {
            QSet<Kanji *> tmpSet;
            tmpSet.insert(searchedMap[indexString]);
            setToFill.intersect(tmpSet);
        }
    } else if(!unite)
        setToFill.clear();
}

void KanjiDB::searchByUnicode(int unicode, QSet<Kanji *> &set, bool unite) const
{
    if(unicode > 0 && kanjis.contains(unicode))
    {
        if(unite)
            set.insert(kanjis[unicode]);
        else
        {
            QSet<Kanji *> tmpSet;
            tmpSet.insert(kanjis[unicode]);
            set.intersect(tmpSet);
        }
    } else if(!unite)
        set.clear();
}

void KanjiDB::findVariants(const Kanji *k, QSet<Kanji *> &variants) const
{
    foreach(int i, k->getUnicodeVariants())
        searchByUnicode(i, variants, true);
    foreach(QString s, k->getJis208Variants())
        searchByStringIndex(s, kanjisJIS208, variants, true);
    foreach(QString s, k->getJis212Variants())
        searchByStringIndex(s, kanjisJIS212, variants, true);
    foreach(QString s, k->getJis213Variants())
        searchByStringIndex(s, kanjisJIS213, variants, true);
}

void KanjiDB::search(char strokeCount, char jlpt, char grade, char radical, QSet<Kanji *> &set) const
{
    bool united = false;
    if(strokeCount > 0)
    {
        if(kanjisByStroke[strokeCount] != 0)
        {
            set.unite(*kanjisByStroke[strokeCount]);
            united = true;
        }
    }
    if(jlpt > 0)
    {
        if(kanjisByJLPT[jlpt] != 0)
        {
            if(!united)
            {
                set.unite(*kanjisByJLPT[jlpt]);
                united = true;
            } else
            {
                set.intersect(*kanjisByJLPT[jlpt]);
            }
        }
    }
    if(grade > 0)
    {
        if(kanjisByGrade[grade] != 0)
        {
            if(!united)
            {
                set.unite(*kanjisByGrade[grade]);
                united = true;
            } else
            {
                set.intersect(*kanjisByGrade[grade]);
            }
        }
    }
    if(radical > 0)
    {
        if(kanjisByRadical[radical] != 0)
        {
            if(!united)
            {
                set.unite(*kanjisByRadical[radical]);
                united = true;
            } else
            {
                set.intersect(*kanjisByRadical[radical]);
            }
        }
    }
}
