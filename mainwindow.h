#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColorDialog>
#include <QColor>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_plus_clicked() ;
    void on_minus_clicked() ;
    void on_makeAScript_clicked();

private:

    QString collectFileText();
    QString writeBlinkProcedure(QString textLine, int stateNum, int blinkTimer);
    QColorDialog *colorDialog;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
