﻿/******************************************************************************
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

#ifndef SIMPLETERMINAL_H
#define SIMPLETERMINAL_H

#include <QObject>
#include <QtDebug>
#include <QString>
#include <QSerialPort>
#include <QMap>

//**********************************************************************************************************************
class CommandParser;

//**********************************************************************************************************************
class SimpleTerminal : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int maxDspTxtChars MEMBER _maxDisplayTextChars NOTIFY maxDspTxtCharsChanged)
    Q_PROPERTY(bool is_msg_open MEMBER _is_msg_open)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged)
    Q_PROPERTY(bool connState READ isConnected NOTIFY connStateChanged)
    Q_PROPERTY(QString som READ getSOM WRITE setSOM NOTIFY somChanged)
    Q_PROPERTY(QString eom READ getEOM WRITE setEOM NOTIFY eomChanged)

public:
    enum class DspType
    {
        NONE,
        READ_MESSAGE,
        WRITE_MESSAGE,
        COMMAND,
        COMMAND_RSP,
        ERROR
    };

    explicit SimpleTerminal(QSerialPort *port, QObject *parent = nullptr);
    ~SimpleTerminal();

    QString statusText() const;
    QString errorText() const;
    bool isConnected() const;
    Q_INVOKABLE QString getPortName() const;
    QString getSOM() const;
    QString getEOM() const;
    int getInputHistoryLen() const;
    Q_INVOKABLE QString getInputHistoryIdx(int idx) const;
    Q_INVOKABLE QString getPrevHistory();
    Q_INVOKABLE QString getNextHistory();

    void modifyDspText(DspType type, const QString &text);
    void setSOM(QString newSOM = QString());
    void setEOM(QString newEOM = QString());
    Q_INVOKABLE void resetHistoryIdx();
    void setError(const QString &msg);

signals:
    void statusTextChanged();
    void errorTextChanged();
    void connStateChanged();
    void somChanged();
    void eomChanged();
    void maxDspTxtCharsChanged();
    void startMsg();
    void appendMsg(QString text);
    void endMsg();
    void newMsg(QString text);
    void clearDisplayText();

public slots:
    void parseInput(const QString &msg);
    void read();

    void connect();
    void disconnect();
    void setPort(QString port);

    void refreshStatusText();
    void settingsChanged();


private:
    static const int MAX_INPUT_HISTORY_LEN = 64;

    void setStatusText(const QString &text);
    void setErrorText(const QString &text);
    void write(const QString &msg);
    void restoreSettings();
    void saveSettings() const;

    QString _statusText;
    QString _errorText;
    QSerialPort *_port;
    QString _som;
    QString _eom;

    QStringList _inputHistory;
    int _inputHistoryIdx;

    int _maxDisplayTextChars = 1024 * 8;
    bool _is_msg_open;  // Actively writing message to console

    CommandParser *_cmdParser;

};

#endif // SIMPLETERMINAL_H
