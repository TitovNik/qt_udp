#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <regex>
#include <QMessageBox>
#include <QFile>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,"Question","Save configuration?",QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
    write_config(device_list); //запись параметров существующих объектов в конфиг файл
    }
    delete ui;

}



void MainWindow::on_pushButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,"Question","Create connection?",QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes){

        int portnum = ui->textEdit_2->toPlainText().toInt();
        int minval = ui->textEdit_3->toPlainText().toInt();
        int maxval = ui->textEdit_4->toPlainText().toInt();
        QString devicename = ui->textEdit_5->toPlainText();
        QString IP = ui->textEdit_6->toPlainText();

        device * window = new device(this, portnum, minval,maxval, devicename, IP);
        device_list.push_back(window);

        window->show();
    }
}


void MainWindow::on_pushButton_2_clicked() //read config
{
    QFile config("config.txt");
    QString line;
    config.open(QIODevice::ReadOnly | QIODevice::Text);

    int x=0; int j=0;
    while(!config.atEnd()){
        line = config.readLine();
        QStringList list = line.split(' ');
        device * window = new device(this, list[1].toInt(),list[2].toInt(),list[3].toInt(),list[0]);
        device_list.push_back(window);
        window->show();
        window->move(x,j);
        x+=320; //горизонтальный размер окна
        if (1920==x){ x=0; j+=300;} //TODO поприкалываться с разрешением экрана
    }

    config.close();
}

void MainWindow::write_config(std::list<device*> device_list){
    QFile config("config.txt");
    config.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&config);
    for (auto & p:device_list){
        out <<p->devicename<<' '<<p->port<<' '<<p->min<<' '<<p->max<<'\n';
    }
    config.close();
}
