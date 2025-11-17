import QtQuick
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.MultiVehicleManager
import QGroundControl.ScreenTools
import QGroundControl.FactSystem
import QGroundControl.FactControls

ToolIndicatorPage {
    showExpand: false

    property var activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    //property bool savingPopupVisible: false

    FactPanelController { id: controller }

    // ENTIRE geotagging mode fact (read from Vehicle C++)
    // Expects you exposed: property int entireGeotagMode in Vehicle
    // Values: 0 = OFF, 1 = ON, 2 = IDLE  (adjust if needed)
    function geotagModeText(mode) {
        return mode.toString();
        // if (mode === 1) return "ON"
        // if (mode === 2) return "IDLE"
        // return "OFF"
    }

    // Parameter facts for payload switching
    property Fact _serial2BaudFact
    property Fact _camTypeFact

    Connections {
        target: QGroundControl.multiVehicleManager
        onActiveVehicleChanged: {
            if (activeVehicle) {
                _serial2BaudFact = controller.getParameterFact(-1, "SERIAL2_BAUD", false)
                _camTypeFact     = controller.getParameterFact(-1, "CAM_TYPE", false)
            } else {
                _serial2BaudFact = undefined
                _camTypeFact = undefined
            }
        }
    }

    Component.onCompleted: {
        if (activeVehicle) {
            _serial2BaudFact = controller.getParameterFact(-1, "SERIAL2_BAUD", false)
            _camTypeFact     = controller.getParameterFact(-1, "CAM_TYPE", false)
        }
    }

    // Helper: apply params & reboot
    function applyPayloadConfig(baud, camtype) {

         if (!activeVehicle || !_serial2BaudFact || !_camTypeFact)
         {
            mainWindow.showMessageDialog(qsTr("Payload Info"), qsTr("Payload params not available"))
            return;
         }

        if (!activeVehicle || !_serial2BaudFact)
            return;

        var changed = false

        if (_serial2BaudFact.value !== baud) {
            _serial2BaudFact.value = baud
            changed = true
        }

        if (_camTypeFact.value !== camtype) {
            _camTypeFact.value = camtype
            changed = true
        }

        if (changed) {
            mainWindow.showMessageDialog(qsTr("Payload Info"), qsTr("Payload parameters changed, rebooting..."))


            Qt.callLater(function() {
                console.log("[Payload] Sending reboot command...")

                // activeVehicle.sendCommand(
                //     1,
                //     QGroundControl.MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN,
                //     true,    // show errors
                //     1,       // param1: reboot autopilot
                //     0,0,0,0,0,0
                // )

                activeVehicle.rebootVehicle()
            })
        }
        else
        {
            mainWindow.showMessageDialog(qsTr("Payload Info"), qsTr("Payload parameters need no change"))
        }

    }

    contentComponent: Component {
        ColumnLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            // ──────────────────────────────
            //   ENTIRE GEOTAGGING STATUS
            // ──────────────────────────────

            SettingsGroupLayout {
                heading: "Geotagging Details"
                visible: activeVehicle

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: ScreenTools.defaultFontPixelHeight / 2

                    QGCLabel { text: "Mode: " + activeVehicle.geoMode }
                    QGCLabel { text: "Session Status: " + activeVehicle.geoStatusText }
                    QGCLabel { text: "Auto-trigger Status: " + activeVehicle.geoAutoTriggerStatus }
                    QGCLabel { text: "Logging Status: " + activeVehicle.geoLoggingStatus }
                    QGCLabel { text: "Progress: " + activeVehicle.geoProgressPercent + "%" }
                    QGCLabel { text: "Photos Taken: " + activeVehicle.geoPhotoCount }
                }
            }

            SettingsGroupLayout {
                heading: "Geotagging Actions"
                visible: activeVehicle

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: ScreenTools.defaultFontPixelHeight / 2

                    // Start Geotagging (132)
                    QGCButton {
                        text: "Geotagging ON"
                        Layout.fillWidth: true
                        onClicked: {
                            if (!activeVehicle) return
                            activeVehicle.sendCommand(
                                105,
                                QGroundControl.MAV_CMD_DO_DIGICAM_CONFIGURE,
                                true,
                                132,0,0,0,0,0,0
                            )
                        }
                    }

                    // Stop Geotagging (133)
                    QGCButton {
                        text: "Geotagging OFF"
                        Layout.fillWidth: true
                        onClicked: {
                            if (!activeVehicle) return
                            activeVehicle.sendCommand(
                                105,
                                QGroundControl.MAV_CMD_DO_DIGICAM_CONFIGURE,
                                true,
                                133,0,0,0,0,0,0
                            )
                        }
                    }
                }
            }

            // Divider
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: qgcPal.buttonText
                opacity: 0.25
            }

            // ──────────────────────────────
            //      PAYLOAD SELECTION
            // ──────────────────────────────
            SettingsGroupLayout {
                heading: "Payload Selection"
                visible: activeVehicle

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: ScreenTools.defaultFontPixelHeight / 2

                    // VIO Payload
                    QGCButton {
                        text: "VIO Payload"
                        Layout.fillWidth: true
                        onClicked: applyPayloadConfig(115, 6)
                    }

                    // ILX-LR1 Payload
                    QGCButton {
                        text: "ILX-LR1 Payload"
                        Layout.fillWidth: true
                        onClicked: applyPayloadConfig(230, 5)
                    }
                }
            }
        }
    }
}
