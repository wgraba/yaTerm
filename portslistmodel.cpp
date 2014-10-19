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

#include "portslistmodel.h"

//**********************************************************************************************************************
PortsListModel::PortsListModel(QObject *parent) :
    QAbstractListModel(parent)
{}

//**********************************************************************************************************************
PortsListModel::~PortsListModel()
{}

//**********************************************************************************************************************
void PortsListModel::setStringList(QStringList &list)
{
    _strings = list;
}

//**********************************************************************************************************************
QStringList PortsListModel::getStringList() const
{
    return _strings;
}

//**********************************************************************************************************************
int PortsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _strings.count();
}

//**********************************************************************************************************************
QVariant PortsListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= _strings.count())
        return QVariant();

    if (role == Qt::DisplayRole)
        return _strings[index.row()];

    return QVariant();
}

//**********************************************************************************************************************
QHash<int, QByteArray> PortsListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    return roles;
}
