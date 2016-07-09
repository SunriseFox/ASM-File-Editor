
#ifndef OPENEVENTFORMAC_H
#define OPENEVENTFORMAC_H

#include "mainwindow.h"
#include <QApplication>
#include <QFileOpenEvent>
#include <QString>
#include <QDebug>
QString openedFile;
class EventFilter : public QApplication
{
public:
    MainWindow* mw = NULL;
    EventFilter(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }

    bool event(QEvent *event) Q_DECL_OVERRIDE
    {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
            openedFile = openEvent->file();
            if(mw)mw->readNewFile();
        }

        return QApplication::event(event);
    }
};

#endif // OPENEVENTFORMAC_H

