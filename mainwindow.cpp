#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    temp_plot();    // add plot func. to constructor to plot graphs at the beginning
    pres_plot();
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeData()));

    stm_available = false;
    stm_port_name = "";
    data = 0;
    temperature = 0;
    pressure = 0;
    altitude = 0;
    latitude = "--.------";
    longtitude = "--.-------";
    serial = new QSerialPort(this);

    /*qDebug() << "Number of available ports: " << QSerialPortInfo::availablePorts().length();
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
            if((serialPortInfo.vendorIdentifier() == stm_vendor_id) && (serialPortInfo.productIdentifier() == stm_product_id)){
                stm_port_name = serialPortInfo.portName();
                stm_available = true;
                qDebug() << "Port name: " << serialPortInfo.portName();
            }
        }
    }

    // Important note: you cannot open port with two different programs. Meaning this, terminate your minicom or
    // whatever you use to try serial port
    if(stm_available){
        qDebug() << "Port opened";
        serial->setPortName(stm_port_name);
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        serial->open(QIODevice::ReadWrite);
        if (!serial->isOpen())
                qDebug() << "Open failed";
        // This signal is emitted once every time new data is available for reading from the device's current read channel
        connect(serial, SIGNAL(readyRead()), this, SLOT(ReadSerial()));
    }
    else{
        QMessageBox::warning(this, "Port error", "Couldn't find the port");
    }
}

void MainWindow::temp_plot()
{
    ui->temperature_plot->addGraph();
    ui->temperature_plot->graph(0)->setPen(QPen(QColor(255, 40, 60))); // red line
    ui->temperature_plot->xAxis->setLabel("Zaman(s)");
    ui->temperature_plot->yAxis->setLabel("Deger");

    ui->temperature_plot->legend->setVisible(true);
    ui->temperature_plot->legend->setBrush(QColor(255, 255, 255, 150));
    ui->temperature_plot->graph(0)->setName("Sicaklik(Celsius)");

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    ui->temperature_plot->xAxis->setTicker(timeTicker);
    ui->temperature_plot->axisRect()->setupFullAxesBox();
    ui->temperature_plot->yAxis->setRange(0, 50);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->temperature_plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->temperature_plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->temperature_plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->temperature_plot->yAxis2, SLOT(setRange(QCPRange)));
}

void MainWindow::pres_plot()
{
    ui->pressure_plot->addGraph();
    ui->pressure_plot->graph(0)->setPen(QPen(QColor(40, 255, 60))); // green line
    ui->pressure_plot->xAxis->setLabel("Zaman(s)");
    ui->pressure_plot->yAxis->setLabel("Deger");
    ui->pressure_plot->addGraph();
    ui->pressure_plot->graph(1)->setPen(QPen(QColor(40, 110, 200))); // blue line

    ui->pressure_plot->legend->setVisible(true);
    ui->pressure_plot->legend->setBrush(QColor(255, 255, 255, 150));
    ui->pressure_plot->graph(0)->setName("Basınc(pa)");
    ui->pressure_plot->graph(1)->setName("Yukseklik(m)");

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    ui->pressure_plot->xAxis->setTicker(timeTicker);
    ui->pressure_plot->axisRect()->setupFullAxesBox();
    ui->pressure_plot->yAxis->setRange(-100, 10000);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->pressure_plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->pressure_plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->pressure_plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->pressure_plot->yAxis2, SLOT(setRange(QCPRange)));
}

void MainWindow::realtimeData()
{
    ui->pressure_lcd->display(pressure);
    ui->temp_lcd->display(temperature);
    ui->altitude_lcd->display(altitude);
    ui->gps_lcd->display(latitude);
    ui->gps_lcd_2->display(longtitude);

    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/500.0; // time elapsed since start of demo, in half seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 1) // at most add point every 1 s
    {
      // add data to lines:
      ui->temperature_plot->graph(0)->addData(key, temperature);
      ui->pressure_plot->graph(0)->addData(key, pressure);
      ui->pressure_plot->graph(1)->addData(key, altitude);
      // rescale value (vertical) axis to fit the current data:
      //ui->temperature_plot->graph(0)->rescaleValueAxis();
      //ui->pressure_plot->graph(0)->rescaleValueAxis();
      //ui->pressure_plot->graph(1)->rescaleValueAxis();
      //ui->pressure_plot->rescaleAxes();
      //ui->pressure_plot->graph(1)->rescaleAxes(true);
      lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 10):
    ui->temperature_plot->xAxis->setRange(key, 15, Qt::AlignRight);
    ui->temperature_plot->replot();
    ui->pressure_plot->xAxis->setRange(key, 15, Qt::AlignRight);
    ui->pressure_plot->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 1.5) // average fps over 1 seconds
    {
      ui->statusBar->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(ui->temperature_plot->graph(0)->data()->size())
            , 0);
      lastFpsKey = key;
      frameCount = 0;
    }
}

void MainWindow::ReadSerial()
{
    data = serial->readAll();
    //qDebug() << data.length();

    if(data.length() > 2) {
        opcode = data[0];
        data_length = data[1];
        if(opcode == 1) {
            if(data.length() == (data_length + 2)) {
                temperature = 0;
                for(int i=0;i<data_length;i++){
                    temperature += ((data[i+2] - 48) * qPow(10,((data_length - 1) - i)));
                }
                qDebug() << "temperaure = " << temperature;
            }
        }
        else if(opcode == 2) {
            if(data.length() == (data_length + 2)) {
                pressure = 0;
                for(int i=0;i<data_length;i++){
                    pressure += ((data[i+2] - 48) * qPow(10,((data_length - 1) - i)));
                }
            }
            qDebug() << "pressure = " << pressure;
        }
        else if(opcode == 3) {
            if(data.length() == (data_length + 2)) {
                altitude = 0;
                for(int i=0;i<data_length;i++){
                    altitude += ((data[i+2] - 48) * qPow(10,((data_length - 1) - i)));
                }
            }
            qDebug() << "altitude" << altitude;
        }
        else if(opcode == 4) {
            if(data.length() == 10){
                latitude = QString::fromStdString(data.toStdString());
                latitude.remove(0,1);
                qDebug() << "latitude" << latitude;
            }
        }
        else if(opcode == 5) {
            if(data.length() == 11){
                longtitude = QString::fromStdString(data.toStdString());
                longtitude.remove(0,1);
                qDebug() << "longitude" << longtitude;
            }
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    dataTimer->start(1000);
}

void MainWindow::on_pushButton_2_clicked()
{
    dataTimer->stop();
}

MainWindow::~MainWindow()
{
    delete ui;
    serial->close();
}
