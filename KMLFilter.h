#ifndef KMLFILTER_H
#define KMLFILTER_H

#include <utility>
#include <vector>
#include <QFile>
#include <QDebug>
#include <QDomDocument>
#include <QVector>
class KMLFilter
{
public:

    inline static const QString UPDATED_FILE_PREFIX = "updated_";

    KMLFilter() = default;
    KMLFilter(const KMLFilter &_other) = delete;
    KMLFilter(KMLFilter &&_other)  = delete;
    KMLFilter& operator=(const KMLFilter &_other) & = delete;
    KMLFilter& operator=(KMLFilter &&_other) && = delete;
    bool ReadKML(const QString &filePath);
    size_t FilterWaypoint(double _distance);
    bool WriteKML(const QString &filePath);

    size_t GetRawCoordinateCount();
    size_t GetFinalCoordinateCount();
private:

    bool ParseStringList(const QStringList &_list);
    //holder for unfiltered coordinates in radians
    std::vector< std::pair<double, double> > m_RawCoordinateHolder;
    //holder for filtered coordinates in degrees
    std::vector< std::pair<double, double> > m_FinalCoordinateHolder;

    double CalculateDistance(double lat1, double lon1, double lat2, double lon2);
};

#endif // KMLFILTER_H
