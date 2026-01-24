/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QGroundControl
import QGroundControl.Controls
import QGroundControl.ScreenTools
import QGroundControl.Palette

Item {
    id:             control
    width:          micromIcon.width * 1.1 + statusColumn.width + margins
    anchors.top:    parent.top
    anchors.bottom: parent.bottom

    property var    _micromController:  QGroundControl.micromController
    property bool   showIndicator:      _micromController !== null
    property var    margins:            ScreenTools.defaultFontPixelWidth
    property var    panelRadius:        ScreenTools.defaultFontPixelWidth * 0.5
    property real   _sliderWidth:       ScreenTools.defaultFontPixelWidth * 25

    // UV color palette data
    property var uvColors: ["#FF0000", "#FF8000", "#FFFF00", "#00FF00", "#00FFFF", "#0080FF", "#8000FF", "#FF00FF"]
    property var uvColorNames: ["Red", "Orange", "Yellow", "Green", "Light Blue", "Blue", "Purple", "Pink"]

    function getUVColor(idx) {
        return uvColors[idx] || "#FF0000"
    }

    function getUVColorName(idx) {
        return uvColorNames[idx] || "Red"
    }

    // Popup control panel
    Component {
        id: micromControlsPage

        ToolIndicatorPage {
            contentComponent: ColumnLayout {
                spacing: ScreenTools.defaultFontPixelHeight * 0.75

                // Title
                QGCLabel {
                    text:               qsTr("MicROM UV Camera")
                    font.pointSize:     ScreenTools.mediumFontPointSize
                    font.weight:        Font.Bold
                    Layout.alignment:   Qt.AlignHCenter
                }

                // Connection and SD card status
                RowLayout {
                    spacing: ScreenTools.defaultFontPixelWidth * 1.5
                    Layout.alignment: Qt.AlignHCenter

                    // Connection status
                    RowLayout {
                        spacing: ScreenTools.defaultFontPixelWidth * 0.5

                        Rectangle {
                            width:  ScreenTools.defaultFontPixelHeight * 0.8
                            height: width
                            radius: width / 2
                            color:  _micromController && _micromController.connected ? "green" : "red"
                        }
                        QGCLabel {
                            text: _micromController && _micromController.connected ? qsTr("Connected") : qsTr("Disconnected")
                        }
                    }

                    // SD Card status
                    RowLayout {
                        spacing: ScreenTools.defaultFontPixelWidth * 0.5
                        visible: _micromController && _micromController.connected

                        Rectangle {
                            width:  ScreenTools.defaultFontPixelHeight * 0.8
                            height: width
                            radius: width / 2
                            color:  _micromController && _micromController.sdCardPresent ? "green" : "orange"
                        }
                        QGCLabel {
                            text: _micromController && _micromController.sdCardPresent ? qsTr("SD Card OK") : qsTr("No SD Card")
                        }
                    }
                }

                // Error message display
                QGCLabel {
                    text:               _micromController ? _micromController.lastError : ""
                    color:              "red"
                    font.pointSize:     ScreenTools.smallFontPointSize
                    Layout.alignment:   Qt.AlignHCenter
                    visible:            _micromController && _micromController.lastError !== ""
                }

                // Separator
                Rectangle {
                    Layout.fillWidth:       true
                    Layout.preferredHeight: 1
                    color:                  qgcPal.windowShade
                }

                // Photo/Video buttons
                RowLayout {
                    spacing:            ScreenTools.defaultFontPixelWidth * 2
                    Layout.alignment:   Qt.AlignHCenter

                    QGCButton {
                        text:               qsTr("Take Photo")
                        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * 14
                        enabled:            _micromController && _micromController.connected
                        onClicked: {
                            if (_micromController) {
                                _micromController.takePhoto()
                            }
                        }
                    }

                    QGCButton {
                        text:               _micromController && _micromController.recording ? qsTr("Stop Recording") : qsTr("Start Recording")
                        Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * 16
                        highlighted:        _micromController && _micromController.recording
                        enabled:            _micromController && _micromController.connected
                        onClicked: {
                            if (_micromController) {
                                if (_micromController.recording) {
                                    _micromController.stopVideo()
                                } else {
                                    _micromController.startVideo()
                                }
                            }
                        }

                        // Recording indicator
                        Rectangle {
                            visible:        _micromController && _micromController.recording
                            width:          ScreenTools.defaultFontPixelHeight * 0.6
                            height:         width
                            radius:         width / 2
                            color:          "red"
                            anchors.left:   parent.left
                            anchors.leftMargin: ScreenTools.defaultFontPixelWidth
                            anchors.verticalCenter: parent.verticalCenter

                            SequentialAnimation on opacity {
                                running:    _micromController && _micromController.recording
                                loops:      Animation.Infinite
                                NumberAnimation { to: 0.3; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                        }
                    }
                }

                // Separator
                Rectangle {
                    Layout.fillWidth:       true
                    Layout.preferredHeight: 1
                    color:                  qgcPal.windowShade
                }

                // Zoom control
                ColumnLayout {
                    spacing: ScreenTools.defaultFontPixelHeight * 0.25
                    Layout.fillWidth: true

                    RowLayout {
                        spacing: ScreenTools.defaultFontPixelWidth
                        Layout.fillWidth: true

                        QGCLabel {
                            text:                   qsTr("Zoom:")
                            Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * 8
                        }

                        QGCLabel {
                            text:                   zoomSlider.value.toFixed(0) + " / 13"
                            Layout.fillWidth:       true
                            horizontalAlignment:    Text.AlignRight
                        }
                    }

                    Slider {
                        id:                     zoomSlider
                        from:                   0
                        to:                     13
                        stepSize:               1
                        value:                  _micromController ? _micromController.zoom : 0
                        Layout.preferredWidth:  _sliderWidth
                        enabled:                _micromController && _micromController.connected

                        onPressedChanged: {
                            if (!pressed && _micromController) {
                                _micromController.setZoom(value)
                            }
                        }
                    }
                }

                // Gain/Sensitivity control
                ColumnLayout {
                    spacing: ScreenTools.defaultFontPixelHeight * 0.25
                    Layout.fillWidth: true

                    RowLayout {
                        spacing: ScreenTools.defaultFontPixelWidth
                        Layout.fillWidth: true

                        QGCLabel {
                            text:                   qsTr("Gain (Sensitivity):")
                            Layout.preferredWidth:  ScreenTools.defaultFontPixelWidth * 16
                        }

                        QGCLabel {
                            text:                   gainSlider.value.toFixed(0) + " / 255"
                            Layout.fillWidth:       true
                            horizontalAlignment:    Text.AlignRight
                        }
                    }

                    Slider {
                        id:                     gainSlider
                        from:                   0
                        to:                     255
                        stepSize:               1
                        value:                  _micromController ? _micromController.gain : 130
                        Layout.preferredWidth:  _sliderWidth
                        enabled:                _micromController && _micromController.connected

                        onPressedChanged: {
                            if (!pressed && _micromController) {
                                _micromController.setGain(value)
                            }
                        }
                    }

                    QGCLabel {
                        text:               qsTr("Higher gain = more UV sensitivity")
                        font.pointSize:     ScreenTools.smallFontPointSize
                        Layout.alignment:   Qt.AlignHCenter
                        opacity:            0.7
                    }
                }

                // Separator
                Rectangle {
                    Layout.fillWidth:       true
                    Layout.preferredHeight: 1
                    color:                  qgcPal.windowShade
                }

                // UV Color Palette control
                ColumnLayout {
                    spacing:    ScreenTools.defaultFontPixelHeight * 0.25
                    Layout.fillWidth: true

                    RowLayout {
                        spacing: ScreenTools.defaultFontPixelWidth
                        Layout.fillWidth: true

                        QGCLabel {
                            text:               qsTr("UV Color:")
                            font.pointSize:     ScreenTools.defaultFontPointSize
                            font.weight:        Font.Medium
                        }

                        QGCLabel {
                            property int colorIndex: _micromController ? _micromController.uvColor : 0
                            text:                   control.getUVColorName(colorIndex)
                            Layout.fillWidth:       true
                            horizontalAlignment:    Text.AlignRight
                        }
                    }

                    Slider {
                        id:                     uvColorSlider
                        from:                   0
                        to:                     7
                        stepSize:               1
                        value:                  _micromController ? _micromController.uvColor : 0
                        Layout.preferredWidth:  _sliderWidth
                        enabled:                _micromController && _micromController.connected

                        background: Item {
                            x:              uvColorSlider.leftPadding
                            y:              uvColorSlider.topPadding + uvColorSlider.availableHeight / 2 - height / 2
                            width:          uvColorSlider.availableWidth
                            height:         ScreenTools.defaultFontPixelHeight * 0.5

                            // Row of colored rectangles for gradient
                            Row {
                                anchors.fill: parent
                                Repeater {
                                    model: 8
                                    Rectangle {
                                        width:  parent.width / 8
                                        height: parent.height
                                        color:  control.getUVColor(index)
                                        radius: index === 0 || index === 7 ? height / 2 : 0
                                    }
                                }
                            }
                        }

                        handle: Rectangle {
                            x:              uvColorSlider.leftPadding + uvColorSlider.visualPosition * (uvColorSlider.availableWidth - width)
                            y:              uvColorSlider.topPadding + uvColorSlider.availableHeight / 2 - height / 2
                            width:          ScreenTools.defaultFontPixelHeight * 1.5
                            height:         width
                            radius:         width / 2
                            color:          uvColorSlider.pressed ? qgcPal.buttonHighlight : qgcPal.button
                            border.color:   qgcPal.buttonText
                            border.width:   1
                        }

                        onPressedChanged: {
                            if (!pressed && _micromController) {
                                _micromController.setUVColor(value)
                            }
                        }
                    }

                    QGCLabel {
                        text:               qsTr("Color of UV detection overlay")
                        font.pointSize:     ScreenTools.smallFontPointSize
                        Layout.alignment:   Qt.AlignHCenter
                        opacity:            0.7
                    }
                }
            }
        }
    }

    // Toolbar icon
    QGCColoredImage {
        id:                     micromIcon
        width:                  height
        anchors.top:            parent.top
        anchors.bottom:         parent.bottom
        source:                 "/qmlimages/CameraComponentIcon.svg"
        fillMode:               Image.PreserveAspectFit
        sourceSize.height:      height
        color:                  qgcPal.buttonText

        // Recording indicator dot
        Rectangle {
            visible:        _micromController && _micromController.recording
            width:          parent.width * 0.35
            height:         width
            radius:         width / 2
            color:          "red"
            anchors.top:    parent.top
            anchors.right:  parent.right
            anchors.margins: 2

            SequentialAnimation on opacity {
                running:    _micromController && _micromController.recording
                loops:      Animation.Infinite
                NumberAnimation { to: 0.3; duration: 500 }
                NumberAnimation { to: 1.0; duration: 500 }
            }
        }
    }

    // Status column
    Column {
        id:                     statusColumn
        anchors.left:           micromIcon.right
        anchors.leftMargin:     ScreenTools.defaultFontPixelWidth * 0.5
        anchors.verticalCenter: parent.verticalCenter
        spacing:                0

        QGCLabel {
            text:           qsTr("MicROM")
            font.pointSize: ScreenTools.smallFontPointSize
        }

        QGCLabel {
            text:           _micromController && _micromController.connected ?
                                (_micromController.recording ? qsTr("REC") : qsTr("Z:%1 G:%2").arg(_micromController.zoom).arg(_micromController.gain)) :
                                qsTr("--")
            font.pointSize: ScreenTools.smallFontPointSize
            color:          _micromController && _micromController.recording ? "red" : qgcPal.buttonText
            opacity:        _micromController && _micromController.recording ? 1.0 : 0.8
        }
    }

    // Click handler
    MouseArea {
        anchors.fill: parent
        onClicked: {
            mainWindow.showIndicatorDrawer(micromControlsPage, control)
        }
    }
}
