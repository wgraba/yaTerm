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

#include "portswatcher.h"
#include <QStringList>

#include <QQmlApplicationEngine>
//#include <QQuickView>
#include <QQmlContext>
#include <QSerialPortInfo>

//**********************************************************************************************************************
PortsWatcher::PortsWatcher(QQmlApplicationEngine &engine, QStringList &portsList, QObject *parent)
    : QObject(parent),
      _engine(engine),
      _portsList(portsList),
      _timer(this)
{
    QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(generatePortsList()));

    _timer.setSingleShot(false);
}

//**********************************************************************************************************************
void PortsWatcher::start()
{
    _timer.start(GET_PORTS_LIST_PERIOD_MS);
}

//**********************************************************************************************************************
void PortsWatcher::generatePortsList()
{
    QStringList ports;
    QList<QSerialPortInfo> availablePorts(QSerialPortInfo::availablePorts());
    if (availablePorts.length() > 0)
    {
        foreach (const QSerialPortInfo &info, availablePorts)
        {
            ports << info.portName();
        }
    }
    else
    {
        ports << "";
    }

    _portsList = ports;

    _engine.rootContext()->setContextProperty("portsListModel", _portsList);
}

