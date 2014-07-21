#include <QApplication>
#include <QQmlApplicationEngine>

#include "simpleterminal.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("yaTerm");
    app.setApplicationVersion("0.1.0");


    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

//    QList<QObject *> list = engine.children();
//    QObject *item = engine.findChild<QObject *>("root");
    QObject *item = engine.rootObjects().value(0);
    SimpleTerminal simpleTerminal;
    QObject::connect(item, SIGNAL(enterPressed(QString)), &simpleTerminal, SLOT(write(QString)));

    return app.exec();
}
