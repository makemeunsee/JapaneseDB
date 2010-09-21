#include "kanji.h"

Kanji::Kanji(const QString &string)
{
    literal = string;
}

const QString Kanji::getLiteral()
{
    return literal;
}

int Kanji::getUnicode()
{
    return unicode;
}

void Kanji::setUnicode(int u)
{
    unicode = u;
}

const QString Kanji::getJis208()
{
    return jis208;
}

void Kanji::setJis208(const QString &s)
{
    jis208 = s;
}

const QString Kanji::getJis212()
{
    return jis212;
}

void Kanji::setJis212(const QString &s)
{
    jis212 = s;
}

const QString Kanji::getJis213()
{
    return jis213;
}

void Kanji::setJis213(const QString &s)
{
    jis213 = s;
}

char Kanji::getClassicalRadical()
{
    return classicalRadical;
}

char Kanji::getNelsonRadical()
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

char Kanji::getGrade()
{
    return grade;
}

void Kanji::setGrade(char g)
{
    grade = g;
}

char Kanji::getStrokeCount()
{
    return strokeCount;
}

void Kanji::setStrokeCount(char count)
{
    strokeCount = count;
}

const QSet<int> & Kanji::getUnicodeVariants()
{
    return unicodeVariants;
}

const QSet<QString> & Kanji::getJis208Variants()
{
    return jis208Variants;
}

const QSet<QString> & Kanji::getJis212Variants()
{
    return jis212Variants;
}

const QSet<QString> & Kanji::getJis213Variants()
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
short Kanji::getFrequency()
{
    return frequency;
}

void Kanji::setFrequency(short freq)
{
    frequency = freq;
}

const QSet<QString> & Kanji::getNamesAsRadical()
{
    return radicalNames;
}

void Kanji::addNameAsRadical(const QString &radicalName)
{
    radicalNames.insert(radicalName);
}

char Kanji::getJLPT()
{
    return jlpt;
}

void Kanji::setJLPT(char j)
{
    jlpt = j;
}

const QList<ReadingMeaningGroup *> & Kanji::getReadingMeaningGroups()
{
    return rmGroups;
}

void Kanji::addReadingMeaningGroup(ReadingMeaningGroup *rmGroup)
{
    rmGroups.append(rmGroup);
}

const QSet<QString> & Kanji::getNanoriReadings()
{
    return nanoriReadings;
}

void Kanji::addNanoriReading(const QString &nanoriReading)
{
    nanoriReadings.insert(nanoriReading);
}
