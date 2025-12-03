import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform
import QtQuick.Dialogs
import Qt.labs.settings
Page {
    id: mainPage
    title: qsTr("数据库管理")
    anchors.fill: parent
    padding: 16

    property string filePathText: ""
    property var allKeys: []  // 存储所有原始key数据
    property string currentKey: ""  // 当前选中的key

    FolderDialog {
        id: folderDialog
        title: "请选择一个文件夹"
        options: FolderDialog.ShowDirsOnly // 仅显示文件夹
        currentFolder: StandardPaths.standardLocations(StandardPaths.DesktopLocation)[0]
        onAccepted: {
            console.log("选中的文件夹路径为: " + selectedFolder )
            filePathText = selectedFolder
            dbPathField.text = filePathText.replace("file:///","")
        }
        onRejected: {
            console.log("取消选择")
        }
    }

    MessageDialog {
        id:msgbox
        buttons: MessageDialog.Ok
        text: "解析Json失败！"
    }

    // 顶部数据库选择区域
    RowLayout {
        id: topLayout
        spacing: 8
        width: parent.width

        Button {
            id: selectDbBtn
            text: qsTr("选择数据库")
            Layout.minimumWidth: 120
            onClicked: {
                folderDialog.open()
            }
        }

        TextField {
            id: dbPathField
            placeholderText: qsTr("数据库路径")
            Layout.fillWidth: true
        }

        Button {
            id: confirmBtn
            text: qsTr("确定")
            Layout.minimumWidth: 80
            onClicked: {
                if (DTS.pathExists(dbPathField.text)) {
                    // 获取所有keys并存储
                    allKeys = DTS.getKeysInDirectory(dbPathField.text)
                    // 应用筛选（初始为空，显示全部）
                    applyFilter()
                } else {
                    console.log("路径不存在！")
                    // 这里可以加个提示dialog
                }
            }
        }
    }

    // 筛选区域
    RowLayout {
        id: filterLayout
        spacing: 8
        width: parent.width
        anchors.top: topLayout.bottom
        anchors.topMargin: 16

        TextField {
            id: filterField
            placeholderText: qsTr("筛选内容")
            Layout.fillWidth: true
        }

        Button {
            id: filterBtn
            text: qsTr("筛选")
            Layout.minimumWidth: 80
            onClicked: {
                applyFilter()
            }
        }

        Button {
            id: formatBtn
            text: qsTr("美化")
            Layout.minimumWidth: 80
            onClicked: {
                applyFormat()
            }
        }
    }

    // 内容显示区域
    RowLayout {
        id: contentLayout
        spacing: 16
        width: parent.width
        anchors.top: filterLayout.bottom
        anchors.topMargin: 16
        anchors.bottom: parent.bottom
        readonly property var clRatio : [0.3,0.7]

        // 左侧key列表
        Rectangle {
            id: keyListArea
            color: "#f5f5f5"
            border.color: "#ddd"
            border.width: 1
            radius: 4
            Layout.fillHeight: true
            Layout.preferredWidth: (contentLayout.width - contentLayout.spacing) * contentLayout.clRatio[0]

            ListView {
                id: keyListView
                anchors.fill: parent
                clip: true
                currentIndex: -1 // 初始无选中项

                delegate: Rectangle {
                    width: parent.width
                    height: 40
                    color: keyListView.currentIndex == index ? "#e0e0ff" : "transparent"
                    border.color: keyListView.currentIndex == index ? "#8a8aff" : "transparent"
                    border.width: keyListView.currentIndex == index ? 1 : 0

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        text: modelData
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // 保存之前可能未保存的更改
                            if (currentKey !== "" && valuetext.text !== DTS.getValue(currentKey)) {
                                // 这里可以添加保存逻辑，例如：
                                // DTS.setValue(currentKey, valuetext.text)
                                console.log("检测到未保存的更改，对key: " + currentKey)
                            }

                            keyListView.currentIndex = index
                            currentKey = modelData
                            valuetext.text = DTS.getValue(modelData)
                            // 重置修改状态
                            isModified = false
                            saveBtn.enabled = false
                        }
                    }
                }
            }
        }

        // 右侧value显示
        Rectangle {
            id: valueArea
            color: "#f5f5f5"
            border.color: "#ddd"
            border.width: 1
            radius: 4
            Layout.fillHeight: true
            Layout.preferredWidth: (contentLayout.width - contentLayout.spacing) * contentLayout.clRatio[1]

            Column {
                anchors.fill: parent
                spacing: 8

                // 保存按钮区域
                RowLayout {
                    width: parent.width
                    spacing: 8
                    visible: saveBtn.enabled

                    Text {
                        text: qsTr("内容已修改")
                        color: "#e67e22"
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Button {
                        id: saveBtn
                        text: qsTr("保存更改")
                        enabled: false
                        onClicked: {
                            if (currentKey !== "") {
                                DTS.setValue(currentKey, valuetext.text)
                                isModified = false
                                saveBtn.enabled = false
                                console.log("已保存key: " + currentKey + " 的更改")
                            }
                        }
                    }
                }

                ScrollView {
                    //anchors.fill: parent
                    implicitWidth: parent.width
                    implicitHeight: parent.height

                    TextArea {
                        id: valuetext
                        width: parent.width
                        text: qsTr("选择一个key查看对应的值")
                        color: "#333"  // 深色文字提高可读性
                        wrapMode: TextArea.Wrap
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignTop
                        font.pixelSize: 16
                        // 允许编辑和选择
                        readOnly: false
                        selectByMouse: true
                        activeFocusOnPress: true
                        // 背景设置
                        background: Rectangle {
                            color: "white"  // 编辑区域使用白色背景
                            border.color: "#ccc"
                            border.width: 1
                            radius: 2
                        }
                        // 内边距
                        leftPadding: 10
                        rightPadding: 10
                        topPadding: 10
                        bottomPadding: 10
                        // 自动调整高度
                        implicitHeight: contentHeight
                        // 当文本修改时标记为已修改
                        onTextChanged: {
                            if (currentKey !== "" && text !== DTS.getValue(currentKey)) {
                                isModified = true
                                saveBtn.enabled = true
                            } else {
                                isModified = false
                                saveBtn.enabled = false
                            }
                        }
                    }
                }
            }
        }
    }

    // 筛选功能函数
    function applyFilter() {
        if (allKeys.length === 0) {
            keyListView.model = []
            return
        }

        var filterText = filterField.text.trim().toLowerCase()
        var filteredKeys = []

        // 简单的包含判断，不区分大小写
        for (var i = 0; i < allKeys.length; i++) {
            var key = allKeys[i]
            if (filterText === "" || key.toLowerCase().includes(filterText)) {
                filteredKeys.push(key)
            }
        }

        keyListView.model = filteredKeys
        console.log("筛选结果数量: " + filteredKeys.length)

        // 重置选中项
        keyListView.currentIndex = -1
        currentKey = ""
        valuetext.text = qsTr("选择一个key查看对应的值")
        saveBtn.enabled = false
    }

    function applyFormat() {
        try{
            const sJson = JSON.parse(valuetext.text)
            valuetext.text = qsTr(JSON.stringify(sJson,null,4))
        }
        catch(err)
        {
            msgbox.open()
        }

    }

    // 文本修改状态
    property bool isModified: false
}
