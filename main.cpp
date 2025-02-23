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

    auto returnWrapper = [&](int code, const QString & _message) {
        if(_message.size() > 0)
            qWarning() << _message;

        out << "Press ENTER to exit" << Qt::flush;
        in.readLine();
        return code;
    };
    QString filepath;
    QString strDistance;
    if(argc < 3 ) {
        qWarning() << "Command line args were not provided or were passed incorrectly.\n"
                   << "Usage: .\\AvMap_TestTask.exe <filepath> <distance(km)>\n";
        out << "Enter filepath: " << Qt::flush;
        filepath = in.readLine();
        out << "Enter distance(in km): " << Qt::flush;
        strDistance = in.readLine();
    } else {
        filepath = argv[1];
        strDistance = argv[2];
    }




    bool okFlag = true;
    const double distance = strDistance.toDouble(&okFlag);
    if(!okFlag)
        return returnWrapper( -4, "Invalid distance value!" );

    if( !filter.ReadKML(filepath) )
        return returnWrapper( -1, "Failed to read KML file!" );

    size_t filteredWaypointCount = filter.FilterWaypoint(distance);
    if( filteredWaypointCount == 0 )
        return returnWrapper( -2, "Failed to filter waypoints!" );

    if( !filter.WriteKML("TrackItaly.kml") )
        return returnWrapper( -3, "Failed to write KML file!" );

    out << "Filtered waypoints file: " << KMLFilter::UPDATED_FILE_PREFIX + filepath << Qt::endl;
    out << "Minimal distance value : " << distance << " km" << Qt::endl;
    out << "Source KML coordinate count: " << filter.GetRawCoordinateCount() << Qt::endl;
    out << "Result KML coordinate count: " << filter.GetFinalCoordinateCount() << Qt::endl;
    return returnWrapper( 0, "" );
}
