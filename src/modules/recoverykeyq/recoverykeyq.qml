/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   ShedOS LUKS recovery-key escrow screen. `config` is the C++ Config
 *   object exposed by RecoveryKeyViewStep.
 */
import io.calamares.ui 1.0

import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {
    width: 740
    height: 420

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Label {
            Layout.fillWidth: true
            font.pointSize: 16
            font.bold: true
            wrapMode: Text.WordWrap
            text: config.encrypted ? qsTr("Save your recovery key")
                                   : qsTr("Encryption is disabled")
        }

        Label {
            Layout.fillWidth: true
            visible: !config.encrypted
            wrapMode: Text.WordWrap
            text: qsTr("This installation will not be encrypted, so there is no recovery key. You can continue.")
        }

        Label {
            Layout.fillWidth: true
            visible: config.encrypted
            wrapMode: Text.WordWrap
            text: qsTr("If you forget your passphrase, this recovery key is the only other way to unlock this computer. Write it down and keep it somewhere safe, separate from this machine. If you lose both the passphrase and this key, the data is gone for good.")
        }

        TextField {
            Layout.fillWidth: true
            Layout.topMargin: 8
            visible: config.encrypted
            readOnly: true
            horizontalAlignment: TextInput.AlignHCenter
            font.family: "monospace"
            font.pointSize: 18
            text: config.recoveryKey
        }

        CheckBox {
            Layout.topMargin: 8
            visible: config.encrypted
            checked: config.acknowledged
            text: qsTr("I have written down my recovery key")
            onToggled: config.acknowledged = checked
        }

        Item { Layout.fillHeight: true }
    }
}
