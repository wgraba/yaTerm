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
#include "stringlistmodel.h"

#include <QApplication>
#include <QSerialPort>
#include <QSerialPortInfo>

//**********************************************************************************************************************
SimpleTerminal::SimpleTerminal(QSerialPort *port, StringListModel *portsList, QObject *parent) :
    QObject(parent),
    _availablePorts(portsList),
    _displayText(QString()),
    _statusText(QString()),
    _port(port),
    _eom("\r")
{
    Q_CHECK_PTR(_port);
    Q_CHECK_PTR(_availablePorts);

    generatePortList();
    if (_availablePorts->getStringList().count() <= 0)
    {
        _port->setBaudRate(QSerialPort::Baud115200);
        _port->setDataBits(QSerialPort::Data8);
        _port->setParity(QSerialPort::NoParity);
        _port->setStopBits(QSerialPort::OneStop);
        _port->setFlowControl(QSerialPort::NoFlowControl);
    }
    else
    {
        _port->setPortName(_availablePorts->getStringList()[0]);
        _port->setBaudRate(QSerialPortInfo::standardBaudRates()[0]);
    }

    refreshStatusText();

    QObject::connect(_port, SIGNAL(readyRead()), this, SLOT(read()));
    QObject::connect(_port, SIGNAL(baudRateChanged(qint32,QSerialPort::Directions)), this, SLOT(refreshStatusText()));
    QObject::connect(_port, SIGNAL(dataBitsChanged(QSerialPort::DataBits)), this, SLOT(refreshStatusText()));
    QObject::connect(_port, SIGNAL(flowControlChanged(QSerialPort::FlowControl)), this, SLOT(refreshStatusText()));
    QObject::connect(_port, SIGNAL(parityChanged(QSerialPort::Parity)), this, SLOT(refreshStatusText()));
    QObject::connect(_port, SIGNAL(stopBitsChanged(QSerialPort::StopBits)), this, SLOT(refreshStatusText()));


}

//**********************************************************************************************************************
SimpleTerminal::~SimpleTerminal()
{
}

//**********************************************************************************************************************
void SimpleTerminal::appendDspText(DspType type, const QString &text)
{
    static const QString readMsgPre = "<p>";
    static const QString readMsgPost = "</p>";

    QString sanitizedText = text.toHtmlEscaped();

    // Format text according to type of message
    QString dspText;
    static bool isReading = false;
    switch(type)
    {
        case DspType::READ_MESSAGE:
        {
            sanitizedText.replace(_eom, _eom + readMsgPost + readMsgPre);
            if (!isReading)
            {
                dspText = readMsgPre + sanitizedText;
                isReading = true;
            }
            else
                dspText = sanitizedText;

            break;
        }

        case DspType::WRITE_MESSAGE:
        {
            QString msg = "<p><b>" + sanitizedText + "</b></p>";
            if (isReading)
            {
                dspText = readMsgPost + msg;
                isReading = false;
            }
            else
                dspText = msg;

            break;
        }

        case DspType::COMMAND:
        {
            QString msg = "<p style = \"color: blue;\"><i>" + sanitizedText + "</i></p>";
            if (isReading)
            {
                dspText = readMsgPost + msg;
                isReading = false;
            }
            else
                dspText = msg;

            break;
        }

        case DspType::ERROR:
        {
            QString msg = "<p style = \"color: red;\"><i>ERROR: " + sanitizedText + "</i></p>";
            if (isReading)
            {
                dspText = readMsgPost + msg;
                isReading = false;
            }
            else
                dspText = msg;

            break;
        }

        default:
            QString msg = "<p>" + sanitizedText + "</p>";
            if (isReading)
            {
                dspText = readMsgPost + msg;
                isReading = false;
            }
            else
                dspText = msg;

            break;
    }

    _displayText += dspText;

    emit displayTextChanged();
}

//**********************************************************************************************************************
void SimpleTerminal::clearDspText()
{
    _displayText.clear();

    emit displayTextChanged();
}

//**********************************************************************************************************************
void SimpleTerminal::generatePortList()
{
    QStringList ports;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ports << info.portName();
    }
    _availablePorts->setStringList(ports);
}

//**********************************************************************************************************************
void SimpleTerminal::setEOM(QString newEOM)
{
    _eom = newEOM;
}

//**********************************************************************************************************************
void SimpleTerminal::parseInput(const QString &msg)
{
    if (msg.startsWith('/'))
    {
        processCommand(msg.mid(1));
    }
    else if (msg.startsWith("\\/"))
    {
        write(msg.mid(1));
    }
    else
    {
        write(msg);
    }
}

//**********************************************************************************************************************
void SimpleTerminal::setPort(QString port)
{
    _port->setPortName(port);
    qDebug() << "Port set to " << _port->portName();

    refreshStatusText();
}

//**********************************************************************************************************************
QString SimpleTerminal::displayText() const
{
    return _displayText;
}

//**********************************************************************************************************************
QString SimpleTerminal::statusText() const
{
    return _statusText;
}

//**********************************************************************************************************************
QString SimpleTerminal::errorText() const
{
    return _errorText;
}

//**********************************************************************************************************************
bool SimpleTerminal::isConnected() const
{
    return _port->isOpen();
}

//**********************************************************************************************************************
QString SimpleTerminal::getEOM() const
{
    return _eom;
}

//**********************************************************************************************************************
void SimpleTerminal::connect()
{
    if (_port->open(QIODevice::ReadWrite))
    {
        refreshStatusText();
        emit connStateChanged();

        qDebug() << "Connected!";
    }
    else
    {
        qWarning() << "Could not connect\nError code: " << _port->error() << "\nError description: "
                   << _port->errorString();
    }
}

//**********************************************************************************************************************
void SimpleTerminal::disconnect()
{
    _port->close();
    refreshStatusText();
    emit connStateChanged();

    qDebug() << "Disconnected";
}

//**********************************************************************************************************************
void SimpleTerminal::write(const QString &msg)
{
    QString txMsg = msg + _eom;

    qDebug() << "Write:" << txMsg << QByteArray(msg.toLocal8Bit()).toHex();

    if (_port->isOpen())
        _port->write((txMsg).toLocal8Bit());
    else
    {
        qWarning() << "Port is not open";
        appendDspText(DspType::ERROR, "Port is not open");
        setErrorText("Error writing");
    }

    appendDspText(DspType::WRITE_MESSAGE, txMsg);
}

//**********************************************************************************************************************
void SimpleTerminal::processCommand(const QString &cmd)
{
    qDebug() << "Command: " << cmd;

    if (cmd.length() < 1)
        return; // Do nothing if there is no command

    DspType dspType = DspType::COMMAND;
    QStringList cmdList = cmd.split(' '); // Command is first item, parameters are what's left
    if (cmdList[0] == "connect")
    {
        if (cmdList.size() > 1)
        {
            setPort(cmdList[1]);
            connect();
        }
    }
    else if (cmdList[0] == "disconnect")
    {
        disconnect();
    }
    else if (cmdList[0] == "quit")
    {
        QApplication::quit();
    }
    else
    {
        dspType = DspType::ERROR;
    }

    appendDspText(dspType, cmd);
}

//**********************************************************************************************************************
void SimpleTerminal::read()
{
    QByteArray data = _port->readAll();
    qDebug() << "Read: " << data << data.toHex();

    appendDspText(DspType::READ_MESSAGE, QString(data));
}

//**********************************************************************************************************************
void SimpleTerminal::setStatusText(QString text)
{
    _statusText = text;
    emit statusTextChanged();
}

//**********************************************************************************************************************
void SimpleTerminal::setErrorText(QString text)
{
    _errorText = text;
    emit errorTextChanged();
}

//**********************************************************************************************************************
void SimpleTerminal::refreshStatusText()
{
    QString newText;
    if (isConnected())
        newText += "<strong>Connected</strong>";
    else
        newText += "<strong>Disconnected</strong>";

    newText += " " + (_port->portName() == "" ? "None" : _port->portName());

    QString dataBits = "-";
    switch (_port->dataBits())
    {
        case QSerialPort::Data5:
            dataBits = "5";
            break;

        case QSerialPort::Data6:
            dataBits = "6";
            break;

        case QSerialPort::Data7:
            dataBits = "7";
            break;

        case QSerialPort::Data8:
            dataBits = "8";
            break;

        default:
            dataBits = "-";
            break;
    }

    QString parity = "-";
    switch (_port->parity())
    {
        case QSerialPort::NoParity:
            parity = "N";
            break;

        case QSerialPort::EvenParity:
            parity = "E";
            break;

        case QSerialPort::OddParity:
            parity = "O";
            break;

        default:
            parity = "-";
            break;
    }

    QString stopBits = "-";
    switch (_port->stopBits())
    {
        case QSerialPort::OneStop:
            stopBits = "1";
            break;

        case QSerialPort::OneAndHalfStop:
            stopBits = "1.5";
            break;

        case QSerialPort::TwoStop:
            stopBits = "2";
            break;

        default:
            stopBits = '-';
            break;
    }

    newText += " " + dataBits + parity + stopBits;

    QString flowControl = "Unknown";
    switch (_port->flowControl())
    {
        case QSerialPort::HardwareControl:
            flowControl = "Hardware";
            break;

        case QSerialPort::SoftwareControl:
            flowControl = "Software";
            break;

        case QSerialPort::NoFlowControl:
            flowControl = "None";
            break;

        default:
            flowControl = "Unknown";
            break;
    }

    newText += " " + flowControl;

    setStatusText(newText);

}