/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Wesley Graba
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
******************************************************************************/

#include "simpleterminal.h"
#include "portslistmodel.h"

#include <QSerialPort>
#include <QSerialPortInfo>


SimpleTerminal::SimpleTerminal(PortsListModel *portsList, QList<qint32> *baudRateList, QObject *parent) :
    QObject(parent),
    _availablePorts(portsList),
    _availableBaudRates(baudRateList),
    _displayText(QString()),
    _displayRead(false),
    _statusText(QString()),
    _port(new QSerialPort(this)),
    _eom("\r\n"),
    _portName(QString()),
    _baudRate(QSerialPort::Baud115200),
    _dataBits(QSerialPort::Data8),
    _flowControl(QSerialPort::NoFlowControl),
    _parity(QSerialPort::NoParity),
    _stopBits(QSerialPort::OneStop)
{
    Q_CHECK_PTR(_port);
    Q_CHECK_PTR(_availablePorts);
    Q_CHECK_PTR(_availableBaudRates);

    QObject::connect(_port, SIGNAL(readyRead()), this, SLOT(read()));

    generatePortList();
    _portName = _availablePorts->getStringList().count() > 0 ? _availablePorts->getStringList()[0] : "";
    *_availableBaudRates = QSerialPortInfo::standardBaudRates();

    refreshStatusText();
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

void SimpleTerminal::generatePortList()
{
    QStringList ports;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ports << info.portName();
    }
    _availablePorts->setStringList(ports);
}

void SimpleTerminal::setSettings(QString port)
{
    _portName = port;
    qDebug() << "Port set to " << _portName;

    refreshStatusText();
}

QString SimpleTerminal::displayText() const
{
    return _displayText;
}

QString SimpleTerminal::statusText() const
{
    return _statusText;
}

bool SimpleTerminal::isConnected() const
{
    return _port->isOpen();
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
        refreshStatusText();
        emit connStateChanged();

        qDebug() << "Connected!";
    }
    else
    {
        qWarning() << "Could not connect";
    }
}

void SimpleTerminal::disconnect()
{
    _port->close();
    refreshStatusText();
    emit connStateChanged();

    qDebug() << "Disconnected";
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

void SimpleTerminal::refreshStatusText()
{
    QString newText;
    if (isConnected())
        newText += "<strong>Connected</strong>";
    else
        newText += "<strong>Disconnected</strong>";

    newText += " - " + _portName;

    setStatusText(newText);

}
