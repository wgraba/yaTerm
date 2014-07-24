#include <QSerialPort>

#include "simpleterminal.h"

SimpleTerminal::SimpleTerminal(QObject *parent) :
    QObject(parent),
    _displayText(QString()),
    _statusText(QString()),
    _port(new QSerialPort(this)),
    _eom("\n\r"),
    _portName(QString())
{
    QObject::connect(_port, SIGNAL(readyRead()), this, SLOT(read()));

    setStatusText("Disconnected");
}

SimpleTerminal::~SimpleTerminal()
{
}

void SimpleTerminal::appendDspText(QString text)
{
    _displayText += text;
    emit displayTextChanged();
}

void SimpleTerminal::clearDspText()
{
    _displayText = QString();
    emit displayTextChanged();
}

QString SimpleTerminal::displayText() const
{
    return _displayText;
}

QString SimpleTerminal::statusText() const
{
    return _statusText;
}

void SimpleTerminal::connect()
{
    _port->setPortName(_portName);
    if (_port->open(QIODevice::ReadWrite))
    {
        qDebug() << "Connected!";
        setStatusText("Connected");
    }
}

void SimpleTerminal::disconnect()
{
    _port->close();
    qDebug() << "Disconnected";
    setStatusText("Disconnected");
}

void SimpleTerminal::write(const QString &msg)
{
    qDebug() << "Write: " << msg;
    appendDspText(msg + "\r");
}

void SimpleTerminal::read()
{
    QByteArray data = _port->readAll();
    displayText().append(data);
}

void SimpleTerminal::setStatusText(QString text)
{
    _statusText = text;
    emit statusTextChanged();
}
