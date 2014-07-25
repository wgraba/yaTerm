#include <QSerialPort>

#include "simpleterminal.h"

SimpleTerminal::SimpleTerminal(QObject *parent) :
    QObject(parent),
    _displayText(QString()),
    _statusText(QString()),
    _port(new QSerialPort(this)),
    _eom("\n\r"),
    _portName(QString("/dev/pts/7")),
    _baudRate(QSerialPort::Baud115200),
    _dataBits(QSerialPort::Data8),
    _flowControl(QSerialPort::NoFlowControl),
    _parity(QSerialPort::NoParity),
    _stopBits(QSerialPort::OneStop)
{
    Q_CHECK_PTR(_port);
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
    _port->setBaudRate(_baudRate);
    _port->setDataBits(_dataBits);
    _port->setFlowControl(_flowControl);
    _port->setParity(_parity);
    _port->setStopBits(_stopBits);
    if (_port->open(QIODevice::ReadWrite))
    {
        qDebug() << "Connected!";
        setStatusText("Connected");
    }
    else
    {
        qWarning() << "Could not connect";
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
    qDebug() << "Write:" << msg << QByteArray(msg.toLocal8Bit()).toHex();
//    QString txMsg = msg + _eom;
    _port->write((msg + _eom).toLocal8Bit());
    QChar pre;
    if (!_displayText.isEmpty())
    {
        QChar lastChar = _displayText[_displayText.length() - 1];
        if (lastChar != '\r' || lastChar != '\n')
            pre ='\n';
    }

    appendDspText(pre + msg + "\n");
}

void SimpleTerminal::read()
{
    QByteArray data = _port->readAll();
    qDebug() << "Read:" << data << data.toHex();
    appendDspText(data);
}

void SimpleTerminal::setStatusText(QString text)
{
    _statusText = text;
    emit statusTextChanged();
}
