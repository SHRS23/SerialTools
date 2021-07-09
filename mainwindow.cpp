#include "mainwindow.h"
#include "ui_mainwindow.h"

int MainWindow::tx_cnt = 0;
int MainWindow::rx_cnt = 0;

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
    fillDataBitsToCmb(ui->cmb_databits);
    fillStopBitsToCmb(ui->cmb_stopbits);
    fillParityToCmb(ui->cmb_parity);
    ui->txt_rx->setReadOnly(true);
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
    connect(ui->btn_clr_rx, &QPushButton::clicked, ui->txt_rx, &QPlainTextEdit::clear);
    connect(ui->btn_clr_tx, &QPushButton::clicked, ui->txt_tx, &QPlainTextEdit::clear);
    connect(ui->btn_clr_cnt, &QPushButton::clicked, this, &MainWindow::clearCnt);
}

void MainWindow::clearCnt(void)
{
    ui->lab_rx_cnt->setText("Rx : 0 Bytes");
    ui->lab_tx_cnt->setText("Tx : 0 Bytes");
    tx_cnt = 0;
    rx_cnt = 0;
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
    cmb->addItem(QStringLiteral("1200"), QSerialPort::Baud1200);
    cmb->addItem(QStringLiteral("2400"), QSerialPort::Baud2400);
    cmb->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    cmb->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    cmb->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    cmb->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    cmb->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    cmb->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    cmb->setCurrentIndex(7);
}

void MainWindow::fillDataBitsToCmb(QComboBox *cmb)
{
    cmb->addItem(QStringLiteral("5"), QSerialPort::Data5);
    cmb->addItem(QStringLiteral("6"), QSerialPort::Data6);
    cmb->addItem(QStringLiteral("7"), QSerialPort::Data7);
    cmb->addItem(QStringLiteral("8"), QSerialPort::Data8);
    cmb->setCurrentIndex(3);
}
void MainWindow::fillStopBitsToCmb(QComboBox *cmb)
{
    cmb->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    // From Qt Help : This is only for the Windows platform.
    cmb->addItem("1.5", QSerialPort::OneAndHalfStop);
#endif
    cmb->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    cmb->setCurrentIndex(0);
}
void MainWindow::fillParityToCmb(QComboBox *cmb)
{
    cmb->addItem("None", QSerialPort::NoParity);
    cmb->addItem("Even", QSerialPort::EvenParity);
    cmb->addItem("Odd", QSerialPort::OddParity);
    cmb->addItem("Mark", QSerialPort::MarkParity);
    cmb->addItem("Space", QSerialPort::SpaceParity);
    cmb->setCurrentIndex(0);
}

void MainWindow::openPort()
{
    /* get port setting */
    serialport->setPortName(ui->cmb_port->currentText());
    serialport->setBaudRate(ui->cmb_baud->currentText().toInt());
    serialport->setDataBits(static_cast<QSerialPort::DataBits>(ui->cmb_databits->itemData(ui->cmb_databits->currentIndex()).toInt()));
    serialport->setStopBits(static_cast<QSerialPort::StopBits>(ui->cmb_stopbits->itemData(ui->cmb_stopbits->currentIndex()).toInt()));
    serialport->setParity(static_cast<QSerialPort::Parity>(ui->cmb_parity->itemData(ui->cmb_parity->currentIndex()).toInt()));
    serialport->setFlowControl(QSerialPort::NoFlowControl);

    /* check port setting */
    if(serialport->portName().isEmpty())
    {
        qDebug("No ports avilable.");
        QMessageBox::critical(this, "Error", "No ports available!");
        return;
    }

    /* open serial port */
    if(serialport->open(QIODevice::ReadWrite))
    {
        /* connect serial port signal and slot */
        connect(serialport, &QSerialPort::readyRead, this, &MainWindow::rxData);
        /* update ui */
        updateUi(open);
        qDebug("open success");
    }
    else
    {
        qDebug("port cannot open");
        QMessageBox::critical(this, "Error", serialport->errorString());
        return;
    }
}

void MainWindow::closePort()
{
    if(serialport->isOpen())
    {
        serialport->close();
        /* update ui */
        updateUi(close);
        qDebug("close success");
    }
}

void MainWindow::rxData()
{
    const QByteArray data = serialport->readAll();

    qDebug("rx data length : %d", data.length());
    if(ui->ckb_hex_rx->isChecked())
    {
        ui->txt_rx->insertPlainText(QString(data.toHex(' ').toUpper() + ' '));
    }
    else
    {
        // ui->txt_rx->appendPlainText(QString(data)); // will start a newline
        ui->txt_rx->insertPlainText(QString(data));
    }

    /* update rx count */
    rx_cnt += data.length();
    /* update rx count lable */
    ui->lab_rx_cnt->setText("Rx : " + QString::number(rx_cnt) + " Bytes");
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

    /* update tx count */
    tx_cnt += data.length();

    if(ui->ckb_newline->isChecked())
    {
        data.append(0x0D);
        data.append(0x0A);
        tx_cnt += 2;
    }

    qDebug("tx data length : %d", data.length());
    /* update tx count lable */
    ui->lab_tx_cnt->setText("Tx : " + QString::number(tx_cnt) + " Bytes");
    /* write data to serial port */
    serialport->write(data);
}

void MainWindow::updateUi(SerialPortStatus status)
{
    if(status == close)
    {
        ui->cmb_port->setEnabled(true);
        ui->cmb_baud->setEnabled(true);
        ui->cmb_databits->setEnabled(true);
        ui->cmb_stopbits->setEnabled(true);
        ui->cmb_parity->setEnabled(true);
        ui->btn_send->setEnabled(false);
        ui->btn_open->setEnabled(true);
        ui->btn_close->setEnabled(false);
        ui->btn_search->setEnabled(true);
        ui->lab_status->setText(serialport->portName() + " is closed.");
    }
    else if(status == open)
    {
        ui->cmb_port->setEnabled(false);
        ui->cmb_baud->setEnabled(false);
        ui->cmb_databits->setEnabled(false);
        ui->cmb_stopbits->setEnabled(false);
        ui->cmb_parity->setEnabled(false);
        ui->btn_send->setEnabled(true);
        ui->btn_open->setEnabled(false);
        ui->btn_close->setEnabled(true);
        ui->btn_search->setEnabled(false);
        ui->lab_status->setText(serialport->portName() + " is opened.");
    }
    else
    {
        qDebug("Wrong serial status");
        ui->lab_status->setText("Wrong");
        return;
    }
}
