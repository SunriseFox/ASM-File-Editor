#ifndef CODEEDITOR_H
#define CODEEDITOR_H
#include "src/typedef.h"
#include <QSize>
#include <QTimer>
#include <QObject>
#include <QWidget>
#include <QSettings>
#include <QRegExp>
#include <QList>
#include <QTimer>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    int zoomLevel;
    CodeEditor(QWidget *parent = 0);
    ~CodeEditor();
    void setMode(editorMode mode);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void formatText();
    void formatCurrentBlock();
    void highlightErrorLine(const QList<int>& line);
    int lineNumberAreaWidth();
    bool ctrlPressed;
    QSettings settings;
    QList<QRegExp> indentInc;
    QList<QRegExp> indentDec;
public slots:

    void highlightCurrentWord();
protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void reconnectHighlighter();
    void reZoom();

private:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    QWidget *lineNumberArea;
    bool strChanged;
    QString str;
    QTimer delayer;

};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const Q_DECL_OVERRIDE {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};

#endif // CODEEDITOR_H
