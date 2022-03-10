#include "device.h"
#include "ui_device.h"
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <thread>
#include "windows.h"
#include <QLabel>
#include <QChar>
#include <string>
#include <QTimer>
#include <QMessageBox>
#include <QThread>

#define CONNECTION_LOST_DURATION 5000

device::device(QWidget *parent, int portnum, int minval, int maxval,QString name, QString IP):
    QMainWindow(parent), port(portnum),  min(minval),max(maxval),
    devicename(name),
    IP(IP),
    ui(new Ui::device)
{
    ui->setupUi(this);
    timenow = time(NULL);
    timeprev = time(NULL);
    this->setWindowTitle(devicename);


    //QTimer *timer = new QTimer;
    connect(&timer,SIGNAL(timeout()),this,SLOT(slotTimerAlarm()));
    timer.setSingleShot(true);
    timer.start(CONNECTION_LOST_DURATION);


    sock=new QUdpSocket(this);
    sock->bind(QHostAddress(IP), port); //QHostAddress("192.168.0.99")
    connect(sock, &QUdpSocket::readyRead,
                this, &device::readPendingDatagrams); //подключение


    ui->label_8->setAlignment(Qt::AlignCenter);
    ui->label_9->setAlignment(Qt::AlignCenter);
    ui->label_10->setAlignment(Qt::AlignCenter);
    ui->progressBar->setFormat("%v");
    ui->progressBar->setValue(min);
    ui->progressBar->setStyleSheet("QProgressBar{"
                                   "border: 1px solid transparent;text-align: center;"
                                   "border-radius: 5px;"
                                   "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(200, 200, 200, 200), stop:1 rgba(0, 0, 0, 200));"
                                   "}"
                                   );


    while (sock->waitForReadyRead(10)){
        while(sock->hasPendingDatagrams()){
            QByteArray buff;
            buff.resize(int(sock->pendingDatagramSize()));
            sock->readDatagram(buff.data(), buff.size());

        }
    }

}

device::~device()
{
    delete ui;
    delete sock;
}

void device::readPendingDatagrams()
{
    timenow = time(NULL);
    ui->label_5->setText(QString::number(timenow-timeprev));


    if ((timenow-timeprev)>5) {
        ui->statusbar->showMessage("delay error",5000);
        delay_error = true;
    }


    ui->progressBar->setRange(min,max);
    ui->progressBar->setTextVisible(true);


    QNetworkDatagram datagram = sock->receiveDatagram();
    QString message = datagram.data();
    QStringList list = message.split(',');

    head = list[0]; //проверка 1го символа
    if (head[0]!='#') {
        smth_is_wrong = true;
        ui->statusbar->showMessage("error",2500);
        return;
    }

//    ui->label_2->setText(datagram.data());

    volt=list[1].toFloat(); //напряжение
    ui->progressBar->setValue(volt);

    if (volt<=30.0){
        ui->progressBar->setStyleSheet("QProgressBar{"
                                       "border: 1px solid transparent;text-align: center;"
                                       "color:black;"
                                       "border-radius: 5px;"
                                       "border-width: 3px;"
                                       "border-image: 9,2,5,2; "
                                       "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(200, 200, 200, 200), stop:1 rgba(0, 0, 0, 200));"
                                        "}"
                                       "QProgressBar::chunk {background-color: red;}"
                                       );
    }
    if (volt>30.0 && volt<=34.0){
        ui->progressBar->setStyleSheet("QProgressBar{"
                                       "border: 1px solid transparent;text-align: center;"
                                       "color:black;"
                                       "border-radius: 5px;"
                                       "border-width: 3px;"
                                       "border-image: 9,2,5,2; "
                                       "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(200, 200, 200, 200), stop:1 rgba(0, 0, 0, 200));"
                                        "}"
                                       "QProgressBar::chunk {background-color: yellow;}"
                                       );
    }
    if (volt>34.0){
        ui->progressBar->setStyleSheet("QProgressBar{"
                                       "border: 1px solid transparent;text-align: center;"
                                       "color:black;"
                                       "border-radius: 5px;"
                                       "border-width: 3px;"
                                       "border-image: 9,2,5,2; "
                                       "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(200, 200, 200, 200), stop:1 rgba(0, 0, 0, 200));"
                                        "}"
                                       "QProgressBar::chunk {background-color: green;}"
                                       );
    }


    if (list[2]=="R"){ //идет ли запись
        is_record = true;
        ui->label_8->setText("OK");
        ui->label_8->setStyleSheet("QLabel {background-color: green}");
    }
    else{
        if(list[2]=="E") {is_record=false;
        ui->label_8->setText("Error");
        ui->label_8->setStyleSheet("QLabel {background-color: red}");
        }

        else {smth_is_wrong = true;}  //если идет не то значение, что задумано
    }



    QString lastpart = list[3]; //последняя часть строки в формате "W*FF"

    if(lastpart[0]==QChar('W')){ //протечка
        is_flood = true;
        ui->label_9->setText("Error");
        ui->label_9->setStyleSheet("QLabel {background-color: red}");
        }
    else{
        if(lastpart[0]==QChar(' ')) {
            is_flood=false;
            ui->label_9->setText("OK");
            ui->label_9->setStyleSheet("QLabel {background-color: green}");
        }
    else {smth_is_wrong = true;}
    }




    CrcBuild(message);

    if (!smth_is_wrong){
        timeprev = timenow;
        timer.start(CONNECTION_LOST_DURATION);
    }
    if (!delay_error){
        ui->label_10->setText("OK");
        ui->label_10->setStyleSheet("QLabel {background-color: green}");
    }
    else {
        ui->statusbar->showMessage("error",2500);
        ui->label_10->setText("Error");
        ui->label_10->setStyleSheet("QLabel {background-color: red}");
    }

    smth_is_wrong=false;
    delay_error = false;
}

void device::CrcBuild(QString message)
{
    char TM_Str [15];
    for (int i = 0;i<=13;i++){
        TM_Str[i] = message[i].toLatin1();
    }

    uint8_t crc = 0;
    for (uint8_t i = 0; i <= 12; i++)
    {
        crc += TM_Str[i];
    }
    uint8_t LB = crc & 0x0f;
    uint8_t HB = crc >> 4;
    uint8_t LB_cnt = 0;
    uint8_t HB_cnt = 0;

    for (uint8_t a = 0; a <= HB; a++)
    {
        HB_cnt++;
    }

    if (HB_cnt == 0) TM_Str[13] = '0';
    else if (HB_cnt == 1) TM_Str[13] = '1';
    else if (HB_cnt == 2) TM_Str[13] = '2';
    else if (HB_cnt == 3) TM_Str[13] = '3';
    else if (HB_cnt == 4) TM_Str[13] = '4';
    else if (HB_cnt == 5) TM_Str[13] = '5';
    else if (HB_cnt == 6) TM_Str[13] = '6';
    else if (HB_cnt == 7) TM_Str[13] = '7';
    else if (HB_cnt == 8) TM_Str[13] = '8';
    else if (HB_cnt == 9) TM_Str[13] = '9';
    else if (HB_cnt == 10) TM_Str[13] = 'A';
    else if (HB_cnt == 11) TM_Str[13] = 'B';
    else if (HB_cnt == 12) TM_Str[13] = 'C';
    else if (HB_cnt == 13) TM_Str[13] = 'D';
    else if (HB_cnt == 14) TM_Str[13] = 'E';
    else if (HB_cnt == 15) TM_Str[13] = 'F';

    for (uint8_t b = 0; b <= LB; b++)
    {
        LB_cnt++;
    }
    if (LB_cnt == 0) TM_Str[14] = '0';
    else if (LB_cnt == 1) TM_Str[14] = '1';
    else if (LB_cnt == 2) TM_Str[14] = '2';
    else if (LB_cnt == 3) TM_Str[14] = '3';
    else if (LB_cnt == 4) TM_Str[14] = '4';
    else if (LB_cnt == 5) TM_Str[14] = '5';
    else if (LB_cnt == 6) TM_Str[14] = '6';
    else if (LB_cnt == 7) TM_Str[14] = '7';
    else if (LB_cnt == 8) TM_Str[14] = '8';
    else if (LB_cnt == 9) TM_Str[14] = '9';
    else if (LB_cnt == 10) TM_Str[14] = 'A';
    else if (LB_cnt == 11) TM_Str[14] = 'B';
    else if (LB_cnt == 12) TM_Str[14] = 'C';
    else if (LB_cnt == 13) TM_Str[14] = 'D';
    else if (LB_cnt == 14) TM_Str[14] = 'E';
    else if (LB_cnt == 15) TM_Str[14] = 'F';

    //на время тестирования
//    ui->label_6->setText(QString(TM_Str[13]));
//    ui->label_7->setText(QString(TM_Str[14]));
    //на время тестирования


    if (message[13]!=QChar(TM_Str[13]) || message[14]!=QChar(TM_Str[14])){
        smth_is_wrong=true;
    }
}

void device::slotTimerAlarm(){
    //QMessageBox::warning(this, "Warning", "Connection lost ");
    ui->statusbar->showMessage("error",2500);
    ui->label_10->setText("Error");
    ui->label_10->setStyleSheet("QLabel {background-color: red}");
//    timer->start(CONNECTION_LOST_DURATION);
}
