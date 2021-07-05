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

private:
    Ui::MainWindow *ui;
    QSerialPort *serialport;

private:
    void initConnections(void);
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

};
#endif // MAINWINDOW_H
