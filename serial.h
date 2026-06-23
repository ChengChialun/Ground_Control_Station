#ifndef SERIAL_H
#define SERIAL_H

#include <QCoreApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QThread>
#include <QObject>

class MainWindow;

class SerialManager : public QObject {
    Q_OBJECT
signals:
    //void requestAppendText(const QString &text);
    void requestAppendList(const QStringList &sList);
public:
    explicit SerialManager(MainWindow *mWin, QObject *parent = nullptr);

    ~SerialManager() {
        if (serial->isOpen()) {
            serial->close();
        }
    }

    // open and set COM port
    //bool setCOM(const QString &text);
    bool Open_COM(const QStringList &comInfo);
    void Close_COM();
    bool COM_Is_Open();
public slots:
    // 寫入資料的函數
    void writeData(const QByteArray &data);
    // 接收資料的槽函數
    void readData();
    // scan port
    void scanPorts();

private:
    QByteArray buf, recvPack;
    QSerialPort *serial;
protected:
    MainWindow *ptrmWin;
};
#endif // SERIAL_H
