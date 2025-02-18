import QtQuick 2.15

Column {
    property var textModel: ListModel{}
    property int maxWidth: 0
    id: containerId

    Repeater {
        model: textModel // Use the passed model

        Text {
            text: model.text
            font.pointSize: 12
            color: "#000000"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            width: maxWidth
            onImplicitWidthChanged: {
                maxWidth = Math.max(maxWidth, implicitWidth);
            }
        }
    }
}
