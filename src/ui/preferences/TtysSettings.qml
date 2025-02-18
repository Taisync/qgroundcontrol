/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts  1.2

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0

GridLayout {
    columns:        2
    rowSpacing:     _rowSpacing
    columnSpacing:  _colSpacing

    function saveSettings() {
       //no need
    }

    QGCLabel { text: qsTr("dev file") }
    QGCComboBox {
        id:ttysFileCombo
        Layout.preferredWidth:  _secondColumnWidth
        model:                  ["DataLink1", "DataLink2" ]
        onActivated:
        {
            if(ttysFileCombo.currentIndex == 0)
            {
                subEditConfig.changDevFile("/dev/ttyHS0")
            }
            if(ttysFileCombo.currentIndex == 1)
            {
                subEditConfig.changDevFile("/dev/ttyHS2")
            }
        }
        Component.onCompleted: {
            if(subEditConfig.devFile.toString().includes("/dev/ttyHS0"))
                ttysFileCombo.currentIndex = 0;
            else
                ttysFileCombo.currentIndex = 1;
        }
    }

    QGCLabel { text: qsTr("Baud Rate") }
    QGCComboBox {
        id:baudCombo
        Layout.preferredWidth:  _secondColumnWidth
        model:                  ["9600","57600", "115200","230400"]
        onActivated: {
            switch(baudCombo.currentIndex)
            {
                case 0:
                    subEditConfig.changBaudRate("9600");
                    break;
                case 1:
                    subEditConfig.changBaudRate("57600");
                    break;
                case 2:
                    subEditConfig.changBaudRate("115200");
                    break;
                case 3:
                    subEditConfig.changBaudRate("230400");
                    break;
            }
        }
        Component.onCompleted: {
            if(subEditConfig.baudRate.toString().includes("9600"))
                baudCombo.currentIndex = 0;
            if(subEditConfig.baudRate.toString().includes("57600"))
                baudCombo.currentIndex = 1;
            if(subEditConfig.baudRate.toString().includes("115200"))
                baudCombo.currentIndex = 2;
            if(subEditConfig.baudRate.toString().includes("230400"))
                baudCombo.currentIndex = 3;
        }
    }
}
