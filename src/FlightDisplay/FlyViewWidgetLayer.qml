/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                  2.15
import QtQuick.Controls         2.4
import QtQuick.Dialogs          1.3
import QtQuick.Layouts          1.12

import QtLocation               5.3
import QtPositioning            5.3
import QtQuick.Window           2.2
import QtQml.Models             2.1

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.Controls      1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FlightDisplay 1.0
import QGroundControl.FlightMap     1.0
import QGroundControl.Palette       1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Vehicle       1.0
import taisyncInfo                  1.0

// This is the ui overlay layer for the widgets/tools for Fly View
Item {
    id: _root

    property var    parentToolInsets
    property var    totalToolInsets:        _totalToolInsets
    property var    mapControl

    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property var    _planMasterController:  globals.planMasterControllerFlyView
    property var    _missionController:     _planMasterController.missionController
    property var    _geoFenceController:    _planMasterController.geoFenceController
    property var    _rallyPointController:  _planMasterController.rallyPointController
    property var    _guidedController:      globals.guidedControllerFlyView
    property real   _margins:               ScreenTools.defaultFontPixelWidth / 2
    property real   _toolsMargin:           ScreenTools.defaultFontPixelWidth * 0.75
    property rect   _centerViewport:        Qt.rect(0, 0, width, height)
    property real   _rightPanelWidth:       ScreenTools.defaultFontPixelWidth * 30
    property alias  _gripperMenu:           gripperOptions

    QGCToolInsets {
        id:                     _totalToolInsets
        leftEdgeTopInset:       toolStrip.leftEdgeTopInset
        leftEdgeCenterInset:    parentToolInsets.leftEdgeCenterInset
        leftEdgeBottomInset:    virtualJoystickMultiTouch.visible ? virtualJoystickMultiTouch.leftEdgeBottomInset : parentToolInsets.leftEdgeBottomInset
        rightEdgeTopInset:      instrumentPanel.rightEdgeTopInset
        rightEdgeCenterInset:   (telemetryPanel.rightEdgeCenterInset > photoVideoControl.rightEdgeCenterInset) ? telemetryPanel.rightEdgeCenterInset : photoVideoControl.rightEdgeCenterInset
        rightEdgeBottomInset:   virtualJoystickMultiTouch.visible ? virtualJoystickMultiTouch.rightEdgeBottomInset : parentToolInsets.rightEdgeBottomInset
        topEdgeLeftInset:       toolStrip.topEdgeLeftInset
        topEdgeCenterInset:     mapScale.topEdgeCenterInset
        topEdgeRightInset:      instrumentPanel.topEdgeRightInset
        bottomEdgeLeftInset:    virtualJoystickMultiTouch.visible ? virtualJoystickMultiTouch.bottomEdgeLeftInset : parentToolInsets.bottomEdgeLeftInset
        bottomEdgeCenterInset:  telemetryPanel.bottomEdgeCenterInset
        bottomEdgeRightInset:   virtualJoystickMultiTouch.visible ? virtualJoystickMultiTouch.bottomEdgeRightInset : parentToolInsets.bottomEdgeRightInset
    }

    FlyViewMissionCompleteDialog {
        missionController:      _missionController
        geoFenceController:     _geoFenceController
        rallyPointController:   _rallyPointController
    }

    Row {
        id:                 multiVehiclePanelSelector
        anchors.margins:    _toolsMargin
        anchors.top:        parent.top
        anchors.right:      parent.right
        width:              _rightPanelWidth
        spacing:            ScreenTools.defaultFontPixelWidth
        visible:            QGroundControl.multiVehicleManager.vehicles.count > 1 && QGroundControl.corePlugin.options.flyView.showMultiVehicleList

        property bool showSingleVehiclePanel:  !visible || singleVehicleRadio.checked

        QGCMapPalette { id: mapPal; lightColors: true }

        QGCRadioButton {
            id:             singleVehicleRadio
            text:           qsTr("Single")
            checked:        true
            textColor:      mapPal.text
        }

        QGCRadioButton {
            text:           qsTr("Multi-Vehicle")
            textColor:      mapPal.text
        }
    }

    MultiVehicleList {
        anchors.margins:    _toolsMargin
        anchors.top:        multiVehiclePanelSelector.bottom
        anchors.right:      parent.right
        width:              _rightPanelWidth
        height:             parent.height - y - _toolsMargin
        visible:            !multiVehiclePanelSelector.showSingleVehiclePanel
    }


    GuidedActionConfirm {
        anchors.margins:            _toolsMargin
        anchors.top:                parent.top
        anchors.horizontalCenter:   parent.horizontalCenter
        z:                          QGroundControl.zOrderTopMost
        guidedController:           _guidedController
        guidedValueSlider:          _guidedValueSlider
    }

    FlyViewInstrumentPanel {
        id:                         instrumentPanel
        anchors.margins:            _toolsMargin
        anchors.top:                multiVehiclePanelSelector.visible ? multiVehiclePanelSelector.bottom : parent.top
        anchors.right:              parent.right
        width:                      _rightPanelWidth
        spacing:                    _toolsMargin
        visible:                    QGroundControl.corePlugin.options.flyView.showInstrumentPanel && multiVehiclePanelSelector.showSingleVehiclePanel
        availableHeight:            parent.height - y - _toolsMargin

        property real rightEdgeTopInset: visible ? parent.width - x : 0
        property real topEdgeRightInset: visible ? y + height : 0
    }

    PhotoVideoControl {
        id:                     photoVideoControl
        anchors.margins:        _toolsMargin
        anchors.right:          parent.right
        width:                  _rightPanelWidth

        property real rightEdgeCenterInset: visible ? parent.width - x : 0

        state:                  _verticalCenter ? "verticalCenter" : "topAnchor"
        states: [
            State {
                name: "verticalCenter"
                AnchorChanges {
                    target:                 photoVideoControl
                    anchors.top:            undefined
                    anchors.verticalCenter: _root.verticalCenter
                }
            },
            State {
                name: "topAnchor"
                AnchorChanges {
                    target:                 photoVideoControl
                    anchors.verticalCenter: undefined
                    anchors.top:            instrumentPanel.bottom
                }
            }
        ]

        property bool _verticalCenter: !QGroundControl.settingsManager.flyViewSettings.alternateInstrumentPanel.rawValue
    }

    TelemetryValuesBar {
        id:                 telemetryPanel
        x:                  recalcXPosition()
        anchors.margins:    _toolsMargin

        property real bottomEdgeCenterInset: 0
        property real rightEdgeCenterInset: 0

        // States for custom layout support
        states: [
            State {
                name: "bottom"
                when: telemetryPanel.bottomMode

                AnchorChanges {
                    target: telemetryPanel
                    anchors.top: undefined
                    anchors.bottom: parent.bottom
                    anchors.right: undefined
                    anchors.verticalCenter: undefined
                }

                PropertyChanges {
                    target: telemetryPanel
                    x: recalcXPosition()
                    bottomEdgeCenterInset: visible ? parent.height-y : 0
                    rightEdgeCenterInset: 0
                }
            },

            State {
                name: "right-video"
                when: !telemetryPanel.bottomMode && photoVideoControl.visible

                AnchorChanges {
                    target: telemetryPanel
                    anchors.top: photoVideoControl.bottom
                    anchors.bottom: undefined
                    anchors.right: parent.right
                    anchors.verticalCenter: undefined
                }
                PropertyChanges {
                    target: telemetryPanel
                    bottomEdgeCenterInset: 0
                    rightEdgeCenterInset: visible ? parent.width - x : 0
                }
            },

            State {
                name: "right-novideo"
                when: !telemetryPanel.bottomMode && !photoVideoControl.visible

                AnchorChanges {
                    target: telemetryPanel
                    anchors.top: undefined
                    anchors.bottom: undefined
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                }
                PropertyChanges {
                    target: telemetryPanel
                    bottomEdgeCenterInset: 0
                    rightEdgeCenterInset: visible ? parent.width - x : 0
                }
            }
        ]

        function recalcXPosition() {
            // First try centered
            var halfRootWidth   = _root.width / 2
            var halfPanelWidth  = telemetryPanel.width / 2
            var leftX           = (halfRootWidth - halfPanelWidth) - _toolsMargin
            var rightX          = (halfRootWidth + halfPanelWidth) + _toolsMargin
            if (leftX >= parentToolInsets.leftEdgeBottomInset || rightX <= parentToolInsets.rightEdgeBottomInset ) {
                // It will fit in the horizontalCenter
                return halfRootWidth - halfPanelWidth
            } else {
                // Anchor to left edge
                return parentToolInsets.leftEdgeBottomInset + _toolsMargin
            }
        }
    }

    /*taisync add*/
    TaisyncInfo
    {
        id:taisyncPro
    }

    property bool _paramBoxShowEnable: QGroundControl.settingsManager.appSettings.taisyncFlyViewShow.value
    property bool _paramBoxVisible: true
    Rectangle
    {
        id: paramBox
        x: parent.width/2-width/2
        y: parent.height/2-height/2
        border.width: 1
        border.color: "black"
        width: 300*3//gridDataView.cellWidth * 2
        height: 125*3
        radius: 5
        clip: true
        color: Qt.rgba(255,255,255,100/255)
        visible: _paramBoxShowEnable && _paramBoxVisible

        MouseArea
        {
            anchors.fill: parent
            drag.target: paramBox
        }

        // Label {
        //     id: modelLabel1
        //     visible: false
        //     text: "000000000"
        //     font.pointSize: 12
        //     color: "#000000"
        // }

        Column
        {
            //row 1
            Row
            {
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            width: 10*3
                            height: 10*3
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 50*3
                            text: "airRSSI1"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: "-"+taisyncPro.airRSSI0 +"dBm"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            width: 50*3
                            text: "gndRSSI1"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: "-"+taisyncPro.gndRSSI0+"dBm"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
            //row 2
            Row
            {
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            width: 10*3
                            height: 10*3
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 50*3
                            text: "airRSSI2"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: "-"+taisyncPro.airRSSI1+"dBm"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            width: 50*3
                            text: "gndRSSI2"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: "-"+taisyncPro.gndRSSI1+"dBm"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            //row 3
            Row
            {
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            width: 10*3
                            height: 10*3
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 50*3
                            text: "airSNR"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.airSNR + "dB"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            width: 50*3
                            text: "gndSNR"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.gndSNR + "dB"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }
            //row 4
            Row
            {
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            width: 10*3
                            height: 10*3
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 50*3
                            text: "airPass"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            id: viewAirPass
                            width: 70*3
                            text: taisyncPro.airLDPCPass
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            width: 50*3
                            text: "gndPass"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.gndLDPCPass
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            //row 5
            Row
            {
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            width: 10*3
                            height: 10*3
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 50*3
                            text: "airFailed"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.airLDPCFailed
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            width: 50*3
                            text: "gndFailed"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.gndLDPCFailed
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            //row 6
            Row
            {
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            width: 10*3
                            height: 10*3
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 50*3
                            text: "airAnt"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.ant
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            width: 50*3
                            text: "gndAnt"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.antGnd
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            //row 7
            Row
            {
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            width: 10*3
                            height: 10*3
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 50*3
                            text: "freq"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.currFreq
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            width: 50*3
                            text: "mcs"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.mcs
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            //row 8
            Row
            {
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            width: 10*3
                            height: 10*3
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 50*3
                            text: "range"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.range + "m"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
                Item
                {
                    width: 150*3
                    height: 15*3
                    Row
                    {
                        spacing: 10*3
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            width: 50*3
                            text: "rate"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: 70*3
                            text: taisyncPro.dataRate+"kbps"
                            font.pointSize: 12
                            color: "#000000"
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            // //row 9
            // Row
            // {
            //     Item
            //     {
            //         width: 70*3
            //         height: 15*3
            //         Label {
            //             anchors.fill: parent
            //             anchors.leftMargin: 20*3
            //             font.pointSize: 12
            //             text: "Save log"
            //             verticalAlignment: Text.AlignVCenter
            //         }
            //     }
            //     Item {
            //         width: 50*3
            //         height: 15*3
            //     }
            //     Item
            //     {
            //         width: 30*3
            //         height: 15*3
            //         Switch
            //         {
            //             id: logSwitch
            //             anchors.fill: parent
            //             checked: false
            //             indicator: Rectangle
            //             {
            //                 x: logSwitch.leftPadding
            //                 y: parent.height/2 - height/2
            //                 implicitHeight: 15*3
            //                 implicitWidth: 30*3
            //                 radius: height/2
            //                 color:logSwitch.checked ? "#B0C4DE" : "#ffffff"
            //                 border.color: "#cccccc"
            //                 Rectangle
            //                 {
            //                     id: smallRect
            //                     x: 1
            //                     y: 1
            //                     width: parent.height-2
            //                     height: parent.height-2
            //                     radius: height/2
            //                     color: "#000000"
            //                     border.color: "#000000"

            //                     NumberAnimation on x
            //                     {
            //                         to: smallRect.width+2
            //                         running: logSwitch.checked ? true : false
            //                         duration: 200
            //                     }

            //                     NumberAnimation on x
            //                     {
            //                         to: 1
            //                         running: logSwitch.checked ? false : true
            //                         duration: 200
            //                     }
            //                 }
            //             }
            //             onCheckedChanged:
            //             {
            //                 if(checked)
            //                 {
            //                     console.log("start save log");
            //                     taisyncPro.startLogSave();
            //                 }
            //                 else
            //                 {
            //                     console.log("stop save log");
            //                     taisyncPro.stopLogSave();
            //                 }
            //             }
            //         }
            //     }
            // }
        }

        Item
        {
            id: hideButton
            anchors.right: parent.right
            anchors.top: parent.top
            width: 25*3
            height: 25*3
            Image {
                anchors.fill: parent
                anchors.margins: paramBox.border.width
                fillMode: Image.PreserveAspectFit
                source: "/res/resources/hide.png"
            }
            MouseArea
            {
                anchors.fill: parent
                propagateComposedEvents: true
                onClicked:
                {
                    _paramBoxVisible = false;
                }
            }
        }
    }
    Rectangle
    {
        id: viewButton
        visible: _paramBoxShowEnable && !_paramBoxVisible
        x: parent.width/2-width/2
        y: parent.height/2-height/2
        width: (25-border.width*2)*3
        height: (25-border.width*2)*3
        border.width: 1
        border.color: "black"
        radius: 5
        color: Qt.rgba(255,255,255,100/255)
        Image {
            anchors.fill: parent
            anchors.margins: viewButton.border.width
            fillMode: Image.PreserveAspectFit
            source: "/res/resources/view.png"
        }
        MouseArea
        {
            anchors.fill: parent
            onClicked:
            {
                _paramBoxVisible = true;
            }
            drag.target: viewButton
        }
    }

    //-- Virtual Joystick
    Loader {
        id:                         virtualJoystickMultiTouch
        z:                          QGroundControl.zOrderTopMost + 1
        width:                      parent.width  - (_pipOverlay.width / 2)
        height:                     Math.min(parent.height * 0.25, ScreenTools.defaultFontPixelWidth * 16)
        visible:                    _virtualJoystickEnabled && !QGroundControl.videoManager.fullScreen && !(_activeVehicle ? _activeVehicle.usingHighLatencyLink : false)
        anchors.bottom:             parent.bottom
        anchors.bottomMargin:       parentToolInsets.leftEdgeBottomInset + ScreenTools.defaultFontPixelHeight * 2
        anchors.horizontalCenter:   parent.horizontalCenter
        source:                     "qrc:/qml/VirtualJoystick.qml"
        active:                     _virtualJoystickEnabled && !(_activeVehicle ? _activeVehicle.usingHighLatencyLink : false)

        property bool autoCenterThrottle: QGroundControl.settingsManager.appSettings.virtualJoystickAutoCenterThrottle.rawValue

        property bool _virtualJoystickEnabled: QGroundControl.settingsManager.appSettings.virtualJoystick.rawValue

        property real bottomEdgeLeftInset: parent.height-y
        property real bottomEdgeRightInset: parent.height-y

        // Width is difficult to access directly hence this hack which may not work in all circumstances
        property real leftEdgeBottomInset: visible ? bottomEdgeLeftInset + width/18 - ScreenTools.defaultFontPixelHeight*2 : 0
        property real rightEdgeBottomInset: visible ? bottomEdgeRightInset + width/18 - ScreenTools.defaultFontPixelHeight*2 : 0
    }

    FlyViewToolStrip {
        id:                     toolStrip
        anchors.leftMargin:     _toolsMargin + parentToolInsets.leftEdgeCenterInset
        anchors.topMargin:      _toolsMargin + parentToolInsets.topEdgeLeftInset
        anchors.left:           parent.left
        anchors.top:            parent.top
        z:                      QGroundControl.zOrderWidgets
        maxHeight:              parent.height - y - parentToolInsets.bottomEdgeLeftInset - _toolsMargin
        visible:                !QGroundControl.videoManager.fullScreen

        onDisplayPreFlightChecklist: preFlightChecklistPopup.createObject(mainWindow).open()


        property real topEdgeLeftInset: visible ? y + height : 0
        property real leftEdgeTopInset: visible ? x + width : 0
    }

    GripperMenu {
        id: gripperOptions
    }

    VehicleWarnings {
        anchors.centerIn:   parent
        z:                  QGroundControl.zOrderTopMost
    }

    MapScale {
        id:                 mapScale
        anchors.margins:    _toolsMargin
        anchors.left:       toolStrip.right
        anchors.top:        parent.top
        mapControl:         _mapControl
        buttonsOnLeft:      true
        visible:            !ScreenTools.isTinyScreen && QGroundControl.corePlugin.options.flyView.showMapScale && mapControl.pipState.state === mapControl.pipState.fullState

        property real topEdgeCenterInset: visible ? y + height : 0
    }

    Component {
        id: preFlightChecklistPopup
        FlyViewPreFlightChecklistPopup {
        }
    }
}
