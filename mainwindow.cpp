#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->temp_lcd->display("23");
    ui->pressure_lcd->display("1025");
    ui->altitude_lcd->display("3");
    temp_plot();    // add plot func. to constructor to plot graphs at the beginning
    pres_plot();
    TickTimer(100);

    stm_available = false;
    stm_port_name = "";
    serial = new QSerialPort(this);

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
            }
        }
    }

    if(stm_available){
        serial->setPortName(stm_port_name);
        serial->open(QIODevice::ReadWrite);
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        //serial->write("Selam");
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
    ui->temperature_plot->xAxis->setLabel("Time");
    ui->temperature_plot->yAxis->setLabel("Value");

    ui->temperature_plot->legend->setVisible(true);
    ui->temperature_plot->legend->setBrush(QColor(255, 255, 255, 150));
    ui->temperature_plot->graph(0)->setName("Temperature");

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    ui->temperature_plot->xAxis->setTicker(timeTicker);
    ui->temperature_plot->axisRect()->setupFullAxesBox();
    ui->temperature_plot->yAxis->setRange(0, 40);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->temperature_plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->temperature_plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->temperature_plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->temperature_plot->yAxis2, SLOT(setRange(QCPRange)));
}

void MainWindow::pres_plot()
{
    ui->pressure_plot->addGraph();
    ui->pressure_plot->graph(0)->setPen(QPen(QColor(40, 255, 60))); // green line
    ui->pressure_plot->xAxis->setLabel("Time");
    ui->pressure_plot->yAxis->setLabel("Value");
    ui->pressure_plot->addGraph();
    ui->pressure_plot->graph(1)->setPen(QPen(QColor(40, 110, 200))); // blue line

    ui->pressure_plot->legend->setVisible(true);
    ui->pressure_plot->legend->setBrush(QColor(255, 255, 255, 150));
    ui->pressure_plot->graph(0)->setName("Pressure");
    ui->pressure_plot->graph(1)->setName("Altitude");

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    ui->pressure_plot->xAxis->setTicker(timeTicker);
    ui->pressure_plot->axisRect()->setupFullAxesBox();
    ui->pressure_plot->yAxis->setRange(0, 40);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->pressure_plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->pressure_plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->pressure_plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->pressure_plot->yAxis2, SLOT(setRange(QCPRange)));
}

void MainWindow::realtimeData()
{
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/500.0; // time elapsed since start of demo, in half seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 1) // at most add point every 1 s
    {
      // add data to lines:
      ui->temperature_plot->graph(0)->addData(key, qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
      ui->pressure_plot->graph(0)->addData(key, (qrand()%40));
      ui->pressure_plot->graph(1)->addData(key, (qrand()%40));
      // rescale value (vertical) axis to fit the current data:
      ui->temperature_plot->graph(0)->rescaleValueAxis();
      ui->pressure_plot->graph(0)->rescaleValueAxis();
      ui->pressure_plot->graph(1)->rescaleValueAxis();
      lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 10):
    ui->temperature_plot->xAxis->setRange(key, 10, Qt::AlignRight);
    ui->temperature_plot->replot();
    ui->pressure_plot->xAxis->setRange(key, 10, Qt::AlignRight);
    ui->pressure_plot->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 1) // average fps over 1 seconds
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
    QByteArray data = serial->readAll();
    qDebug() << data;

    //return data;
}

void MainWindow::WriteData(const QByteArray &data)
{
    serial->write(data);
}

void MainWindow::TickTimer(int interval){
    // setup a timer that repeatedly calls MainWindow::realtimeData:
    QTimer *dataTimer = new QTimer(this);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeData()));
    dataTimer->start(interval); // Interval 0 means to refresh as fast as possible
}

MainWindow::~MainWindow()
{
    delete ui;
    serial->close();
}
