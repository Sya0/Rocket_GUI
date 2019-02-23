#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void temp_plot();
    void realtimeData();
    void ReadSerial();    
    void WriteData(const QByteArray &data);
    void pres_plot();
    void TickTimer(int interval);

private:
    Ui::MainWindow *ui;
    QSerialPort *serial = nullptr;
    // Alt kısmı doldur
    static const quint16 stm_vendor_id = 1659;
    static const quint16 stm_product_id = 8963;
    QString stm_port_name;
    bool stm_available;
};

#endif // MAINWINDOW_H
