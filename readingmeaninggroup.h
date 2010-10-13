#ifndef READINGMEANINGGROUP_H
#define READINGMEANINGGROUP_H

#include <QSet>
#include <QDataStream>

class QString;

class ReadingMeaningGroup
{
public:
    ReadingMeaningGroup();
    void addFrenchMeaning(const QString &);
    void addEnglishMeaning(const QString &);
    void addOnReading(const QString &);
    void addKunReading(const QString &);
    const QSet<QString> & getOnReadings();
    const QSet<QString> & getKunReadings();
    const QSet<QString> & getFrenchMeanings();
    const QSet<QString> & getEnglishMeanings();

    friend QDataStream &operator >>(QDataStream &stream, ReadingMeaningGroup &rmg);
    friend QDataStream &operator <<(QDataStream &stream, const ReadingMeaningGroup &rmg);

private:
    QSet<QString> onReadings;
    QSet<QString> kunReadings;
    QSet<QString> englishMeanings;
    QSet<QString> frenchMeanings;
    // other languages to come???
};

#endif // READINGMEANINGGROUP_H
