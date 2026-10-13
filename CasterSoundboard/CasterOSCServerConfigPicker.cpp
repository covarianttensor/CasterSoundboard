#include "CasterOSCServerConfigPicker.h"
#include <QMediaPlayer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QHostAddress>


CasterOSCServerConfigPicker::CasterOSCServerConfigPicker(QString _inbound_ipv4, int _inbound_port, QString _outbound_ipv4, int _outbound_port, QWidget *parent) : QDialog(parent)
{
    // Main Settings
    this->setMinimumWidth(400);
    this->setWindowTitle("Open Sound Control Server Settings");
    ok = false;
    this->inbound_ipv4 = new QString(_inbound_ipv4);
    this->inbound_port = _inbound_port;
    this->outbound_ipv4 = new QString(_outbound_ipv4);
    this->outbound_port = _outbound_port;

    // Validators
    // IPv4 Validator
    QRegularExpression rx_ip("^((1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})\\.){3,3}(1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})$");
    QRegularExpressionValidator *validator_ip = new QRegularExpressionValidator(rx_ip, this);
    // Port Number Validator
    QRegularExpression rx_port("^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
    QRegularExpressionValidator *validator_port = new QRegularExpressionValidator(rx_port, this);

    // UI
    label_General = new QLabel("Open Sound Control server with one-way and two-way communication.\nTouchOSC app is recommended for two-way communication.\n(NOT AFFILIATED with TouchOSC).\n OSC api will be documented in the repo's wiki.");
    label_Inbound = new QLabel("Inbound Traffic:");
    label_Outbound = new QLabel("Outbound Traffic:");
    textBox_Inbound_IPv4 = new QLineEdit;
    textBox_Inbound_IPv4->setText(inbound_ipv4->toUtf8());
    textBox_Inbound_IPv4->setPlaceholderText("IP Address");
    textBox_Inbound_IPv4->setReadOnly(true);
    textBox_Inbound_Port = new QLineEdit;
    textBox_Inbound_Port->setText(QString::number(inbound_port).toUtf8());
    textBox_Inbound_Port->setPlaceholderText("Port");
    textBox_Inbound_Port->setValidator(validator_port);
    textBox_Outbound_IPv4 = new QLineEdit;
    textBox_Outbound_IPv4->setText(outbound_ipv4->toUtf8());
    textBox_Outbound_IPv4->setPlaceholderText("OSC Client IP Address");
    textBox_Outbound_IPv4->setValidator(validator_ip);
    textBox_Outbound_Port = new QLineEdit;
    textBox_Outbound_Port->setText(QString::number(outbound_port).toUtf8());
    textBox_Outbound_Port->setPlaceholderText("Port");
    textBox_Outbound_Port->setValidator(validator_port);
    button_Cancel = new QPushButton("Cancel");
    button_ToggleServer = new QPushButton("Start");


    // Layouts
    layout_main = new QVBoxLayout(this);
    layout_general_info = new QHBoxLayout;
    layout_inbound_info = new QHBoxLayout;
    layout_inbound_controls = new QHBoxLayout;
    layout_outbound_info = new QHBoxLayout;
    layout_outbound_controls = new QHBoxLayout;
    layout_buttons = new QHBoxLayout;

    // Set Layouts
    layout_general_info->addWidget(label_General);
    layout_inbound_info->addWidget(label_Inbound);
    layout_inbound_controls->addWidget(textBox_Inbound_IPv4);
    layout_inbound_controls->addWidget(textBox_Inbound_Port);
    layout_outbound_info->addWidget(label_Outbound);
    layout_outbound_controls->addWidget(textBox_Outbound_IPv4);
    layout_outbound_controls->addWidget(textBox_Outbound_Port);
    layout_buttons->addWidget(button_Cancel);
    layout_buttons->addWidget(button_ToggleServer);

    // Set Overall Layout
    layout_main->addLayout(layout_general_info);
    layout_main->addLayout(layout_inbound_info);
    layout_main->addLayout(layout_inbound_controls);
    layout_main->addLayout(layout_outbound_info);
    layout_main->addLayout(layout_outbound_controls);
    layout_main->addLayout(layout_buttons);

    //Connect Sub-Widget Events
    connect(button_Cancel,SIGNAL(clicked()),this,SLOT(button_Cancel_Clicked()));
    connect(button_ToggleServer,SIGNAL(clicked()),this,SLOT(button_ToggleServer_Clicked()));
}

CasterOSCServerConfigPicker::~CasterOSCServerConfigPicker()
{
    delete inbound_ipv4;
    delete outbound_ipv4;
}

//=======Protected=======
void CasterOSCServerConfigPicker::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
}

//======Public Slots======

//SLOTS
void CasterOSCServerConfigPicker::button_ToggleServer_Clicked(){
    if(server_is_running){
        ok = true;
        Result = CasterOSCServerConfigPicker::Action_Kill_Server;
        this->close();
    } else {
        if(!textBox_Inbound_Port->text().isEmpty()
                && !textBox_Outbound_IPv4->text().isEmpty()
                && !textBox_Outbound_Port->text().isEmpty()){
            inbound_port = textBox_Inbound_Port->text().toInt();
            *outbound_ipv4 = textBox_Outbound_IPv4->text();
            outbound_port = textBox_Outbound_Port->text().toInt();
            ok = true;
            Result = CasterOSCServerConfigPicker::Action_Start_Server;
            this->close();
        } else {
            QMessageBox msgBox;
            msgBox.setText("IP and Port numbers can not be empty!\n");
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.setDefaultButton(QMessageBox::Close);
            msgBox.setModal(true);
            msgBox.setWindowTitle("Input Error");
            msgBox.setWindowIcon(QIcon(":/res/img/about.png"));
            msgBox.exec();
        }
    }
}

void CasterOSCServerConfigPicker::button_Cancel_Clicked(){
    ok = false;
    Result = CasterOSCServerConfigPicker::Action_Neutral;
    this->close();
}


//======Public Properties======
//Inbound
void CasterOSCServerConfigPicker::setInBoundIPv4(QString *ip){
    *inbound_ipv4 = *ip;
    textBox_Inbound_IPv4->setText(*inbound_ipv4);
}

void CasterOSCServerConfigPicker::setInBoundPort(int port){
    inbound_port = port;
    textBox_Inbound_Port->setText(QString::number(inbound_port).toUtf8());
}

int CasterOSCServerConfigPicker::getInboundPort(){
    return inbound_port;
}

//Outbound
void CasterOSCServerConfigPicker::setOutBoundIPv4(QString *ip){
    *outbound_ipv4 = *ip;
    textBox_Outbound_IPv4->setText(*outbound_ipv4);
}

QString CasterOSCServerConfigPicker::getOutboundIPv4(){
    return *outbound_ipv4;
}

void CasterOSCServerConfigPicker::setOutBoundPort(int port){
    outbound_port = port;
    textBox_Outbound_Port->setText(QString::number(outbound_port).toUtf8());
}

int CasterOSCServerConfigPicker::getOutboundPort(){
    return outbound_port;
}

// MISC

void CasterOSCServerConfigPicker::setIsServerRunning(bool isRunning)
{\
    this->server_is_running = isRunning;
    if(server_is_running){
        textBox_Inbound_Port->setReadOnly(true);
        textBox_Outbound_IPv4->setReadOnly(true);
        textBox_Outbound_Port->setReadOnly(true);
        button_ToggleServer->setText("Stop");
    } else {
        textBox_Inbound_Port->setReadOnly(false);
        textBox_Outbound_IPv4->setReadOnly(false);
        textBox_Outbound_Port->setReadOnly(false);
        button_ToggleServer->setText("Start");
    }
}

bool CasterOSCServerConfigPicker::getIsServerRunning()
{
    return server_is_running;
}
