#include <QSerialPort>

#include "simpleterminal.h"

SimpleTerminal::SimpleTerminal(QObject *parent) :
    QObject(parent),
    _displayText(QString()),
    _displayRead(false),
    _statusText(QString()),
    _port(new QSerialPort(this)),
    _eom("\r\n"),
    _portName(QString("/dev/pts/9")),
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
    int overFill = _displayText.size() + text.size() - MAX_NUM_DISP_CHARS;
    if (overFill <= 0)  // No over-fill
        _displayText += text;
    else
    {
        _displayText.remove(0, overFill);
    }

    emit displayTextChanged();
}

void SimpleTerminal::clearDspText()
{
    _displayText.clear();
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
    QString txMsg = msg + _eom;
    qDebug() << "Write:" << txMsg << QByteArray(msg.toLocal8Bit()).toHex();
    if (_port->isOpen())
        _port->write((txMsg).toLocal8Bit());
    else
        qWarning() << "Port is not open";

    QString pre;
    if (_displayRead)
    {
        pre = "</samp></p>";
        _displayRead = false;
    }
    appendDspText(pre + "<p><strong><kbd>" + txMsg.toHtmlEscaped() + "</kbd></strong></p>");
}

void SimpleTerminal::read()
{
    QByteArray data = _port->readAll();
    qDebug() << "Read:" << data << data.toHex();

    QString pre;
    if (!_displayRead)
    {
        pre = "<p><samp>";
        _displayRead = true;
    }
    appendDspText(pre + QString(data).toHtmlEscaped());
}

void SimpleTerminal::setStatusText(QString text)
{
    _statusText = text;
    emit statusTextChanged();
}
