#include "datatransport.h"
#include <rocksdb/db.h>
#include "rocksdbconnect.h"
dataTransport::dataTransport(QObject *parent)
    : QObject{parent}
{}

QStringList dataTransport::getKeysInDirectory(const QString &path) {
    RocksDBConnect::setDBFilePath(path.toStdString());
    RocksDBConnect rdc;
    auto res = rdc.getAllKeys();
    QStringList qstrList;

    // 预分配空间提高效率
    qstrList.reserve(res.size());

    // 遍历并转换
    for (const auto& str : res) {
        qstrList.append(QString::fromStdString(str));
    }
    return qstrList;
}

QString dataTransport::getValue(const QString &sKey) {

    std::string sValue;
    std::string key = sKey.toStdString();
    RocksDBConnect rdc;
    sValue = rdc.Get(key);
    return QString::fromStdString(sValue);
}
