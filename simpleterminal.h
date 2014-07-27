#ifndef SIMPLETERMINAL_H
#define SIMPLETERMINAL_H

#include <QObject>
#include <QtDebug>
#include <QString>
#include <QSerialPort>

class SimpleTerminal : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString displayText READ displayText RESET clearDspText NOTIFY displayTextChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(bool connState READ isConnected NOTIFY connStateChanged)


public:
    explicit SimpleTerminal(QObject *parent = 0);
    ~SimpleTerminal();

    QString displayText() const;
    QString statusText() const;
    bool isConnected() const;

    void appendDspText(QString text);
    void clearDspText();

signals:
    void displayTextChanged();
    void statusTextChanged();
    void connStateChanged();

public slots:
    void write(const QString &msg);
    void read();

    void connect();
    void disconnect();

private:
    static const int MAX_NUM_DISP_CHARS = 1024 * 16;

    void setStatusText(QString text);
    void refreshStatusText();

    QString _displayText;
    bool _displayRead;
    QString _statusText;
    QSerialPort *_port;
    QString _eom;
    QString _portName;
    QSerialPort::BaudRate _baudRate;
    QSerialPort::DataBits _dataBits;
    QSerialPort::FlowControl _flowControl;
    QSerialPort::Parity _parity;
    QSerialPort::StopBits _stopBits;

};

#endif // SIMPLETERMINAL_H
