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

