/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.3
//import QtQuick.Controls 2.3
import QtLocation       5.3
import QtPositioning    5.3

import QGroundControl           1.0
import QGroundControl.Palette   1.0
import QGroundControl.Controls  1.0
import QGroundControl.ScreenTools       1.0

/// The MissionDistanceView control is used to add distance label between mission items
MapItemView {
    property var    mapControl
//    delegate: MapPolyline {
//        line.width: 5
//        // Note: Special visuals for ROI are hacked out for now since they are not working correctly
//        line.color: "red"
//        z:          QGroundControl.zOrderWaypointLines
//        path:       object && object.coordinate1.isValid && object.coordinate2.isValid ? [ object.coordinate1, object.coordinate2 ] : []
//    }
    delegate: MapQuickItem {
        id:             labelSegmentItem
        anchorPoint.x:  sourceItem.width / 2
        anchorPoint.y:  sourceItem.height / 2
        z:              QGroundControl.zOrderWaypointLines
        coordinate:     _getLabelCoord()

//        sourceItem: QGCLabel {
//            text: Math.round(_getDistanceValue()) + " m"
//            font.pointSize: ScreenTools.largeFontPointSize
//            color:          "white"
//        }

        sourceItem: QGCMapLabel {
            map:                mapControl
            font.family:        ScreenTools.demiboldFontFamily
            text:               Math.round(_getDistanceValue()) + " m"
        }

        function _getLabelCoord() {
            if (object && object.coordinate1.isValid && object.coordinate2.isValid) {
                var distance = object.coordinate1.distanceTo(object.coordinate2)
                var azimuth = object.coordinate1.azimuthTo(object.coordinate2)
                return object.coordinate1.atDistanceAndAzimuth(distance / 3, azimuth)
            } else {
                return QtPositioning.coordinate()
            }
        }

        function _getDistanceValue() {
            if (object && object.coordinate1.isValid && object.coordinate2.isValid) {
                var distance = object.coordinate1.distanceTo(object.coordinate2)
                return distance
            } else {
                return 0
            }
        }
    }
}
