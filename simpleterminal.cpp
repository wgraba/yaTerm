#include <QSerialPort>

#include "simpleterminal.h"

SimpleTerminal::SimpleTerminal(QObject *parent) :
    QObject(parent),
    _displayText(QString()),
    _statusText(QString()),
    _port(new QSerialPort(this))
{
    QObject::connect(_port, SIGNAL(readyRead()), this, SLOT(read()));
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

void SimpleTerminal::write(const QString &msg)
{
    qDebug() << "Console write: " << msg;
    appendDspText("<p><b>" + msg + "</b></p>");
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
