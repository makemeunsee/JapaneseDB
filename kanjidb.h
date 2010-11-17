#ifndef KANJIDB_H
#define KANJIDB_H

#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDataStream>
#include "kanji.h"

class QDomElement;

class KanjiDB
{
public:
    KanjiDB();
    ~KanjiDB();

    void clear();

    const Kanji *getByUnicode(unsigned int) const;
    void searchByUnicode(unsigned int, KanjiSet &, bool, int) const;
    void searchByIntIndex(unsigned int, const QMap<unsigned int, QSet<Kanji *> *> &, KanjiSet &, bool) const;
    void searchByStringIndex(const QString &, const QMap<QString, Kanji *> &, KanjiSet &, bool) const;
    void search(const QString &, KanjiSet &) const;
    const QMap<unsigned char, Kanji *> &getRadicalsMap() const;
    QString parseKey(QString &parsedString, const QString &key, bool &unite) const;
    void findVariants(const Kanji *k, KanjiSet &setToFill) const;

    friend QDataStream &operator <<(QDataStream &stream, const KanjiDB &);
    friend QDataStream &operator >>(QDataStream &stream, KanjiDB &);
    void initRadicals();
    bool readIndex(QIODevice *);
    bool readKanjiDic(QIODevice *);
    bool writeIndex(QIODevice *) const;
    const QString errorString() const;

    static const quint32 magic;
    static const quint32 version;

    static const QString unionSeps;
    static const QString interSeps;
    static const QString seps;
    static const QString notSeps;
    static const QString ucsKey;
    static const QString gradeKey;
    static const QString jlptKey;
    static const QString strokesKey;
    static const QString strokesMoreKey;
    static const QString strokesLessKey;
    static const QString jis208Key;
    static const QString jis212Key;
    static const QString jis213Key;
    static const QString radicalKey;
    static const int keyCount = 10;
    static const QString allKeys[keyCount];
    static const QString regexp;
    static const QRegExp searchRegexp;

    static inline QStringList fromArray(const QString *list, unsigned int size){
        QStringList l;
        for(unsigned int i = 0; i < size; ++i)
            l << list[i];
        return l;
    }

private:
    void parseCharacterElement(const QDomElement &);

    QMap<unsigned char, Kanji *> radicalsMap;
    QMap<unsigned int, unsigned char> radicalsByUcs;
    QMap<unsigned int, Kanji *> kanjis;
    QMap<QString, Kanji *> kanjisJIS208;
    QMap<QString, Kanji *> kanjisJIS212;
    QMap<QString, Kanji *> kanjisJIS213;
    QMap<unsigned int, QSet<Kanji *> *> kanjisByStroke;
    QMap<unsigned int, QSet<Kanji *> *> kanjisByRadical;
    QMap<unsigned int, QSet<Kanji *> *> kanjisByGrade;
    QMap<unsigned int, QSet<Kanji *> *> kanjisByJLPT;
    unsigned int minStrokes, maxStrokes;

    mutable QString error;
};

#endif // KANJIDB_H
