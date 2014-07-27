#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDebug>

#include "simpleterminal.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("yaTerm");
    app.setApplicationVersion("0.1.0");
    app.setWindowIcon(QIcon(":/images/utilities-terminal-icon.png"));

    QQmlApplicationEngine engine;

//    QList<QObject *> list = engine.children();
//    QObject *item = engine.findChild<QObject *>("root");
    SimpleTerminal simpleTerminal(&app);
    engine.rootContext()->setContextProperty("SimpleTerminal", &simpleTerminal);

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    QObject *item = engine.rootObjects().value(0);
    Q_CHECK_PTR(item);

    QObject::connect(item, SIGNAL(consoleInputEntered(QString)), &simpleTerminal, SLOT(write(QString)));
    QObject::connect(item, SIGNAL(connect()), &simpleTerminal, SLOT(connect()));
    QObject::connect(item, SIGNAL(disconnect()), &simpleTerminal, SLOT(disconnect()));

    return app.exec();
}
