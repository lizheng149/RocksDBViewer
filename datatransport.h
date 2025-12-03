#ifndef DATATRANSPORT_H
#define DATATRANSPORT_H

#include <QObject>
#include <QStringList>
#include <QDir>

class dataTransport : public QObject
{
    Q_OBJECT
public:
    explicit dataTransport(QObject *parent = nullptr);

    Q_INVOKABLE QStringList getKeysInDirectory(const QString &path);

    Q_INVOKABLE QString getValue(const QString &filename);

    // 额外送你一个功能 - 检查路径是否存在
    Q_INVOKABLE bool pathExists(const QString &path) {
        return QDir(path).exists();
    }
signals:
};

#endif // DATATRANSPORT_H
