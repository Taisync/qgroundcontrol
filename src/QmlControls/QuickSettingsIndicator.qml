/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * Quick Settings Indicator
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.MultiVehicleManager
import QGroundControl.ScreenTools
import QGroundControl.Palette
import QGroundControl.FactSystem
import QGroundControl.FactControls

Item {
    id:             control
    width:          quickSettingsIndicatorRow.width
    anchors.top:    parent.top
    anchors.bottom: parent.bottom

    property bool   showIndicator:          true

    property var    _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle

    FactPanelController { id: controller }

    // Pull relevant Facts
    // property Fact _wpnavSpeedFact: controller.getParameterFact(-1, "WPNAV_SPEED", false)
    // property Fact _rtlAltFact:     controller.getParameterFact(-1, "RTL_ALT", false)
    property Fact _wpnavSpeedFact
    property Fact _rtlAltFact

    // 🔹 Fetch facts after vehicle connection
    Connections {
        target: QGroundControl.multiVehicleManager
        onActiveVehicleChanged: {
            if (QGroundControl.multiVehicleManager.activeVehicle) {
                _wpnavSpeedFact = controller.getParameterFact(-1, "WPNAV_SPEED", false)
                _rtlAltFact = controller.getParameterFact(-1, "RTL_ALT", false)
            } else {
                _wpnavSpeedFact = undefined
                _rtlAltFact = undefined
            }
        }
    }

    // Also handle case where component loads *after* vehicle is already active
    Component.onCompleted: {
        if (_activeVehicle) {
            _wpnavSpeedFact = controller.getParameterFact(-1, "WPNAV_SPEED", false)
            _rtlAltFact = controller.getParameterFact(-1, "RTL_ALT", false)
        }
    }

    Row {
        id:             quickSettingsIndicatorRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom
        spacing:        ScreenTools.defaultFontPixelWidth / 2

        Row {
            anchors.top:    parent.top
            anchors.bottom: parent.bottom
            spacing:        -ScreenTools.defaultFontPixelWidth / 2

            QGCColoredImage {
                id:                 quickSettingsIcon
                width:              height
                anchors.top:        parent.top
                anchors.bottom:     parent.bottom
                source:             "/qmlimages/Gears.svg"
                fillMode:           Image.PreserveAspectFit
                sourceSize.height:  height
                color:              qgcPal.buttonText
            }
        }

        Column {
            id:                     quickSettingsValuesColumn
            anchors.verticalCenter: parent.verticalCenter
            spacing:                0

            // Waypoint Speed
            QGCLabel {
                id:     wpnavSpeedLabel
                color:  qgcPal.buttonText
                text:   "Speed"
                font.pointSize: ScreenTools.smallFontPointSize
            }

            // RTL Altitude
            QGCLabel {
                id:     rtlAltLabel
                color:  qgcPal.buttonText
                text:   "RTL"
                font.pointSize: ScreenTools.smallFontPointSize
            }
        }
    }

    // Tap to open page
    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showIndicatorDrawer(quickSettingsIndicatorPage, control)
    }

    Component {
        id: quickSettingsIndicatorPage
        QuickSettingsIndicatorPage { }
    }
}
