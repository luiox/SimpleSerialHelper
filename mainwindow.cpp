#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serialPort = new QSerialPort;
    pollTimer = new QTimer;

    InitialSetting();

    // 关联按钮信号槽
    for (size_t i = 1; i < 6; ++i) {
        QPushButton* btn = ui->groupBoxMessage->findChild<QPushButton*>(
            QString("pushButtonSend%1").arg(QString::number(i)));
        connect(btn, SIGNAL(clicked()), this, SLOT(SingleSendData()));
    }

    connect(pollTimer,&QTimer::timeout, this, &MainWindow::CycleSendData);

    //setWindowIcon(QIcon(":/images/images/SerialMaster.jpg"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//实现软件参变量的初始化设定
void MainWindow::InitialSetting()
{
    //填充串口号组合框
    SearchSerialPorts();

    //填充串口波特率
    ui->comboBoxBaud->addItem("9600");
    ui->comboBoxBaud->addItem("14400");
    ui->comboBoxBaud->addItem("19200");
    ui->comboBoxBaud->addItem("28800");
    ui->comboBoxBaud->addItem("38400");
    ui->comboBoxBaud->addItem("56000");
    ui->comboBoxBaud->addItem("57600");
    ui->comboBoxBaud->addItem("115200");
    ui->comboBoxBaud->addItem("128000");
    ui->comboBoxBaud->addItem("230400");
    ui->comboBoxBaud->addItem("256000");

    //填充串口数据位
    ui->comboBoxData->addItem("7位");
    ui->comboBoxData->addItem("8位");
    ui->comboBoxData->addItem("9位");
    ui->comboBoxData->setCurrentIndex(1);

    //填充串口校验位
    ui->comboBoxParity->addItem("无校验");
    ui->comboBoxParity->addItem("奇校验");
    ui->comboBoxParity->addItem("偶校验");

    //填充串口停止位
    ui->comboBoxStop->addItem("1位");
    ui->comboBoxStop->addItem("1.5位");
    ui->comboBoxStop->addItem("2位");

    //设置界面操作的初始状态
    ui->spinBoxInterval->setValue(1000);
    ui->checkBoxSend->setChecked(true);
    ui->checkBoxRecieve->setChecked(true);

    //为变量赋初值
    receivedBytes=0;
    transmitBytes=0;
    snIndex=1;
}

//搜索可用的串口，并添加到串口组合框
void MainWindow::SearchSerialPorts()
{
    ui->comboBoxPort->clear();

    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        ui->comboBoxPort->addItem(info.portName());
    }
}

//从串口接收数据
void MainWindow::ReadSerialData()
{
    QByteArray rxDatas;
    QString context;

    rxDatas=serialPort->readAll();

    if(!rxDatas.isNull()) {
        if(ui->checkBoxRecieve->isChecked()) {   //十六进制显示
             context = rxDatas.toHex(' ');
             context = context.toUpper();
        }
        else {   //ASCII显示
            context = rxDatas;
        }

        QString timeStrLine="["+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"][接收]: ";
        context = timeStrLine+context+"\n\r";

        QString content = "<span style=\" color:blue;\">"+context+"</span>";
        ui->textBrowser->append(content);

        receivedBytes=receivedBytes+rxDatas.size();
        ui->lcdNumberRecieve->display(receivedBytes);

        ui->statusbar->showMessage(tr("成功读取%1字节数据").arg(rxDatas.size()));
    }

    rxDatas.clear();
}

//定时周期发送
void MainWindow::CycleSendData()
{
    QCheckBox* cbSend;

    while(true)
    {
        snIndex=snIndex>=6?1:snIndex;

        cbSend=ui->groupBoxMessage->findChild<QCheckBox*>(QString("checkBoxSendEnable%1").arg(QString::number(snIndex)));

        if(cbSend->isChecked())
        {
            WriteSerialData(snIndex);
            snIndex++;
            break;
        }

        snIndex++;
    }
}

//按钮触发发送
void MainWindow::SingleSendData()
{
    // 判断如果Sender是QPushButton就执行
    if (QPushButton* btn = dynamic_cast<QPushButton*>(sender()))
    {
        QString senderName;
        int sn=0;

        senderName = btn->objectName();
        sn = senderName.replace("pushButtonSend", "").toInt();

        if((0<sn) && (sn<6))
        {
            WriteSerialData(sn);
        }
    }
}

//向串口发送数据
void MainWindow::WriteSerialData(int sn)
{
    QByteArray buffer;
    QString msg;
    QLineEdit *lineEdit;
    QString text;

    lineEdit= ui->groupBoxMessage->findChild<QLineEdit*>(QString("lineEditSend%1").arg(QString::number(sn)));
    text=lineEdit->text();

    if(text.isNull()||text.isEmpty())
    {
        ui->statusbar->showMessage(tr("消息%1内容为空！").arg(sn));
        return;
    }

    if(ui->checkBoxSend->isChecked())   //按十六进制发送
    {
        bool ok;
        char data;
        QStringList list;
        if ( text.indexOf(" ") > 0)
        {
            list = text.split(" ");
            for (int i = 0; i < list.count(); i++)
            {
                if (list.at(i) == " ")
                    continue;
                if (list.at(i).isEmpty())
                    continue;
                data = (char)list.at(i).toInt(&ok, 16);
                if (!ok)
                {
                    QMessageBox::information(this, tr("提示消息"), tr("输入的数据格式有错误！"), QMessageBox::Ok);

                    return;
                }
                buffer.append(data);
            }
             msg = text.toUpper();
        }
        else
        {
            buffer = QByteArray::fromHex(text.toLatin1());
            QByteArray tmp, src;
            src = text.toLatin1();
            for (int i = 0; i < src.size(); i++ )
            {
                tmp.append(src.at(i));
                if (i % 2) tmp.append(0x20);
            }
            msg = tmp;
        }
    }
    else    //按ASCII字符发送
    {
#if QT_VERSION < 0x050000
        buffer = text.toAscii();
#else
        buffer = text.toLocal8Bit();
#endif
        msg = buffer;
    }

    //发送数据
    if (serialPort && serialPort->isOpen() && serialPort->isWritable())
    {
        qDebug()<<sn<<"**************************buf : "<<buffer.toHex();
        QString timeStrLine="["+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"][发送]: ";
        QString content = "<span style=\" color:red;\">"+timeStrLine +msg+"\n\r</span>";
        ui->textBrowser->append(content);

        serialPort->write(buffer);
        ui->statusbar->showMessage(tr("发送数据成功"));

        //界面控制
        ui->textBrowser->setTextColor(Qt::red);
        transmitBytes +=buffer.size();
        ui->lcdNumberSend->display(transmitBytes);
    }
    else
    {
        if (QMessageBox::Ok ==
            QMessageBox::question(this, tr("提示"),
                QString(tr("串口 %1 并没有打开, 是否要打开串口 %2 ?"))
                .arg(ui->comboBoxPort->currentText(), ui->comboBoxPort->currentText()),
                QMessageBox::Ok, QMessageBox::Cancel))
        {
            on_actionConnect_triggered();
        }
    }
}



//退出软件
void MainWindow::on_actionExit_triggered()
{
    this->close();
}

//打开串口
void MainWindow::on_actionConnect_triggered()
{
    serialPort->setPortName(ui->comboBoxPort->currentText());

    if(serialPort->open(QIODevice::ReadWrite))              //打开串口成功
    {
        serialPort->setBaudRate(ui->comboBoxBaud->currentText().toInt());       //设置波特率

        switch(ui->comboBoxData->currentIndex())                   //设置数据位数
        {
            case 1:serialPort->setDataBits(QSerialPort::Data8);break;
            default: break;
        }

        switch(ui->comboBoxParity->currentIndex())                   //设置奇偶校验
        {
            case 0: serialPort->setParity(QSerialPort::NoParity);break;
            default: break;
        }

        switch(ui->comboBoxStop->currentIndex())                     //设置停止位
        {
            case 1: serialPort->setStopBits(QSerialPort::OneStop);break;
            case 2: serialPort->setStopBits(QSerialPort::TwoStop);break;
            default: break;
        }

        serialPort->setFlowControl(QSerialPort::NoFlowControl);     //设置流控制

        //连接槽函数
        QObject::connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::ReadSerialData);

        // 设置控件可否使用
        ui->actionConnect->setEnabled(false);
        ui->actionClose->setEnabled(true);
        ui->actionRefresh->setEnabled(false);
    }
    else    //打开失败提示
    {

        QMessageBox::information(this,tr("错误"),tr("打开串口失败！"),QMessageBox::Ok);
    }
}

//关闭串口
void MainWindow::on_actionClose_triggered()
{
    serialPort->clear();
    serialPort->close();

    // 设置控件可否使用
    ui->actionConnect->setEnabled(true);
    ui->actionClose->setEnabled(false);
    ui->actionRefresh->setEnabled(true);
}

//将接收窗口的消息保存到文件
void MainWindow::on_actionSave_triggered()
{
    if(ui->textBrowser->toPlainText().isEmpty()){
        QMessageBox::information(this, "提示消息", tr("貌似还没有数据! 您需要在发送编辑框中输入要发送的数据"), QMessageBox::Ok);
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, tr("保存为"), tr("未命名.txt"));
    QFile file(filename);
    //如果用户取消了保存则直接退出函数
    if(file.fileName().isEmpty()){
        return;
    }

    //如果打开失败则给出提示并退出函数
    if(!file.open(QFile::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(this, tr("保存文件"), tr("打开文件 %1 失败, 无法保存\n%2").arg(filename).arg(file.errorString()), QMessageBox::Ok);
        return;
    }
    //写数据到文件
    QTextStream out(&file);
    out<<ui->textBrowser->toPlainText();
    file.close();
}

//刷新串口列表
void MainWindow::on_actionRefresh_triggered()
{
    //填充串口号组合框
    SearchSerialPorts();
}

//清除发送字节计数
void MainWindow::on_pushButtonClearCounterRecieve_clicked()
{
    transmitBytes=0;
    ui->lcdNumberSend->display(transmitBytes);
}

//清除接收窗口的数据
void MainWindow::on_pushButtonClearDataRecieve_clicked()
{
    ui->textBrowser->clear();
}

//清除发送字节计数
void MainWindow::on_pushButtonClearCounterSend_clicked()
{
    receivedBytes=0;
    ui->lcdNumberRecieve->display(receivedBytes);
}

//清除发送窗口数据
void MainWindow::on_pushButtonClearDataSend_clicked()
{
    QLineEdit *lineEdit;

    for(int i=1;i<6;i++)
    {
        lineEdit= ui->groupBoxMessage->findChild<QLineEdit*>(QString("lineEditSend%1").arg(QString::number(i)));
        lineEdit->clear();
    }
}

//自动发送改变
void MainWindow::on_checkBoxPoll_clicked(bool checked)
{
    if(checked)     //自动循环发送
    {
        if (serialPort && serialPort->isOpen() && serialPort->isWritable())
        {
            pollTimer->setInterval(ui->spinBoxInterval->text().toInt());
            pollTimer->start();

            ui->statusbar->showMessage(tr("启用循环发送"));
        }
        else
        {
            if (QMessageBox::Ok ==
                QMessageBox::question(this, tr("提示"),
                    QString(tr("串口 %1 并没有打开, 是否要打开串口 %2 ?"))
                    .arg(ui->comboBoxPort->currentText(), ui->comboBoxPort->currentText()),
                    QMessageBox::Ok, QMessageBox::Cancel))
            {
                on_actionConnect_triggered();
            }
            ui->checkBoxPoll->setChecked(false);
        }
    }
    else            //取消循环发送
    {
        pollTimer->stop();

        ui->statusbar->showMessage(tr("取消循环发送"));
    }
}
