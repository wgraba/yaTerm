import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id: root
    visible: true
    width: 640
    height: 480
    title: Qt.application.name

    signal consoleInputEntered(string msg)
    signal connect();
    signal disconnect();
    function inputEntered() {
        consoleInputEntered(consoleInput.text)
        consoleInput.text = ""
    }

    menuBar: MenuBar {
        id: menu
        Menu {
            title: qsTr("&File")

            MenuItem {
                text: qsTr("&Connect")
                enabled: !SimpleTerminal.connState
                onTriggered: root.connect()
            }

            MenuItem {
                text: qsTr("&Disconnect")
                enabled: SimpleTerminal.connState
                onTriggered: root.disconnect()
            }

            MenuItem {
                text: qsTr("&Settings")
            }

            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }

        Menu {
            title: qsTr("&Help")

            MenuItem {
                text: qsTr("&About")
                onTriggered: aboutDialog.open()
            }
        }
    }

    statusBar: StatusBar {
        id: status
        RowLayout {
            Label { text: qsTr(SimpleTerminal.statusText) }
        }
    }

    TextField {
        id: consoleInput

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Keys.onEnterPressed: root.inputEntered()
        Keys.onReturnPressed: root.inputEntered()

        KeyNavigation.tab: consoleOutput
        focus: true
    }

    TextArea {
        id: consoleOutput
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: consoleInput.top
        anchors.top: parent.top

        KeyNavigation.tab: consoleInput

        text: SimpleTerminal.displayText
        readOnly: true
        textFormat: TextEdit.RichText

    }

    MessageDialog {
        id: aboutDialog
        icon: StandardIcon.Information
        modality: Qt.ApplicationModal
        text: { "<p><strong>y</strong>et <strong>a</strong>nother Serial <strong>Term</strong>inal " +
                Qt.application.version +
                "<p><em>by Wesley Graba</em></p>" +
                "<p>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, " +
                "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>" }
        title: "About " + Qt.application.name
    }
}


