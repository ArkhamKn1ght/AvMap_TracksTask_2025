#include "KMLFilter.h"

bool KMLFilter::ReadKML(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qDebug() << "Failed to parse KML file.";
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "kml") {
        qDebug() << "Not a valid KML file.";
        return false;
    }

    QDomNodeList placemarks = root.elementsByTagName("Placemark");
    for (int i = 0; i < placemarks.count(); ++i) {

        QDomNode node = placemarks.at(i);
        if (!node.isElement()) continue;

        const QDomElement placemark = node.toElement();
        const QDomElement nameElement = placemark.firstChildElement("name");
        const QString name = nameElement.isNull() ? "Unnamed" : nameElement.text().trimmed();

        const QDomElement multiGeometry = placemark.firstChildElement("MultiGeometry");
        if (multiGeometry.isNull()) {
            qWarning() << "Failed to parse xml: missing MultiGeometry element";
            return false;
        }

        const QDomElement lineString = multiGeometry.firstChildElement("LineString");
        if (lineString.isNull()) {
            qWarning() << "Failed to parse xml: missing LineString element";
            return false;
        }

        const QDomElement coordinatesElement = lineString.firstChildElement("coordinates");
        if (coordinatesElement.isNull()) {
            qWarning() << "Failed to parse xml: missing coordinates element";
            return false;
        }

        QString tempStr = coordinatesElement.text().trimmed();
        tempStr.replace(' ', ',');
        const QStringList coordinates = tempStr.split(',');

        if( !ParseStringList(coordinates) ) {
            qWarning() << "Failed to parse xml: corrupt or invalid coordinate data!";
            return false;
        }
    }
    return true;
}

size_t KMLFilter::FilterWaypoint(double _distance) {
    if( _distance <= 0 ) return 0;
    if( m_RawCoordinateHolder.size() <= 1 ) return 0;
    size_t res = 1;
    if( !m_FinalCoordinateHolder.empty() ) m_FinalCoordinateHolder.clear();
    //add initial point
    m_FinalCoordinateHolder.push_back( std::pair( qRadiansToDegrees( m_RawCoordinateHolder[0].first ),
                                                qRadiansToDegrees( m_RawCoordinateHolder[0].second ) ) );
    for(size_t i = 1, curIdx = 0; i < m_RawCoordinateHolder.size(); ++i) {
        const auto tempVal1 = m_RawCoordinateHolder[curIdx];
        const auto tempVal2 = m_RawCoordinateHolder[i];
        if( CalculateDistance(tempVal2.first, tempVal2.second,
                              tempVal1.first, tempVal1.second)  < _distance ) {
            continue;
        } else {
            m_FinalCoordinateHolder.push_back( std::pair( qRadiansToDegrees( tempVal2.first ),
                                                        qRadiansToDegrees( tempVal2.second ) ) );
            curIdx = i;
            ++res;
        }
    }
    return res;
}

bool KMLFilter::WriteKML(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for reading:" << file.errorString();
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Failed to parse KML file.";
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "kml") {
        qWarning() << "Not a valid KML file.";
        return false;
    }

    QDomNodeList placemarks = root.elementsByTagName("Placemark");
    if (placemarks.isEmpty()) {
        qWarning() << "No placemarks found in KML.";
        return false;
    }

    for (int i = 0; i < placemarks.count(); ++i) {
        QDomNode node = placemarks.at(i);
        if ( !node.isElement() ) continue;

        QDomElement placemark = node.toElement();
        QDomElement multiGeometry = placemark.firstChildElement("MultiGeometry");
        if (multiGeometry.isNull()) continue;

        QDomElement lineString = multiGeometry.firstChildElement("LineString");
        if (lineString.isNull()) continue;

        QDomElement coordinatesElement = lineString.firstChildElement("coordinates");
        if (coordinatesElement.isNull()) continue;

        QString newCoordinatesStr;
        for(auto it = m_FinalCoordinateHolder.begin(), ite = m_FinalCoordinateHolder.end(); it != ite; ++it) {
            newCoordinatesStr += QString::number(it->second, 'f', 9) + "," + QString::number(it->first,  'f', 9) + " ";
        }
        newCoordinatesStr = newCoordinatesStr.trimmed();

        QDomText newCoordinatesText = doc.createTextNode(newCoordinatesStr);
        coordinatesElement.firstChild().setNodeValue(newCoordinatesStr);
    }
    QFile outFile(UPDATED_FILE_PREFIX + filePath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Failed to open file for writing:" << outFile.errorString();
        return false;
    }

    QTextStream stream(&outFile);
    stream << doc.toString();
    outFile.close();

    return true;
}

size_t KMLFilter::GetRawCoordinateCount() {
    return m_RawCoordinateHolder.size();
}

size_t KMLFilter::GetFinalCoordinateCount() {
    return m_FinalCoordinateHolder.size();
}

bool KMLFilter::ParseStringList(const QStringList &_list) {
    if( !m_RawCoordinateHolder.empty() ) m_RawCoordinateHolder.clear();

    bool successFlag = true;
    for(int i = 0; i < _list.size(); i += 2) {
        static double longtitude = 0;
        static double latitude = 0;
        longtitude = _list[i].toDouble(&successFlag);
        if(!successFlag) {
            qWarning() << "Corrupted coordinates of a waypoint!";
            return false;
        }
        latitude = _list[i + 1].toDouble(&successFlag);
        if(!successFlag) {
            qWarning() << "Corrupted coordinates of a waypoint!";
            return false;
        }
        m_RawCoordinateHolder.push_back( std::pair( qDegreesToRadians( latitude ), qDegreesToRadians( longtitude ) ) );
    }
    return true;
}

double KMLFilter::CalculateDistance(double lat1, double lon1, double lat2, double lon2)
{
    const double earthRadius = 6372.795; // in km
    double angularDistance;

    // calculate angular distance between points
    angularDistance = 2*asin(sqrt(sin((lat2-lat1)/2)*sin((lat2-lat1)/2) +
                                    cos(lat1)*cos(lat2)*sin(fabs(lon1-lon2)/2)*sin(fabs(lon1-lon2)/2)));
    // convert angular distance to km
    return angularDistance * earthRadius;
}
