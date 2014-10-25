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

#ifndef SIMPLETERMINAL_H
#define SIMPLETERMINAL_H

#include <QObject>
#include <QtDebug>
#include <QString>
#include <QSerialPort>
#include <QList>

class StringListModel;

//**********************************************************************************************************************
class SimpleTerminal : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString displayText READ displayText RESET clearDspText NOTIFY displayTextChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(bool connState READ isConnected NOTIFY connStateChanged)
    Q_PROPERTY(QString eom READ getEOM WRITE setEOM NOTIFY eomChanged)

public:
    explicit SimpleTerminal(QSerialPort *port, StringListModel *portsList, QObject *parent = 0);
    ~SimpleTerminal();

    QString displayText() const;
    QString statusText() const;
    bool isConnected() const;
    QString getEOM() const;

    void appendDspText(QString text);
    void clearDspText();
    void generatePortList();
    void setEOM(QString newEOM);

    StringListModel *_availablePorts;

signals:
    void displayTextChanged();
    void statusTextChanged();
    void connStateChanged();
    void eomChanged();

public slots:
    void write(const QString &msg);
    void read();

    void connect();
    void disconnect();

    void setPort(QString port);
    void refreshStatusText();


private:
    static const int MAX_NUM_DISP_CHARS = 1024 * 16;

    void setStatusText(QString text);

    QString _displayText;
    bool _displayRead;
    QString _statusText;
    QSerialPort *_port;
    QString _eom;
    QString _portName;

};

#endif // SIMPLETERMINAL_H
