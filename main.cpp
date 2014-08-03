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

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDebug>
#include <QList>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("yaTerm");
    app.setApplicationVersion("0.1.0");
    app.setWindowIcon(QIcon(":/images/utilities-terminal-icon.png"));

    QQmlApplicationEngine engine;

//    QList<QObject *> list = engine.children();
//    QObject *item = engine.findChild<QObject *>("root");
    PortsListModel portsListModel;
    QList<qint32> baudRatesModel;
    SimpleTerminal simpleTerminal(&portsListModel, &baudRatesModel, &app);
    engine.rootContext()->setContextProperty("SimpleTerminal", &simpleTerminal);
    engine.rootContext()->setContextProperty("portsListModel", &portsListModel);
//    engine.rootContext()->setContextProperty("baudRatesModel", QVariant::fromValue(baudRatesModel));

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    QObject *item = engine.rootObjects().value(0);
    Q_CHECK_PTR(item);

    QObject::connect(item, SIGNAL(consoleInputEntered(QString)), &simpleTerminal, SLOT(write(QString)));
    QObject::connect(item, SIGNAL(connect()), &simpleTerminal, SLOT(connect()));
    QObject::connect(item, SIGNAL(disconnect()), &simpleTerminal, SLOT(disconnect()));
    QObject::connect(item, SIGNAL(newSettings(QString)), &simpleTerminal, SLOT(setSettings(QString)));

    return app.exec();
}

