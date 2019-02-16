#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    stm_available = false;
    stm_port_name = "";
    serial = new QSerialPort;

  /*  qDebug() << "Number of available ports: " << QSerialPortInfo::availablePorts().length();
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        qDebug() << "has Vendor ID: " << serialPortInfo.hasVendorIdentifier();
        if(serialPortInfo.hasVendorIdentifier()){
            qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier();
        }
        qDebug() << "has Product ID: " << serialPortInfo.hasProductIdentifier();
        if(serialPortInfo.hasProductIdentifier()){
            qDebug() << "Product ID: " << serialPortInfo.productIdentifier();
        }
    }*/

    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
            if((serialPortInfo.vendorIdentifier() == stm_vendor_id) &&
                    (serialPortInfo.productIdentifier() == stm_product_id)){
                stm_port_name = serialPortInfo.portName();
                stm_available = true;
                qDebug() << serialPortInfo.portName();
            }
        }
    }

    if(stm_available){
        serial->setPortName("/dev/ttyUSB0");
        serial->open(QIODevice::ReadWrite);
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        serial->write("Selam");
    }
    else{
        QMessageBox::warning(this, "Port error", "Couldn't find the port");
    }

}

MainWindow::~MainWindow()
{
    delete ui;
    serial->close();
}
