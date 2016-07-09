#include "myhighlighter.h"

MyHighLighter::MyHighLighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    settings.beginGroup("color");
    keywordColor = settings.value("keywordColor","#FF4500").toString();
    registerColor = settings.value("registerColor","#A8660B").toString();
    literalColor = settings.value("literalColor","#FF8800").toString();
    expressionColor = settings.value("expressionColor","#0000FF").toString();
    commentColor =  settings.value("commentColor","#009900").toString();
    labelColor = settings.value("labelColor","#9900FF").toString();
    settings.endGroup();

    keywordFormat.setForeground(QBrush(keywordColor));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bMOV\\b" << "\\bPUSH\\b" << "\\bPOP\\b"
                    << "\\bXCHG\\b" << "\\bIN\\b" << "\\bOUT\\b"
                    << "\\LEA\\b" << "\\bLDS\\b" << "\\bLES\\b"
                    << "\\bLAHF\\b" << "\\bSHAF\\b" << "\\bPUSHF\\b"
                    << "\\bPOPF\\b" << "\\bADD\\b" << "\\bADC\\b"
                    << "\\bINC\\b" << "\\bSUB\\b" << "\\bDEC\\b"
                    << "\\bNEG\\b" << "\\bCMP\\b" << "\\bMUL\\b"
                    << "\\bIMUL\\b" << "\\bDIV\\b" << "\\bIDIV\\b"
                    << "\\bCBW\\b" << "\\bCWD\\b" << "\\bAND\\b"
                    << "\\bOR\\b" << "\\bNOT\\b" << "\\bOFFSET\\b"
                    << "\\bXOR\\b" << "\\bTEST\\b" << "\\bSHL\\b"
                    << "\\bSAL\\b" << "\\bSHR\\b" << "\\bROL\\b"
                    << "\\ROR\\b" << "\\bRCL\\b" << "\\bRCR\\b"
                    << "\\bMOVS\\b" << "\\bCMPS\\b" << "\\bSCAS\\b"
                    << "\\bJZ\\b" << "\\bJS\\b" << "\\bJNS\\b"
                    << "\\bJO\\b" << "\\bJNO\\b" << "\\bJP\\b"
                    << "\\bJMP\\b" << "\\bJZ\\b" << "\\bJNZ\\b"
                    << "\\bJA\\b" << "\\bJB\\b" << "\\bJGE\\b"
                    << "\\bJLE\\b" << "\\bJE\\b" << "\\bJNE\\b"
                    << "\\bJNP\\b" << "\\bLOOP\\b" << "\\bLOOPPZ\\b"
                    << "\\bILOOPE\\b" << "\\bLOOPNZ\\b" << "\\bLOOPNE\\b"
                    << "\\bCALL\\b" << "\\bRET\\b" << "\\bINT\\b"
                    << "\\bINTO\\b" << "\\bRIET\\b"<< "\\bASSUME\\b"
                    << "\\bCLC\\b" << "\\bCMC\\b" << "\\bSTC\\b"
                    << "\\bCLD\\b" << "\\bSTD\\b" << "\\bCLI\\b"
                    << "\\bSTI\\b" << "\\bNOP\\b" << "\\bHLT\\b"
                    << "\\bWAIT\\b" << "\\bESC\\b"<< "\\bLOC\\b"
                    << "\\bSEGMENT\\b" << "\\bEND\\b"<< "\\bENDS\\b"
                    << "\\bDB\\b" << "\\bDW\\b" << "\\bDD\\b"
                    << "\\bPROC\\b" << "\\bNEAR\\b" << "\\bFAR\\b"
                    << "\\bENDP\\b" << "\\bREP\\b" << "\\bJC\\b";

    foreach (const QString &pattern, keywordPatterns) {
        tempRule.pattern = QRegExp(pattern);
        tempRule.format = keywordFormat;
        highlightingRules.append(tempRule);
    }

    QStringList registerPatterns;
    registerPatterns << "\\bE?AX\\b"<<"\\bE?BX\\b"<<"\\bE?CX\\b"
                     <<"\\bE?DX\\b"<<"\\bE?SP\\b"<<"\\bE?BP\\b"
                     <<"\\bE?SI\\b"<<"\\bE?DI\\b"<<"\\bAH\\b"
                     <<"\\bAL\\b"<<"\\bBH\\b"<<"\\bBL\\b"<<"\\bCH\\b"
                     <<"\\bCL\\b"<<"\\bDH\\b"<<"\\bDL\\b"
                     <<"\\bE?FLAGS\\b"<<"\\bE?IP\\b"<<"\\bCS\\b"
                     <<"\\bDS\\b"<<"\\bES\\b"<<"\\bSS\\b";

    //TODO:
    labelFormat.setFontWeight(QFont::Bold);
    labelFormat.setForeground(QBrush(labelColor));

    tempRule.pattern = QRegExp("\\b[A-Z_][A-Z0-9_]*:(?![\\w\[])");
    tempRule.format = labelFormat;
    highlightingRules.append(tempRule);
    tempRule.pattern = QRegExp("\\b[A-Z_][A-Z0-9_]*(?=\\s*ENDP\\s*)");
    tempRule.format = labelFormat;
    highlightingRules.append(tempRule);
    tempRule.pattern = QRegExp("\\b[A-Z_][A-Z0-9_]*(?=\\s*PROC\\s*)");
    tempRule.format = labelFormat;
    highlightingRules.append(tempRule);

    registerFormat.setFontItalic(true);
    registerFormat.setForeground(QBrush(registerColor));

    foreach (const QString &pattern, registerPatterns) {
        tempRule.pattern = QRegExp(pattern);
        tempRule.format = registerFormat;
        highlightingRules.append(tempRule);
    }

    literalFormat.setFontWeight(QFont::Bold);
    literalFormat.setForeground(QBrush(literalColor));
    tempRule.pattern = QRegExp("\\b[0-9][0-9A-F]*[BDH]?\\b");
    tempRule.format = literalFormat;
    highlightingRules.append(tempRule);

    singleLineCommentFormat.setForeground(QBrush(commentColor));
    tempRule.pattern = QRegExp(";[^\n]*");
    tempRule.format = singleLineCommentFormat;
    highlightingRules.append(tempRule);

    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::blue);
    tempRule.pattern = QRegExp("[\'\"].*[\'\"]");
    tempRule.format = quotationFormat;
    highlightingRules.append(tempRule);


    expressionFormat.setForeground(QBrush(expressionColor));
    tempRule.pattern = QRegExp("\\'[\\w\\d\\s]*\\'");
    tempRule.format = expressionFormat;
    highlightingRules.append(tempRule);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void MyHighLighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
}
