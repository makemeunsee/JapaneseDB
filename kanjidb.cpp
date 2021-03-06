#include "kanjidb.h"
#include <QDomDocument>
#include <QTextStream>
#include "readingmeaninggroup.h"
#include <QRegExp>
#include <QTextCodec>
#include "radicals.h"

#include <iostream>

const QString KanjiDB::kanjiDBIndexFilename("kanjidb.index");
const QString KanjiDB::defaultKanjiDic2Filename("kanjidic2.xml");
const QString KanjiDB::defaultKRadFilename("kradfile");
const QString KanjiDB::defaultKRad2Filename("kradfile2");
const QString KanjiDB::defaultRadKXFilename("radkfilexUTF8");

const quint32 KanjiDB::magic = 0x5AD5AD15;
const quint32 KanjiDB::version = 153;

const QString KanjiDB::interSeps("&\\+");
const QString KanjiDB::unionSeps(" ,;");
const QString KanjiDB::seps("["+interSeps+unionSeps+"]");
const QString KanjiDB::notSeps("[^"+interSeps+unionSeps+"]");
const QString KanjiDB::ucsKey("ucs=");
const QString KanjiDB::gradeKey("grade=");
const QString KanjiDB::jlptKey("jlpt=");
const QString KanjiDB::strokesKey("strokes=");
const QString KanjiDB::strokesMoreKey("strokes>");
const QString KanjiDB::strokesLessKey("strokes<");
const QString KanjiDB::jis208Key("jis208=");
const QString KanjiDB::jis212Key("jis212=");
const QString KanjiDB::jis213Key("jis213=");
const QString KanjiDB::radicalKey("radical=");
const QString KanjiDB::componentKey("component=");
const QString KanjiDB::allKeys[keyCount] = {gradeKey, jlptKey, jis208Key, jis212Key, jis213Key, componentKey, radicalKey, strokesKey, strokesLessKey, strokesMoreKey, ucsKey};
const QString KanjiDB::regexp(fromArray(allKeys, keyCount).join("|"));
const QRegExp KanjiDB::searchRegexp("(("+regexp+")"+notSeps+"+)("+seps+"("+regexp+")"+notSeps+"+)*");

KanjiDB::KanjiDB()
{
    //empty list returned when no match found
    maxStrokes = 0;
    minStrokes = 255;
    initRadicals();
}

KanjiDB::~KanjiDB()
{
    clear();
    foreach(Kanji *k, radicals.values())
        delete k;
    radicals.clear();
    radicalsByIndex.clear();
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
    foreach(QSet<Kanji *> *set, kanjisByGrade)
    {
        set->clear();
        delete set;
    }
    kanjisByGrade.clear();
    foreach(QSet<Kanji *> *set, kanjisByRadical)
    {
        set->clear();
        delete set;
    }
    kanjisByRadical.clear();
    foreach(Kanji *k, components.values())
        delete k;
    components.clear();
    componentIndexes.clear();
    foreach(QSet<Kanji *> *set, kanjisByComponent)
    {
        set->clear();
        delete set;
    }
    kanjisByComponent.clear();
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
    unsigned int size;
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        Unicode ucs;
        Kanji *k = new Kanji;
        quintptr p;
        stream >> ucs >> *k >> p;
        //used to match old reference to new references
        memMap.insert(p, k);
        //build unicode map (contains reference to all kanjis)
        db.kanjis.insert(ucs, k);
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        QString jis208;
        quintptr p;
        stream >> jis208 >> p;
        //build other map getting new reference from old reference
        db.kanjisJIS208.insert(jis208, memMap.value(p));
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        QString jis212;
        quintptr p;
        stream >> jis212 >> p;
        db.kanjisJIS212.insert(jis212, memMap.value(p));
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        QString jis213;
        quintptr p;
        stream >> jis213 >> p;
        db.kanjisJIS213.insert(jis213, memMap.value(p));
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        unsigned int key, subsize;
        stream >> key >> subsize;
        QSet<Kanji *> *set = new QSet<Kanji *>;
        for(unsigned int j = 0; j < subsize; ++j)
        {
            quintptr p;
            stream >> p;
            set->insert(memMap.value(p));
        }
        db.kanjisByStroke.insert(key, set);
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        unsigned int key, subsize;
        stream >> key >> subsize;
        QSet<Kanji *> *set = new QSet<Kanji *>;
        for(unsigned int j = 0; j < subsize; ++j)
        {
            quintptr p;
            stream >> p;
            set->insert(memMap.value(p));
        }
        db.kanjisByRadical.insert(key, set);
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        unsigned int key, subsize;
        stream >> key >> subsize;
        QSet<Kanji *> *set = new QSet<Kanji *>;
        for(unsigned int j = 0; j < subsize; ++j)
        {
            quintptr p;
            stream >> p;
            set->insert(memMap.value(p));
        }
        db.kanjisByGrade.insert(key, set);
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        unsigned int key, subsize;
        stream >> key >> subsize;
        QSet<Kanji *> *set = new QSet<Kanji *>;
        for(unsigned int j = 0; j < subsize; ++j)
        {
            quintptr p;
            stream >> p;
            set->insert(memMap.value(p));
        }
        db.kanjisByJLPT.insert(key, set);
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        unsigned int key, subsize;
        stream >> key >> subsize;
        QSet<Kanji *> *set = new QSet<Kanji *>;
        for(unsigned int j = 0; j < subsize; ++j)
        {
            quintptr p;
            stream >> p;
            set->insert(memMap.value(p));
        }
        db.kanjisByComponent.insert(key, set);
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        Unicode ucs;
        Kanji *k = new Kanji;
        stream >> ucs >> *k;
        db.components.insert(ucs, k);
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        unsigned char componentIndex;
        Unicode ucs;
        stream >> (quint8&) componentIndex >> ucs;
        db.componentIndexes.insert(componentIndex, ucs);
    }
    stream >> size;
    for(unsigned int i = 0; i < size; ++i)
    {
        Unicode ucs;
        QString faultyName;
        stream >> ucs >> faultyName;
        db.faultyComponents.insert(ucs, faultyName);
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
    KanjiSetConstIterator i(db.kanjis);
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
    QMapIterator<unsigned int, QSet<Kanji *> *> k(db.kanjisByStroke);
    while (k.hasNext()) {
        k.next();
        stream << k.key() << k.value()->size();
        foreach(Kanji *kj, *(k.value()))
            stream << (quintptr&) kj;
    }
    stream << db.kanjisByRadical.size();
    k = QMapIterator<unsigned int, QSet<Kanji *> *>(db.kanjisByRadical);
    while (k.hasNext()) {
        k.next();
        stream << k.key() << k.value()->size();
        foreach(Kanji *kj, *(k.value()))
            stream << (quintptr&) kj;
    }
    stream << db.kanjisByGrade.size();
    k = QMapIterator<unsigned int, QSet<Kanji *> *>(db.kanjisByGrade);
    while (k.hasNext()) {
        k.next();
        stream << k.key() << k.value()->size();
        foreach(Kanji *kj, *(k.value()))
            stream << (quintptr&) kj;
    }
    stream << db.kanjisByJLPT.size();
    k = QMapIterator<unsigned int, QSet<Kanji *> *>(db.kanjisByJLPT);
    while (k.hasNext()) {
        k.next();
        stream << k.key() << k.value()->size();
        foreach(Kanji *kj, *(k.value()))
            stream << (quintptr&) kj;
    }
    stream << db.kanjisByComponent.size();
    k = QMapIterator<Unicode, QSet<Kanji *> *>(db.kanjisByComponent);
    while (k.hasNext()) {
        k.next();
        stream << k.key() << k.value()->size();
        foreach(Kanji *kj, *(k.value()))
            stream << (quintptr&) kj;
    }
    stream << db.components.size();
    i = KanjiSetConstIterator(db.components);
    while (i.hasNext()) {
        i.next();
        stream << i.key() << *(i.value());
    }
    stream << db.componentIndexes.size();
    QMapIterator<unsigned char, Unicode> l(db.componentIndexes);
    while (l.hasNext()) {
        l.next();
        stream << l.key() << l.value();
    }
    stream << db.faultyComponents.size();
    QMapIterator<Unicode, QString> m(db.faultyComponents);
    while (m.hasNext()) {
        m.next();
        stream << m.key() << m.value();
    }
    stream << db.minStrokes;
    stream << db.maxStrokes;
    return stream;
}

void KanjiDB::initRadicals()
{
    bool b;
    for(unsigned int i = 0; i < Radicals::radicalsSize; ++i)
    {
        QStringList parts = Radicals::radicals[i].split(":");
        unsigned char strokes = parts[1].toUShort(&b);

        Kanji *masterRadical = new Kanji();
        masterRadical->setClassicalRadical(i+1);
        masterRadical->setLiteral(parts[0].at(0));
        masterRadical->setUnicode(parts[0].at(0).unicode());
        masterRadical->setStrokeCount(strokes);

        radicalsByIndex.insert(i+1, masterRadical->getUnicode());
        radicals.insert(masterRadical->getUnicode(), masterRadical);

        foreach(QChar c, parts[0].mid(1))
        {
            Kanji *variantRadical = new Kanji();
            variantRadical->setClassicalRadical(i+1);
            variantRadical->setLiteral(c);
            variantRadical->setUnicode(c.unicode());
            variantRadical->setStrokeCount(strokes);
            radicals.insert(c.unicode(), variantRadical);
            masterRadical->addUnicodeVariant(c.unicode());
        }
    }
}

int KanjiDB::readResources(const QDir &basedir)
{
    error = QString();
    bool b_allDataRead, b_baseDataRead, b_indexSaved;
    b_allDataRead = b_baseDataRead = b_indexSaved = false;

    QFile index(basedir.absolutePath().append("/").append(kanjiDBIndexFilename));
    if (index.open(QIODevice::ReadOnly)) {
        if(readIndex(&index))
        {
            b_allDataRead = b_baseDataRead = b_indexSaved = true;
        } else
        {
            //TODO log: index unreadable
        }
        index.close();
    } else
    {
        //TODO log: no index found
    }

    if(!b_allDataRead)
    {
        QFile kanjiDicFile(basedir.absolutePath().append("/").append(defaultKanjiDic2Filename));
        if (!kanjiDicFile.open(QIODevice::Text | QIODevice::ReadOnly)) {
            error = QString("Cannot open kanjidic file %1.")
                              .arg(defaultKanjiDic2Filename);
            return noDataRead;
        }

        if (readKanjiDic(&kanjiDicFile))
        {
            b_allDataRead = b_baseDataRead = true;
        }
        else
            error = QString("Cannot read kanjidic file %1:\n%2.")
                              .arg(defaultKanjiDic2Filename)
                              .arg(error);
        kanjiDicFile.close();

        if(b_allDataRead)
        {
            QFile radKXFile(basedir.absolutePath().append("/").append(defaultRadKXFilename));
            if (!radKXFile.open(QIODevice::Text | QIODevice::ReadOnly)) {
                error = QString("Cannot open RADKFILEX file %1.")
                                  .arg(defaultRadKXFilename);
                b_allDataRead = false;
            } else {
                if (!readRadK(&radKXFile))
                {
                    error = QString("Cannot read RADKFILEX file %1:\n%2.")
                                      .arg(defaultRadKXFilename)
                                      .arg(error);
                    b_allDataRead = false;
                }
                radKXFile.close();
            }

//            QFile kRadFile(basedir.absolutePath().append("/").append(defaultKRadFilename));
//            if (!kRadFile.open(QIODevice::Text | QIODevice::ReadOnly)) {
//                error = QString("Cannot open KRADFILE file %1.")
//                                  .arg(defaultKRadFilename);
//                b_allDataRead = false;
//            } else {
//                if (!readKRad(&kRadFile))
//                {
//                    error = QString("Cannot read KRADFILE file %1:\n%2.")
//                                      .arg(defaultKRadFilename)
//                                      .arg(error);
//                    b_allDataRead = false;
//                }
//                kRadFile.close();
//            }

//            QFile kRad2File(basedir.absolutePath().append("/").append(defaultKRad2Filename));
//            if (!kRad2File.open(QIODevice::Text | QIODevice::ReadOnly)) {
//                error = QString("Cannot open KRAD2FILE file %1.")
//                                  .arg(defaultKRad2Filename);
//                b_allDataRead = false;
//            } else {
//                if (!readKRad(&kRad2File))
//                {
//                    error = QString("Cannot read KRAD2FILE file %1:\n%2.")
//                                      .arg(defaultKRad2Filename)
//                                      .arg(error);
//                    b_allDataRead = false;
//                }
//                kRad2File.close();
//            }
        }

        //only save index when all resources have been freshly read
        if(b_allDataRead)
        {
            if(!index.open(QIODevice::WriteOnly))
                error = QString("Cannot open index file %1 for writing.")
                                  .arg(kanjiDBIndexFilename);
            else
            {
                if(writeIndex(&index))
                    b_indexSaved = true;
                else
                    error = QString("Cannot write index file %1.")
                                      .arg(kanjiDBIndexFilename);
                index.close();
            }
        }
    }

    if(b_indexSaved)
        return allDataReadAndSaved;
    else if(b_allDataRead)
        return allDataReadButNotSaved;
    else if(b_baseDataRead)
        return baseDataRead;
    else
        return noDataRead;
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
    quint32 _version;
    in >> _version;
    if (_version < version)
    {
        error = QString("Unsupported index file, too old");
        return false;
    }
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

bool KanjiDB::readKRad(QIODevice *)
{
    //TODO
    if(false)
    {
        error = QString("Error in KRAD reading");
        return false;
    }
    return true;
}

bool KanjiDB::readRadK(QIODevice *device)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextStream ts(device);
    ts.setAutoDetectUnicode(false);
    ts.setCodec(codec);
    QString line;
    unsigned char index = 0;
    bool ok;
    Unicode currentRadical = 0;
    while(!(line = ts.readLine()).isNull())
    {
        if(!line.startsWith("#") && !line.size() == 0)
        {
            if(line.startsWith("$"))
            {
                const QChar &c_component = line.at(2);
                unsigned char strokes = line.split(" ").at(2).toUShort(&ok);
                unsigned short unicode = c_component.unicode();
                Kanji *k_component =  new Kanji;
                k_component->setUnicode(unicode);
                k_component->setLiteral(QString(c_component));
                k_component->setStrokeCount(strokes);
                if(line.count(" ") == 3)
                    faultyComponents.insert(unicode, line.mid(line.lastIndexOf(" ")+1));
                components.insert(unicode, k_component);
                componentIndexes.insert(index++, unicode);
                currentRadical = unicode;
                kanjisByComponent.insert(currentRadical, new QSet<Kanji *>);
            } else
            {
                if(currentRadical == 0)
                {
                    error = QString("Unrecognized radk file");
                    return false;
                }
                foreach(QChar c, line)
                {
                    Unicode u = c.unicode();
                    if(kanjis.contains(u))
                    {
                        Kanji *container = kanjis.value(u);
                        kanjisByComponent.value(currentRadical)->insert(container);
                        container->addComponent(currentRadical);
                    }
                }
            }
        }
    }
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
    Kanji *k = new Kanji();
    k->setLiteral(title);

    bool ok;

    QDomElement codepoint= element.firstChildElement("codepoint");
    QDomElement child = codepoint.firstChildElement("cp_value");
    while (!child.isNull())
    {
        if(QString::compare(child.attribute("cp_type", ""), "ucs") == 0)
        {
            int unicode = child.text().toUInt(&ok, 16);
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
            unsigned char cRad = child.text().toUInt(&ok, 10);
            k->setClassicalRadical(cRad);
            QSet<Kanji *> *set = kanjisByRadical.value(cRad);
            if(set == 0)
            {
                set = new QSet<Kanji *>();
                kanjisByRadical[cRad] = set;
            }
            set->insert(k);
        }
        else if(QString::compare(child.attribute("rad_type", ""), "nelson_c") == 0)
            k->setNelsonRadical(child.text().toUInt(&ok, 10));
        child = child.nextSiblingElement("rad_value");
    }

    //parsing misc element of character
    QDomElement misc = element.firstChildElement("misc");

    child = misc.firstChildElement("grade");
    if(!child.isNull())
    {
        unsigned int grade = child.text().toUInt(&ok, 10);
        k->setGrade(grade);
        QSet<Kanji *> *set = kanjisByGrade.value(grade);
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
            k->addUnicodeVariant(child.text().toUInt(&ok, 16));
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
        k->setFrequency(child.text().toUInt(&ok, 10));

    child = misc.firstChildElement("rad_name");
    while (!child.isNull())
    {
        k->addNameAsRadical(child.text());
        child = child.nextSiblingElement("rad_name");
    }

    child = misc.firstChildElement("jlpt");
    if(!child.isNull())
    {
        unsigned int jlpt = child.text().toUInt(&ok, 10);
        k->setJLPT(jlpt);
        QSet<Kanji *> *set = kanjisByJLPT.value(jlpt);
        if(set == 0)
        {
            set = new QSet<Kanji *>();
            kanjisByJLPT[jlpt] = set;
        }
        set->insert(k);
    }

    unsigned int strokeCount = misc.firstChildElement("stroke_count").text().toUInt(&ok, 10);

    k->setStrokeCount(strokeCount);
    QSet<Kanji *> *set = kanjisByStroke.value(strokeCount);
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
    else
    {
        // attempt to read each character and look it up
        if(!searchRegexp.exactMatch(s))
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
                    Unicode ucs = ucsValue.toUInt(&ok, 16);
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
                    unsigned int jlpt = parseKey(copy, jlptKey, unite).toUInt(&ok, 10);
                    if(ok)
                        searchByIntIndex(jlpt, kanjisByJLPT, set, previousUnite);
                    else if(!previousUnite)
                        set.clear();
                } else if(copy.startsWith(gradeKey))
                {
                    bool ok;
                    unsigned int grade = parseKey(copy, gradeKey, unite).toUInt(&ok, 10);
                    if(ok)
                        searchByIntIndex(grade, kanjisByGrade, set, previousUnite);
                    else if(!previousUnite)
                        set.clear();
                } else if(copy.startsWith(radicalKey))
                {
                    bool ok;
                    QString key = parseKey(copy, radicalKey, unite);
                    unsigned int radical = key.toUInt(&ok, 10);
                    if(ok)
                        searchByIntIndex(radical, kanjisByRadical, set, previousUnite);
                    else if(key.size() == 1)
                        if(radicals.contains(key[0].unicode()))
                            searchByIntIndex(radicals.value(key[0].unicode())->getClassicalRadical(), kanjisByRadical, set, previousUnite);
                        else
                            set.clear();
                    else if(!previousUnite)
                        set.clear();
                } else if(copy.startsWith(componentKey))
                {
                    QString key = parseKey(copy, componentKey, unite);
                    if(key.size() == 1)
                        searchByIntIndex(key.at(0).unicode(), kanjisByComponent, set, previousUnite);
                    else if(!previousUnite)
                        set.clear();
                } else if(copy.startsWith(strokesKey))
                {
                    bool ok;
                    unsigned int strokes = parseKey(copy, strokesKey, unite).toUInt(&ok, 10);
                    if(ok)
                        searchByIntIndex(strokes, kanjisByStroke, set, previousUnite);
                    else if(!previousUnite)
                        set.clear();
                } else if(copy.startsWith(strokesLessKey))
                {
                    bool ok;
                    unsigned int strokes = parseKey(copy, strokesLessKey, unite).toUInt(&ok, 10);
                    if(ok)
                    {
                        KanjiSet tmpSet;
                        for(unsigned int i = minStrokes; i < strokes; ++i)
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
                    unsigned int strokes = parseKey(copy, strokesMoreKey, unite).toUInt(&ok, 10);
                    if(ok)
                    {
                        KanjiSet tmpSet;
                        for(unsigned int i = maxStrokes; i > strokes ; --i)
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
    int index = parsedString.indexOf(QRegExp(seps));
    // no more ' ' or ',' means it the last keygroup, unite is meaningless at this point
    if(index == -1)
    {
        result = parsedString;
        parsedString = QString();
    } else
    {
        unite = unionSeps.contains(parsedString.at(index));
        result = parsedString.mid(0, index);
        parsedString = parsedString.mid(index+1, -1);
    }
    return result;
}

void KanjiDB::searchByIntIndex(unsigned int index, const QMap<unsigned int, QSet<Kanji *> *> &searchedMap, KanjiSet &setToFill, bool unite) const
{
    if(index > 0 && searchedMap.contains(index))
    {
        if(unite)
            foreach(Kanji *k, *searchedMap.value(index))
                setToFill.insert(k->getUnicode(), k);
        else
        {
            KanjiSetIterator iter(setToFill);
            while(iter.hasNext())
            {
                iter.next();
                if(!searchedMap.value(index)->contains(iter.value()))
                    iter.remove();
            }
        }
    } else if(!unite)
        setToFill.clear();
}

void KanjiDB::searchByStringIndex(const QString &indexString, const QMap<QString, Kanji *> &searchedMap, KanjiSet &setToFill, bool unite) const
{
    if(indexString.size() > 0 && searchedMap.contains(indexString))
    {
        Kanji *k = searchedMap.value(indexString);
        if(unite)
            setToFill.insert(k->getUnicode(), k);
        else
        {
            bool contained = setToFill.contains(k->getUnicode());
            setToFill.clear();
            if(contained)
                setToFill.insert(k->getUnicode(), k);
        }
    } else if(!unite)
        setToFill.clear();
}

const Kanji *KanjiDB::getByUnicode(Unicode unicode) const
{
    return kanjis.value(unicode);
}

void KanjiDB::searchByUnicode(Unicode unicode, KanjiSet &set, bool unite, int position) const
{
    if(unicode > 0 && kanjis.contains(unicode))
    {
        Kanji *k = kanjis.value(unicode);
        if(unite)
            set.insert(position, k);
        else
        {
            // position is always equal to unicode when unite is false
            bool contained = set.contains(unicode);
            set.clear();
            if(contained)
                set.insert(unicode, k);
        }
    } else if(!unite)
        set.clear();
}

void KanjiDB::findVariants(const Kanji *k, KanjiSet &variants) const
{
    foreach(Unicode i, k->getUnicodeVariants())
        searchByUnicode(i, variants, true, i);
    foreach(QString s, k->getJis208Variants())
        searchByStringIndex(s, kanjisJIS208, variants, true);
    foreach(QString s, k->getJis212Variants())
        searchByStringIndex(s, kanjisJIS212, variants, true);
    foreach(QString s, k->getJis213Variants())
        searchByStringIndex(s, kanjisJIS213, variants, true);
}

const Kanji *KanjiDB::getRadicalVariant(Unicode u) const
{
    return radicals.value(u);
}

const Kanji *KanjiDB::getRadicalById(unsigned char c) const
{
    return radicals.value(radicalsByIndex.value(c));
}

const Kanji *KanjiDB::getComponent(Unicode u) const
{
    return components.value(u);
}

const Kanji *KanjiDB::getComponentById(unsigned char c) const
{
    return components.value(componentIndexes.value(c));
}

const KanjiSet &KanjiDB::getAllKanjis() const
{
    return kanjis;
}

const KanjiSet &KanjiDB::getAllRadicals() const
{
    return radicals;
}

const KanjiSet &KanjiDB::getAllComponents() const
{
    return components;
}

const QMap<Unicode, QString> &KanjiDB::getFaultyComponents() const
{
    return faultyComponents;
}
