#include "kanjidb.h"
#include <QDomDocument>
#include "readingmeaninggroup.h"
#include <QRegExp>

const quint32 KanjiDB::magic = 0x5AD5AD15;
const quint32 KanjiDB::version = 100;

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
    maxStrokes = 0;
    minStrokes = 255;
}

KanjiDB::~KanjiDB()
{
    clear();
}

void KanjiDB::clear()
{
    //all kanjis are referenced by the unicode map
    //delete them once from here, clean the rest
    foreach(Kanji *k, kanjis.values())
        delete k;
    kanjis.clear();
    kanjisJIS208.clear();
    kanjisJIS212.clear();
    kanjisJIS213.clear();
    foreach(QSet<Kanji *> *set, kanjisByStroke)
    {
        set->clear();
        delete set;
    }
    kanjisByStroke.clear();
    foreach(QSet<Kanji *> *set, kanjisByJLPT)
    {
        set->clear();
        delete set;
    }
    kanjisByJLPT.clear();
    foreach(QSet<Kanji *> *set, kanjisByRadical)
    {
        set->clear();
        delete set;
    }
    kanjisByRadical.clear();
    foreach(QSet<Kanji *> *set, kanjisByGrade)
    {
        set->clear();
        delete set;
    }
    kanjisByGrade.clear();
}

QDataStream &operator >>(QDataStream &stream, KanjiDB &db)
{
    db.clear();
    //all kanjis maps contain only kanji pointers,
    //and each kanji exists only once, but has a pointer
    //to it stored in different maps.
    //kanjis are stored along with the value of their pointer
    //we use this value to rebuild the maps efficiently:
    //create a kanji only once when building the unicode map,
    //then match old references to new references
    //to build the other maps
    QMap<quintptr, Kanji *> memMap;
    int size;
    stream >> size;
    for(int i = 0; i < size; ++i)
    {
        int ucs;
        Kanji *k = new Kanji;
        quintptr p;
        stream >> ucs >> *k >> p;
        //used to match old reference to new references
        memMap.insert(p, k);
        //build unicode map (contains reference to all kanjis)
        db.kanjis.insert(ucs, k);
    }
    stream >> size;
    for(int i = 0; i < size; ++i)
    {
        QString jis208;
        quintptr p;
        stream >> jis208 >> p;
        //build other map getting new reference from old reference
        db.kanjisJIS208.insert(jis208, memMap[p]);
    }
    stream >> size;
    for(int i = 0; i < size; ++i)
    {
        QString jis212;
        quintptr p;
        stream >> jis212 >> p;
        db.kanjisJIS212.insert(jis212, memMap[p]);
    }
    stream >> size;
    for(int i = 0; i < size; ++i)
    {
        QString jis213;
        quintptr p;
        stream >> jis213 >> p;
        db.kanjisJIS213.insert(jis213, memMap[p]);
    }
    stream >> size;
    for(int i = 0; i < size; ++i)
    {
        int key, subsize;
        stream >> key >> subsize;
        QSet<Kanji *> *set = new QSet<Kanji *>;
        for(int j = 0; j < subsize; ++j)
        {
            quintptr p;
            stream >> p;
            set->insert(memMap[p]);
        }
        db.kanjisByStroke.insert(key, set);
    }
    stream >> size;
    for(int i = 0; i < size; ++i)
    {
        int key, subsize;
        stream >> key >> subsize;
        QSet<Kanji *> *set = new QSet<Kanji *>;
        for(int j = 0; j < subsize; ++j)
        {
            quintptr p;
            stream >> p;
            set->insert(memMap[p]);
        }
        db.kanjisByRadical.insert(key, set);
    }
    stream >> size;
    for(int i = 0; i < size; ++i)
    {
        int key, subsize;
        stream >> key >> subsize;
        QSet<Kanji *> *set = new QSet<Kanji *>;
        for(int j = 0; j < subsize; ++j)
        {
            quintptr p;
            stream >> p;
            set->insert(memMap[p]);
        }
        db.kanjisByGrade.insert(key, set);
    }
    stream >> size;
    for(int i = 0; i < size; ++i)
    {
        int key, subsize;
        stream >> key >> subsize;
        QSet<Kanji *> *set = new QSet<Kanji *>;
        for(int j = 0; j < subsize; ++j)
        {
            quintptr p;
            stream >> p;
            set->insert(memMap[p]);
        }
        db.kanjisByJLPT.insert(key, set);
    }
    stream >> (quint32&) db.minStrokes;
    stream >> (quint32&) db.maxStrokes;
    return stream;
}

QDataStream &operator <<(QDataStream &stream, const KanjiDB &db)
{
    //maps contain only pointers so a specific streaming is required
    //the unicode map contains reference to all the kanjis
    //we store the kanji by reading the unicode map,
    //and store their pointer value along.
    //other maps stream only the reference.
    stream << db.kanjis.size();
    QMapIterator<int, Kanji *> i(db.kanjis);
    while (i.hasNext()) {
        i.next();
        stream << i.key() << *(i.value()) << (quintptr&) i.value();
    }
    stream << db.kanjisJIS208.size();
    QMapIterator<QString, Kanji *> j(db.kanjisJIS208);
    while (j.hasNext()) {
        j.next();
        stream << j.key() << (quintptr&) j.value();
    }
    stream << db.kanjisJIS212.size();
    j = QMapIterator<QString, Kanji *>(db.kanjisJIS212);
    while (j.hasNext()) {
        j.next();
        stream << j.key() << (quintptr&) j.value();
    }
    stream << db.kanjisJIS213.size();
    j = QMapIterator<QString, Kanji *>(db.kanjisJIS213);
    while (j.hasNext()) {
        j.next();
        stream << j.key() << (quintptr&) j.value();
    }
    stream << db.kanjisByStroke.size();
    QMapIterator<int, QSet<Kanji *> *> k(db.kanjisByStroke);
    while (k.hasNext()) {
        k.next();
        stream << k.key() << k.value()->size();
        foreach(Kanji *kj, *(k.value()))
            stream << (quintptr&) kj;
    }
    stream << db.kanjisByRadical.size();
    k = QMapIterator<int, QSet<Kanji *> *>(db.kanjisByRadical);
    while (k.hasNext()) {
        k.next();
        stream << k.key() << k.value()->size();
        foreach(Kanji *kj, *(k.value()))
            stream << (quintptr&) kj;
    }
    stream << db.kanjisByGrade.size();
    k = QMapIterator<int, QSet<Kanji *> *>(db.kanjisByGrade);
    while (k.hasNext()) {
        k.next();
        stream << k.key() << k.value()->size();
        foreach(Kanji *kj, *(k.value()))
            stream << (quintptr&) kj;
    }
    stream << db.kanjisByJLPT.size();
    k = QMapIterator<int, QSet<Kanji *> *>(db.kanjisByJLPT);
    while (k.hasNext()) {
        k.next();
        stream << k.key() << k.value()->size();
        foreach(Kanji *kj, *(k.value()))
            stream << (quintptr&) kj;
    }
    stream << db.minStrokes;
    stream << db.maxStrokes;
    return stream;
}

bool KanjiDB::readIndex(QIODevice *device)
{
    QDataStream in(device);

    // Read and check the header
    quint32 _magic;
    in >> _magic;
    if (_magic != magic)
    {
        error = QString("Bad file format, not a recognized index file");
        return false;
    }

    // Read the version
    qint32 _version;
    in >> _version;
//    if (_version < 100)
//        return XXX_BAD_FILE_TOO_OLD;
//    if (version > 123)
//        return XXX_BAD_FILE_TOO_NEW;

//    if (version <= 110)
//        in.setVersion(QDataStream::Qt_3_2);
//    else
    in.setVersion(QDataStream::Qt_4_0);

    error = QString();
    // Read the data
    in >> *this;

    return true;
}

bool KanjiDB::readKanjiDic(QIODevice *device)
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    QDomDocument domDocument;
    if (!domDocument.setContent(device, true, &errorStr, &errorLine,
                                &errorColumn))
    {
        error = QString("At line %1, column %2: ").arg(errorLine).arg(errorColumn) + errorStr;
        return false;
    }

    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "kanjidic2")
    {
        error = QString("Not a kanjidic2 file");
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

    error = QString();
    return true;
}

bool KanjiDB::writeIndex(QIODevice *device) const
{
    QDataStream out(device);

    // Write a header with a "magic number" and a version
    out << (quint32)magic;
    out << (qint32)version;

    out.setVersion(QDataStream::Qt_4_0);

    // Write the data
    out << *this;

    error = QString();
    return true;
}

const QString KanjiDB::errorString() const
{
    return error;
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

void KanjiDB::search(const QString &s, KanjiSet &set) const
{
    unsigned int size = s.size();
    if(size < 1)
        set.clear();
    else if(size == 1)
        searchByUnicode(s.at(0).unicode(), set, true, s.at(0).unicode());
    else
    {
        QString regexp(ucsKey+"|"+gradeKey+"|"+jlptKey+"|"+strokesKey+"|"
                       +strokesMoreKey+"|"+strokesLessKey+"|"+jis208Key+"|"+
                       jis212Key+"|"+jis213Key);
        QRegExp fullRegexp("(("+regexp+")[^ ,]+)([, ]("+regexp+")[^ ,]+)*");
        // attempt to read each character and look it up
        if(!fullRegexp.exactMatch(s))
            for(int i = 0; i < s.length(); ++i)
                searchByUnicode(s[i].unicode(), set, true, i);
        else
        {
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
                        searchByUnicode(ucs, set, previousUnite, ucs);
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
                        KanjiSet tmpSet;
                        for(int i = minStrokes; i < strokes; ++i)
                            searchByIntIndex(i, kanjisByStroke, tmpSet, true);
                        foreach(Kanji *k, tmpSet)
                            if(previousUnite)
                                set.insert(k->getUnicode(), k);
                            else
                                if(!set.contains(k->getUnicode()))
                                    set.remove(k->getUnicode());
                    } else if(!previousUnite)
                        set.clear();
                } else if(copy.startsWith(strokesMoreKey))
                {
                    bool ok;
                    int strokes = parseKey(copy, strokesMoreKey, unite).toInt(&ok, 10);
                    if(ok)
                    {
                        KanjiSet tmpSet;
                        for(int i = maxStrokes; i > strokes ; --i)
                            searchByIntIndex(i, kanjisByStroke, tmpSet, true);
                        foreach(Kanji *k, tmpSet)
                            if(previousUnite)
                                set.insert(k->getUnicode(), k);
                            else
                                if(!set.contains(k->getUnicode()))
                                    set.remove(k->getUnicode());
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
        }
        // keywords (ucs=, jis208=, jis212=, jis213=, jlpt=, strokes[<>=], grade=, ',', ' ')
    }
    return;
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

void KanjiDB::searchByIntIndex(int index, const QMap<int, QSet<Kanji *> *> &searchedMap, KanjiSet &setToFill, bool unite) const
{
    if(index > 0 && searchedMap.contains(index))
        foreach(Kanji *k, *searchedMap[index])
            if(unite)
                setToFill.insert(k->getUnicode(), k);
            else
                if(!setToFill.contains(k->getUnicode()))
                    setToFill.remove(k->getUnicode());
    else if(!unite)
        setToFill.clear();
}

void KanjiDB::searchByStringIndex(const QString &indexString, const QMap<QString, Kanji *> &searchedMap, KanjiSet &setToFill, bool unite) const
{
    if(indexString.size() > 0 && searchedMap.contains(indexString))
    {
        Kanji *k = searchedMap[indexString];
        if(unite)
            setToFill.insert(k->getUnicode(), k);
        else
            if(!setToFill.contains(k->getUnicode()))
                setToFill.remove(k->getUnicode());
    } else if(!unite)
        setToFill.clear();
}

void KanjiDB::searchByUnicode(int unicode, KanjiSet &set, bool unite, int position) const
{
    if(unicode > 0 && kanjis.contains(unicode))
    {
        Kanji *k = kanjis[unicode];
        if(unite)
            set.insert(position, k);
        else
            if(!set.contains(k->getUnicode()))
                set.remove(k->getUnicode());
    } else if(!unite)
        set.clear();
}

void KanjiDB::findVariants(const Kanji *k, KanjiSet &variants) const
{
    foreach(int i, k->getUnicodeVariants())
        searchByUnicode(i, variants, true, i);
    foreach(QString s, k->getJis208Variants())
        searchByStringIndex(s, kanjisJIS208, variants, true);
    foreach(QString s, k->getJis212Variants())
        searchByStringIndex(s, kanjisJIS212, variants, true);
    foreach(QString s, k->getJis213Variants())
        searchByStringIndex(s, kanjisJIS213, variants, true);
}
