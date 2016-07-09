#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#ifndef Q_OS_WINRT
#include <QProcess>
#endif
#include <QList>
#include <QFile>
#include <QString>
#include <QEventLoop>
#include <QTimer>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QShortcut>
#include <QWheelEvent>
#include <QSettings>
#include <QDateTime>
#include <QCloseEvent>
#include <QInputDialog>
#include "ui_mainwindow.h"
#include "src/codeeditor.h"
#include "src/myhighlighter.h"
#include "src/typedef.h"
#ifdef Q_OS_MAC
extern QString openedFile;
#endif
class MainWindow : public QMainWindow,Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
#ifdef Q_OS_MAC
    void readNewFile();
#endif
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
private slots:
    bool compileOnly();
    void runOnly();
    void compileAndRun();
    void saveFile();
    void commentSelection();
    void on_saveAs_triggered();
    void on_newFile_triggered();
    void on_openFile_triggered();
    void on_debugAction_triggered();
    void on_toUpper_triggered();
    void on_formatText_triggered();
    void on_aboutAction_triggered();
    void on_setArgvAction_triggered();
    void on_gDebugAction_triggered();
    void on_manualAction_triggered();
    void recentFileAction(QAction*);
    void stopZoom();
    void setChanged();
    void on_setColor_triggered();

    void on_action_2_triggered();

private:
    CodeEditor *configEditor;
#ifndef Q_OS_WINRT
    QProcess*  dosbox_p;
#endif
    QFile* write_f;
    QString location_s;
    QString currentFile_s;
    QShortcut* comment;
    QSettings settings;
    QStringList recentFiles;
    QString argv;
    MyHighLighter *highlighter;

    bool notSaved;
    bool alertNotSaved();
};

#endif // MAINWINDOW_H
