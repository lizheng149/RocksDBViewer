import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Window 6.2
import QtQuick.Controls.Material 6.2  // 导入Material风格模块

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1280
    height: 800
    title: qsTr("数据库查看器")

    // 正确设置Material风格（Qt 6方式）
    Material.theme: Material.Light  // 直接使用Material附加属性
    Material.accent: Material.Blue  // 可选：设置强调色

    // 加载主界面
    MainForm {
        anchors.fill: parent
    }
}
