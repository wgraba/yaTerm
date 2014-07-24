#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

#include "simpleterminal.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("yaTerm");
    app.setApplicationVersion("0.1.0");
    app.setWindowIcon(QIcon(":/utilities-terminal-icon.png"));


    QQmlApplicationEngine engine;

//    QList<QObject *> list = engine.children();
//    QObject *item = engine.findChild<QObject *>("root");
    SimpleTerminal simpleTerminal(&app);
    engine.rootContext()->setContextProperty("SimpleTerminal", &simpleTerminal);

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));
    QObject *item = engine.rootObjects().value(0);
    QObject::connect(item, SIGNAL(consoleInputEntered(QString)), &simpleTerminal, SLOT(write(QString)));

    return app.exec();
}
