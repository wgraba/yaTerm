#ifndef PORTSLISTMODEL_H
#define PORTSLISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class PortsListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PortsListModel(QObject *parent = 0);

    ~PortsListModel();

    void setStringList(QStringList &list);
    QStringList getStringList() const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

signals:

public slots:

protected:
    virtual QHash<int, QByteArray> roleNames() const;

private:
    QStringList _strings;

};

#endif // PORTSLISTMODEL_H
