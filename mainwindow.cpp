#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>

QSerialPort static serial;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serial.setPortName("/dev/ttyUSB0");
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    serial.open(QIODevice::ReadWrite);
    serial.write("Selam");
}

MainWindow::~MainWindow()
{
    delete ui;
    serial.close();
}
