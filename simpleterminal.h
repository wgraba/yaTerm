#ifndef SIMPLETERMINAL_H
#define SIMPLETERMINAL_H

#include <QObject>
#include <QtDebug>

class SimpleTerminal : public QObject
{
    Q_OBJECT
public:
    explicit SimpleTerminal(QObject *parent = 0);

signals:

public slots:
    void write(const QString &msg)
    {
        qDebug() << "Console write: " << msg;
    }

};

#endif // SIMPLETERMINAL_H
