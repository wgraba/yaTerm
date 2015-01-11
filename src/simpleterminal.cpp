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
const QMap<QString, SimpleTerminal::CmdFunc> SimpleTerminal::cmdMap = {
//    { "/clear", SimpleTerminal::cmdClear },
    { "/connect", SimpleTerminal::cmdConnect },
    { "/disconnect", SimpleTerminal::cmdDisconnect },
    { "/help", SimpleTerminal::cmdHelp },
    { "/quit", SimpleTerminal::cmdQuit },
};

//**********************************************************************************************************************
const QMap<QString, QStringList> SimpleTerminal::cmdHelpMap = {
//    { "/clear", { "", "Clear the screen" } },
    { "/connect", { "portName", "Connect to port" } },
    { "/disconnect", { "", "Disconnect from port" } },
    { "/help", { "[command]", "Get help" } },
    { "/quit", { "", "Quit" } },
};

//**********************************************************************************************************************
SimpleTerminal::SimpleTerminal(QSerialPort *port, StringListModel *portsList, QObject *parent) :
    QObject(parent),
    _availablePorts(portsList),
    _statusText(QString()),
    _port(port),
    _eom("\r"),
    _inputHistory(),
    _inputHistoryIdx(-1)
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
        _port->setBaudRate(QSerialPort::Baud9600);
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
    static const QString readMsgPost = "</span><br>";

    QString sanitizedText = text.toHtmlEscaped();

    // Format text according to type of message
    QString dspText;
    static bool isReading = false;
    switch(type)
    {
        case DspType::READ_MESSAGE:
        {
            static const QString readMsgPre = "<span>";
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
            QString msg = "<span><b>" + sanitizedText + "</b></span><br>";
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
            QString msg = "<span style = \"color: blue;\"><b>$ " + sanitizedText + "</b></span><br>";
            if (isReading)
            {
                dspText = readMsgPost + msg;
                isReading = false;
            }
            else
                dspText = msg;

            break;
        }

        case DspType::COMMAND_RSP:
        {
            QString msg = "<span style = \"color: blue;\">" + sanitizedText + "</span><br>";
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
            QString msg = "<span style = \"color: red;\">ERROR: " + sanitizedText + "</span><br>";
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
            QString msg = "<span>" + sanitizedText + "</span><br>";
            if (isReading)
            {
                dspText = readMsgPost + msg;
                isReading = false;
            }
            else
                dspText = msg;

            break;
    }

    emit newDisplayText(dspText);
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
void SimpleTerminal::resetHistoryIdx()
{
    _inputHistoryIdx = _inputHistory.size();
}

//**********************************************************************************************************************
void SimpleTerminal::parseInput(const QString &msg)
{
    if (msg.startsWith('/'))
        processCommand(msg);
    else if (msg.startsWith("\\/"))
        write(msg.mid(1));
    else
        write(msg);

    // Add to history
    if (_inputHistory.size() >= MAX_INPUT_HISTORY_LEN)
        _inputHistory.removeFirst();

    _inputHistory << msg;
    _inputHistoryIdx = _inputHistory.size();
}

//**********************************************************************************************************************
void SimpleTerminal::setPort(QString port)
{
    _port->setPortName(port);
    qDebug() << "Port set to " << _port->portName();

    refreshStatusText();
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
int SimpleTerminal::getInputHistoryLen() const
{
    return _inputHistory.size();
}

//**********************************************************************************************************************
QString SimpleTerminal::getInputHistoryIdx(int idx) const
{
    if (idx >= _inputHistory.length() || idx < 0)
        return QString();

    return _inputHistory[idx];
}

//**********************************************************************************************************************
QString SimpleTerminal::getPrevHistory()
{
    QString retval;
    if (_inputHistoryIdx >= 0 && _inputHistory.size() > 0)
    {
        if (--_inputHistoryIdx < 0)
            _inputHistoryIdx = 0;

        retval = _inputHistory[_inputHistoryIdx];
    }

    return retval;
}

//**********************************************************************************************************************
QString SimpleTerminal::getNextHistory()
{
    QString retval;

    if (_inputHistoryIdx >= 0 && _inputHistory.size() > 1)
    {
        if (++_inputHistoryIdx >= _inputHistory.size())
        {
            _inputHistoryIdx = _inputHistory.size();
            retval = QString();
        }
        else
            retval = _inputHistory[_inputHistoryIdx];
    }

    return retval;
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

        setError("Connect failed");
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

    appendDspText(DspType::WRITE_MESSAGE, txMsg);
    if (_port->isOpen())
        _port->write((txMsg).toLocal8Bit());
    else
    {
        qWarning() << "Port is not open";
        setError("Port is not open");
    }
}

//**********************************************************************************************************************
void SimpleTerminal::setError(const QString &msg)
{
    // @todo: Do something more in the future...like an error queue?
    appendDspText(DspType::ERROR, msg);
}

//**********************************************************************************************************************
void SimpleTerminal::cmdConnect(SimpleTerminal &st, const QStringList &args)
{
    if (args.size() > 0)
    {
        st.setPort(args[0]);
        st.connect();
    }
    else
    {
        st.setError("Wrong number of arguments");
    }
}

//**********************************************************************************************************************
void SimpleTerminal::cmdDisconnect(SimpleTerminal &st, const QStringList &)
{
    st.disconnect();
}

//**********************************************************************************************************************
void SimpleTerminal::cmdQuit(SimpleTerminal &, const QStringList &)
{
    QApplication::quit();
}

//**********************************************************************************************************************
void SimpleTerminal::cmdHelp(SimpleTerminal &st, const QStringList &args)
{
    if (args.size() > 0)
    {
        if (cmdHelpMap.contains(args[0]))
        {
            QStringList help = cmdHelpMap.value(args[0]);

            st.appendDspText(DspType::COMMAND_RSP, help[1]);
            st.appendDspText(DspType::COMMAND_RSP, "Usage: " + args[0] + " " + help[0]);
        }
        else
            st.setError("Unkown command");
    }
    else
    {
        QMap<QString, QStringList>::const_iterator cmd = cmdHelpMap.constBegin();
        while (cmd != cmdHelpMap.constEnd())
        {
            QStringList help = cmd.value();
            QString cmdName = cmd.key();

            st.appendDspText(DspType::COMMAND_RSP, cmdName + " - " + help[1]);

            ++cmd;
        }
    }
}

//**********************************************************************************************************************
void SimpleTerminal::processCommand(const QString &cmd)
{
    qDebug() << "Command: " << cmd;

    if (cmd.length() < 1)
        return; // Do nothing if there is no command

    appendDspText(DspType::COMMAND, cmd);

    QStringList cmdList = cmd.split(' '); // Command is first item, parameters are what's left
    QString cmdName = cmdList[0];
    cmdList.removeFirst();
    QStringList &args = cmdList;
    if (cmdMap.contains(cmdName))
    {
        CmdFunc cmdFunc = (cmdMap.value(cmdName));

        Q_CHECK_PTR(cmdFunc);
        cmdFunc(*this, args);
    }
    else
    {
        setError("Invalid command");
    }
}

//**********************************************************************************************************************
void SimpleTerminal::read()
{
    QByteArray data = _port->readAll();
//    qDebug() << "Read: " << data << data.toHex();

    appendDspText(DspType::READ_MESSAGE, QString(data));
}

//**********************************************************************************************************************
void SimpleTerminal::setStatusText(const QString &text)
{
    _statusText = text;
    emit statusTextChanged();
}

//**********************************************************************************************************************
void SimpleTerminal::setErrorText(const QString &text)
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

    newText += " " + (_port->portName() == "" ? "None" : _port->portName()) + " " + QString::number( _port->baudRate());

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
