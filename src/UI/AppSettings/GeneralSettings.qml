/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import QGroundControl
import QGroundControl.FactSystem
import QGroundControl.FactControls
import QGroundControl.Controls
import QGroundControl.ScreenTools
import QGroundControl.MultiVehicleManager
import QGroundControl.Palette
import QGroundControl.NTRIP 1.0

SettingsPage {
    property var    _settingsManager:           QGroundControl.settingsManager
    property var    _appSettings:               _settingsManager.appSettings
    property var    _brandImageSettings:        _settingsManager.brandImageSettings
    property Fact   _appFontPointSize:          _appSettings.appFontPointSize
    property Fact   _userBrandImageIndoor:      _brandImageSettings.userBrandImageIndoor
    property Fact   _userBrandImageOutdoor:     _brandImageSettings.userBrandImageOutdoor
    property Fact   _appSavePath:               _appSettings.savePath
    property var    ntripSettings:             _settingsManager.ntripSettings
    // 🔹 Use the global NTRIP singleton we registered in C++
    //property var ntrip: NTRIP


    // ────────────────────────────────
    // General Settings
    // ────────────────────────────────
    SettingsGroupLayout {
        Layout.fillWidth:   true
        heading:            qsTr("General")

        LabelledFactComboBox {
            label:      qsTr("Language")
            fact:       _appSettings.qLocaleLanguage
            indexModel: false
            visible:    _appSettings.qLocaleLanguage.visible
        }

        LabelledFactComboBox {
            label:      qsTr("Color Scheme")
            fact:       _appSettings.indoorPalette
            indexModel: false
            visible:    _appSettings.indoorPalette.visible
        }

        LabelledFactComboBox {
            label:       qsTr("Stream GCS Position")
            fact:       _appSettings.followTarget
            indexModel: false
            visible:    _appSettings.followTarget.visible
        }

        FactCheckBoxSlider {
            Layout.fillWidth: true
            text:           qsTr("Mute all audio output")
            fact:           _audioMuted
            visible:        _audioMuted.visible
            property Fact _audioMuted: _appSettings.audioMuted
        }

        FactCheckBoxSlider {
            Layout.fillWidth: true
            text:       qsTr("Save application data to SD Card")
            fact:       _androidSaveToSDCard
            visible:    _androidSaveToSDCard.visible
            property Fact _androidSaveToSDCard: _appSettings.androidSaveToSDCard
        }

        QGCCheckBoxSlider {
            Layout.fillWidth: true
            text:       qsTr("Clear all settings on next start")
            checked:    false
            onClicked: {
                if (checked) {
                    QGroundControl.deleteAllSettingsNextBoot()
                } else {
                    QGroundControl.clearDeleteAllSettingsNextBoot()
                }
            }
        }

        FactCheckBoxSlider {
            Layout.fillWidth: true
            text:       qsTr("Enable RID on vehicle connect")
            fact:       _appSettings.enableRIDOnConnect
            visible:    _appSettings.enableRIDOnConnect.visible
        }

        // UI Scaling
        RowLayout {
            Layout.fillWidth: true
            spacing: ScreenTools.defaultFontPixelWidth * 2
            visible: _appFontPointSize.visible

            QGCLabel {
                Layout.fillWidth: true
                text: qsTr("UI Scaling")
            }

            RowLayout {
                spacing: ScreenTools.defaultFontPixelWidth * 2

                QGCButton {
                    Layout.preferredWidth:  height
                    height:                 baseFontEdit.height * 1.5
                    text:                   "-"
                    onClicked: {
                        if (_appFontPointSize.value > _appFontPointSize.min) {
                            _appFontPointSize.value = _appFontPointSize.value - 1
                        }
                    }
                }

                QGCLabel {
                    id: baseFontEdit
                    width: ScreenTools.defaultFontPixelWidth * 6
                    text: (QGroundControl.settingsManager.appSettings.appFontPointSize.value / ScreenTools.platformFontPointSize * 100).toFixed(0) + "%"
                }

                QGCButton {
                    Layout.preferredWidth:  height
                    height:                 baseFontEdit.height * 1.5
                    text:                   "+"
                    onClicked: {
                        if (_appFontPointSize.value < _appFontPointSize.max) {
                            _appFontPointSize.value = _appFontPointSize.value + 1
                        }
                    }
                }
            }
        }

        // Application Save Path
        RowLayout {
            Layout.fillWidth: true
            spacing: ScreenTools.defaultFontPixelWidth * 2
            visible: _appSavePath.visible && !ScreenTools.isMobile

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                QGCLabel { text: qsTr("Application Load/Save Path") }
                QGCLabel {
                    Layout.fillWidth: true
                    font.pointSize: ScreenTools.smallFontPointSize
                    text: _appSavePath.rawValue === "" ? qsTr("<default location>") : _appSavePath.value
                    elide: Text.ElideMiddle
                }
            }

            QGCButton {
                text: qsTr("Browse")
                onClicked: savePathBrowseDialog.openForLoad()
                QGCFileDialog {
                    id: savePathBrowseDialog
                    title: qsTr("Choose the location to save/load files")
                    folder: _appSavePath.rawValue
                    selectFolder: true
                    onAcceptedForLoad: (file) => _appSavePath.rawValue = file
                }
            }
        }
    }

    // ────────────────────────────────
    // Units Settings
    // ────────────────────────────────
    SettingsGroupLayout {
        Layout.fillWidth: true
        heading: qsTr("Units")
        visible: QGroundControl.settingsManager.unitsSettings.visible

        Repeater {
            model: [
                QGroundControl.settingsManager.unitsSettings.horizontalDistanceUnits,
                QGroundControl.settingsManager.unitsSettings.verticalDistanceUnits,
                QGroundControl.settingsManager.unitsSettings.areaUnits,
                QGroundControl.settingsManager.unitsSettings.speedUnits,
                QGroundControl.settingsManager.unitsSettings.temperatureUnits
            ]

            LabelledFactComboBox {
                label: modelData.shortDescription
                fact: modelData
                indexModel: false
            }
        }
    }

    // ────────────────────────────────
    // 🛰️ NTRIP / RTCM Settings Section (Safe Null Binding)
    // ────────────────────────────────
    SettingsGroupLayout {
        Layout.fillWidth: true
        heading: qsTr("NTRIP / RTCM")

        visible: ntripSettings ? ntripSettings.visible : false

        FactCheckBox {
            text:    ntripSettings ? ntripSettings.ntripServerConnectEnabled.shortDescription : ""
            fact:    ntripSettings ? ntripSettings.ntripServerConnectEnabled : null
            visible: ntripSettings ? ntripSettings.ntripServerConnectEnabled.visible : false
        }

        FactCheckBox {
            text:    ntripSettings ? ntripSettings.ntripEnableVRS.shortDescription : ""
            fact:    ntripSettings ? ntripSettings.ntripEnableVRS : null
            visible: ntripSettings ? ntripSettings.ntripEnableVRS.visible : false
        }

        LabelledFactTextField {
            label:   ntripSettings ? ntripSettings.ntripServerHostAddress.shortDescription : ""
            fact:    ntripSettings ? ntripSettings.ntripServerHostAddress : null
            visible: ntripSettings ? ntripSettings.ntripServerHostAddress.visible : false
        }

        LabelledFactTextField {
            label:   ntripSettings ? ntripSettings.ntripServerPort.shortDescription : ""
            fact:    ntripSettings ? ntripSettings.ntripServerPort : null
            visible: ntripSettings ? ntripSettings.ntripServerPort.visible : false
        }

        LabelledFactTextField {
            label:   ntripSettings ? ntripSettings.ntripUsername.shortDescription : ""
            fact:    ntripSettings ? ntripSettings.ntripUsername : null
            visible: ntripSettings ? ntripSettings.ntripUsername.visible : false
        }

        LabelledFactTextField {
            label:   ntripSettings ? ntripSettings.ntripPassword.shortDescription : ""
            fact:    ntripSettings ? ntripSettings.ntripPassword : null
            visible: ntripSettings ? ntripSettings.ntripPassword.visible : false
        }

        LabelledFactTextField {
            label:   ntripSettings ? ntripSettings.ntripMountpoint.shortDescription : ""
            fact:    ntripSettings ? ntripSettings.ntripMountpoint : null
            visible: ntripSettings ? ntripSettings.ntripMountpoint.visible : false
        }

        LabelledFactTextField {
            label:   ntripSettings ? ntripSettings.ntripWhitelist.shortDescription : ""
            fact:    ntripSettings ? ntripSettings.ntripWhitelist : null
            visible: ntripSettings ? ntripSettings.ntripWhitelist.visible : false
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight
            visible: NTRIP && NTRIP.masterEnable
            spacing: ScreenTools.defaultFontPixelHeight

            QGCLabel {
                text: qsTr("NTRIP Status")
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            QGCButton {
                text: NTRIP && NTRIP.enabled ? qsTr("Disconnect NTRIP") : qsTr("Connect NTRIP")
                Layout.alignment: Qt.AlignHCenter
                onClicked: if (NTRIP) NTRIP.enabled = !NTRIP.enabled
            }

            QGCLabel {
                Layout.alignment: Qt.AlignHCenter
                text: {
                    if (!NTRIP) return "NTRIP STATUS: Unknown"
                    switch (NTRIP.connectionStatus) {
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
