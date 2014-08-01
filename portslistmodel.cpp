#include "portslistmodel.h"

PortsListModel::PortsListModel(QObject *parent) :
    QAbstractListModel(parent)
{}

PortsListModel::~PortsListModel()
{}

void PortsListModel::setStringList(QStringList &list)
{
    _strings = list;
}

QStringList PortsListModel::getStringList() const
{
    return _strings;
}

int PortsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _strings.count();
}

QVariant PortsListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= _strings.count())
        return QVariant();

    if (role == Qt::DisplayRole)
        return _strings[index.row()];

    return QVariant();
}

QHash<int, QByteArray> PortsListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    return roles;
}
