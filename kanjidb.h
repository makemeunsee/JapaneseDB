#ifndef KANJIDB_H
#define KANJIDB_H

#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDataStream>
#include "kanji.h"

class QDomElement;

class KanjiDB
{
public:
    KanjiDB();
    ~KanjiDB();

    void clear();

    const Kanji *getByUnicode(Unicode) const;
    void searchByUnicode(Unicode, KanjiSet &, bool, int) const;
    void searchByIntIndex(unsigned int, const QMap<unsigned int, QSet<Kanji *> *> &, KanjiSet &, bool) const;
    void searchByStringIndex(const QString &, const QMap<QString, Kanji *> &, KanjiSet &, bool) const;
    void search(const QString &, KanjiSet &) const;
    void findVariants(const Kanji *k, KanjiSet &setToFill) const;

    const KanjiSet &getAllKanjis() const;
    const KanjiSet &getAllRadicals() const;
    const KanjiSet &getAllComponents() const;
    const QMap<Unicode, QString> &getFaultyComponents() const;

    const Kanji *getRadicalVariant(Unicode) const;
    const Kanji *getRadicalById(unsigned char) const;
    const Kanji *getComponentById(unsigned char) const;
    const Kanji *getComponent(Unicode) const;

    friend QDataStream &operator <<(QDataStream &stream, const KanjiDB &);
    friend QDataStream &operator >>(QDataStream &stream, KanjiDB &);
    int readResources(const QDir &);
    bool readIndex(QIODevice *);
    bool readKanjiDic(QIODevice *);
    bool readRadK(QIODevice *);
    bool readKRad(QIODevice *);
    bool writeIndex(QIODevice *) const;

    const QString errorString() const;

    static const QString kanjiDBIndexFilename;
    static const QString defaultKanjiDic2Filename;
    static const QString defaultKRadFilename;
    static const QString defaultKRad2Filename;
    static const QString defaultRadKXFilename;

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
    static const QString componentKey;
    static const int keyCount = 11;
    static const QString allKeys[keyCount];
    static const QString regexp;
    static const QRegExp searchRegexp;

    static const int allDataReadAndSaved = 0;
    static const int noDataRead = 1;
    static const int baseDataRead = 2;
    static const int allDataReadButNotSaved = 3;

    static inline QStringList fromArray(const QString *list, unsigned int size){
        QStringList l;
        for(unsigned int i = 0; i < size; ++i)
            l << list[i];
        return l;
    }

private:
    void initRadicals();
    void parseCharacterElement(const QDomElement &);
    QString parseKey(QString &parsedString, const QString &key, bool &unite) const;

    KanjiSet kanjis;
    QMap<QString, Kanji *> kanjisJIS208;
    QMap<QString, Kanji *> kanjisJIS212;
    QMap<QString, Kanji *> kanjisJIS213;
    QMap<unsigned int, QSet<Kanji *> *> kanjisByStroke;
    QMap<unsigned int, QSet<Kanji *> *> kanjisByRadical;
    QMap<unsigned int, QSet<Kanji *> *> kanjisByGrade;
    QMap<unsigned int, QSet<Kanji *> *> kanjisByJLPT;

    //classical radicals
    QMap<unsigned char, Unicode> radicalsByIndex;
    KanjiSet radicals;

    //radk components
    QMap<unsigned char, Unicode> componentIndexes;
    KanjiSet components;
    QMap<Unicode, QString> faultyComponents;

    QMap<Unicode, QSet<Kanji *> *> kanjisByComponent;

    unsigned int minStrokes, maxStrokes;

    mutable QString error;
};

#endif // KANJIDB_H
