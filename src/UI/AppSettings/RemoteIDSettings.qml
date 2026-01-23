/****************************************************************************
 *
 * (c) 2009-2022 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
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

SettingsPage {

    // Visual properties
    property real _margins:             ScreenTools.defaultFontPixelWidth
    // Flags visual properties
    property real   flagsWidth:         ScreenTools.defaultFontPixelWidth * 15
    property real   flagsHeight:        ScreenTools.defaultFontPixelWidth * 7
    property int    radiusFlags:        5

    // Flag to get active vehicle and active RID
    property var  _activeRID:           _activeVehicle && _activeVehicle.remoteIDManager ? _activeVehicle.remoteIDManager : null

    // Healthy connection with RID device
    property bool commsGood:            _activeVehicle && _activeVehicle.remoteIDManager ? _activeVehicle.remoteIDManager.commsGood : false

    // General properties
    property var  _activeVehicle:       QGroundControl.multiVehicleManager.activeVehicle
    property var  _offlineVehicle:      QGroundControl.multiVehicleManager.offlineEditingVehicle
    property int  _regionOperation:     QGroundControl.settingsManager.remoteIDSettings.region.value
    property int  _locationType:        QGroundControl.settingsManager.remoteIDSettings.locationType.value
    property int  _classificationType:  QGroundControl.settingsManager.remoteIDSettings.classificationType.value
    property var  _remoteIDManager:     _activeVehicle ? _activeVehicle.remoteIDManager : null


    property var  remoteIDSettings:QGroundControl.settingsManager.remoteIDSettings
    property Fact regionFact:           remoteIDSettings.region
    property Fact sendOperatorIdFact:   remoteIDSettings.sendOperatorID
    property Fact locationTypeFact:     remoteIDSettings.locationType
    property Fact operatorIDFact:       remoteIDSettings.operatorID
    property bool isEURegion:           regionFact.rawValue === RemoteIDSettings.RegionOperation.EU
    property bool isFAARegion:          regionFact.rawValue === RemoteIDSettings.RegionOperation.FAA
    property real textFieldWidth:       ScreenTools.defaultFontPixelWidth * 24
    property real textLabelWidth:       ScreenTools.defaultFontPixelWidth * 30

    enum RegionOperation {
        FAA,
        EU
    }

    enum LocationType {
        LIVE = 1,
        FIXED = 2
    }

    enum ClassificationType {
        UNDEFINED,
        EU
    }

    // GPS properties
    property var    gcsPosition:        QGroundControl.qgcPositionManger.gcsPosition

    QGCPalette { id: qgcPal }

    // GPS status helper function
    function getGpsStatusText() {
        if (_locationType === RemoteIDSettings.LocationType.FIXED) {
            return qsTr("Using Fixed Location")
        }
        if (!gcsPosition || !gcsPosition.isValid) {
            return qsTr("Waiting for GPS fix...")
        }
        if (_activeRID && !_remoteIDManager.gcsGPSGood) {
            // Check if we have position but it's stale or missing altitude
            if (gcsPosition.isValid) {
                if (isNaN(gcsPosition.altitude) || gcsPosition.altitude < 0) {
                    return qsTr("GPS Error: No altitude data")
                }
                return qsTr("GPS Error: Position data stale")
            }
            return qsTr("GPS Error: Invalid position")
        }
        if (_activeRID && _remoteIDManager.gcsGPSGood) {
            return qsTr("GPS Fix OK")
        }
        // No active vehicle connected
        if (gcsPosition && gcsPosition.isValid) {
            if (isNaN(gcsPosition.altitude) || gcsPosition.altitude < 0) {
                return qsTr("GPS: No altitude (connect vehicle to verify)")
            }
            return qsTr("GPS: Position available (connect vehicle to verify)")
        }
        return qsTr("Waiting for GPS fix...")
    }

    function getGpsStatusColor() {
        if (_locationType === RemoteIDSettings.LocationType.FIXED) {
            return qgcPal.colorGreen
        }
        if (_activeRID && _remoteIDManager.gcsGPSGood) {
            return qgcPal.colorGreen
        }
        if (!gcsPosition || !gcsPosition.isValid) {
            return qgcPal.colorOrange
        }
        if (_activeRID && !_remoteIDManager.gcsGPSGood) {
            return qgcPal.colorRed
        }
        // No active vehicle - show orange as we can't verify
        return qgcPal.colorOrange
    }

    Item {
        id:                 flagsItem
        width:              parent.width
        height:             flagsColumn.height
        Layout.alignment:   Qt.AlignHCenter

        ColumnLayout {
            id:                         flagsColumn
            anchors.horizontalCenter:   parent.horizontalCenter
            spacing:                    _margins

            // ---------------------------------------- STATUS -----------------------------------------
            // Status flags. Visual representation for the state of all necesary information for remoteID
            // to work propely.
            Rectangle {
                id:                     flagsRectangle
                Layout.preferredHeight: statusGrid.height + (_margins * 2)
                Layout.preferredWidth:  statusGrid.width + (_margins * 2)
                color:                  qgcPal.windowShade
                visible:                _activeVehicle
                Layout.fillWidth:       true

                GridLayout {
                    id:                         statusGrid
                    anchors.margins:            _margins
                    anchors.top:                parent.top
                    anchors.horizontalCenter:   parent.horizontalCenter
                    rows:                       1
                    rowSpacing:                 _margins * 3
                    columnSpacing:              _margins * 2

                    Rectangle {
                        id:                     armFlag
                        Layout.preferredHeight: flagsHeight
                        Layout.preferredWidth:  flagsWidth
                        color:                  _activeRID ? (_remoteIDManager.armStatusGood ? qgcPal.colorGreen : qgcPal.colorRed) : qgcPal.colorGrey
                        radius:                 radiusFlags
                        visible:                commsGood

                        QGCLabel {
                            anchors.fill:           parent
                            text:                   qsTr("ARM STATUS")
                            wrapMode:               Text.WordWrap
                            horizontalAlignment:    Text.AlignHCenter
                            verticalAlignment:      Text.AlignVCenter
                            font.bold:              true
                        }
                    }

                    Rectangle {
                        id:                     commsFlag
                        Layout.preferredHeight: flagsHeight
                        Layout.preferredWidth:  flagsWidth
                        color:                  _activeRID ? (_remoteIDManager.commsGood ? qgcPal.colorGreen : qgcPal.colorRed) : qgcPal.colorGrey
                        radius:                 radiusFlags

                        QGCLabel {
                            anchors.fill:           parent
                            text:                   _activeRID && _remoteIDManager.commsGood ? qsTr("RID COMMS") : qsTr("NOT CONNECTED")
                            wrapMode:               Text.WordWrap
                            horizontalAlignment:    Text.AlignHCenter
                            verticalAlignment:      Text.AlignVCenter
                            font.bold:              true
                        }
                    }

                    Rectangle {
                        id:                     gpsFlag
                        Layout.preferredHeight: flagsHeight
                        Layout.preferredWidth:  flagsWidth
                        color:                  _activeRID ? (_remoteIDManager.gcsGPSGood ? qgcPal.colorGreen : qgcPal.colorRed) : qgcPal.colorGrey
                        radius:                 radiusFlags
                        visible:                commsGood

                        QGCLabel {
                            anchors.fill:           parent
                            text:                   qsTr("GCS GPS")
                            wrapMode:               Text.WordWrap
                            horizontalAlignment:    Text.AlignHCenter
                            verticalAlignment:      Text.AlignVCenter
                            font.bold:              true
                        }
                    }

                    // Basic ID and Operator ID flags hidden - not used in simplified UI
                }
            }
        }
    }

    RowLayout {
        spacing: ScreenTools.defaultFontPixelWidth

        Connections {
            target: regionFact
            onRawValueChanged: {
                if (regionFact.rawValue === RemoteIDSettings.EU) {
                    sendOperatorIdFact.rawValue = true
                }
                if (regionFact.rawValue === RemoteIDSettings.FAA) {
                    locationTypeFact.value = RemoteIDSettings.LocationType.LIVE
                }
            }
        }

        ColumnLayout {
            spacing:            ScreenTools.defaultFontPixelHeight / 2
            Layout.alignment:   Qt.AlignTop

            SettingsGroupLayout {
                Layout.fillWidth:   true

                LabelledFactComboBox {
                    label:              fact.shortDescription
                    fact:               QGroundControl.settingsManager.remoteIDSettings.region
                    visible:            QGroundControl.settingsManager.remoteIDSettings.region.visible
                    Layout.fillWidth:   true
                }
            }
            SettingsGroupLayout {
                outerBorderColor: _activeRID ? (_remoteIDManager.armStatusGood ? defaultBorderColor : qgcPal.colorRed) : defaultBorderColor
                visible:            armStatusLabel.labelText !== ""
                LabelledLabel {
                    id :                armStatusLabel
                    label:              qsTr("Arm Status Error")
                    labelText:          _remoteIDManager?_remoteIDManager.armStatusError:"Vehicle Not Connected"
                    visible:            labelText !== ""
                    Layout.fillWidth:   true
                }
            }

            // Basic ID, Operator ID, and Self ID sections hidden - not used in simplified UI
        }

        ColumnLayout {
            spacing:            ScreenTools.defaultFontPixelHeight / 2
            Layout.alignment:   Qt.AlignTop
            SettingsGroupLayout {
                heading:            qsTr("GroundStation Location")
                Layout.fillWidth:   true
                outerBorderColor : _activeRID ? (_remoteIDManager.gcsGPSGood ? defaultBorderColor : qgcPal.colorRed) : defaultBorderColor
                LabelledFactComboBox {
                    label:              locationTypeFact.shortDescription
                    fact:               locationTypeFact
                    indexModel:         false
                    Layout.fillWidth:   true
                }

                LabelledFactTextField {
                    label:                      _fact.shortDescription
                    fact:                       _fact
                    textField.maximumLength:    20
                    visible:                    locationTypeFact.rawValue === RemoteIDSettings.LocationType.FIXED
                    Layout.fillWidth:           true
                    textFieldPreferredWidth:    textFieldWidth

                    property Fact _fact: remoteIDSettings.latitudeFixed
                }

                LabelledFactTextField {
                    label:                      _fact.shortDescription
                    fact:                       _fact
                    textField.maximumLength:    20
                    visible:                    locationTypeFact.rawValue === RemoteIDSettings.LocationType.FIXED
                    Layout.fillWidth:           true
                    textFieldPreferredWidth:    textFieldWidth

                    property Fact _fact: remoteIDSettings.longitudeFixed
                }

                LabelledFactTextField {
                    label:                      _fact.shortDescription
                    fact:                       _fact
                    textField.maximumLength:    20
                    visible:                    locationTypeFact.rawValue === RemoteIDSettings.LocationType.FIXED
                    Layout.fillWidth:           true
                    textFieldPreferredWidth:    textFieldWidth

                    property Fact _fact: remoteIDSettings.altitudeFixed
                }

                // GPS Status display
                RowLayout {
                    Layout.fillWidth:   true
                    spacing:            ScreenTools.defaultFontPixelWidth

                    Rectangle {
                        width:  ScreenTools.defaultFontPixelWidth * 1.5
                        height: width
                        radius: width / 2
                        color:  getGpsStatusColor()
                    }

                    QGCLabel {
                        text:               getGpsStatusText()
                        Layout.fillWidth:   true
                        font.bold:          true
                    }
                }

                // Live GPS coordinates display - shown when Live GNSS is selected
                LabelledLabel {
                    label:              qsTr("Latitude")
                    labelText:          gcsPosition && gcsPosition.isValid ? gcsPosition.latitude.toFixed(7) : qsTr("--")
                    visible:            locationTypeFact.rawValue === RemoteIDSettings.LocationType.LIVE
                    Layout.fillWidth:   true
                }

                LabelledLabel {
                    label:              qsTr("Longitude")
                    labelText:          gcsPosition && gcsPosition.isValid ? gcsPosition.longitude.toFixed(7) : qsTr("--")
                    visible:            locationTypeFact.rawValue === RemoteIDSettings.LocationType.LIVE
                    Layout.fillWidth:   true
                }

                LabelledLabel {
                    label:              qsTr("Altitude")
                    labelText:          gcsPosition && gcsPosition.isValid && !isNaN(gcsPosition.altitude) ? gcsPosition.altitude.toFixed(1) + " m" : qsTr("--")
                    visible:            locationTypeFact.rawValue === RemoteIDSettings.LocationType.LIVE
                    Layout.fillWidth:   true
                }

            }


            // EU Vehicle Info section hidden - not used in simplified UI
        }
    }

}
