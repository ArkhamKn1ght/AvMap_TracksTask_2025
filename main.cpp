#include <QCoreApplication>
#include "KMLFilter.h"
//use QT 6.8.0 bundled MinGW
//use QT 6.8.0
//in Projects -> Run: enable "Run in terminal" for testing under "Run" command in Qt Creator
//if using as independent .exe file, dont forget to put qt dlls in executable root dir.
//there are debug dlls in this repo, but if you dont trust those
//feel free to use your own.


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    KMLFilter filter;

    QTextStream in(stdin);
    QTextStream out(stdout);

    out << "Enter filepath: " << Qt::flush;
    const QString filepath = in.readLine();

    out << "Enter distance(in km): " << Qt::flush;
    const QString strDistance = in.readLine();

    bool okFlag = true;
    const double distance = strDistance.toDouble(&okFlag);
    if(!okFlag) {
        qWarning() << "Invalid distance value!";
        return -4;
    }

    if( !filter.ReadKML(filepath) ) {
        qWarning() << "Failed to read KML file!";
        return -1;
    }
    size_t filteredWaypointCount = filter.FilterWaypoint(0.1);
    if( filteredWaypointCount == 0 ) {
        qWarning() << "Failed to filter waypoints!";
        return -2;
    }

    if( !filter.WriteKML("TrackItaly.kml") ) {
        qWarning() << "Failed to write KML file!";
        return -3;
    }
    out << "Filtered waypoints file: " << KMLFilter::UPDATED_FILE_PREFIX + filepath << Qt::endl;
    out << "Minimal distance value : " << distance << " km" << Qt::endl;
    out << "Source KML coordinate count: " << filter.GetRawCoordinateCount() << Qt::endl;
    out << "Result KML coordinate count: " << filter.GetFinalCoordinateCount() << Qt::endl;
    return 0;
}
