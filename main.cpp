#include "mainwindow.h"
#include <QApplication>
#ifdef Q_OS_MAC
#include "openeventformac.h"
#endif
int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("SunriseFox");
    QCoreApplication::setOrganizationDomain("sunrisefox.tk");
    QCoreApplication::setApplicationName("ASMEditor");
#ifdef Q_OS_MAC
    EventFilter e(argc,argv);
    MainWindow w;
    e.mw = &w;
#else
    QApplication a(argc,argv);
    MainWindow w;
#endif
    w.show();
#ifdef Q_OS_MAC
    return e.exec();
#else
    return a.exec();
#endif
}
