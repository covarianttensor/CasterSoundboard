#ifndef CASTEROSCSERVERCONFIGPICKER_H
#define CASTEROSCSERVERCONFIGPICKER_H
#include <QDialog>

//forward declarations
class QWidget;
class QSlider;
class QLineEdit;
class QPushButton;
class QLabel;
class QString;
class QVBoxLayout;
class QHBoxLayout;
class QHostAddress;



class CasterOSCServerConfigPicker : public QDialog
{
    Q_OBJECT
public:
    // Static
    static const int Action_Neutral = 0;
    static const int Action_Start_Server = 1;
    static const int Action_Kill_Server = 2;

    //Constructor
    CasterOSCServerConfigPicker(QString _inbound_ipv4, int _inbound_port, QString _outbound_ipv4, int _outbound_port);


    //PROPERTIES
    bool ok;//DiagResult
    int Result = 0;

    //METHODS
    //Inbound
    void setInBoundIPv4(QString *ip);
    void setInBoundPort(int port);
    int getInboundPort();

    //Outbound
    void setOutBoundIPv4(QString *ip);
    QString getOutboundIPv4();
    void setOutBoundPort(int port);
    int getOutboundPort();

    //MISC
    void setIsServerRunning(bool isRunning);
    bool getIsServerRunning();

    //WIDGETS


protected:
    //PROPERTIES

    //METHODS
    void closeEvent(QCloseEvent *event);

    //WIDGETS

private:
    //PROPERTIES
    bool server_is_running = false;
    QString *inbound_ipv4;
    int inbound_port = 8000;
    QString *outbound_ipv4;
    int outbound_port = 9000;

    //METHODS

    //WIDGETS
    QLabel *label_General;
    QLabel *label_Inbound;
    QLabel *label_Outbound;
    QLineEdit *textBox_Inbound_IPv4;
    QLineEdit *textBox_Inbound_Port;
    QLineEdit *textBox_Outbound_IPv4;
    QLineEdit *textBox_Outbound_Port;
    QPushButton *button_Cancel;
    QPushButton *button_ToggleServer;

    QVBoxLayout *layout_main;
    QHBoxLayout *layout_general_info;
    QHBoxLayout *layout_inbound_info;
    QHBoxLayout *layout_inbound_controls;
    QHBoxLayout *layout_outbound_info;
    QHBoxLayout *layout_outbound_controls;
    QHBoxLayout *layout_buttons;

signals:
    //SIGNALS

public slots:
    //SLOTS
    void button_ToggleServer_Clicked();
    void button_Cancel_Clicked();

};

#endif // CASTEROSCSERVERCONFIGPICKER_H
