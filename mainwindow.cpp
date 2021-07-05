#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    serialport = new QSerialPort;

    ui->setupUi(this);
    setFixedSize(this->width(), this->height());
    setWindowTitle("Serial Tools");
    initConnections();
    searchPorts();
    fillBaudToCmb(ui->cmb_baud);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initConnections()
{
    connect(ui->btn_search, &QPushButton::clicked, this, &MainWindow::searchPorts);
    connect(ui->btn_open, &QPushButton::clicked, this, &MainWindow::openPort);
    connect(ui->btn_close, &QPushButton::clicked, this, &MainWindow::closePort);
    connect(ui->btn_send, &QPushButton::clicked, this, &MainWindow::txData);
    connect(serialport, &QSerialPort::readyRead, this, &MainWindow::rxData);
    connect(ui->btn_clr_rx, &QPushButton::clicked, ui->txt_rx, &QPlainTextEdit::clear);
    connect(ui->btn_clr_tx, &QPushButton::clicked, ui->txt_tx, &QPlainTextEdit::clear);

}

void MainWindow::searchPorts(void)
{
    fillPortToCmb(ui->cmb_port);
}

void MainWindow::fillPortToCmb(QComboBox *cmb)
{
    cmb->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
    {
        cmb->addItem(info.portName());
        // code below will freeze UI for a while
        /***
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            cmb->addItem(serial.portName());
            serial.close();
        }
        ***/
    }
}

void MainWindow::fillBaudToCmb(QComboBox *cmb)
{
    cmb->clear();
    cmb->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    cmb->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    cmb->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    cmb->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    cmb->setCurrentText(QStringLiteral("115200"));
}

void MainWindow::openPort()
{
    /* get port setting */
    serialport->setPortName(ui->cmb_port->currentText());
    serialport->setBaudRate(ui->cmb_baud->currentText().toInt());
    serialport->setDataBits(QSerialPort::Data8);
    serialport->setStopBits(QSerialPort::OneStop);
    serialport->setParity(QSerialPort::NoParity);
    serialport->setFlowControl(QSerialPort::NoFlowControl);

    /* check port setting */
    if(serialport->portName().isEmpty())
    {
        qDebug("No ports avilable.");
        QMessageBox::critical(this, tr("Error"), "No ports available!");
        return;
    }

    /* open serial port */
    if(serialport->open(QIODevice::ReadWrite))
    {
        /* update ui */
        ui->cmb_port->setEnabled(false);
        ui->cmb_baud->setEnabled(false);
        ui->btn_send->setEnabled(true);
        qDebug("open success");
    }
    else
    {
        qDebug("port cannot open");
        QMessageBox::critical(this, tr("Error"), serialport->errorString());
        return;
    }
}

void MainWindow::closePort()
{
    if(serialport->isOpen())
    {
        serialport->close();
        /* update ui */
        ui->cmb_port->setEnabled(true);
        ui->cmb_baud->setEnabled(true);
        ui->btn_send->setEnabled(false);
    }
}

void MainWindow::rxData()
{
    const QByteArray data = serialport->readAll();

    if(ui->ckb_hex_rx->isChecked())
    {
        ui->txt_rx->insertPlainText(QString(data.toHex(' ').toUpper() + ' '));
    }
    else
    {
        // ui->txt_rx->appendPlainText(QString(data)); // will start a newline
        ui->txt_rx->insertPlainText(QString(data));
    }
}

QByteArray MainWindow::convertStringToHex(QString str)
{
    QByteArray data = QByteArray();

    /* Split string with ' ' or '\n' or '\a' or '\t' */
    QRegExp rx("(\\ |\\n|\\r|\\t)");
    QStringList strList = str.split(rx);

    data.clear();
    bool ok;

    for(QString &temp : strList)
        {
            for(int i=0;i<temp.length()/2;i++)
            {
                data += temp.midRef(i * 2, 2).toInt(&ok, 16);
            }
            if(temp.length() % 2 != 0)
            {
                data += temp.midRef(temp.length()-1, 1).toInt(&ok, 16);
            }
        }
    return data;
}

void MainWindow::txData()
{
    QString str = ui->txt_tx->toPlainText();
    QByteArray data = QByteArray();

    if(ui->ckb_hex_tx->isChecked())
    {
        data = convertStringToHex(str);
    }
    else
    {
        data = str.toUtf8();
    }

    if(ui->ckb_newline->isChecked())
    {
        data.append(0x0D);
        data.append(0x0A);
    }

    serialport->write(data);
}
