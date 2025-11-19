/****************************************************************************
 * Mavlink Sender Page (QML-only payload detection)
 ****************************************************************************/

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

    FactPanelController { id: controller }

    //--------------------------------------
    // Payload-related parameters
    //--------------------------------------
    property Fact _serial2BaudFact
    property Fact _camTypeFact
    property int payloadType: 0   // 0=unknown, 1=ILX, 2=VIO

    //--------------------------------------
    // Detect payload based on parameters
    //--------------------------------------
    function detectPayloadType() {
        if (!_serial2BaudFact || !_camTypeFact)
            return 0

        const baud = _serial2BaudFact.value
        const cam  = _camTypeFact.value

        if (baud === 230 && cam === 5) return 1   // ILX-LR1
        if (baud === 115 && cam === 6) return 2   // VIO

        return 0
    }

    //--------------------------------------
    // Load parameters when vehicle changes
    //--------------------------------------
    Connections {
        target: QGroundControl.multiVehicleManager
        onActiveVehicleChanged: {
            if (activeVehicle) {
                _serial2BaudFact = controller.getParameterFact(-1, "SERIAL2_BAUD", false)
                _camTypeFact     = controller.getParameterFact(-1, "CAM1_TYPE", false)
                payloadType = detectPayloadType()
            } else {
                _serial2BaudFact = undefined
                _camTypeFact     = undefined
                payloadType = 0
            }
        }
    }

    Component.onCompleted: {
        if (activeVehicle) {
            _serial2BaudFact = controller.getParameterFact(-1, "SERIAL2_BAUD", false)
            _camTypeFact     = controller.getParameterFact(-1, "CAM1_TYPE", false)
            payloadType = detectPayloadType()
        }
    }

    //--------------------------------------
    // Update payloadType if params change
    //--------------------------------------
    Connections {
        target: _serial2BaudFact
        onValueChanged: payloadType = detectPayloadType()
    }

    Connections {
        target: _camTypeFact
        onValueChanged: payloadType = detectPayloadType()
    }

    //--------------------------------------
    // Apply payload configuration
    //--------------------------------------
    function applyPayloadConfig(baud, camtype) {

        if (!activeVehicle || !_serial2BaudFact || !_camTypeFact) {
            mainWindow.showMessageDialog(
                "Payload Info",
                "Payload params not available"
            )
            return
        }

        var changed = false

        if (_serial2BaudFact.value !== baud) {
            _serial2BaudFact.value = baud
            changed = true
        }

        if (_camTypeFact.value !== camtype) {
            _camTypeFact.value = camtype
            changed = true
        }

        // Update RTSP automatically (temporary logic)
        if (baud === 115)
            QGroundControl.settingsManager.videoSettings.rtspUrl2.value =
                "rtsp://192.168.144.10:8554/vio"
        else if (baud === 230)
            QGroundControl.settingsManager.videoSettings.rtspUrl2.value =
                "rtsp://192.168.144.121:8554/main.264"

        if (changed) {
            mainWindow.showMessageDialog("Payload Info", "Payload parameters changed, rebooting...")

            Qt.callLater(function() {
                activeVehicle.rebootVehicle()
                mainWindow.closeIndicatorDrawer()
            })
        } else {
            mainWindow.showMessageDialog("Payload Info", "Payload parameters need no change")
        }
    }


    //============================================================
    //                       PAGE CONTENT
    //============================================================
    contentComponent: Component {
        ColumnLayout {
            spacing: ScreenTools.defaultFontPixelHeight

            //----------------------------------------------------
            // ILX Geotag Section (only for ILX payload)
            //----------------------------------------------------
            SettingsGroupLayout {
                heading: "Geotagging Details"
                visible: payloadType === 1

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: ScreenTools.defaultFontPixelHeight / 2

                    QGCLabel { text: "Mode: " + activeVehicle.geoMode }
                    QGCLabel { text: "Session Status: " + activeVehicle.geoStatusText }
                    QGCLabel { text: "Auto-trigger Status: " + activeVehicle.geoAutoTriggerStatus }
                    QGCLabel { text: "Logging Status: " + activeVehicle.geoLoggingStatus }
                    QGCLabel { text: "Progress: " + activeVehicle.geoProgressPercent + "%" }
                    QGCLabel { text: "Photos Taken: " + activeVehicle.imageCount }
                }
            }

            SettingsGroupLayout {
                heading: "Geotagging Actions"
                visible: payloadType === 1

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: ScreenTools.defaultFontPixelPixelHeight / 2

                    QGCButton {
                        text: "Geotagging ON"
                        Layout.fillWidth: true
                        onClicked: {
                            activeVehicle.sendCommand(
                                105,
                                202,
                                true,
                                132,0,0,0,0,0,0
                            )
                        }
                    }

                    QGCButton {
                        text: "Geotagging OFF"
                        Layout.fillWidth: true
                        onClicked: {
                            activeVehicle.sendCommand(
                                105,
                                202,
                                true,
                                133,0,0,0,0,0,0
                            )
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: qgcPal.buttonText
                opacity: 0.25
            }

            //----------------------------------------------------
            // PAYLOAD SELECTION
            //----------------------------------------------------
            SettingsGroupLayout {
                heading: "Payload Selection"
                visible: activeVehicle

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: ScreenTools.defaultFontPixelHeight / 2

                    QGCButton {
                        text: "VIO Payload"
                        Layout.fillWidth: true
                        backgroundColor: (payloadType === 2) ? "green" : "gray"
                        onClicked: applyPayloadConfig(115, 6)
                    }

                    QGCButton {
                        text: "ILX-LR1 Payload"
                        Layout.fillWidth: true
                        backgroundColor: (payloadType === 1) ? "green" : "gray"
                        onClicked: applyPayloadConfig(230, 5)
                    }
                }
            }
        }
    }
}
