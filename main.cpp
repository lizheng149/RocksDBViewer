#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include "datatransport.h"
int main(int argc, char *argv[])
{
    // Qt 6中高DPI缩放默认开启，无需额外设置
    QGuiApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/res/icon/Icon/DB.png")); // 设置全局窗口图标

    QQuickStyle::setStyle("Fusion");

    QQmlApplicationEngine engine;

    // 注册应用版本
    engine.rootContext()->setContextProperty("APP_VERSION", "1.0.0");


    // ...
    dataTransport DTS;
    engine.rootContext()->setContextProperty("DTS", &DTS);
    // 加载主QML文件 (Qt 6中QUrl解析更严格，确保路径正确)
    const QUrl url(u"qrc:/App.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    // Qt 6中推荐使用QML_IMPORT_PATH添加模块搜索路径
    engine.addImportPath(u"qrc:/src"_qs);
    engine.load(url);

    return app.exec();
}
