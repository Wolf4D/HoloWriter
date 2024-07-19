#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    colorDialog = new QColorDialog(this);
    colorDialog->setCurrentColor(QColor(65, 250, 255));
    connect(ui->colorButton, SIGNAL(clicked(bool)), colorDialog, SLOT(show()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_plus_clicked()
{
    QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
    item->setText("---");
    item->setFlags(item->flags() | Qt::ItemIsEditable);
}

void MainWindow::on_minus_clicked()
{
    QListWidgetItem * item = ui->listWidget->takeItem(ui->listWidget->currentRow());
    if (item!=nullptr) delete item;
}

void MainWindow::on_makeAScript_clicked()
{
    QFileDialog fileDialog(0, "Save script file",  ".",  "FPI Script (*.fpi)");
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    if (!fileDialog.exec()) return;
    if (fileDialog.selectedFiles().empty()) return;

    QString currentFileName = fileDialog.selectedFiles().at(0);

    QString fileContent = collectFileText();

    if (currentFileName.isEmpty()) return;

    QFile * currentFile = new QFile();

    currentFile->setFileName(currentFileName);
    currentFile->open(QIODevice::WriteOnly | QIODevice::Text);

    if (!currentFile->isOpen())
    {
        QMessageBox::critical(0,"Error", "File not avalible!");
        return;
    }

    QTextStream dataStream;

    dataStream.setDevice(currentFile);

    dataStream << fileContent;

    if (dataStream.status()!= QTextStream::Ok)
    {
        QMessageBox::critical(0,"Error", "File not written!");
        ui->statusbar->showMessage("File was NOT succesfully saved");
    }
    else
    {
        ui->statusbar->showMessage("File was succesfully saved");
        QMessageBox::information(this, "File was saved", "Script file was saved succesfully!");
    }

    currentFile->close();

}

QString MainWindow::collectFileText()
{
    QString text;
    text.append(";Artificial Intelligence Script\n\n;Header\n\ndesc          = "
                "Handwriter text made by Holowriter (by Navy Lik)\n\n;Triggers\n\n");
    QString tmpText = QString(":state=0:fpgcrawtextsize=%1,fpgcrawtextfont=verdana,"
                               "fpgcrawtextr=%2,fpgcrawtextg=%3,fpgcrawtextb=%4,fpgcrawtextx=%5,"
                               "fpgcrawtexty=%6,makesw=BlinkTimer 1,state=1\n").arg(
                ui->font->text(),
                QString::number(colorDialog->selectedColor().red()),
                QString::number(colorDialog->selectedColor().green()),
                QString::number(colorDialog->selectedColor().blue()),
                ui->X->text(),ui->Y->text());

    text.append(tmpText);

    // Начало мигания курсора
    if (ui->activateByTrigger->isChecked())
        text.append(":state=1,plrwithinzone=1:state=2,startsw=BlinkTimer 1");
    else
        text.append(":state=1,activated=1:state=2,startsw=BlinkTimer 1");

    // Звук начала
    if (!ui->startSound->text().isEmpty())
        text.append(QString(",plrsound=%1").arg(ui->startSound->text()));
    text.append("\n");

    if (ui->preDelay->value()>0)
    {

        tmpText = writeBlinkProcedure("",2, ui->preDelay->value());

        text.append(tmpText);
    }
    else
        text.append(":state=2:state=4,stopsw=BlinkTimer,etimerstart\n");

    // Начинам писать все строчки
    int currentLineState = 4;
    int gatheredDelay = 0;

    // Читаем построчно
    for(int i=0; i<ui->listWidget->count(); i++)
    {
        // Фикс запятой
        QString lineText = ui->listWidget->item(i)->text().replace(",", "‚");


        text.append("\n;Line output for: " + lineText +"\n");


        text.append(QString(":state=%1:fpgcrawtext=|\n").arg(currentLineState));

        QString firstHalf;
        QString secondHalf = QString(":fpgcrawtext=");

        // Читаем побуквенно строку
        while(!lineText.isEmpty())
        {
            // Считаем новую задержку
            gatheredDelay += ui->letterTime->value();

            // Собираем первую половину строки
            firstHalf = QString(":state=%1,etimergreater=").arg(currentLineState);

            // Начинаем собирать строку
            tmpText = firstHalf + QString::number(gatheredDelay);

            // Добавим букву
            secondHalf += lineText.at(0);
            tmpText += secondHalf + "|";

            // Обрежем строку
            lineText = lineText.right(lineText.count()-1);

            // Добавим звук, если нужен
            if (ui->letterSound->text().isEmpty())
                tmpText += QString(",state=%1\n").arg(currentLineState+1);
            else
                tmpText += ",plrsound=" + ui->letterSound->text() + QString(",state=%1\n").arg(currentLineState+1);
            text.append(tmpText);

            // Следующий стейт
            currentLineState++;

        }

        // А это уже мигание. Соберём готовую строку

        if (ui->lineTime->value()>0)
        {
            // Добавить стейт со стартом таймера
            tmpText = QString(":state=%1:state=%2,startsw=BlinkTimer 1\n").arg(QString::number(currentLineState),QString::number(currentLineState+1));
            text.append(tmpText);

            currentLineState++;

            tmpText = writeBlinkProcedure(ui->listWidget->item(i)->text().replace(",", "‚"), currentLineState, ui->lineTime->value());
            currentLineState += 2;

            text.append(tmpText);
        }

        gatheredDelay = 0;

    }

    // Звук конца
    if (!ui->endSound->text().isEmpty())
        text.append(QString("state=%1:state=%2,plrsound=%3").arg(QString::number(currentLineState), QString::number(currentLineState+1), ui->endSound->text()));


    return text;
}

QString MainWindow::writeBlinkProcedure(QString textLine, int stateNum, int blinkTimer)
{
    QString tmpText = QString("\n;Playing blink on delay\n"
                      ":state=%4:state=%5,etimerstart\n"
                      ":state=%5:fpgcrawtext=%6.\n"
                      ":state=%5,etimergreater=%1:fpgcrawtext=%6|\n"
                      ":state=%5,etimergreater=%2:state=%4\n"
                      ":state=%5,swgreater=BlinkTimer %3:state=%7,stopsw=BlinkTimer,etimerstart\n\n").
            arg(ui->letterTime->text(),
                QString::number(ui->letterTime->value()*2),
                QString::number(blinkTimer),
                QString::number(stateNum),
                QString::number(stateNum+1),
                textLine, QString::number(stateNum+2));

    return tmpText;
}

