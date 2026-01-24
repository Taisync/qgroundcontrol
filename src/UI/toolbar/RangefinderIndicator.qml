/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
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

//-------------------------------------------------------------------------
//-- Rangefinder (Downward Distance Sensor) Indicator
Item {
    id:             control
    width:          contentRow.width * 1.1
    anchors.top:    parent.top
    anchors.bottom: parent.bottom

    property bool showIndicator: _activeVehicle && _showDownRangefinder.rawValue

    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property var    _showDownRangefinder:   QGroundControl.settingsManager.flyViewSettings.showDownRangefinder
    property var    _distanceSensors:   _activeVehicle ? _activeVehicle.distanceSensors : null
    property real   _distance:          _distanceSensors ? _distanceSensors.rotationPitch270.rawValue : NaN
    property bool   _isActive:          !isNaN(_distance)
    property string _distanceStr:       _distanceSensors ? _distanceSensors.rotationPitch270.valueString : "--.--"
    property string _units:             _distanceSensors && _distanceSensors.rotationPitch270.units ? _distanceSensors.rotationPitch270.units : "m"

    // Determine status color based on distance and activity
    function getStatusColor() {
        if (!_isActive) {
            return qgcPal.colorGrey  // Inactive/no data
        }
        if (_distance < 1.0) {
            return qgcPal.colorRed  // Very close - warning
        }
        if (_distance < 3.0) {
            return qgcPal.colorOrange  // Getting close
        }
        return qgcPal.colorGreen  // Normal operating range
    }

    Component {
        id: rangefinderInfoPage

        ToolIndicatorPage {
            showExpand: false

            contentComponent: SettingsGroupLayout {
                heading: qsTr("Rangefinder Status")

                LabelledLabel {
                    label:      qsTr("Status")
                    labelText:  _isActive ? qsTr("Active") : qsTr("No Data")
                }

                LabelledLabel {
                    label:      qsTr("Distance")
                    labelText:  _isActive ? _distanceStr + " " + _units : qsTr("--")
                }
            }
        }
    }

    Row {
        id:             contentRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom
        spacing:        ScreenTools.defaultFontPixelWidth * 0.5

        // Rangefinder icon - using terrain icon to represent ground distance
        QGCColoredImage {
            id:                     rangefinderIcon
            width:                  height
            anchors.top:            parent.top
            anchors.bottom:         parent.bottom
            sourceSize.height:      height
            source:                 "/res/terrain.svg"
            fillMode:               Image.PreserveAspectFit
            color:                  getStatusColor()
            opacity:                _isActive ? 1.0 : 0.5
        }

        // Distance value text
        QGCLabel {
            anchors.verticalCenter: parent.verticalCenter
            text:                   _isActive ? _distanceStr : "--"
            color:                  getStatusColor()
            font.pointSize:         ScreenTools.mediumFontPointSize
        }
    }

    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showIndicatorDrawer(rangefinderInfoPage, control)
    }
}