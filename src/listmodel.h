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
#ifndef LISTMODEL_H
#define LISTMODEL_H

#include <QAbstractListModel>
#include <QList>

//**********************************************************************************************************************
template <class T>
class ListModel : public QAbstractListModel
{
public:
    explicit ListModel();
    ListModel(const ListModel<T> &listModel);

    ~ListModel();

    void setList(QList<T> &list);
    QList<T> getList() const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

protected:
    virtual QHash<int, QByteArray> roleNames() const;

private:
    QList<T> _list;

};

//**********************************************************************************************************************
template <class T>
ListModel<T>::ListModel() :
    QAbstractListModel()
{}

//**********************************************************************************************************************
template <class T>
ListModel<T>::ListModel(const ListModel<T> &listModel) :
    QAbstractListModel()
{
    _list = listModel._list;
}

//**********************************************************************************************************************
template <class T>
ListModel<T>::~ListModel()
{}

//**********************************************************************************************************************
template <class T>
void ListModel<T>::setList(QList<T> &list)
{
    _list = list;
}

//**********************************************************************************************************************
template <class T>
QList<T> ListModel<T>::getList() const
{
    return _list;
}

//**********************************************************************************************************************
template <class T>
int ListModel<T>::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return _list.count();
}

//**********************************************************************************************************************
template <class T>
QVariant ListModel<T>::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= _list.count())
        return QVariant();

    if (role == Qt::DisplayRole)
        return _list[index.row()];

    return QVariant();
}

//**********************************************************************************************************************
template <class T>
QHash<int, QByteArray> ListModel<T>::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    return roles;
}

#endif // LISTMODEL_H
