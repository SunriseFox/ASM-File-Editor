#include "selectcolor.h"
#include "ui_selectcolor.h"

SelectColor::SelectColor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectColor)
{
    ui->setupUi(this);
    settings.beginGroup("color");
    keywordColor = settings.value("keywordColor","#FF4500").toString();
    labelColor = settings.value("labelColor","#9900FF").toString();
    registerColor = settings.value("registerColor","#A8660B").toString();
    literalColor = settings.value("literalColor","#FF8800").toString();
    expressionColor = settings.value("expressionColor","#0000FF").toString();
    commentColor =  settings.value("commentColor","#009900").toString();
    settings.endGroup();
    updateButtonColor();
}

SelectColor::~SelectColor()
{
    delete ui;
}

void SelectColor::hideEvent(QHideEvent *event)
{
    confirmOnHide();
    QDialog::hideEvent(event);
}

void SelectColor::updateButtonColor()
{

    ui->keywordButton->setStyleSheet(QString("background: ")+keywordColor.name());
    ui->labelButton->setStyleSheet(QString("background: ")+labelColor.name());
    ui->registerButton->setStyleSheet(QString("background: ")+registerColor.name());
    ui->literalButton->setStyleSheet(QString("background: ")+literalColor.name());
    ui->expressionButton->setStyleSheet(QString("background: ")+expressionColor.name());
    ui->commentButton->setStyleSheet(QString("background: ")+commentColor.name());
}

void SelectColor::confirmOnHide()
{
    settings.beginGroup("color");
    settings.setValue("keywordColor",keywordColor.name());
    settings.setValue("labelColor",labelColor.name());
    settings.setValue("registerColor",registerColor.name());
    settings.setValue("literalColor",literalColor.name());
    settings.setValue("expressionColor",expressionColor.name());
    settings.setValue("commentColor",commentColor.name());
    settings.endGroup();
}

void SelectColor::on_keywordButton_clicked()
{
    QColor temp = QColorDialog::getColor(keywordColor,this);
    if(temp.isValid())
        keywordColor = temp;
    ui->keywordButton->setStyleSheet(QString("background: ")+keywordColor.name());
}

void SelectColor::on_registerButton_clicked()
{
    QColor temp = QColorDialog::getColor(registerColor,this);
    if(temp.isValid())
        registerColor = temp;
    ui->registerButton->setStyleSheet(QString("background: ")+registerColor.name());
}

void SelectColor::on_literalButton_clicked()
{
    QColor temp = QColorDialog::getColor(literalColor,this);
    if(temp.isValid())
        literalColor = temp;
    ui->literalButton->setStyleSheet(QString("background: ")+literalColor.name());
}

void SelectColor::on_expressionButton_clicked()
{
    QColor temp = QColorDialog::getColor(expressionColor,this);
    if(temp.isValid())
        expressionColor = temp;
    ui->expressionButton->setStyleSheet(QString("background: ")+expressionColor.name());
}

void SelectColor::on_labelButton_clicked()
{
    QColor temp = QColorDialog::getColor(labelColor,this);
    if(temp.isValid())
        labelColor = temp;
    ui->labelButton->setStyleSheet(QString("background: ")+labelColor.name());
}

void SelectColor::on_commentButton_clicked()
{
    QColor temp = QColorDialog::getColor(commentColor,this);
    if(temp.isValid())
        commentColor = temp;
    ui->commentButton->setStyleSheet(QString("background: ")+commentColor.name());
}

void SelectColor::on_pushButton_clicked()
{
    //Note: this is OK button
    reject();
}

void SelectColor::on_pushButton_2_clicked()
{
    //Note: this is Reset button
    accept();
}
