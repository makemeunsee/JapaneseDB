#include "readingmeaninggroup.h"

ReadingMeaningGroup::ReadingMeaningGroup()
{
}

const QSet<QString> & ReadingMeaningGroup::getOnReadings()
{
    return onReadings;
}

void ReadingMeaningGroup::addOnReading(const QString &onReading)
{
    onReadings.insert(onReading);
}

const QSet<QString> & ReadingMeaningGroup::getKunReadings()
{
    return kunReadings;
}

void ReadingMeaningGroup::addKunReading(const QString &kunReading)
{
    kunReadings.insert(kunReading);
}

const QSet<QString> & ReadingMeaningGroup::getFrenchMeanings()
{
    return frenchMeanings;
}

void ReadingMeaningGroup::addFrenchMeaning(const QString &frenchMeaning)
{
    frenchMeanings.insert(frenchMeaning);
}

const QSet<QString> & ReadingMeaningGroup::getEnglishMeanings()
{
    return englishMeanings;
}

void ReadingMeaningGroup::addEnglishMeaning(const QString &englishMeaning)
{
    englishMeanings.insert(englishMeaning);
}
