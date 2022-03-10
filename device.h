#ifndef DEVICE_H
#define DEVICE_H

#include <QMainWindow>
#include <QUdpSocket>
#include <ctime>
#include <QTimer>

namespace Ui {
class device;
}

class device : public QMainWindow
{
    Q_OBJECT

public:

    device(QWidget *parent = nullptr);
    device(QWidget *parent = nullptr, int portnum = 0, int minval = 0, int maxval = 0, QString devicename = "Empty", QString IP = "127.0.0.1");

    ~device();

    std::uint16_t min;
    std::uint16_t max;
    std::uint16_t port;
    QString devicename;

    QString IP;


    void setPort(int portnum){
        port = portnum;}

    void setMaxVal(int maxval){
        max = maxval;}

    void setMinVal(int minval){
        min = minval;}
    void CrcBuild(QString message);


    QString head, mask;
    float volt;
    bool is_record, is_flood;

    bool smth_is_wrong = false;
    bool delay_error = false;
    time_t timenow, timeprev;
    QTimer timer;



private slots:
    void readPendingDatagrams();
    void slotTimerAlarm();

private:
    Ui::device *ui;
    QUdpSocket *sock = nullptr;
};

#endif // DEVICE_H
