/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.MultiVehicleManager
import QGroundControl.ScreenTools
import QGroundControl.Palette
import QGroundControl.NTRIP 1.0

// Used as the base class control for nboth VehicleGPSIndicator and RTKGPSIndicator

Item {
    id:             control
    width:          gpsIndicatorRow.width
    anchors.top:    parent.top
    anchors.bottom: parent.bottom

    property var    _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property bool   _rtkConnected:  QGroundControl.gpsRtk.connected.value
    //property var    _ntrip:         QGroundControl.ntrip

    Row {
        id:             gpsIndicatorRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom
        spacing:        ScreenTools.defaultFontPixelWidth / 2

        Row {
            anchors.top:    parent.top
            anchors.bottom: parent.bottom
            spacing:        -ScreenTools.defaultFontPixelWidth / 2

            QGCLabel {
                id:                     gpsLabel
                rotation:               90
                text:                   qsTr("RTK")
                color:                  qgcPal.buttonText
                anchors.verticalCenter: parent.verticalCenter
                visible:                _rtkConnected
            }

            QGCColoredImage {
                id:                 gpsIcon
                width:              height
                anchors.top:        parent.top
                anchors.bottom:     parent.bottom
                //source:             "/qmlimages/Gps.svg"
                source: {
                    const lock = _activeVehicle ? _activeVehicle.gps.lock.rawValue : 0
                    const ntripEnabled = NTRIP.masterEnable && NTRIP.enabled
                    const ntripConnected = NTRIP.connectionStatus === 2

                    if (lock === 6) { // RTK Fixed
                        if (!ntripEnabled) return "/qmlimages/RTK-fixed.svg"
                        return ntripConnected ? "/qmlimages/RTK-fixed-ntrip-good.svg" : "/qmlimages/RTK-fixed-ntrip-bad.svg"
                    }
                    if (lock === 5) { // RTK Float
                        if (!ntripEnabled) return "/qmlimages/RTK-float.svg"
                        return ntripConnected ? "/qmlimages/RTK-float-ntrip-good.svg" : "/qmlimages/RTK-float-ntrip-bad.svg"
                    }
                    // Normal GPS (lock != 5,6)
                    if (!ntripEnabled) return "/qmlimages/Gps.svg"
                    return ntripConnected ? "/qmlimages/Gps-ntrip-good.svg" : "/qmlimages/Gps-ntrip-bad.svg"
                }
                color: {
                    const ntripEnabled = NTRIP.masterEnable && NTRIP.enabled
                    const ntripConnected = NTRIP.connectionStatus === 2
                    if (ntripEnabled) {
                        return ntripConnected ? "green" : "red"
                    }
                    return qgcPal.buttonText
                }
                fillMode:           Image.PreserveAspectFit
                sourceSize.height:  height
                opacity:            (_activeVehicle && _activeVehicle.gps.count.value >= 0) ? 1 : 0.5
                //color:              qgcPal.buttonText
            }
        }

        Column {
            id:                     gpsValuesColumn
            anchors.verticalCenter: parent.verticalCenter
            visible:                _activeVehicle && !isNaN(_activeVehicle.gps.hdop.value)
            spacing:                0

            QGCLabel {
                anchors.horizontalCenter:   hdopValue.horizontalCenter
                color:              qgcPal.buttonText
                text:               _activeVehicle ? _activeVehicle.gps.count.valueString : ""
            }

            QGCLabel {
                id:     hdopValue
                color:  qgcPal.buttonText
                text:   _activeVehicle ? _activeVehicle.gps.hdop.value.toFixed(1) : ""
            }
        }
    }

    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showIndicatorDrawer(gpsIndicatorPage, control)
    }

    Component {
        id: gpsIndicatorPage

        GPSIndicatorPage { }//{ ntrip: _ntrip }
    }
}
