#ifndef KANJIDB_H
#define KANJIDB_H

#include <QDomDocument>
#include <QMap>
#include <QSet>
#include <QString>
#include <QFile>
#include <QDataStream>

using namespace std;

class Kanji;

typedef QSet<Kanji *> KanjiSet;

class KanjiDB
{
public:
    KanjiDB();
    ~KanjiDB();

    void clear();

    void searchByUnicode(int, QSet<Kanji *> &, bool) const;
    void searchByIntIndex(int, const QMap<int, QSet<Kanji *> *> &, QSet<Kanji *> &, bool) const;
    void searchByStringIndex(const QString &, const QMap<QString, Kanji *> &, QSet<Kanji *> &, bool) const;
    void searchByJIS208(const QString &, QSet<Kanji *> &, bool) const;
    void searchByJIS212(const QString &, QSet<Kanji *> &, bool) const;
    void searchByJIS213(const QString &, QSet<Kanji *> &, bool) const;
    void search(const QString &, QSet<Kanji *> &) const;
    QString parseKey(QString &parsedString, const QString &key, bool &unite) const;
    void findVariants(const Kanji *k, QSet<Kanji *> &setToFill) const;

    friend QDataStream &operator <<(QDataStream &stream, const KanjiDB &);
    friend QDataStream &operator >>(QDataStream &stream, KanjiDB &);
    bool readIndex(QIODevice *);
    bool readKanjiDic(QIODevice *);
    bool writeIndex(QIODevice *) const;
    const QString errorString() const;

    static const quint32 magic;
    static const quint32 version;

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

    mutable QString error;
};

#endif // KANJIDB_H
