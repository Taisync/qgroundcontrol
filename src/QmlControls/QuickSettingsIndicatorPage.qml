/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * Quick Settings Indicator Page
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

ToolIndicatorPage {
    showExpand: false

    property var activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property var _unitsConversion: QGroundControl.unitsConversion
    FactPanelController { id: controller }

    property Fact _wpnavSpeedFact: controller.getParameterFact(-1, "WPNAV_SPEED", false)
    property Fact _rtlAltFact:     controller.getParameterFact(-1, "RTL_ALT", false)

    // Helper functions for unit conversion (cm <-> user preferred vertical distance units)
    function cmToDisplayUnits(cm) {
        var meters = cm / 100.0
        return _unitsConversion.metersToAppSettingsVerticalDistanceUnits(meters)
    }

    function displayUnitsToCm(displayValue) {
        var meters = _unitsConversion.appSettingsVerticalDistanceUnitsToMeters(displayValue)
        return meters * 100.0
    }

    contentComponent: Component {
        ColumnLayout {
            spacing: ScreenTools.defaultFontPixelHeight / 2

            // Waypoint Speed (m/s) and RTL Altitude (m) with robust clamping
            SettingsGroupLayout {
                heading: qsTr("Flight Parameters")
                visible: activeVehicle

                // Waypoint Speed (m/s)
                RowLayout {
                    Layout.fillWidth: true

                    QGCLabel {
                        Layout.fillWidth: true
                        text: qsTr("Waypoint Speed (m/s)")
                    }

                    QGCTextField {
                        id: wpnavSpeedField
                        text: _wpnavSpeedFact ? (_wpnavSpeedFact.value / 100).toFixed(1) : "--"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 10

                        onEditingFinished: {
                            if (!_wpnavSpeedFact) return
                            var value = parseFloat(text)
                            if (isNaN(value)) value = 0
                            // Clamp value in display units
                            value = Math.max(0.5, Math.min(20.0, value))
                            // Write back in internal units (cm/s)
                            _wpnavSpeedFact.value = Math.round(value * 100)
                            // Update text to the clamped value
                            wpnavSpeedField.text = (value).toFixed(1)
                        }

                        Connections {
                            target: _wpnavSpeedFact
                            onValueChanged: {
                                var val = _wpnavSpeedFact ? (_wpnavSpeedFact.value / 100) : 0
                                wpnavSpeedField.text = val.toFixed(1)
                            }
                        }
                    }
                }

                // RTL Altitude (uses app settings for vertical distance units)
                RowLayout {
                    Layout.fillWidth: true

                    QGCLabel {
                        Layout.fillWidth: true
                        text: qsTr("RTL Altitude (%1)").arg(_unitsConversion.appSettingsVerticalDistanceUnitsString)
                    }

                    QGCTextField {
                        id: rtlAltField
                        text: _rtlAltFact ? cmToDisplayUnits(_rtlAltFact.value).toFixed(1) : "--"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 10

                        onEditingFinished: {
                            if (!_rtlAltFact) return
                            var value = parseFloat(text)
                            if (isNaN(value)) value = 0
                            // Clamp value in display units (0-300m or 0-984ft equivalent)
                            var maxInDisplayUnits = _unitsConversion.metersToAppSettingsVerticalDistanceUnits(300)
                            value = Math.max(0, Math.min(maxInDisplayUnits, value))
                            // Write back in internal units (cm)
                            _rtlAltFact.value = Math.round(displayUnitsToCm(value))
                            // Update text to clamped value
                            rtlAltField.text = value.toFixed(1)
                        }

                        Connections {
                            target: _rtlAltFact
                            onValueChanged: {
                                rtlAltField.text = _rtlAltFact ? cmToDisplayUnits(_rtlAltFact.value).toFixed(1) : "--"
                            }
                        }
                    }
                }
            }

        }
    }
}
