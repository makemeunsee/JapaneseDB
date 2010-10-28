#include "kanji.h"

Kanji::Kanji() : unicode(0), classicalRadical(0), nelsonRadical(0), grade(0), strokeCount(0), frequency(0), jlpt(0)
{
}

Kanji::~Kanji()
{
    foreach(ReadingMeaningGroup *rmg, rmGroups)
        delete rmg;
    rmGroups.clear();
}

QDataStream &operator >>(QDataStream &stream, Kanji &k)
{
    foreach(ReadingMeaningGroup *rmg, k.rmGroups)
        delete rmg;
    k.rmGroups.clear();
    k.unicodeVariants.clear();;
    k.jis208Variants.clear();
    k.jis212Variants.clear();
    k.jis213Variants.clear();
    k.radicalNames.clear();
    k.nanoriReadings.clear();

    stream >> k.literal;
    stream >> k.unicode;
    stream >> k.jis208;
    stream >> k.jis212;
    stream >> k.jis213;
    stream >> (quint8&) k.classicalRadical;
    stream >> (quint8&) k.nelsonRadical;
    stream >> (quint8&) k.grade;
    stream >> (quint8&) k.strokeCount;
    stream >> k.unicodeVariants;
    stream >> k.jis208Variants;
    stream >> k.jis212Variants;
    stream >> k.jis213Variants;
    stream >> (quint16&) k.frequency;
    stream >> k.radicalNames;
    stream >> (quint8&) k.jlpt;
    quint32 rmgSize;
    stream >> rmgSize;
    for(quint32 i = 0; i < rmgSize; ++i)
    {
        ReadingMeaningGroup *rmg = new ReadingMeaningGroup;
        stream >> *rmg;
        k.rmGroups.append(rmg);
    }
    stream >> k.nanoriReadings;
    return stream;
}

QDataStream &operator <<(QDataStream &stream, const Kanji &k)
{
    stream << k.literal;
    stream << k.unicode;
    stream << k.jis208;
    stream << k.jis212;
    stream << k.jis213;
    stream << (quint8&) k.classicalRadical;
    stream << (quint8&) k.nelsonRadical;
    stream << (quint8&) k.grade;
    stream << (quint8&) k.strokeCount;
    stream << k.unicodeVariants;
    stream << k.jis208Variants;
    stream << k.jis212Variants;
    stream << k.jis213Variants;
    stream << (quint16&) k.frequency;
    stream << k.radicalNames;
    stream << (quint8&) k.jlpt;
    stream << k.rmGroups.size();
    foreach(ReadingMeaningGroup *rmg, k.rmGroups)
        stream << *rmg;
    stream << k.nanoriReadings;
    return stream;
}

const QString &Kanji::getLiteral() const
{
    return literal;
}

void Kanji::setLiteral(const QString &s)
{
    literal = s;
}

int Kanji::getUnicode() const
{
    return unicode;
}

void Kanji::setUnicode(int u)
{
    unicode = u;
}

const QString &Kanji::getJis208() const
{
    return jis208;
}

void Kanji::setJis208(const QString &s)
{
    jis208 = s;
}

const QString &Kanji::getJis212() const
{
    return jis212;
}

void Kanji::setJis212(const QString &s)
{
    jis212 = s;
}

const QString &Kanji::getJis213() const
{
    return jis213;
}

void Kanji::setJis213(const QString &s)
{
    jis213 = s;
}

char Kanji::getClassicalRadical() const
{
    return classicalRadical;
}

char Kanji::getNelsonRadical() const
{
    return nelsonRadical;
}

void Kanji::setClassicalRadical(char radical)
{
    classicalRadical = radical;
}

void Kanji::setNelsonRadical(char radical)
{
    nelsonRadical = radical;
}

char Kanji::getGrade() const
{
    return grade;
}

void Kanji::setGrade(char g)
{
    grade = g;
}

char Kanji::getStrokeCount() const
{
    return strokeCount;
}

void Kanji::setStrokeCount(char count)
{
    strokeCount = count;
}

const QSet<int> & Kanji::getUnicodeVariants() const
{
    return unicodeVariants;
}

const QSet<QString> & Kanji::getJis208Variants() const
{
    return jis208Variants;
}

const QSet<QString> & Kanji::getJis212Variants() const
{
    return jis212Variants;
}

const QSet<QString> & Kanji::getJis213Variants() const
{
    return jis213Variants;
}

void Kanji::addUnicodeVariant(int ucs)
{
    unicodeVariants.insert(ucs);
}

void Kanji::addJis208Variant(const QString &s)
{
    jis208Variants.insert(s);
}

void Kanji::addJis212Variant(const QString &s)
{
    jis212Variants.insert(s);
}

void Kanji::addJis213Variant(const QString &s)
{
    jis213Variants.insert(s);
}

// if 0 -> not one of the 2500 most frequent
short Kanji::getFrequency() const
{
    return frequency;
}

void Kanji::setFrequency(short freq)
{
    frequency = freq;
}

const QSet<QString> & Kanji::getNamesAsRadical() const
{
    return radicalNames;
}

void Kanji::addNameAsRadical(const QString &radicalName)
{
    radicalNames.insert(radicalName);
}

char Kanji::getJLPT() const
{
    return jlpt;
}

void Kanji::setJLPT(char j)
{
    jlpt = j;
}

const QList<ReadingMeaningGroup *> & Kanji::getReadingMeaningGroups() const
{
    return rmGroups;
}

void Kanji::addReadingMeaningGroup(ReadingMeaningGroup *rmGroup)
{
    rmGroups.append(rmGroup);
}

const QSet<QString> & Kanji::getNanoriReadings() const
{
    return nanoriReadings;
}

void Kanji::addNanoriReading(const QString &nanoriReading)
{
    nanoriReadings.insert(nanoriReading);
}
