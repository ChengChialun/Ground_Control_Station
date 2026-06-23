#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSpacerItem>
#include <QStatusBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QWidget>
#include <QWidget>
#include <vector>
#include <QThread>

class SerialManager;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    //QTextEdit* getTextEdit() const { return ui->edtText_UART; }
    ~MainWindow() override;
    void Append_Text(const QString &text);
    void Append_TextList(const QStringList &textList);

private:
    Ui::MainWindow *ui;
    SerialManager *serial;
    void serial_manager(MainWindow *mWin, QWidget *parent = nullptr);

private slots:
    //void on_cmboBox_COM_Clicked();
    void on_getCOMList(const QStringList &serList);
    void on_selCOM(const QString &text);
    void on_btnCOM();
    void on_btnSend();
protected:
    // override Windows 底層事件攔截函式
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
};
#endif // MAAINWINDOW_H
