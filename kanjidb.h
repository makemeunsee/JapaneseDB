#ifndef KANJIDB_H
#define KANJIDB_H

#include <QDomDocument>
#include <QMap>
#include <QSet>
#include <QString>

class Kanji;

class KanjiDB
{
public:
    KanjiDB();

    void searchByUnicode(int, QSet<Kanji *> &, bool);
    void searchByIntIndex(int, const QMap<int, QSet<Kanji *> *> &, QSet<Kanji *> &, bool);
    void searchByStringIndex(const QString &, const QMap<QString, Kanji *> &, QSet<Kanji *> &, bool);
    void searchByJIS208(const QString &, QSet<Kanji *> &, bool);
    void searchByJIS212(const QString &, QSet<Kanji *> &, bool);
    void searchByJIS213(const QString &, QSet<Kanji *> &, bool);
    void search(char strokeCount, char jlpt, char grade, char radical, QSet<Kanji *> &filledList);
    void search(const QString &, QSet<Kanji *> &);
    bool read(QIODevice *);
    QString parseKey(QString &parsedString, const QString &key, bool &unite);
    void findVariants(const Kanji *k, QSet<Kanji *> &setToFill);

    static const QString ucsKey;
    static const QString gradeKey;
    static const QString jlptKey;
    static const QString strokesKey;
    static const QString strokesMoreKey;
    static const QString strokesLessKey;
    static const QString jis208Key;
    static const QString jis212Key;
    static const QString jis213Key;

private:
    void parseCharacterElement(const QDomElement &);
    QDomDocument domDocument;
    QMap<int, Kanji *> kanjis;
    QMap<QString, Kanji *> kanjisJIS208;
    QMap<QString, Kanji *> kanjisJIS212;
    QMap<QString, Kanji *> kanjisJIS213;
    QMap<int, QSet<Kanji *> *> kanjisByStroke;
    QMap<int, QSet<Kanji *> *> kanjisByRadical;
    QMap<int, QSet<Kanji *> *> kanjisByGrade;
    QMap<int, QSet<Kanji *> *> kanjisByJLPT;
    int minStrokes, maxStrokes;
};

#endif // KANJIDB_H
