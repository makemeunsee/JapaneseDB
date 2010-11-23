#ifndef KANJI_H
#define KANJI_H

#include <QString>
#include <QMap>
#include <QList>
#include <QDataStream>
#include "readingmeaninggroup.h"

typedef unsigned int Unicode;

class Kanji
{
public:
    Kanji();
    ~Kanji();
    const QString &getLiteral() const;
    Unicode getUnicode() const;
    const QString &getJis208() const;
    const QString &getJis212() const;
    const QString &getJis213() const;
    unsigned char getClassicalRadical() const;
    unsigned char getNelsonRadical() const;
    unsigned char getGrade() const;
    unsigned char getStrokeCount() const;
    const QSet<Unicode> & getUnicodeVariants() const;
    const QSet<QString> & getJis208Variants() const;
    const QSet<QString> & getJis212Variants() const;
    const QSet<QString> & getJis213Variants() const;
    // if 0 -> not one of the 2500 most frequent
    unsigned short getFrequency() const;
    const QSet<QString> & getNamesAsRadical() const;
    unsigned char getJLPT() const;
    const QList<ReadingMeaningGroup *> & getReadingMeaningGroups() const;
    const QSet<QString> & getNanoriReadings() const;

    friend QDataStream &operator <<(QDataStream &stream, const Kanji &);
    friend QDataStream &operator >>(QDataStream &stream, Kanji &);

    void setLiteral(const QString &);
    void setUnicode(Unicode);
    void setJis208(const QString &);
    void setJis212(const QString &);
    void setJis213(const QString &);
    void setClassicalRadical(unsigned char);
    void setNelsonRadical(unsigned char);
    void setGrade(unsigned char);
    void setStrokeCount(unsigned char);
    void addUnicodeVariant(unsigned int);
    void addJis208Variant(const QString &);
    void addJis212Variant(const QString &);
    void addJis213Variant(const QString &);
    void setFrequency(unsigned short);
    void addNameAsRadical(const QString &);
    void setJLPT(unsigned char);
    void addReadingMeaningGroup(ReadingMeaningGroup *);
    void addNanoriReading(const QString &);


private:
    QString literal;
    Unicode unicode;
    QString jis208;
    QString jis212;
    QString jis213;
    unsigned char classicalRadical;
    unsigned char nelsonRadical;
    unsigned char grade;
    unsigned char strokeCount;
    QSet<unsigned int> unicodeVariants;
    QSet<QString> jis208Variants;
    QSet<QString> jis212Variants;
    QSet<QString> jis213Variants;
    unsigned short frequency;
    // names in hiragana if this kanji is a radical and has a name
    QSet<QString> radicalNames;
    unsigned char jlpt;
    QList<ReadingMeaningGroup *> rmGroups;
    // readings for names only
    QSet<QString> nanoriReadings;
};

typedef QMap<Unicode, Kanji *> KanjiSet;
typedef QMutableMapIterator<Unicode, Kanji *> KanjiSetIterator;
typedef QMapIterator<Unicode, Kanji *> KanjiSetConstIterator;

#endif // KANJI_H
