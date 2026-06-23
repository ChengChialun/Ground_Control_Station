#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serial.h"
#include "debug.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbt.h>
#endif

std::vector<QFrame*> hLines;

#define H_LINE_CNT 3
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QVBoxLayout *central_VL = new QVBoxLayout();
    QGridLayout *central_up_GL = new QGridLayout();
    QGridLayout *com_GL = new QGridLayout();
    QGridLayout *tab_GL = new QGridLayout();
    QVBoxLayout *tab_Text_Main_VL = new QVBoxLayout();
    QGridLayout *tab_Text_GL = new QGridLayout();
    QGridLayout *send_GL = new QGridLayout();
    for (int i = 0; i < H_LINE_CNT; ++i) {
        QFrame *line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        // 不同顏色可以在這裡個別設定
        // line->setStyleSheet("color: red;");
        hLines.push_back(line);
    }
    central_VL->addWidget(hLines[0]);
    central_VL->addLayout(central_up_GL, 10);
    central_up_GL->addWidget(ui->chkbox_UART, 0, 0);
    central_up_GL->addLayout(com_GL, 1, 0);

    //com port block start
    com_GL->addWidget(ui->lbl_ComPort, 0, 0);
    com_GL->addWidget(ui->cmboBox_COM, 0, 1);
    com_GL->addWidget(ui->lbl_baudrate, 1, 0);
    com_GL->addWidget(ui->lEdt_baudrate, 1, 1);
    com_GL->addWidget(ui->btn_com, 2, 1);
    ui->btn_com->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    com_GL->setAlignment(ui->btn_com, Qt::AlignRight);
    //com port block end

    central_VL->addWidget(hLines[1]);
    central_VL->addLayout(tab_GL, 90);
    tab_GL->addWidget(ui->tabWidget);

    tab_Text_Main_VL->addLayout(tab_Text_GL, 95);
    tab_Text_Main_VL->addWidget(hLines[2]);
    //hLines[2]->setStyleSheet("margin-top: 10px; margin-bottom: 10px;");
    hLines[2]->setVisible(false);
    tab_Text_Main_VL->addLayout(send_GL, 5);
    tab_Text_GL->addWidget(ui->edtText, 0, 0);
    tab_Text_GL->setContentsMargins(0, 0, 5, 0);
    send_GL->addWidget(ui->ledt_send, 0, 0);
    send_GL->addWidget(ui->btn_send, 0, 1);
    send_GL->setColumnStretch(0, 9);
    send_GL->setColumnStretch(1, 1);

    ui->tab_TEXT->setLayout(tab_Text_Main_VL);

    ui->edtText->setMinimumWidth(0);
    ui->edtText->setMaximumWidth(QWIDGETSIZE_MAX);
    ui->edtText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->centralwidget->setLayout(central_VL);

    serial_manager(this, this);
}

void MainWindow::serial_manager(MainWindow *mWin, QWidget *parent)
{
    serial = new SerialManager(mWin, parent);

    connect(serial, &SerialManager::requestAppendList, this, &MainWindow::on_getCOMList);
    //connect(ui->cmboBox_COM, &QComboBox::currentTextChanged, this, &MainWindow::on_selCOM);
    connect(ui->btn_com, &QPushButton::clicked, this, &MainWindow::on_btnCOM);
  //  connect(serial, &SerialManager::requestAppendText, ui->edtText_UART, &QTextEdit::append);
    connect(ui->btn_send, &QPushButton::clicked, this, &MainWindow::on_btnSend);
    serial->scanPorts();

}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
#ifdef Q_OS_WIN
    // 轉換成 Windows 的 MSG 結構
    MSG *msg = static_cast<MSG *>(message);

    // WM_DEVICECHANGE 代表系統硬體有變動 (插上或拔除)
    if (msg->message == WM_DEVICECHANGE) {
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:        // 1. 有新裝置插上
        case DBT_DEVICEREMOVECOMPLETE: // 2. 有裝置被拔除
            ui->edtText->append("偵測到硬體變更，自動更新 COM Port 清單！");
#if DEBUG_ENABLE
            qDebug() << "偵測到硬體變更，自動更新 COM Port 清單！";
#endif
            serial->scanPorts();       // scan again
            break;
        }
    }
#endif

    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::on_getCOMList(const QStringList &serList)
{
    if(serList.isEmpty())
    {
        ui->cmboBox_COM->clear();
        ui->btn_com->setText("打開");
        return;
    }
    //暫時關閉 ComboBox 的訊號發射，避免它亂觸發 on_selCOM
    //ui->cmboBox_COM->blockSignals(true);

    QString currentSelected = ui->cmboBox_COM->currentText();
    ui->cmboBox_COM->clear();          // 先清空舊的裝置
    ui->cmboBox_COM->addItems(serList); // 一次將所有偵測到的 COM Port 塞入選單
    // 恢復原本選取的項目
    int index = ui->cmboBox_COM->findText(currentSelected);
    if (index != -1) {
        ui->cmboBox_COM->setCurrentIndex(index);
    }

    // 2. UI 更新完畢後，重新打開訊號發射功能
    //ui->cmboBox_COM->blockSignals(false);
}

void MainWindow::on_selCOM(const QString &text)
{
    /*if (text.isEmpty()) {
        return;
    }

    bool err = serial->setCOM(text);

    if(!err) {
        qDebug() << "設置序列埠失敗！";
    }
    else
    {
        qDebug() << "設置序列埠成功！";
    }*/
}

void MainWindow::on_btnCOM()
{
    if(!serial->COM_Is_Open())
    {
        QStringList comInfo;
        comInfo.append(ui->cmboBox_COM->currentText());
        comInfo.append(ui->lEdt_baudrate->text());
        if (comInfo.at(0).isEmpty() || comInfo.at(1).isEmpty()) {
            return;
        }

        bool err = serial->Open_COM(comInfo);

        if(!err) {
            ui->edtText->append("設置序列埠失敗！");
#if DEBUG_ENABLE
            qDebug() << "設置序列埠失敗！";
#endif
        }
        else
        {
            ui->edtText->append("設置序列埠成功！");
            ui->btn_com->setText("關閉");
#if DEBUG_ENABLE
            qDebug() << "設置序列埠成功！";
#endif
        }
    }
    else
    {
        serial->Close_COM();
        ui->btn_com->setText("打開");
    }
}

void MainWindow::on_btnSend()
{
    if(!ui->ledt_send->text().isEmpty())
    {
        QByteArray data;
        data.append(ui->ledt_send->text().toUtf8());
        serial->writeData(data);
    }
}

void MainWindow::Append_Text(const QString &text)
{
    ui->edtText->append(text);
}

void MainWindow::Append_TextList(const QStringList &textList)
{
    for(const QString &text : textList)
        ui->edtText->append(text);
}


MainWindow::~MainWindow()
{
    delete ui;
}
