#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QMessageBox>
#include <QtDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    static int tx_cnt;
    static int rx_cnt;

private:
    Ui::MainWindow *ui;
    QSerialPort *serialport;
    enum SerialPortStatus{open = 1, close = 0};

private:
    void initConnections(void);
    void updateUi(SerialPortStatus status);
    void searchPorts(void);
    void fillPortToCmb(QComboBox *cmb = nullptr);
    void fillBaudToCmb(QComboBox *cmb = nullptr);
    void fillDataBitsToCmb(QComboBox *cmb = nullptr);
    void fillStopBitsToCmb(QComboBox *cmb = nullptr);
    void fillParityToCmb(QComboBox *cmb = nullptr);
    QByteArray convertStringToHex(QString str);

private slots:
    void openPort(void);
    void closePort(void);
    void rxData(void);
    void txData(void);
    void clearCnt(void);
};
#endif // MAINWINDOW_H
