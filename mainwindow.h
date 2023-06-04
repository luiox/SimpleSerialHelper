#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTime>
#include <QtCharts>

#include <QtCharts/QChartGlobal>

QT_BEGIN_NAMESPACE
class QChart;
class QLineSeries;
class QValueAxis;
class QChartView;
QT_END_NAMESPACE


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

    void on_actionConnect_triggered();

    void on_actionClose_triggered();

    void on_actionSave_triggered();

    void on_actionRefresh_triggered();

    void on_pushButtonClearCounterRecieve_clicked();

    void on_pushButtonClearDataRecieve_clicked();

    void on_pushButtonClearCounterSend_clicked();

    void on_pushButtonClearDataSend_clicked();

    void on_checkBoxPoll_clicked(bool checked);

    void ReadSerialData();

    void CycleSendData();

    void SingleSendData();

private:
    void InitialSetting();

    void SearchSerialPorts();

    void WriteSerialData(int sn);

private:
    Ui::MainWindow *ui;
    QSerialPort *serialPort;
    QTimer *pollTimer;

    int receivedBytes;
    int transmitBytes;
    uint8_t snIndex;
};
#endif // MAINWINDOW_H
