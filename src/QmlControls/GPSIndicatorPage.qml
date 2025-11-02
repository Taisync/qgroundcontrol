/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * Modified to include Arcsky NTRIP status and controls
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
import QGroundControl.NTRIP 1.0

ToolIndicatorPage {
    showExpand: false

    property var activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property string na: qsTr("N/A")
    property string valueNA: qsTr("--.--")

    // 🔹 Use the global NTRIP singleton we registered in C++
    property var ntrip: NTRIP


    contentComponent: Component {
        ColumnLayout {
            spacing: ScreenTools.defaultFontPixelHeight / 2

            // --- GPS Status section ---
            SettingsGroupLayout {
                heading: qsTr("Vehicle GPS Status")
                visible: activeVehicle

                LabelledLabel {
                    label:      qsTr("Satellites")
                    labelText:  activeVehicle ? activeVehicle.gps.count.valueString : na
                }

                LabelledLabel {
                    label:      qsTr("GPS Lock")
                    labelText:  activeVehicle ? activeVehicle.gps.lock.enumStringValue : na
                }

                LabelledLabel {
                    label:      qsTr("HDOP")
                    labelText:  activeVehicle ? activeVehicle.gps.hdop.valueString : valueNA
                }

                LabelledLabel {
                    label:      qsTr("VDOP")
                    labelText:  activeVehicle ? activeVehicle.gps.vdop.valueString : valueNA
                }

                LabelledLabel {
                    label:      qsTr("Course Over Ground")
                    labelText:  activeVehicle ? activeVehicle.gps.courseOverGround.valueString : valueNA
                }
            }

            // --- NTRIP section ---
            SettingsGroupLayout {
                heading: qsTr("NTRIP Correction Link")
                visible: ntrip && ntrip.masterEnable

                QGCButton {
                    id: ntripToggleButton
                    text: ntrip && ntrip.enabled ? qsTr("Disconnect NTRIP") : qsTr("Connect NTRIP")
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: {
                        if (ntrip)
                            ntrip.enabled = !ntrip.enabled
                    }
                }

                QGCLabel {
                    id: ntripStatusLabel
                    Layout.alignment: Qt.AlignHCenter
                    text: {
                        if (!ntrip) return "NTRIP STATUS: Unknown"
                        switch (ntrip.connectionStatus) {
                            case 0: return "NTRIP STATUS: Off"
                            case 1: return "NTRIP STATUS: Connecting"
                            case 2: return "NTRIP STATUS: Connected"
                            case 3: return "NTRIP STATUS: Retrying"
                            case 4: return "NTRIP STATUS: Timed Out"
                            default: return "NTRIP STATUS: Unknown"
                        }
                    }
                }
            }
        }
    }
}
