#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QLabel>
#include <QMessageBox>
#include <QtWidgets>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),

/*##############################################################################################*/
/*                                       Creación de Objetos                                    */
/*##############################################################################################*/
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_console(new Console),
    m_settings(new SettingsDialog),

    m_serial(new QSerialPort(this))


/*##############################################################################################*/
/*                                           Constructor                                        */
/*##############################################################################################*/
{
    m_ui->setupUi(this);
    m_console->setEnabled(false);
    setCentralWidget(m_console);

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);
    m_ui->sendTrama->setEnabled(false);

    m_ui->statusBar->addWidget(m_status);

    /*------------------------------------------------------------------------------------------*/
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendingPing()));
    timer->start(1000);

    pcOnline = false;

/*##############################################################################################*/
/*                          Organización de la ventana con Docks                                */
/*##############################################################################################*/
    QDockWidget *dock = new QDockWidget(tr("Outgoing"), this);
    dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dock->setWidget(m_ui->inputTramaHex);
    addDockWidget(Qt::BottomDockWidgetArea, dock);

/*##############################################################################################*/
/*           Configuración de la visualización del QTextEdit inputTramaHex                      */
/*##############################################################################################*/
    m_ui->inputTramaHex->setStyleSheet("background-color: white;"
                                     "border-style: outset; border-width: 1px; "
                                     "border-radius: 10px; border-color: black;");
    m_ui->inputTramaHex->setTextColor(QColor("darkRed"));

/*##############################################################################################*/
/*              Configuración del tipo de letra del QTexEdit inputTramaHex                      */
/*##############################################################################################*/
    //QFont Text_Edit ("Nimbus Mono L",10, QFont::Bold);
    QFont Text_Edit ("Nimbus Mono L",10);
    m_ui->inputTramaHex->setFont(Text_Edit);

 /*##############################################################################################*/
 /*                            Conexión de signals a slots                                       */
/*##############################################################################################*/
    initActionsConnections();

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    connect(m_console, &Console::getData, this, &MainWindow::writeData);
}

/*##############################################################################################*/
/*                                          Destructor                                          */
/*##############################################################################################*/
MainWindow::~MainWindow()
{
    delete m_settings;
    delete m_ui;
}


/*##############################################################################################*/
/*                   Función miembro private: Conexión de signals a slots                       */
/*##############################################################################################*/
void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(m_ui->actionClear, &QAction::triggered, m_console, &Console::clear);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

    connect(m_ui->sendTrama, &QAction::triggered, this, &MainWindow::setTrama);
    connect(this, &MainWindow::senialSendTrama, this, &MainWindow::writeData);
    connect(this, &MainWindow::senialCalculationCRC, this, &MainWindow::calculationCRC);
}


/*##############################################################################################*/
/*                   Función miembro private slot: Apertura del puerto serial                   */
/*##############################################################################################*/
void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_console->setEnabled(true);
        m_console->setLocalEchoEnabled(p.localEchoEnabled);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6") //Create a message that displays
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        showStatusMessage(tr("Open error"));
    }

    m_ui->sendTrama->setEnabled(true);
}

/*##############################################################################################*/
/*                   Función miembro private slot: Cierre del puerto serial                     */
/*##############################################################################################*/
void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    m_console->setEnabled(false);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    m_ui->sendTrama->setEnabled(false);
    showStatusMessage(tr("Disconnected"));
}

/*##############################################################################################*/
/*              Función miembro private slot: Información acerca de la aplicación               */
/*##############################################################################################*/
void MainWindow::about()
{
    QMessageBox::about(this, tr("About Terminal"),
                       tr("<b>Terminal</b>."));
}

/*##############################################################################################*/
/*              Función miembro private slot: Escritura por el por puerto serial                */
/*##############################################################################################*/
void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

/*##############################################################################################*/
/*                   Función miembro private slot: Lectura del por puerto serial                */
/*##############################################################################################*/
void MainWindow::readData()
{
    const QByteArray data = m_serial->readAll();
    m_console->putData(data.toHex(' '));
    m_console->putData("\n");
}

/*##############################################################################################*/
/*                   Función miembro private slot: Error durante la comunicaión                 */
/*##############################################################################################*/
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        closeSerialPort();
    }
}


/*##############################################################################################*/
/*                    Función miembro private: Visualización de un mensaje                      */
/*##############################################################################################*/
void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

/*##############################################################################################*/
/*                           Ingreso de la Trama que se le enviará al Radio                     */
/*##############################################################################################*/
void MainWindow::on_inputTramaHex_textChanged()
{
    m_ui->inputTramaHex->setTextColor(QColor("darkRed"));

    QString trama;
    QString byteTrama;
    QTextCursor cursor_inputTramaHex;
    bool ok;

    cursor_inputTramaHex = QTextCursor();
    trama = m_ui->inputTramaHex->document()->toPlainText();
    cursor_inputTramaHex = m_ui->inputTramaHex->textCursor();

    for (int i = 0; i <= 99; i++)
        i_Trama[i] = 0;

    for (int i = 0; i <= trama.length()-1; i++)
    {
        byteTrama = trama[i];
        i_Trama[i] = byteTrama.toUInt(&ok,16);
    }

    bytesReceived = trama.length()-1;

}

/*##############################################################################################*/
/*                   Organización de la Trama paque se ingreso en la Terminal                   */
/*##############################################################################################*/
void MainWindow::setTrama()
{
    QByteArray i_TramaByteArrayAux;
    int resultOperMod;
    int bytesEven; //Pares
    int bytesOdd;  //Impares

    i_TramaByteArray.clear();

    for(int i = 0; i <= bytesReceived; ++i)
    {

        i_TramaByteArrayAux.append((char)(i_Trama[i]&(0xFF << i) >>i));
        resultOperMod = i%2;

        if (resultOperMod <= 0)
          i_TramaByteArrayAux[i] = i_TramaByteArrayAux[i]<<4;
    }

    for(int i = 0; i <= bytesReceived/2; ++i)
    {
        bytesEven = i*2;
        bytesOdd = ((i+1)*2)-1;
        i_TramaByteArray[i] = i_TramaByteArrayAux[bytesEven] | i_TramaByteArrayAux[bytesOdd];
    }

    emit senialCalculationCRC();

}

void MainWindow::sendingPing()
{
    i_TramaPingArray.clear();

//    i_TramaPingArray[0] = 0xFE;
//    i_TramaPingArray[1] = 0x0B;
//    i_TramaPingArray[2] = 0x85; //Dirección alta de la PC
//    i_TramaPingArray[3] = 0x00; //Dirección baja de la PC
//    i_TramaPingArray[4] = 0x30; //Dirección alta del Radio
//    i_TramaPingArray[5] = 0x72; //Dirección baja del Radio
//    i_TramaPingArray[6] = 0xFF;
//    i_TramaPingArray[7] = 0x00;
//    i_TramaPingArray[8] = 0x00;
//    i_TramaPingArray[9] = 0x85; //ID de la PC
//    i_TramaPingArray[10] = 0x7D; //CRC_H
//    i_TramaPingArray[11] = 0xD3; //CRC_L
//    i_TramaPingArray[12] = 0xEF;

    i_TramaPingArray[0] = 0xFE;
    i_TramaPingArray[1] = 0x0B;
    i_TramaPingArray[2] = 0x86; //Dirección alta de la PC
    i_TramaPingArray[3] = 0x00; //Dirección baja de la PC
    i_TramaPingArray[4] = 0x30; //Dirección alta del Radio
    i_TramaPingArray[5] = 0x71; //Dirección baja del Radio
    i_TramaPingArray[6] = 0xFF;
    i_TramaPingArray[7] = 0x00;
    i_TramaPingArray[8] = 0x00;
    i_TramaPingArray[9] = 0x86; //ID de la PC
    i_TramaPingArray[10] = 0x39; //CRC_H
    i_TramaPingArray[11] = 0xC7; //CRC_L
    i_TramaPingArray[12] = 0xEF;

    if (pcOnline && m_serial->isOpen())
        emit senialSendTrama(i_TramaPingArray);
}

void MainWindow::calculationCRC()
{
    uint tramaCRC[100];
    uint crc = 0xFFFF;

    for (int i = 0; i < 100; i++)
        tramaCRC[i]=0;

    int j = 0;

    for (int i = 0; i < i_TramaByteArray.length(); i++)
    {
        tramaCRC[i] = (i_Trama[j]<<4)+(i_Trama[j+1]);

        crc ^= tramaCRC[i];
        j += 2;

        for (int k = 0; k < 8; k++)
        {    // Loop over each bit
            if ((crc & 0x0001) != 0)          // If the LSB is set
            {
                crc >>= 1;                    // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else
            {// Else LSB is not set
                crc >>= 1;                    // Just shift right
            }
        }
    }
    i_TramaByteArray.append(crc);
    i_TramaByteArray.append(crc>>8);
    i_TramaByteArray.append(0xEF); //Fin de Trama


    dataToSerial.clear();
    dataToSerial.append(0xFE);
    dataToSerial.append(i_TramaByteArray);

    pcOnline = true;
    emit senialSendTrama(dataToSerial);
}
