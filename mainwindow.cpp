#include "mainwindow.h"
#include "selectcolor.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    this->setupUi(this);

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(1280, 720)).toSize());
    move(settings.value("pos", QPoint(0, 100)).toPoint());
    settings.endGroup();

    settings.beginGroup("RecentFiles");
    connect(menu,SIGNAL(triggered(QAction*)),this,SLOT(recentFileAction(QAction*)));
    foreach(const QString& file,settings.allKeys())
        menu->addAction(settings.value(file).toString());
    settings.endGroup();

    notSaved = false;

    connect(this->action_C,SIGNAL(triggered()),this,SLOT(commentSelection()));
    connect(this->saveNormal,SIGNAL(triggered()),this,SLOT(saveFile()));
    connect(this->action_R,SIGNAL(triggered()),this,SLOT(compileAndRun()));
    connect(this->action_Q,SIGNAL(triggered()),this,SLOT(runOnly()));
    connect(this->action_B,SIGNAL(triggered()),this,SLOT(compileOnly()));
    connect(this->action_X,SIGNAL(triggered()),qApp,SLOT(quit()));
    connect(qApp,SIGNAL(focusChanged(QWidget*,QWidget*)),this,SLOT(stopZoom()));

    location_s = QCoreApplication::applicationDirPath();
#ifdef Q_OS_MAC
    location_s.remove(location_s.indexOf(".app/"),location_s.length());
    location_s.remove(location_s.lastIndexOf("/"),location_s.length());
#endif //Q_OS_MAC

#ifndef Q_OS_WINRT
    dosbox_p = new QProcess;
#endif //Q_OS_WINRT

    configEditor = new CodeEditor();
    configEditor->setMode(EDIT);

#ifdef Q_OS_WIN
    configEditor->setFont(QFont("Courier New"));
#endif //Q_OS_WIN

    connect(this->action_Z,SIGNAL(triggered()),configEditor,SLOT(undo()));
    connect(this->action_Y,SIGNAL(triggered()),configEditor,SLOT(redo()));
    gridLayout->addWidget(configEditor);

#ifdef Q_OS_MAC
    if(!openedFile.isEmpty())
    {
        setWindowTitle(currentFile_s+"-ASM Editor");
        currentFile_s = openedFile;
    }
#else
    if(QCoreApplication::arguments().count()>1)
    {
        currentFile_s=QCoreApplication::arguments().at(1);
        setWindowTitle(currentFile_s+"-ASM Editor");
    }
#endif //Q_OS_MAC
    else
    {
        setWindowTitle("临时文件-ASM Editor");
        currentFile_s = location_s+"/bin/tempfile.asm";
    }

    write_f = new QFile(currentFile_s);
    write_f->open(QIODevice::ReadOnly);

    if(write_f->isOpen())
        configEditor->setPlainText(write_f->readAll());
    else
        this->on_newFile_triggered();

    write_f->close();
    write_f->deleteLater();

    highlighter = new MyHighLighter(configEditor->document());
    connect(configEditor,SIGNAL(textChanged()),this,SLOT(setChanged()));
}

MainWindow::~MainWindow()
{
    if(!this->isMaximized() && !this->isMinimized())
    {
        settings.beginGroup("MainWindow");
        settings.setValue("size", size());
        settings.setValue("pos", pos());
        settings.endGroup();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(notSaved){ if(!alertNotSaved())event->ignore();}
    else event->accept();
}

bool MainWindow::compileOnly()
{
    write_f = new QFile(location_s+"/bin/tempfile.asm");
    write_f->open(QIODevice::WriteOnly);
    write_f->write(configEditor->toPlainText().toLatin1());
    write_f->close();
    write_f->deleteLater();

#ifndef Q_OS_WINRT
#ifdef Q_OS_MAC
    dosbox_p->start("open -a "+ location_s +"/DOSBox.app --args -noautoexec -c \"mount c: "+location_s+"/\" -c c: -c \"cd bin\" -c \"ml tempfile.asm /nologo >tempfile.err\" -c exit");
    QEventLoop waitMac;
    QTimer::singleShot(5000,&waitMac,SLOT(quit()));
    waitMac.exec();
#else
    if(dosbox_p->isOpen())dosbox_p->close();
    dosbox_p->waitForFinished(3000);
    dosbox_p->start("dosbox -noautoexec -noconsole -c \"mount c: "+location_s+"\" -c c: -c \"cd bin\" -c \"ml tempfile.asm /nologo >tempfile.err\" -c exit");
    dosbox_p->waitForStarted(3000);
    if(dosbox_p->state()!=QProcess::Running)
        dosbox_p->close();
    else
    {
        dosbox_p->waitForFinished(10000);
        if(dosbox_p->state()==QProcess::Running)
            dosbox_p->close();
    }
#endif //Q_OS_MAC
#endif //Q_OS_WINRT

    write_f = new QFile(location_s+"/bin/TEMPFILE.ERR");
    write_f->open(QIODevice::ReadOnly);
    QString errinfo = write_f->readAll();
    write_f->close();
    write_f->deleteLater();

    if(errinfo.indexOf("error")==-1)
    {
        static QMessageBox* sucmsg = new QMessageBox(this);
        sucmsg->setWindowTitle("Success!");
        sucmsg->setText("编译成功!");
        sucmsg->setModal(false);
        sucmsg->setWindowFlags(sucmsg->windowFlags()|Qt::WindowDoesNotAcceptFocus);
        sucmsg->show();
        connect(qApp,SIGNAL(focusObjectChanged(QObject*)),sucmsg,SLOT(close()));
    }
    else
    {
        QMessageBox* errmsg = new QMessageBox(this);
        errmsg->setWindowTitle("Error Detected...");
        errmsg->setText(errinfo);
        errmsg->show();
        QRegExp findLine("\\d+(?=\\))");
        int pos = 0;
        QList<int> errLine;
        while ((pos = findLine.indexIn(errinfo, pos)) != -1)
        {
            errLine.append(findLine.cap(0).toInt());
            pos += findLine.matchedLength();
        }
        configEditor->highlightErrorLine(errLine);
        return false;
    }
    return true;
}

void MainWindow::saveFile()
{
    write_f = new QFile(currentFile_s);
    write_f->open(QIODevice::WriteOnly);
    if(write_f->isOpen())
        write_f->write(configEditor->toPlainText().toUtf8());
    else
    {
        write_f->deleteLater();
        on_saveAs_triggered();
        return;
    }
    write_f->close();
    write_f->deleteLater();
    setWindowTitle(currentFile_s+"-ASM Editor");
    notSaved = false;
}

void MainWindow::commentSelection()
{
    QTextCursor cur = configEditor->textCursor();
    if(cur.hasSelection())
    {
        int a = cur.selectionStart();
        int b = cur.selectionEnd()-1;
        cur.movePosition(QTextCursor::Start);
        cur.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,a);
        cur.movePosition(QTextCursor::StartOfBlock);
        while(cur.position() <= b)
        {
            cur.movePosition(QTextCursor::StartOfBlock);
            cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
            while(cur.selectedText()==" ")
            {
                cur.movePosition(QTextCursor::NextWord);
                cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
            }
            if(cur.selectedText()==";")
            {
                cur.removeSelectedText();
                b--;
            }
            else
            {
                cur.movePosition(QTextCursor::PreviousCharacter);
                cur.insertText(";");
                b++;
            }
            int temp = cur.position();
            cur.movePosition(QTextCursor::NextBlock);
            if(temp == cur.position())break;
        }

    }
    else
    {
        cur.movePosition(QTextCursor::StartOfBlock);
        cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
        while(cur.selectedText()==" ")
        {
            cur.movePosition(QTextCursor::NextWord);
            cur.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
        }
        if(cur.selectedText()==";")
            cur.removeSelectedText();
        else
        {
            cur.movePosition(QTextCursor::PreviousCharacter);
            cur.insertText(";");
        }
    }
}

void MainWindow::runOnly()
{
#ifndef Q_OS_WINRT
#ifdef Q_OS_MAC
    dosbox_p->start("open -a "+ location_s +"/DOSBox.app --args -noautoexec -c \"mount c: "+location_s+"/\" -c c: -c \"cd bin\" -c CLS -c \"@echo off\" -c \"tempfile "+argv+"\" -c echo. -c pause -c exit");
#else
    if(dosbox_p->isOpen())dosbox_p->close();
    dosbox_p->waitForFinished(3000);
    dosbox_p->start("dosbox -noautoexec -noconsole -c \"mount c: "+location_s+"\" -c c: -c \"cd bin\" -c CLS -c \"@echo off\" -c \"tempfile "+argv+"\" -c echo. -c pause -c exit");
    dosbox_p->waitForStarted(3000);
#endif //Q_OS_MAC
#endif //Q_OS_WINRT
}

void MainWindow::compileAndRun()
{
    if(compileOnly())
        runOnly();
}

void MainWindow::on_saveAs_triggered()
{
    QString temp;
    temp=QFileDialog::getSaveFileName(this,tr("Save To..."),currentFile_s==location_s+"/bin/tempfile.asm"?location_s+"temp.asm":currentFile_s,tr("Source Code (*.asm)"));
    if(!temp.isEmpty())
    {
        currentFile_s = temp;
        saveFile();
    }
}

void MainWindow::on_newFile_triggered()
{
    if(notSaved) if(!alertNotSaved())return;
    configEditor->clear();
    configEditor->appendPlainText("DATAS SEGMENT\n    \nDATAS ENDS\n\nSTACKS SEGMENT\n    \nSTACKS ENDS\n\nCODES SEGMENT\n    ASSUME CS:CODES,DS:DATAS,SS:STACKS\nSTART:\n    MOV AX,DATAS\n    MOV DS,AX\n    \n    MOV AH,4CH\n    INT 21H\nCODES ENDS\n    END START\n");
    setWindowTitle("临时文件-ASM Editor");
    currentFile_s = location_s+"/bin/tempfile.asm";
    notSaved = false;
}

void MainWindow::on_openFile_triggered()
{
    if(notSaved) if(!alertNotSaved())return;

    QString temp = QFileDialog::getOpenFileName(this,tr("Open..."),"",tr("Source Code (*.asm)"));
    if(temp.isEmpty())return;

    currentFile_s=temp;
    setWindowTitle(currentFile_s+"-ASM Editor");

    settings.beginGroup("RecentFiles");
    settings.setValue(QDateTime::currentDateTime().toString("yymmddhhmmss"),currentFile_s);
    settings.endGroup();

    configEditor->clear();
    write_f = new QFile(currentFile_s);
    write_f->open(QIODevice::ReadOnly);
    configEditor->setPlainText(write_f->readAll());
    write_f->close();
    write_f->deleteLater();
    menu->addAction(currentFile_s);
    notSaved = false;
}

void MainWindow::on_debugAction_triggered()
{
    if(compileOnly())
    {
#ifndef Q_OS_WINRT
#ifdef Q_OS_MAC
        dosbox_p->start("open -a "+ location_s +"/DOSBox.app --args -noautoexec -c \"mount c: "+location_s+"/\" -c c: -c \"cd bin\" -c CLS -c \"@echo off\" -c \"debug tempfile.exe "+argv+"\" -c echo. -c pause -c exit");
#else
        dosbox_p->start("dosbox -noautoexec -noconsole -c \"mount c: "+location_s+"\" -c c: -c \"cd bin\" -c CLS -c \"@echo off\" -c \"debug tempfile.exe "+argv+"\" -c echo. -c pause -c exit");
        dosbox_p->waitForStarted(3000);
#endif
#endif
    }
}


void MainWindow::on_toUpper_triggered()
{
    if(configEditor->textCursor().hasSelection())
    {
        QString temp = configEditor->textCursor().selectedText();
        configEditor->insertPlainText(temp.toUpper());
    }
    else
    {
        QStringList temp = configEditor->toPlainText().split(QRegExp("[\'\"]"));
        for(int i=0;i<temp.size();i+=2)
            temp[i] = temp.at(i).toUpper();
        configEditor->selectAll();
        configEditor->insertPlainText(temp.join("\'"));
    }
}

void MainWindow::on_formatText_triggered()
{
    configEditor->formatText();
}

void MainWindow::recentFileAction(QAction *action)
{
    if(notSaved) if(!alertNotSaved())return;
    write_f = new QFile(action->text());
    if(!write_f->exists())
    {
        menu->removeAction(action);
        settings.beginGroup("RecentFiles");
        foreach(const QString& file,settings.allKeys())
            if(settings.value(file).toString()==action->text())
                settings.remove(file);
        settings.endGroup();
        action->deleteLater();
        write_f->deleteLater();
        return;
    }
    currentFile_s = action->text();
    setWindowTitle(currentFile_s+"-ASM Editor");
    write_f = new QFile(currentFile_s);
    write_f->open(QIODevice::ReadOnly);
    configEditor->setPlainText(write_f->readAll());
    write_f->close();
    write_f->deleteLater();
    notSaved = false;
}

void MainWindow::stopZoom()
{
    configEditor->ctrlPressed = false;
}

void MainWindow::setChanged()
{
    static bool dontChange = true;
    if(dontChange) dontChange = false;
    else notSaved = true;
}

bool MainWindow::alertNotSaved()
{
    static QMessageBox alertSave;
    alertSave.setText("The document has been modified.");
    alertSave.setInformativeText("Do you want to save your changes?");
    alertSave.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    alertSave.setDefaultButton(QMessageBox::Save);
    int ret = alertSave.exec();
    switch (ret) {
    case QMessageBox::Save:
        saveFile();
        break;
    case QMessageBox::Discard:
        notSaved = false;
        break;
    case QMessageBox::Cancel:
        return false;
    default:
        return false;
        break;
    }
    return true;
}
#ifdef Q_OS_MAC
void MainWindow::readNewFile()
{
    if(notSaved) if(!alertNotSaved())return;
    if(!openedFile.isEmpty())
        currentFile_s = openedFile;
    else currentFile_s = location_s+"/bin/tempfile.asm";
    setWindowTitle(currentFile_s+"-ASM Editor");
    write_f = new QFile(currentFile_s);
    write_f->open(QIODevice::ReadOnly);
    configEditor->setPlainText(write_f->readAll());
    write_f->close();
    write_f->deleteLater();
}
#endif

void MainWindow::on_aboutAction_triggered()
{
    static QMessageBox about;
    about.setText("\n其实并没有关于这种东西啦>.<\n");
    about.show();
}

void MainWindow::on_setArgvAction_triggered()
{
    static QInputDialog dialog;
    dialog.setTextValue(argv);
    dialog.resize(this->width()*0.6,dialog.height());
    if(dialog.exec()==QDialog::Accepted)
        argv = dialog.textValue();
}

void MainWindow::on_gDebugAction_triggered()
{
    if(compileOnly())
    {
#ifndef Q_OS_WINRT
#ifdef Q_OS_MAC
        dosbox_p->start("open -a "+ location_s +"/DOSBox.app --args -noautoexec -c \"mount c: "+location_s+"/\" -c c: -c \"cd bin\" -c CLS -c \"@echo off\" -c \"cv tempfile.exe "+argv+"\" -c echo. -c pause -c exit");
#else
        dosbox_p->start("dosbox -noautoexec -noconsole -c \"mount c: "+location_s+"\" -c c: -c \"cd bin\" -c CLS -c \"@echo off\" -c \"cv tempfile.exe "+argv+"\" -c echo. -c pause -c exit");
        dosbox_p->waitForStarted(3000);
#endif
#endif
    }
}

void MainWindow::on_manualAction_triggered()
{
#ifndef Q_OS_WINRT
#ifdef Q_OS_MAC
    dosbox_p->start("open -a "+ location_s +"/DOSBox.app --args -noautoexec -c \"mount c: "+location_s+"/\" -c c: -c \"cd bin\"");
#else
    if(dosbox_p->isOpen())dosbox_p->close();
    dosbox_p->waitForFinished(3000);
    dosbox_p->start("dosbox -noautoexec -noconsole -c \"mount c: "+location_s+"\" -c c: -c \"cd bin\"");
    dosbox_p->waitForStarted(3000);
#endif
#endif
}

void MainWindow::on_setColor_triggered()
{
    SelectColor *color = new SelectColor;
    if(color->exec()==QDialog::Accepted)
    {
        settings.beginGroup("color");
        settings.remove("");
        settings.endGroup();
    }
    highlighter->deleteLater();
    highlighter = new MyHighLighter(configEditor->document());
    color->deleteLater();
}

void MainWindow::on_action_2_triggered()
{
    //TODO:
}
