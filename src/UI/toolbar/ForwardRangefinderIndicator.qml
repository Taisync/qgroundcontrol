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
//-- Forward Rangefinder (Forward Distance Sensor) Indicator
Item {
    id:             control
    width:          contentRow.width * 1.1
    anchors.top:    parent.top
    anchors.bottom: parent.bottom

    property bool showIndicator: _activeVehicle && _showForwardRangefinder.rawValue

    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property var    _showForwardRangefinder: QGroundControl.settingsManager.flyViewSettings.showForwardRangefinder
    property var    _distanceSensors:       _activeVehicle ? _activeVehicle.distanceSensors : null
    property real   _distance:              _distanceSensors ? _distanceSensors.rotationNone.rawValue : NaN
    property bool   _isActive:              !isNaN(_distance)

    // RC Channel monitoring for rangefinder enable/disable state
    property var    _rcChannelSetting:      QGroundControl.settingsManager.flyViewSettings.forwardRangefinderRCChannel
    property int    _rcChannelNumber:       _rcChannelSetting ? _rcChannelSetting.rawValue : 0  // 1-based channel number (0 = disabled)
    property var    _rcChannelValues:       _activeVehicle ? _activeVehicle.rcChannelValues : []
    property int    _rcChannelValue:        (_rcChannelNumber > 0 && _rcChannelValues.length >= _rcChannelNumber) ? _rcChannelValues[_rcChannelNumber - 1] : -1
    property bool   _rcMonitoringEnabled:   _rcChannelNumber > 0  // RC monitoring is enabled
    property bool   _rcDataValid:           _rcMonitoringEnabled && _rcChannelValue > 0  // Have valid RC data
    property bool   _rcEnabled:             _rcChannelValue > 1500  // Above 1500 = enabled

    // Unit conversion for system units
    property var    _unitsConversion:       QGroundControl.unitsConversion
    property real   _displayDistance:       _isActive ? _unitsConversion.metersToAppSettingsHorizontalDistanceUnits(_distance) : NaN
    property string _displayDistanceStr:    _isActive ? _displayDistance.toFixed(1) : "--"
    property string _units:                 _unitsConversion.appSettingsHorizontalDistanceUnitsString

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

    // Determine RC enable/disable indicator color
    function getRCStatusColor() {
        if (!_rcDataValid) {
            return "transparent"      // No RC data available
        }
        return _rcEnabled ? qgcPal.colorGreen : qgcPal.colorGrey
    }

    // Determine combined status text
    function getStatusText() {
        if (!_isActive) {
            return qsTr("No Data")
        }
        // TODO: Add "Error" status when rangefinder error detection is available
        if (_rcDataValid && _rcEnabled) {
            return qsTr("Active")
        }
        if (_rcDataValid && !_rcEnabled) {
            return qsTr("Inactive")
        }
        return qsTr("Active")  // No RC monitoring, just show active if we have data
    }

    Component {
        id: forwardRangefinderInfoPage

        ToolIndicatorPage {
            showExpand: false

            contentComponent: SettingsGroupLayout {
                heading: qsTr("Forward Rangefinder Status")

                LabelledLabel {
                    label:      qsTr("Status")
                    labelText:  getStatusText()
                }

                LabelledLabel {
                    label:      qsTr("Distance (%1)").arg(_unitsConversion.appSettingsHorizontalDistanceUnitsString)
                    labelText:  _isActive ? _displayDistanceStr : qsTr("--")
                }
            }
        }
    }

    Row {
        id:             contentRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom
        spacing:        ScreenTools.defaultFontPixelWidth * 0.5

        // Container for icon with RC status indicator
        Item {
            width:                  rangefinderIcon.width + (rcStatusDot.visible ? rcStatusDot.width * 0.5 : 0)
            anchors.top:            parent.top
            anchors.bottom:         parent.bottom

            // Rangefinder icon - using radar icon to represent forward obstacle detection
            QGCColoredImage {
                id:                     rangefinderIcon
                width:                  height
                anchors.top:            parent.top
                anchors.bottom:         parent.bottom
                sourceSize.height:      height
                source:                 "/InstrumentValueIcons/radar.svg"
                fillMode:               Image.PreserveAspectFit
                color:                  (_rcDataValid && !_rcEnabled) ? qgcPal.colorGrey : getStatusColor()
                opacity:                (_isActive && (!_rcDataValid || _rcEnabled)) ? 1.0 : 0.5
            }

            // RC status indicator dot (bottom-right corner of icon)
            Rectangle {
                id:                     rcStatusDot
                width:                  ScreenTools.defaultFontPixelHeight * 0.6
                height:                 width
                radius:                 width / 2
                color:                  getRCStatusColor()
                visible:                _rcDataValid
                anchors.bottom:         rangefinderIcon.bottom
                anchors.right:          rangefinderIcon.right
                anchors.bottomMargin:   -height * 0.1
                anchors.rightMargin:    -width * 0.1

                // Border for better visibility
                border.width:           1
                border.color:           "white"
            }
        }

        // Distance value text
        QGCLabel {
            anchors.verticalCenter: parent.verticalCenter
            text:                   _isActive ? _displayDistanceStr : "--"
            color:                  (_rcDataValid && !_rcEnabled) ? qgcPal.colorGrey : getStatusColor()
            font.pointSize:         ScreenTools.mediumFontPointSize
        }
    }

    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showIndicatorDrawer(forwardRangefinderInfoPage, control)
    }
}
