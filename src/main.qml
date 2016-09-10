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

import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 700
    height: 480

    title: Qt.application.name

    signal consoleInputEntered(string msg)
    signal connect();
    signal disconnect();
    signal newPort(string port);
    signal settingsChanged();

    function inputEntered() {
        consoleInputEntered(consoleInput.text)
        consoleInput.text = ""
    }

    Settings {
        category: "ApplicationWindow"
        property alias x: root.x
        property alias y: root.y
        property alias width: root.width
        property alias height: root.height
    }

    menuBar: MenuBar {
        id: menu
        Menu {
            title: qsTr("&File")

            MenuItem {
                text: { simpleTerminal.connState ? qsTr("&Disconnect") : qsTr("&Connect") }
                onTriggered: { simpleTerminal.connState ? root.disconnect() : root.connect()}
            }

            MenuItem {
                text: qsTr("&Settings...")
                onTriggered: settingsDialog.open()
                enabled: !simpleTerminal.connState
            }

            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }

        Menu {
            title: qsTr("&View")

            MenuItem {
                text: qsTr("&Autoscroll")
                onTriggered: { consoleOutput.autoscroll = !consoleOutput.autoscroll }
                checked: consoleOutput.autoscroll
                checkable: true
            }

            MenuItem {
                text: qsTr("&Clear")
                onTriggered: {
                    consoleOutput.remove(0, consoleOutput.length)
                }
            }

            MenuItem {
                text: qsTr("&Font...")
                onTriggered: {
                    fontDialog.font = consoleOutput.font
                    fontDialog.open()
                }
            }

//            MenuItem {
//                text: qsTr("&Source")
//                onTriggered: {
//                    consoleOutput.textFormat = (consoleOutput.textFormat == TextEdit.RichText ?
//                                                    TextEdit.PlainText : TextEdit.RichText)
//                    consoleInput.readOnly = (consoleOutput.textFormat == TextEdit.PlainText)
//                }
//                checked: (consoleOutput.textFormat == TextEdit.PlainText)
//                checkable: true
//            }

            MenuItem {
                text : qsTr("&Wrap")
                onTriggered: {
                    consoleOutput.wrapMode = consoleOutput.wrapMode == TextEdit.NoWrap ?
                                TextEdit.WrapAtWordBoundaryOrAnywhere : TextEdit.NoWrap
                }
                checked: consoleOutput.wrapMode == TextEdit.WrapAtWordBoundaryOrAnywhere
                checkable: true
            }

        }

        Menu {
            title: qsTr("&Help")

            MenuItem {
                text: qsTr("&About...")
                onTriggered: aboutDialog.open()
            }

            MenuItem {
                text: qsTr("&Manual...")
                enabled: false
            }
        }
    }

    statusBar: StatusBar {
        id: status
        RowLayout {
            anchors.fill: parent
            Label {
                id: notification
                text: qsTr(simpleTerminal.statusText)
                horizontalAlignment: Text.AlignLeft
                font.wordSpacing: 5.0
            }

            Label {
                id: error
                color: "red"
                text: qsTr(simpleTerminal.errorText)
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    TextField {
        id: consoleInput

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Keys.onEnterPressed: root.inputEntered()
        Keys.onReturnPressed: root.inputEntered()
        Keys.onUpPressed: { text = simpleTerminal.getPrevHistory() }
        Keys.onDownPressed: { text = simpleTerminal.getNextHistory() }
        Keys.onEscapePressed: {
            text = ""
            simpleTerminal.resetHistoryIdx()
        }

        KeyNavigation.tab: consoleOutput
        focus: true
        font: consoleOutput.font
    }

    TextArea {
        id: consoleOutput

        property bool autoscroll: true

        menu: null

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: consoleInput.top
        anchors.top: parent.top

        KeyNavigation.tab: consoleInput

        Connections {
            target: simpleTerminal
            onInsertDisplayText: {
                if (consoleOutput.length > simpleTerminal.maxDspTxtChars) {
                    consoleOutput.remove(0, consoleOutput.length - simpleTerminal.maxDspTxtChars)
                }

                consoleOutput.insert(consoleOutput.length, text)
                if (consoleOutput.autoscroll) {
                    consoleOutput.cursorPosition = consoleOutput.length
                }
            }

            onAppendDisplayText: {
                if (consoleOutput.length > simpleTerminal.maxDspTxtChars) {
                    consoleOutput.remove(0, consoleOutput.length - simpleTerminal.maxDspTxtChars)
                }

                consoleOutput.append(text)
                if (consoleOutput.autoscroll) {
                    consoleOutput.cursorPosition = consoleOutput.length
                }
            }

            onClearDisplayText: {
                consoleOutput.remove(0, consoleOutput.length)
            }
        }

        readOnly: true
        textFormat: TextEdit.RichText
        wrapMode: TextEdit.WrapAtWordBoundaryOrAnywhere

        font.family: "Courier New"
        font.pointSize: 10
        font.weight: Font.Normal

        Settings {
            category: "ConsoleOutput"
            property alias fontFamily: consoleOutput.font.family
            property alias fontPointSize: consoleOutput.font.pointSize
            property alias fontWeight: consoleOutput.font.weight
        }

    }

    MessageDialog {
        id: aboutDialog
        icon: StandardIcon.Information
        modality: Qt.ApplicationModal
        text: { "<p><strong>y</strong>et <strong>a</strong>nother Serial "+ "<strong>Term</strong>inal " +
                Qt.application.version + "</p>" +
                "<p><em>by Wesley Graba</em></p>" +
                "<p><a href=\"https://github.com/wgraba/yaTerm/blob/master/LICENSE.md\">License</a></p>" +
                "<p><a href=\"https://github.com/wgraba/yaTerm\">GitHub project</a></p>" +
                "<p>The program is provided AS IS with NO WARRANTY OF ANY KIND.</p>" }
        title: qsTr("About ") + Qt.application.name
    }

    Dialog {
        id: settingsDialog
        modality: Qt.WindowModal
        standardButtons: StandardButton.Ok | StandardButton.Cancel
        title: qsTr("Settings")
        width: settingsLayout.width + 50

        onVisibleChanged: {
            if (visible) {
                // Ports
                portCombo.currentIndex = portCombo.find(simpleTerminal.getPortName())

                // Baud Rate
                baudRateCombo.currentIndex = baudRateCombo.find((serialPort.baudRate).toString())

                // Data Bits
                dataBitsCombo.currentIndex = dataBitsCombo.find((serialPort.dataBits).toString())

                // Parity
                switch (serialPort.parity)
                {
                    default:
                    case 0:
                        parityCombo.currentIndex = 0
                        break

                    case 2:
                        parityCombo.currentIndex = 1
                        break

                    case 3:
                        parityCombo.currentIndex = 2
                        break

                }

                // Stop bits
                switch (serialPort.stopBits)
                {
                    default:
                    case 1:
                        stopCombo.currentIndex = 0
                        break

                    case 3:
                        stopCombo.currentIndex = 1
                        break

                    case 2:
                        stopCombo.currentIndex = 2
                        break
                }

                // Flow control
                switch (serialPort.flowControl)
                {
                    default:
                    case 0:
                        flowCombo.currentIndex = 0
                        break

                    case 1:
                        flowCombo.currentIndex = 1
                        break

                    case 2:
                        flowCombo.currentIndex = 2
                        break
                }

                // EOM
                switch (simpleTerminal.eom)
                {
                    default:
                    case "\r":
                        eomCombo.currentIndex = 0
                        break;

                    case "\n":
                        eomCombo.currentIndex = 1
                        break;

                    case "\r\n":
                        eomCombo.currentIndex = 2
                        break;

                    case "":
                        eomCombo.currentIndex = 3
                }

            }
        }

        onAccepted: {
            console.log("Applying new settings: " + portCombo.currentText + " " + baudRateCombo.currentText + " " +
                        dataBitsCombo.currentText + " " + parityCombo.currentText + " " + stopCombo.currentText + " " +
                        flowCombo.currentText + " " + somCombo.currentText + " " + eomCombo.currentText)

            // Baud rate
            serialPort.baudRate = baudRateCombo.currentText

            // Data bits
            serialPort.dataBits = "Data" + dataBitsCombo.currentText

            // Parity
            switch (parityCombo.currentIndex)
            {
                case 0:
                    serialPort.parity = "NoParity"
                    break;

                case 1:
                    serialPort.parity = "EvenParity"
                    break;

                case 2:
                    serialPort.parity = "OddParity"
                    break;

                 default:
                     serialPort.parity = "UnknownParity"
                     break;
            }

            // Stop bits
            switch (stopCombo.currentIndex)
            {
                case 0:
                    serialPort.stopBits = "OneStop"
                    break;

                case 1:
                    serialPort.stopBits = "OneAndHalfStop"
                    break;

                case 2:
                    serialPort.stopBits = "TwoStop"
                    break;

                default:
                    serialPort.stopBits = "UnknownStopBits"
                    break;
            }

            // Flow control
            switch (flowCombo.currentIndex)
            {
                case 0:
                    serialPort.flowControl = "NoFlowControl"
                    break;

                case 1:
                    serialPort.flowControl = "HardwareControl"
                    break;

                case 2:
                    serialPort.flowControl = "SoftwareControl"
                    break;

                default:
                    serialPort.flowControl = "UnknownFlowControl"
                    break;
            }
            // SOM
            switch (somCombo.currentIndex)
            {
                default:
                case 0:
                    simpleTerminal.som = "";
                    break;

                case 1:
                    simpleTerminal.som = "@";
                    break;

                case 2:
                    simpleTerminal.som = "#"
                    break;
            }

            // EOM
            switch (eomCombo.currentIndex)
            {
                default:
                case 0:
                    simpleTerminal.eom = "\r";
                    break;

                case 1:
                    simpleTerminal.eom = "\n";
                    break;

                case 2:
                    simpleTerminal.eom = "\r\n";
                    break;

                case 3:
                    simpleTerminal.eom = "";
                    break;
            }


            // Port
            newPort(portCombo.currentText)

            settingsChanged()
        }

        GridLayout {
            id: settingsLayout
            columns: 2

            Label { text: "<strong>Port</strong>" }
            ComboBox {
                id: portCombo
                model: portsListModel
            }

            Label { text: "<strong>Baud Rate</strong>" }
            ComboBox {
                id: baudRateCombo
                model: baudListModel
                currentIndex: { baudListModel.count - 1}
            }

            Label { text: "<strong>Data Bits</strong>" }
            ComboBox {
                id: dataBitsCombo
                model: [5, 6, 7, 8]
                currentIndex: 3
            }

            Label { text: "<strong>Parity</strong>" }
            ComboBox {
                id: parityCombo
                model: ["None", "Even", "Odd"]
                currentIndex: 0
            }

            Label { text: "<strong>Stop Bits</strong>" }
            ComboBox {
                id: stopCombo
                model: [1, 1.5, 2]
                currentIndex: 0
            }

            Label { text: "<strong>Flow Control</strong>" }
            ComboBox {
                id: flowCombo
                model: ["None", "Hardware", "Software"]
                currentIndex: 0
            }

            Label { text: "<strong>Start-of-Message Prefix</strong>" }
            ComboBox {
                id: somCombo
                model: ["None", "@", "#"]
                currentIndex: 0
            }

            Label { text: "<strong>End-of-Message Terminator</strong>" }
            ComboBox {
                id: eomCombo
                model: ["CR", "LF", "CR+LF", "None"]
                currentIndex: 0
            }
        }
    }

    FontDialog {
        id: fontDialog
        modality: Qt.ApplicationModal
        title: qsTr("Font")

        monospacedFonts: true
        nonScalableFonts: false
        proportionalFonts: false
        scalableFonts: false

        onAccepted: {
            // The font weight should do nothing
            consoleOutput.font.family = font.family
            consoleOutput.font.pointSize = font.pointSize
        }
    }
}




