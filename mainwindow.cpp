#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::temp_plot();    // GUI başlangıcında çizdirmek için constructure fonksiyonunda plot fonksiyonlarını çağır

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
        serial->write("Selam");
    }
    else{
        QMessageBox::warning(this, "Port error", "Couldn't find the port");
    }
}

void MainWindow::temp_plot(){
    ui->temperature_plot->addGraph(); // blue line
    ui->temperature_plot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->temperature_plot->addGraph(); // red line
    ui->temperature_plot->graph(1)->setPen(QPen(QColor(255, 110, 40)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    ui->temperature_plot->xAxis->setTicker(timeTicker);
    ui->temperature_plot->axisRect()->setupFullAxesBox();
    ui->temperature_plot->yAxis->setRange(0, 40);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->temperature_plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->temperature_plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->temperature_plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->temperature_plot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeData:
    QTimer *dataTimer = new QTimer(this);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeData()));
    dataTimer->start(1000); // Interval 0 means to refresh as fast as possible
}

void MainWindow::realtimeData(){
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 1) // at most add point every 1 s
    {
      // add data to lines:
      ui->temperature_plot->graph(0)->addData(key, qSin(key)+qrand()/(double)RAND_MAX*1*qSin(key/0.3843));
      ui->temperature_plot->graph(1)->addData(key, qCos(key)+qrand()/(double)RAND_MAX*0.5*qSin(key/0.4364));
      // rescale value (vertical) axis to fit the current data:
      ui->temperature_plot->graph(0)->rescaleValueAxis();
      ui->temperature_plot->graph(1)->rescaleValueAxis(true);
      lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->temperature_plot->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->temperature_plot->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
      ui->statusBar->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(ui->temperature_plot->graph(0)->data()->size()+ui->temperature_plot->graph(1)->data()->size())
            , 0);
      lastFpsKey = key;
      frameCount = 0;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    serial->close();
}

