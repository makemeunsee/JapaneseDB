#ifndef KANJI_H
#define KANJI_H

#include <QString>
#include <QMap>
#include <QList>
#include <QDataStream>
#include "readingmeaninggroup.h"

class Kanji
{
public:
    Kanji();
    ~Kanji();
    const QString &getLiteral() const;
    int getUnicode() const;
    const QString &getJis208() const;
    const QString &getJis212() const;
    const QString &getJis213() const;
    char getClassicalRadical() const;
    char getNelsonRadical() const;
    char getGrade() const;
    char getStrokeCount() const;
    const QSet<int> & getUnicodeVariants() const;
    const QSet<QString> & getJis208Variants() const;
    const QSet<QString> & getJis212Variants() const;
    const QSet<QString> & getJis213Variants() const;
    // if 0 -> not one of the 2500 most frequent
    short getFrequency() const;
    const QSet<QString> & getNamesAsRadical() const;
    char getJLPT() const;
    const QList<ReadingMeaningGroup *> & getReadingMeaningGroups() const;
    const QSet<QString> & getNanoriReadings() const;

    friend QDataStream &operator <<(QDataStream &stream, const Kanji &);
    friend QDataStream &operator >>(QDataStream &stream, Kanji &);

    void setLiteral(const QString &);
    void setUnicode(int);
    void setJis208(const QString &);
    void setJis212(const QString &);
    void setJis213(const QString &);
    void setClassicalRadical(char);
    void setNelsonRadical(char);
    void setGrade(char);
    void setStrokeCount(char);
    void addUnicodeVariant(int);
    void addJis208Variant(const QString &);
    void addJis212Variant(const QString &);
    void addJis213Variant(const QString &);
    void setFrequency(short);
    void addNameAsRadical(const QString &);
    void setJLPT(char);
    void addReadingMeaningGroup(ReadingMeaningGroup *);
    void addNanoriReading(const QString &);


private:
    QString literal;
    int unicode;
    QString jis208;
    QString jis212;
    QString jis213;
    char classicalRadical;
    char nelsonRadical;
    char grade;
    char strokeCount;
    QSet<int> unicodeVariants;
    QSet<QString> jis208Variants;
    QSet<QString> jis212Variants;
    QSet<QString> jis213Variants;
    short frequency;
    // names in hiragana if this kanji is a radical and has a name
    QSet<QString> radicalNames;
    char jlpt;
    QList<ReadingMeaningGroup *> rmGroups;
    // readings for names only
    QSet<QString> nanoriReadings;
};

typedef QMap<int, Kanji *> KanjiSet;
typedef QMutableMapIterator<int, Kanji *> KanjiSetIterator;

#endif // KANJI_H
