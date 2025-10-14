/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.ScreenTools
import QGroundControl.Palette

GridLayout {
    columns:        2
    rowSpacing:     _rowSpacing
    columnSpacing:  _colSpacing

    function saveSettings() {
       //no need
    }

    QGCLabel { text: qsTr("Dev file") }
    QGCComboBox {
        property var ttysPaths: ["/dev/ttyHS1", "/dev/ttyHS5"]
        id:ttysFileCombo
        Layout.preferredWidth:  _secondColumnWidth
        model:                  ["DataLink1", "DataLink2"]
        onActivated:
        {
            subEditConfig.devFile = ttysPaths[ttysFileCombo.currentIndex]
        }
        Component.onCompleted: {
            let index = ttysPaths.indexOf(subEditConfig.devFile)
            if (index < 0) index = 0
            ttysFileCombo.currentIndex = index
            subEditConfig.devFile = ttysPaths[ttysFileCombo.currentIndex]
        }
    }

    QGCLabel { text: qsTr("Baud Rate") }
    QGCComboBox {
        id:baudCombo
        Layout.preferredWidth:  _secondColumnWidth
        model:                  ["9600","57600", "115200","230400"]
        onActivated: {
            subEditConfig.baudRate = model[baudCombo.currentIndex]
        }
        Component.onCompleted: {
            if (subEditConfig.baudRate === "") subEditConfig.baudRate = model[2]
            baudCombo.currentIndex = model.indexOf(subEditConfig.baudRate)
        }
    }
}
