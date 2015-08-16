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

#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QMap>
#include <QString>
#include <QStringList>

//**********************************************************************************************************************
class SimpleTerminal;

//**********************************************************************************************************************
class CommandParser
{
public:
    explicit CommandParser(SimpleTerminal &terminal);
    ~CommandParser() {}

    void processCommand(const QString &cmd);

private:
    SimpleTerminal &_terminal;

    typedef void (*CmdFunc)(SimpleTerminal &, const QStringList &);
    static const QMap<QString, CmdFunc> cmdMap;
    static const QMap<QString, QStringList> cmdHelpMap;

    // Commands
    static void cmdClear(SimpleTerminal &st, const QStringList &);
    static void cmdConnect(SimpleTerminal &st, const QStringList &args);
    static void cmdDisconnect(SimpleTerminal &st, const QStringList &);
    static void cmdQuit(SimpleTerminal &st, const QStringList &);
    static void cmdHelp(SimpleTerminal &st, const QStringList &args);
};

#endif // COMMANDPARSER_H
