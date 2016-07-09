
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ASMEditor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
		selectcolor.cpp \
    src/codeeditor.cpp \
    src/myhighlighter.cpp

HEADERS  += mainwindow.h \
		openeventformac.h \
		selectcolor.h \
    src/codeeditor.h \
    src/myhighlighter.h \
    src/typedef.h

FORMS    += mainwindow.ui \
    selectcolor.ui
