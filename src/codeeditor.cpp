#include "codeeditor.h"
#include <QtWidgets>
#include <QDebug>
CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    settings.beginGroup("editor");
    zoomLevel = settings.value("ZoomLevel",0).toInt();
    settings.endGroup();

    strChanged = true;

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this,SIGNAL(textChanged()),this,SLOT(reconnectHighlighter()));
    connect(&delayer,SIGNAL(timeout()),this,SLOT(highlightCurrentWord()));
    delayer.setInterval(500);
    delayer.setSingleShot(true);
    ctrlPressed = false;
    updateLineNumberAreaWidth(0);
    setMode(BROWSE);

    indentInc << QRegExp("\\bSEGMENT\\b") << QRegExp("\\bPROC\\b")<< QRegExp("\\bENDS\\b") << QRegExp("\\bENDP\\b") << QRegExp(":$") << QRegExp(":\\s");
    indentDec << QRegExp("\\bSEGMENT\\b") << QRegExp("\\bPROC\\b")<< QRegExp("\\bENDP\\b") << QRegExp("\\bENDS\\b") << QRegExp(":$") << QRegExp(":\\s");
    QTimer::singleShot(100,this,SLOT(reZoom()));
}

CodeEditor::~CodeEditor()
{
    settings.beginGroup("editor");
    settings.setValue("ZoomLevel",zoomLevel);
    settings.endGroup();
}
int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::highlightCurrentWord()
{
    QList<QTextEdit::ExtraSelection> extraSelectionList = extraSelections();
    QTextCursor temp = this->textCursor();
    temp.select(QTextCursor::WordUnderCursor);
    static QRegExp word;
    word.setPattern(QString("%1%2%1").arg("\\b").arg(temp.selectedText().isEmpty()?"EmptyString":temp.selectedText()));
    if(strChanged)
    {
        str = this->toPlainText();
        strChanged = false;
    }
    int i = 0, times = 0;
    while((i = str.indexOf(word,i))!=-1) {
        temp.setPosition(i);
        temp.select(QTextCursor::WordUnderCursor);
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::green).lighter(170);
        selection.format.setBackground(lineColor);
        selection.cursor = temp;
        extraSelectionList.append(selection);
        i++;
        times++;
    }
    if(times<2)return;
    setExtraSelections(extraSelectionList);
}
void CodeEditor::keyPressEvent(QKeyEvent* event)
{
    if(event->key()==Qt::Key_Tab)
    {
        formatCurrentBlock();
        event->accept();
    }
    else if(event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter)
    {
        if(this->textCursor().atBlockEnd())
            this->insertPlainText("\n    ");
        else if(this->textCursor().atBlockStart())
            this->insertPlainText("    \n");
        else this->insertPlainText("\n");
        event->accept();
    }
    else if(event->key()==Qt::Key_Colon)
    {
        this->insertPlainText(":");
        if(this->textCursor().atBlockEnd())
            this->formatCurrentBlock();
        event->accept();
    }
    else if(event->key()==Qt::Key_Control)
    {
        ctrlPressed = true;
    }
    else
    {
        QPlainTextEdit::keyPressEvent(event);
        event->ignore();
    }
}

void CodeEditor::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
        ctrlPressed = false;
}

void CodeEditor::wheelEvent(QWheelEvent *event)
{
    if(ctrlPressed)
    {
        if(event->angleDelta().y()>0)
        {
            this->zoomIn();
            if(zoomLevel<25)zoomLevel++;
        }
        else if(event->angleDelta().y()<0)
        {
            this->zoomOut();
            if(zoomLevel>-7)zoomLevel--;
        }
        event->accept();
    }
    else
    {
        QPlainTextEdit::wheelEvent(event);
        event->ignore();
    }
}
void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::reconnectHighlighter()
{
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    strChanged = true;
}

void CodeEditor::reZoom()
{
    this->zoomIn(zoomLevel);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}
void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelectionList;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(190);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        (selection.cursor = textCursor()).movePosition(QTextCursor::StartOfBlock);
        int endPos = selection.cursor.block().next().position();
        selection.cursor.clearSelection();
        extraSelectionList.append(selection);
        while(selection.cursor.movePosition(QTextCursor::Down)&&selection.cursor.position()<endPos)
        {
            selection.cursor.clearSelection();
            extraSelectionList.append(selection);
        }
    }
    setExtraSelections(extraSelectionList);
    delayer.start();
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(-2, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::formatText()
{
    //这个函数非常慢...你要原谅
    this->moveCursor(QTextCursor::Start);
    this->formatCurrentBlock();
    QTextCursor temp = this->textCursor();
    while(temp.movePosition(QTextCursor::NextBlock))
    {
        this->formatCurrentBlock();
        this->setTextCursor(temp);
    }
}
void CodeEditor::formatCurrentBlock()
{
    int indentLevel = 0;
    QTextCursor curTemp = this->textCursor();
    curTemp.select(QTextCursor::BlockUnderCursor);
    QString temp = curTemp.selectedText();

    if(temp.at(0)==QChar(0x2029))
        curTemp.insertText(QChar(0x2029));
    curTemp.insertText(temp = temp.simplified());
    curTemp.movePosition(QTextCursor::StartOfBlock);
    QTextCursor prevTemp = curTemp;
    while(prevTemp.movePosition(QTextCursor::PreviousBlock))
    {
        prevTemp.select(QTextCursor::BlockUnderCursor);
        if(!prevTemp.selectedText().isEmpty())break;
    }
    for(int i=0;i<indentDec.size();i++)
    {
        if(temp.contains(indentDec.at(i)))
        {
            indentLevel--;
            break;
        }
    }
    for(int i=0;i<indentInc.size();i++)
    {
        if(prevTemp.selectedText().contains(indentInc.at(i)))
        {
            indentLevel++;
            break;
        }
    }
    prevTemp.movePosition(QTextCursor::StartOfBlock);
    prevTemp.movePosition(QTextCursor::NextWord,QTextCursor::KeepAnchor);

    if(indentLevel==-1){
        if(prevTemp.selectedText().startsWith("    "))
            curTemp.insertText(prevTemp.selectedText().remove(0,4));
    }
    else{
        if(prevTemp.selectedText().at(0)==' ')
            curTemp.insertText(prevTemp.selectedText());
    }
    if(indentLevel==1)
        curTemp.insertText("    ");
}

void CodeEditor::highlightErrorLine(const QList<int>& line)
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::red).lighter(180);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    foreach(int lineNum,line)
    {
        lineNum--;
        selection.cursor.movePosition(QTextCursor::Start);
        selection.cursor.movePosition(QTextCursor::NextBlock,QTextCursor::KeepAnchor,lineNum);
        int endPos = selection.cursor.block().next().position();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
        while(selection.cursor.movePosition(QTextCursor::Down)&&selection.cursor.position()<endPos)
        {
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }
    }
    setExtraSelections(extraSelections);
    disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
}


void CodeEditor::setMode(editorMode mode)
{
    if(mode == BROWSE)
    {
        this->setReadOnly(true);
        this->setStyleSheet("background:#f2f2f3;");
         highlightCurrentLine();
    }
    else if(mode == EDIT)
    {
        this->setReadOnly(false);
        this->setStyleSheet("background:#ffffff;");
         highlightCurrentLine();
    }
}
