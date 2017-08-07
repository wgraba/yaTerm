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
    _som(),
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
    QObject::connect(_port, SIGNAL(baudRateChanged(qint32,QSerialPort::Directions)), this, SLOT(settingsChanged()));
    QObject::connect(_port, SIGNAL(dataBitsChanged(QSerialPort::DataBits)), this, SLOT(settingsChanged()));
    QObject::connect(_port, SIGNAL(flowControlChanged(QSerialPort::FlowControl)), this, SLOT(settingsChanged()));
    QObject::connect(_port, SIGNAL(parityChanged(QSerialPort::Parity)), this, SLOT(settingsChanged()));
    QObject::connect(_port, SIGNAL(stopBitsChanged(QSerialPort::StopBits)), this, SLOT(settingsChanged()));
    QObject::connect(this, SIGNAL(somChanged()), this, SLOT(settingsChanged()));
    QObject::connect(this, SIGNAL(eomChanged()), this, SLOT(settingsChanged()));

}

//**********************************************************************************************************************
SimpleTerminal::~SimpleTerminal()
{
    delete _cmdParser;
}

//**********************************************************************************************************************
void SimpleTerminal::modifyDspText(DspType type, const QString &text)
{
    // Format text according to type of message
    static bool is_new_msg = true;
    switch(type)
    {
        case DspType::READ_MESSAGE:
        {
            // Parse message and look for EOM string(s) - there could be 0 - n in this message
            // Emit approriate details to system - end of message? Start of message? Append message?
            QString msg = text.toHtmlEscaped();
//            QString msg = text;
            QString new_msg; // Message queue
            static bool is_eom_detected = false;
            static int eom_index = 0; // Keeps track of what part of EOM string has been detected
            for (int i = 0; i < msg.length(); ++i)
            {

                new_msg.append(msg[i]);

                // Starting a new message or continuing an old message?
                if (is_new_msg)
                {
                    is_new_msg = false;
                    emit startMsg();
                }

                if (msg[i] == _eom[eom_index])
                {
                    // Possible EOM string detected
                    is_eom_detected = true;
                    eom_index++;

                    if (eom_index >= _eom.length())
                    {
                        // Full EOM string detected!
                        emit appendMsg(new_msg);
                        new_msg.clear();

                        is_new_msg = true;
                        emit endMsg();

                        is_eom_detected = false;
                        eom_index = 0;
                    }
                }
                else if (is_eom_detected)
                {
                    // Full EOM string not realized - start over
                    is_eom_detected = false;
                    eom_index = 0;
                }
            }

            if (new_msg.length() > 0)
            {
                // Emit remaining character in queue
                emit appendMsg(new_msg);
            }


            break;
        }

        case DspType::WRITE_MESSAGE:
        {
            QString msg = "<span><b>" + text.toHtmlEscaped() + "</b></span>";

            if (!is_new_msg)
            {
                is_new_msg = true;
                emit endMsg();
            }

            emit newMsg(msg);

            break;
        }

        case DspType::COMMAND:
        {
            QString msg = "<span style = \"color: blue;\"><b>$ " + text.toHtmlEscaped() + "</b></span>";

            if (!is_new_msg)
            {
                is_new_msg = true;
                emit endMsg();
            }

            emit newMsg(msg);

            break;
        }

        case DspType::COMMAND_RSP:
        {
            QString msg = "<span style = \"color: green;\">" + text + "</span>";

            if (!is_new_msg)
            {
                is_new_msg = true;
                emit endMsg();
            }

            emit newMsg(msg);

            break;
        }

        case DspType::ERROR:
        {
            QString msg = "<span style = \"color: red;\">ERROR: " + text + "</span>";

            if (!is_new_msg)
            {
                is_new_msg = true;
                emit endMsg();
            }

            emit newMsg(msg);

            break;
        }
    }
}

//**********************************************************************************************************************
void SimpleTerminal::setSOM(QString newSOM)
{
    _som = newSOM;

    emit somChanged();
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
QString SimpleTerminal::getSOM() const
{
    return _som;
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
    QString txMsg = _som + msg + _eom;

    qDebug() << "Write:" << txMsg << QByteArray(txMsg.toLocal8Bit()).toHex();

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
        if (stopbits == 1.0f)
        {
            _port->setStopBits(QSerialPort::OneStop);
        }
        else if (stopbits == 1.5f)
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

    // SOM
    _som = settings.value("port/som", "").toString();

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

        case QSerialPort::Data8:
            settings.setValue("port/databits", 8);
            break;

        case QSerialPort::UnknownDataBits:
            qWarning() << "Unhandled Data Bits to save";
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

        case QSerialPort::NoParity:
            settings.setValue("port/parity", "None");
            break;

        case QSerialPort::UnknownParity:
        case QSerialPort::MarkParity:
        case QSerialPort::SpaceParity:
            qWarning() << "Unhandled parity to save";
            break;
    }

    // Stop Bits
    switch (_port->stopBits())
    {
        case QSerialPort::OneStop:
            settings.setValue("port/stopbits", 1.0);
            break;

        case QSerialPort::OneAndHalfStop:
            settings.setValue("port/stopbits", 1.5);
            break;

        case QSerialPort::TwoStop:
            settings.setValue("port/stopbits", 2.0);
            break;

        case QSerialPort::UnknownDataBits:
            qWarning() << "Unhandled parity to save";
            break;
    }

    // Flow Control
    switch (_port->flowControl())
    {
        case QSerialPort::NoFlowControl:
            settings.setValue("port/flowcontrol", "None");
            break;

        case QSerialPort::HardwareControl:
            settings.setValue("port/flowcontrol", "Hardware");
            break;

        case QSerialPort::SoftwareControl:
            settings.setValue("port/flowcontrol", "Software");
            break;

        case QSerialPort::UnknownFlowControl:
            qWarning() << "Unhandled Flow Control to save";
    }

    // SOM
    settings.setValue("port/som", _som);

    // EOM
    settings.setValue("port/eom", _eom);

    // Port
    settings.setValue("port/name", _port->portName());
}

//**********************************************************************************************************************
void SimpleTerminal::read()
{
    QByteArray data = _port->readAll();
    qDebug() << "Read: " << data << data.toHex();

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

        case QSerialPort::UnknownDataBits:
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

        case QSerialPort::UnknownParity:
        case QSerialPort::MarkParity:
        case QSerialPort::SpaceParity:
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

        case QSerialPort::UnknownStopBits:
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

        case QSerialPort::UnknownFlowControl:
            flowControl = "-";
            break;
    }

    newText += " " + flowControl;

    QString SOM = "-";
    if (_som.length() > 0)
        SOM = _som;

    else
        SOM = "-";

    newText += " " + SOM;

    QString EOM = "-";
    if (_eom == "\r")
        EOM = "CR";

    else if (_eom == "\n")
        EOM = "LF";

    else if (_eom == "\r\n")
        EOM = "CR+LF";

    else
        if (_eom.length() > 0)
            EOM = _eom;

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
