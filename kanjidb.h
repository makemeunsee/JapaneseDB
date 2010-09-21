#ifndef KANJIDB_H
#define KANJIDB_H

#include <QDomDocument>
#include <QMap>
#include <QSet>

class Kanji;

class KanjiDB
{
public:
    KanjiDB();

    void searchByUnicode(int, QSet<Kanji *> &);
    void searchByJIS208(const QString &, QSet<Kanji *> &);
    void searchByJIS212(const QString &, QSet<Kanji *> &);
    void searchByJIS213(const QString &, QSet<Kanji *> &);
    void search(char strokeCount, char jlpt, char grade, char radical, QSet<Kanji *> &filledList);
    bool read(QIODevice *);

private:
    void parseCharacterElement(const QDomElement &);
    QDomDocument domDocument;
    QMap<int, Kanji *> kanjis;
    QMap<QString, Kanji *> kanjisJIS208;
    QMap<QString, Kanji *> kanjisJIS212;
    QMap<QString, Kanji *> kanjisJIS213;
    QMap<int, QSet<Kanji *> *> kanjisByStroke;
    QMap<char, QSet<Kanji *> *> kanjisByRadical;
    QMap<char, QSet<Kanji *> *> kanjisByGrade;
    QMap<char, QSet<Kanji *> *> kanjisByJLPT;
};

#endif // KANJIDB_H