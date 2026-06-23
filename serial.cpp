#include "serial.h"
#include "mainwindow.h"
#include "comm.h"
extern Comm_Frame_t cPack;

SerialManager::SerialManager(MainWindow *mWin, QObject *parent)
    :QObject(parent), ptrmWin(mWin)
{
    serial = new QSerialPort(this);
    // 1. 列出所有可用的序列埠
    qDebug() << "--- 可用的序列埠 ---";
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        qDebug() << "名稱:" << info.portName() << "描述:" << info.description();
    }

}

//bool SerialManager::setCOM(const QString &text)
bool SerialManager::Open_COM(const QStringList &comInfo)
{
    //if (text.isEmpty()) return false;

    QString com = comInfo.at(0).split(" ").at(0);
    if(serial->isOpen())
    {
        qDebug() << "偵測到序列埠原本為開啟狀態，先進行關閉：" << serial->portName();
        serial->close();
        QThread::msleep(50);
    }
    serial->setPortName(com);

    serial->setBaudRate(comInfo.at(1).toInt());     // 波特率
    serial->setDataBits(QSerialPort::Data8);         // 資料位元 8
    serial->setParity(QSerialPort::NoParity);         // 無校驗
    serial->setStopBits(QSerialPort::OneStop);       // 停止位元 1
    serial->setFlowControl(QSerialPort::NoFlowControl); // 無流控

    // 開啟序列埠 (讀寫模式)
    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "成功開啟序列埠！";

        // 連接訊號槽：當收到資料時，觸發 readData
        connect(serial, &QSerialPort::readyRead, this, &SerialManager::readData);

        // 測試寫入資料
        //writeData("Hello UART!");
    } else {
        ptrmWin->Append_Text(QString("開啟失敗，錯誤代碼: %1").arg(serial->error()) );
        qDebug() << "開啟失敗，錯誤代碼:" << serial->error();
        return false;
    }
    return true;
}

void SerialManager::Close_COM()
{
    if(serial->isOpen())
    {
        serial->close();
        ptrmWin->Append_Text("已關閉序列埠");
    }
}

void SerialManager::scanPorts()
{
    const auto infos = QSerialPortInfo::availablePorts();
    QStringList serList;
    for (const QSerialPortInfo &info : infos) {
        QString serText = QString( "%1 (%2)").arg( info.portName() , info.description() );
        qDebug() << info.portName() << " (" << info.description() << ")";
        serList.append(serText);
        //emit requestAppendText(serText);
    }

    emit requestAppendList(serList);
}

void SerialManager::writeData(const QByteArray &data)
{
    if (serial->isOpen()) {
        serial->write(data);
        qDebug() << "已發送:" << data;
    }
}

void SerialManager::readData()
{
    if (!serial->isOpen()) {
        return;
    }
    // 讀取所有暫存區內的資料
    QByteArray data = serial->readAll();
    uint16_t crc_calc = 0;
    if(!data.isEmpty()) {
        for (int i = 0; i < data.size(); ++i) {
            if (data.at(i) == 0x00) {
                if(buf.size() > 0)
                {
                    cobs_decode(buf, buf.size(), recvPack);
                    crc_calc = deserialize(recvPack, &cPack);
                    if(0xFFFF == crc_calc )//error
                    {
                        buf.clear();
                        return;
                    }
                    ptrmWin->Append_Text(QString("======== PACKET ========"));
                    ptrmWin->Append_Text(QString("topic : %1").arg( topic_to_string(cPack.topic)) );
                    ptrmWin->Append_Text(QString("type : %1").arg( type_to_string(cPack.type)) );
                    ptrmWin->Append_Text(QString("len : %1").arg( cPack.len) );
                    ptrmWin->Append_Text(QString("seq : %1").arg( cPack.seq) );
                    QByteArray rawData((const char*)cPack.payload, cPack.len);
                    ptrmWin->Append_Text(QString("payload(Hex) : %1").arg( QString(rawData.toHex(' ').toUpper())) );
                    switch( cPack.type )
                    {
                        case VOLTAGE:
                            ptrmWin->Append_Text(QString("voltage : %1").arg( bytes_to_float(cPack.payload) ));
                        //default:
                    }

                    //ptrmWin->Append_Text(QString("crc_low : %1").arg( (uint16_t)(uint8_t)(cPack.crc_lo) ));
                    //ptrmWin->Append_Text(QString("crc_hi : %1").arg( (uint16_t)(uint8_t)(cPack.crc_hi) ));
                    uint16_t crc_recv = (uint16_t)((uint8_t)cPack.crc_lo) | ((uint16_t)(uint8_t)cPack.crc_hi << 8);
                    ptrmWin->Append_Text(QString("recv crc16 : %1").arg((uint16_t)crc_recv) );
                    ptrmWin->Append_Text(QString("calc crc16 : %1").arg((uint16_t)crc_calc) );
                    //clear and print raw data
                    ptrmWin->Append_Text(QString("raw packet(Hex) : %1").arg(recvPack.toHex(' ').toUpper()));
                    ptrmWin->Append_Text(QString("COBS packet(Hex) : %1").arg(buf.toHex(' ').toUpper()));
                    buf.clear();
                    ptrmWin->Append_Text(QString("========================"));
                }
            }
            else
                buf.append(data.at(i));
        }
        //ptrmWin->Append_Text(data.toHex());
        qDebug() << "收到資料:" << data << " (Hex:" << data.toHex() << ")";
    }
}

bool SerialManager::COM_Is_Open()
{
    return serial->isOpen();
}
