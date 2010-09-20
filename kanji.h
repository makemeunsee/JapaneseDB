#ifndef KANJI_H
#define KANJI_H

#include <QString>
#include <QSet>
#include <QList>

class ReadingMeaningGroup;

class Kanji
{
public:
    Kanji(const QString &);
    ~Kanji();
    const QString getLiteral();
    int getUnicode();
    const QString getJis208();
    const QString getJis212();
    const QString getJis213();
    char getClassicalRadical();
    char getNelsonRadical();
    char getGrade();
    char getStrokeCount();
    const QSet<int> & getUnicodeVariants();
    const QSet<QString> & getJis208Variants();
    const QSet<QString> & getJis212Variants();
    const QSet<QString> & getJis213Variants();
    // if 0 -> not one of the 2500 most frequent
    short getFrequency();
    const QSet<QString> & getNamesAsRadical();
    char getJLPT();
    const QList<ReadingMeaningGroup *> & getReadingMeaningGroups();
    const QSet<QString> & getNanoriReadings();

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

#endif // KANJI_H
