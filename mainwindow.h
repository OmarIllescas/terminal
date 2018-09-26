#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


signals:
    void senialSendTrama(const QByteArray &data);   //Señal conectada al Slot writeData()
    void senialCalculationCRC();                    //Señal conectada al Slot calculationCRC()

private slots:
    void openSerialPort();
    void closeSerialPort();
    void about();
    void writeData(const QByteArray &data);
    void readData();

    void handleError(QSerialPort::SerialPortError error);

    void on_inputTramaHex_textChanged();
    void setTrama();                        //Slot que se activa con el boton de Enviar
    void sendingPing();                     //Slot que se activa con un TIMER
    void calculationCRC();

private:
    void initActionsConnections();

private:
    void showStatusMessage(const QString &message);

    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;
    Console *m_console = nullptr;
    SettingsDialog *m_settings = nullptr;
    QSerialPort *m_serial = nullptr;

    QByteArray i_TramaByteArray; //Organizacion de la Trama Escrita en la terminal
    QByteArray dataToSerial;     //Trama que se enviará por el puerto Serial
    QByteArray i_TramaPingArray;

    int bytesReceived; //Número de bytes escritos en la terminal
    uint i_Trama[100]; //Trama que se recibio en la terminal
    bool pcOnline;     //Se añadio una PC a la RED
};

#endif // MAINWINDOW_H
