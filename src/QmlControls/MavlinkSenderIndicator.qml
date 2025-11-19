/****************************************************************************
 * Toolbar MAVLink Sender Indicator
 ****************************************************************************/

import QtQuick
import QGroundControl
import QGroundControl.Controls
import QGroundControl.MultiVehicleManager
import QGroundControl.ScreenTools
import QGroundControl.Palette

Item {
    id: root
    width: indicatorRow.width
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    property bool showIndicator: true
    property var activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property bool savingPopupVisible: false

    Connections {
        target: activeVehicle

        onGeoStatusChanged: {
            if (!activeVehicle) return

            // Trigger popup when geotagging enters Saving mode
            if (activeVehicle.geoSessionStatus === 3 && !savingPopupVisible) {
                savingPopupVisible = true
                mainWindow.showMessageDialog(
                    "Geotagging in Progress",
                    "Geotagging started for " + activeVehicle.geoPhotoCount + " photos."
                )
            }
            // Close popup if mode leaves Saving
            else if (activeVehicle.geoSessionStatus !== 3) {
                savingPopupVisible = false
            }
        }

        // Completion popup if needed
        onGeoCompletedTriggered: {
            mainWindow.showMessageDialog(
                "Geotagging Complete",
                "Geotagging finished successfully.\n" +
                activeVehicle.geoPhotoCount + " photos processed."
            )
            savingPopupVisible = false
        }
    }

    Row {
        id: indicatorRow
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        spacing: ScreenTools.defaultFontPixelWidth / 2

        // ICON
        QGCColoredImage {
            id: icon
            width: height
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            source: "/qmlimages/CameraIcon.svg"
            fillMode: Image.PreserveAspectFit
            color: {
                if ((!activeVehicle) && (activeVehicle.payloadType !== 1)) return qgcPal.buttonText
                switch (activeVehicle.geoSessionStatus) {
                    case 0:  return "lightgray"   // idle
                    case 1:  return "green"       // init
                    case 2:  return "green"       // running
                    case 3:  return "yellow"      // saving
                    case 100:return "red"         // error
                    default: return qgcPal.buttonText
                }
            }
        }

        // LABEL: ON / OFF / IDLE
        QGCLabel {
            id: statusLabel
            anchors.verticalCenter: parent.verticalCenter
            visible: activeVehicle && activeVehicle.payloadType === 1
            color: icon.color
            text: activeVehicle
                  ? activeVehicle.geoStatusText + " (" + activeVehicle.imageCount + ")"
                  : "--"
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: mainWindow.showIndicatorDrawer(senderPage, root)
    }

    Component {
        id: senderPage
        MavlinkSenderPage { }
    }
}
