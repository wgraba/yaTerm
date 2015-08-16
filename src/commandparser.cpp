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

#include "commandparser.h"
#include "simpleterminal.h"

#include <QApplication>

//**********************************************************************************************************************
const QMap<QString, CommandParser::CmdFunc> CommandParser::cmdMap = {
    { "/clear", CommandParser::cmdClear },
    { "/connect", CommandParser::cmdConnect },
    { "/disconnect", CommandParser::cmdDisconnect },
    { "/help", CommandParser::cmdHelp },
    { "/quit", CommandParser::cmdQuit },
};

//**********************************************************************************************************************
// @todo: Combine cmdHelpMap and cmdMap?
const QMap<QString, QStringList> CommandParser::cmdHelpMap = {
    { "/clear", { "", "Clear the screen" } },
    { "/connect", { "portName", "Connect to port" } },
    { "/disconnect", { "", "Disconnect from port" } },
    { "/help", { "[command]", "Get help" } },
    { "/quit", { "", "Quit" } },
};

//**********************************************************************************************************************
CommandParser::CommandParser(SimpleTerminal &terminal)
    : _terminal(terminal)
{

}

//**********************************************************************************************************************
void CommandParser::cmdClear(SimpleTerminal &st, const QStringList &)
{
    emit st.clearDisplayText();
}

//**********************************************************************************************************************
void CommandParser::cmdConnect(SimpleTerminal &st, const QStringList &args)
{
    if (args.size() > 0)
    {
        st.disconnect();
        st.setPort(args[0]);
        st.connect();
    }
    else
    {
        st.setError("Wrong number of arguments");
    }
}

//**********************************************************************************************************************
void CommandParser::cmdDisconnect(SimpleTerminal &st, const QStringList &)
{
    st.disconnect();
}

//**********************************************************************************************************************
void CommandParser::cmdQuit(SimpleTerminal &st, const QStringList &)
{
    st.disconnect();
    QApplication::quit();
}

//**********************************************************************************************************************
void CommandParser::cmdHelp(SimpleTerminal &st, const QStringList &args)
{
    QString rspStr;
    if (args.size() > 0)
    {
        if (cmdHelpMap.contains(args[0]))
        {
            QStringList help = cmdHelpMap.value(args[0]);

            rspStr.append("<span style=\"color: BlueViolet;\">Usage</span>: " + args[0] + " <i>" + help[0] +
                    "</i><br><br>");
            rspStr.append(help[1]);
        }
        else
            st.setError("Unknown command");
    }
    else
    {
        QMap<QString, QStringList>::const_iterator cmd = cmdHelpMap.constBegin();
        bool isFirst = true; // @todo: Find a better way
        while (cmd != cmdHelpMap.constEnd())
        {
            QStringList help = cmd.value();
            QString cmdName = cmd.key();

            if (isFirst)
                isFirst = false;
            else
                rspStr.append("<br>");

            rspStr.append("<span style=\"color: BlueViolet;\">" + cmdName + "</span>: " + help[1]);

            ++cmd;
        }
    }

    if (rspStr.length() > 0)
        st.modifyDspText(SimpleTerminal::DspType::COMMAND_RSP, rspStr);
}

//**********************************************************************************************************************
void CommandParser::processCommand(const QString &cmd)
{
    qDebug() << "Command: " << cmd;

    if (cmd.length() < 1)
        return; // Do nothing if there is no command

    _terminal.modifyDspText(SimpleTerminal::DspType::COMMAND, cmd);

    QStringList cmdList = cmd.split(' '); // Command is first item, parameters are what's left
    QString cmdName = cmdList[0];
    cmdList.removeFirst();
    QStringList &args = cmdList;
    if (cmdMap.contains(cmdName))
    {
        CmdFunc cmdFunc = (cmdMap.value(cmdName));

        Q_CHECK_PTR(cmdFunc);
        cmdFunc(_terminal, args);
    }
    else
    {
        _terminal.setError("Invalid command");
    }
}
