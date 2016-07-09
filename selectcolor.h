#ifndef SELECTCOLOR_H
#define SELECTCOLOR_H

#include <QDialog>
#include <QSettings>
#include <QColor>
#include <QColorDialog>

namespace Ui {
class SelectColor;
}

class SelectColor : public QDialog
{
    Q_OBJECT

public:
    explicit SelectColor(QWidget *parent = 0);
    ~SelectColor();

private slots:
    void on_keywordButton_clicked();
    void on_registerButton_clicked();
    void on_literalButton_clicked();
    void on_expressionButton_clicked();
    void on_labelButton_clicked();
    void on_commentButton_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::SelectColor *ui;
    QSettings settings;
    QColor keywordColor;
    QColor labelColor;
    QColor registerColor;
    QColor literalColor;
    QColor expressionColor;
    QColor commentColor;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    void updateButtonColor();
    void confirmOnHide();
};

#endif // SELECTCOLOR_H
