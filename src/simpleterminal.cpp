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
#include "commandparser.h"

#include <QApplication>
#include <QSerialPort>
#include <QSettings>



//**********************************************************************************************************************
SimpleTerminal::SimpleTerminal(QSerialPort *port, QObject *parent) :
    QObject(parent),
    _statusText(QString()),
    _port(port),
    _eom("\r"),
    _inputHistory(),
    _inputHistoryIdx(-1),
    _cmdParser(nullptr)
{
    Q_CHECK_PTR(_port);

    _cmdParser = new CommandParser(*this);

    // Restore settings
    restoreSettings();

    refreshStatusText();

    QObject::connect(_port, SIGNAL(readyRead()), this, SLOT(read()));
//    QObject::connect(_port, SIGNAL(baudRateChanged(qint32,QSerialPort::Directions)), this, SLOT(settingsChanged()));
//    QObject::connect(_port, SIGNAL(dataBitsChanged(QSerialPort::DataBits)), this, SLOT(settingsChanged()));
//    QObject::connect(_port, SIGNAL(flowControlChanged(QSerialPort::FlowControl)), this, SLOT(settingsChanged()));
//    QObject::connect(_port, SIGNAL(parityChanged(QSerialPort::Parity)), this, SLOT(settingsChanged()));
//    QObject::connect(_port, SIGNAL(stopBitsChanged(QSerialPort::StopBits)), this, SLOT(settingsChanged()));
//    QObject::connect(this, SIGNAL(eomChanged()), this, SLOT(settingsChanged()));

}

//**********************************************************************************************************************
SimpleTerminal::~SimpleTerminal()
{
    delete _cmdParser;
}

//**********************************************************************************************************************
void SimpleTerminal::modifyDspText(DspType type, const QString &text)
{
//    static bool isReading = false;
//    if (isReading && type != DspType::READ_MESSAGE)
//        isReading = false;

    static bool newLine = true;
    if (type != DspType::READ_MESSAGE)
        newLine = true;

    QString sanitizedText = text.toHtmlEscaped();

    // Format text according to type of message
    switch(type)
    {
        case DspType::READ_MESSAGE:
        {
            static QString end_msg("");

            if (_eom.length() > 1 && !newLine && end_msg.length() > 0)
            {
                // EOM split across reads?

                QString endStr(end_msg + sanitizedText.left(_eom.length() - 1));
                sanitizedText.remove(0, _eom.length() - 1);

                int eomIndex = endStr.indexOf(_eom);
                if (eomIndex > -1)
                {
                    emit insertDisplayText(endStr.mid(end_msg.length(),
                                                      _eom.length() - (end_msg.length() - eomIndex)));
                    newLine = true;
                }
                else
                {
                    emit insertDisplayText(endStr.mid(end_msg.length()));
                }
            }

            // Find EOMs
            QStringList textList = sanitizedText.split(_eom);

            // First break at Line Feeds
            for (int i = 0; i < textList.length(); ++i)
            {
                textList[i].replace("\n", "\n<br>");
            }

            if (newLine)
                emit appendDisplayText(textList.at(0));
            else
                emit insertDisplayText(textList.at(0));

            for (int i = 1; i < textList.length(); ++i)
            {
                emit insertDisplayText(_eom);
                if (textList[i].length() > 0)
                    emit appendDisplayText(textList[i]);
            }

            if (textList[textList.length() - 1] == "")
            {
                newLine = true;
                end_msg = "";
            }
            else
            {
                newLine = false;
                end_msg = sanitizedText.right(_eom.length() - 1);
            }

            break;
        }

        case DspType::WRITE_MESSAGE:
        {
            QString msg = "<br><span><b>" + sanitizedText + "</b></span>";
//            QString msg = "<span><b>" + sanitizedText + "</b></span>";

            emit appendDisplayText(msg);

            break;
        }

        case DspType::COMMAND:
        {
            QString msg = "<br><span style = \"color: blue;\"><b>$ " + sanitizedText + "</b></span>";
//            QString msg = "<span style = \"color: blue;\"><b>$ " + sanitizedText + "</b></span>";

            emit appendDisplayText(msg);

            break;
        }

        case DspType::COMMAND_RSP:
        {
//            QString msg = "<br><span style = \"color: blue;\">" + text + "</span>";
            QString msg = "<span style = \"color: blue;\">" + text + "</span>";

            emit appendDisplayText(msg);

            break;
        }

        case DspType::ERROR:
        {
            QString msg = "<br><span style = \"color: red;\">ERROR: " + text + "</span>";

            emit appendDisplayText(msg);

            break;
        }

        default:
            QString msg = "<br><span>" + sanitizedText + "</span>";

            emit appendDisplayText(msg);

            break;
    }
}

//**********************************************************************************************************************
void SimpleTerminal::setEOM(QString newEOM)
{
    _eom = newEOM;

    emit eomChanged();
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
        _cmdParser->processCommand(msg);
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
    if (_port->isOpen())
    {
        _port->disconnect();
        _port->setPortName(port);
        connect();
    }
    else
    {
        _port->setPortName(port);
    }

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
QString SimpleTerminal::getPortName() const
{
    return _port->portName();
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

        setError("Connect attempt failed");
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

    modifyDspText(DspType::WRITE_MESSAGE, txMsg);
    if (_port->isOpen())
        _port->write((txMsg).toLocal8Bit());
    else
    {
        qWarning() << "Port is not open\nError code: " << _port->error() << "\nError description: "
                   << _port->errorString();

        setError("Port is not open");
    }
}

//**********************************************************************************************************************
void SimpleTerminal::setError(const QString &msg)
{
    // @todo: Do something more in the future...like an error queue?
    modifyDspText(DspType::ERROR, msg);
}

//**********************************************************************************************************************
void SimpleTerminal::restoreSettings()
{
    QSettings settings;

    // Baud Rate
    _port->setBaudRate(settings.value("port/baudrate", _port->baudRate()).toInt());

    // Data Bits
    if (settings.contains("port/databits"))
    {
        qint32 dataBits = settings.value("port/databits").toInt();
        switch (dataBits)
        {
            case 5:
                _port->setDataBits(QSerialPort::Data5);
                break;

            case 6:
                _port->setDataBits(QSerialPort::Data6);
                break;

            case 7:
                _port->setDataBits(QSerialPort::Data7);
                break;

            default:
            case 8:
                _port->setDataBits(QSerialPort::Data8);
                break;
        }
    }

    // Parity
    if (settings.contains("port/parity"))
    {
        QString parity = settings.value("port/parity").toString();
        if (parity == "Even")
        {
            _port->setParity(QSerialPort::EvenParity);
        }
        else if (parity == "Odd")
        {
            _port->setParity(QSerialPort::OddParity);
        }
        else
        {
            _port->setParity(QSerialPort::NoParity);
        }
    }

    // Stop Bits
    if (settings.contains("port/stopbits"))
    {
        float stopbits = settings.value("port/stopbits").toFloat();
        if (stopbits == 1)
        {
            _port->setStopBits(QSerialPort::OneStop);
        }
        else if (stopbits == 1.5)
        {
            _port->setStopBits(QSerialPort::OneAndHalfStop);
        }
        else
        {
            _port->setStopBits(QSerialPort::TwoStop);
        }
    }

    // Flow Control
    if (settings.contains("port/flowcontrol"))
    {
        QString flow = settings.value("port/flowcontrol").toString();
        if (flow == "Hardware")
        {
            _port->setFlowControl(QSerialPort::HardwareControl);
        }
        else if (flow == "Software")
        {
            _port->setFlowControl(QSerialPort::SoftwareControl);
        }
        else
        {
            _port->setFlowControl(QSerialPort::NoFlowControl);
        }
    }

    // EOM
    _eom = settings.value("port/eom", "\r").toString();

    // Port
    if (settings.contains("port/name"))
    {
        QString portName = settings.value("port/name").toString();
        setPort(portName);
    }
}

//**********************************************************************************************************************
void SimpleTerminal::saveSettings() const
{
    QSettings settings;

    // Baud Rate
    settings.setValue("port/baudrate", _port->baudRate());

    // Data Bits
    switch (_port->dataBits())
    {
        case QSerialPort::Data5:
            settings.setValue("port/databits", 5);
            break;

        case QSerialPort::Data6:
            settings.setValue("port/databits", 6);
            break;

        case QSerialPort::Data7:
            settings.setValue("port/databits", 7);
            break;

        default:
        case QSerialPort::Data8:
            settings.setValue("port/databits", 8);
            break;
    }

    // Parity
    switch (_port->parity())
    {
        case QSerialPort::EvenParity:
            settings.setValue("port/parity", "Even");
            break;

        case QSerialPort::OddParity:
            settings.setValue("port/parity", "Odd");
            break;

        default:
        case QSerialPort::NoParity:
            settings.setValue("port/parity", "None");
            break;
    }

    // Stop Bits
    switch (_port->stopBits())
    {
        default:
        case QSerialPort::OneStop:
            settings.setValue("port/stopbits", 1.0);
            break;

        case QSerialPort::OneAndHalfStop:
            settings.setValue("port/stopbits", 1.5);
            break;

        case QSerialPort::TwoStop:
            settings.setValue("port/stopbits", 2.0);
            break;
    }

    // Flow Control
    switch (_port->flowControl())
    {
        default:
        case QSerialPort::NoFlowControl:
            settings.setValue("port/flowcontrol", "None");
            break;

        case QSerialPort::HardwareControl:
            settings.setValue("port/flowcontrol", "Hardware");
            break;

        case QSerialPort::SoftwareControl:
            settings.setValue("port/flowcontrol", "Software");
            break;
    }

    // EOM
    settings.setValue("port/eom", _eom);

    // Port
    settings.setValue("port/name", _port->portName());
}

//**********************************************************************************************************************
void SimpleTerminal::read()
{
    QByteArray data = _port->readAll();
//    qDebug() << "Read: " << data << data.toHex();

    modifyDspText(DspType::READ_MESSAGE, QString(data));
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

    newText += " " + (getPortName() == "" ? "None" : getPortName()) + " " + QString::number( _port->baudRate());

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

    QString flowControl = "-";
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
            flowControl = "-";
            break;
    }

    newText += " " + flowControl;

    QString EOM = "-";
    if (_eom == "\r")
        EOM = "CR";
    else if (_eom == "\n")
        EOM = "LF";
    else if (_eom == "\r\n")
        EOM = "CR+LF";
    else
        EOM = "-";

    newText += " " + EOM;

    setStatusText(newText);

}

//**********************************************************************************************************************
void SimpleTerminal::settingsChanged()
{
    refreshStatusText();

    saveSettings();
}
