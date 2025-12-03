#ifndef ROCKSDBCONNECT_H
#define ROCKSDBCONNECT_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <atomic>
#include <mutex>
#include <rocksdb/db.h>
#include <QDebug>
class RocksDBConnect
{
private:
    rocksdb::DB* db_; // 当前实例持有的数据库指针
    // 全局单例数据库指针
    static rocksdb::DB* globalDB_;
    static std::atomic<int> refCount_;
    static std::mutex initMutex_;
    void acquireDB();
    void releaseDB();
public:
    RocksDBConnect();
    ~RocksDBConnect();
    // 禁止拷贝
    RocksDBConnect(const RocksDBConnect&) = delete;
    RocksDBConnect& operator=(const RocksDBConnect&) = delete;
    // 允许移动
    RocksDBConnect(RocksDBConnect&& other) noexcept;
    RocksDBConnect& operator=(RocksDBConnect&& other) noexcept;
    static void setDBFilePath(std::string filename);
    // 检查是否还有活跃连接
    static bool hasActiveConnections();
    // 强制关闭所有连接（谨慎使用）
    static void forceCloseAllConnections();
    // 基本Set/Get方法
    void Set(std::string key1, std::string value);
    std::string Get(std::string key1);
    // 两键组合
    void Set(std::string key1, std::string key2, std::string value);
    std::string Get(std::string key1, std::string key2);
    // 三键组合
    void Set(std::string key1, std::string key2, std::string key3, std::string value);
    std::string Get(std::string key1, std::string key2, std::string key3);
    // 检查键是否存在的模板函数
    template <typename... Args>
    int exists_key(Args&&... args) {
        if (db_ == nullptr) {
            throw std::runtime_error("数据库连接无效");
        }

        std::ostringstream oss;
        bool first = true;
        // Lambda表达式处理每个参数
        auto add_segment = [&](auto&& arg) {
            if (!first) oss << "|+|";  // 非首个参数前添加分隔符
            oss << std::forward<decltype(arg)>(arg);  // 输出参数
            first = false;
        };
        // 展开参数包（C++17折叠表达式）
        (add_segment(std::forward<Args>(args)), ...);

        std::string key = oss.str();
        std::string value;
        rocksdb::Status status = db_->Get(rocksdb::ReadOptions(), key, &value);

        if (status.ok()) {
            // 键存在
            return 1;
        }
        else if (status.IsNotFound()) {
            // 键不存在
            return 0;
        }
        else {
            // 其他错误
            std::ostringstream oss_err;
            oss_err << "检查键是否存在时出错 (key: " << key << "): " << status.ToString();
            qDebug()<<(oss_err.str().c_str());
            throw std::runtime_error(oss_err.str());
        }
    }

    std::vector<std::string> getAllKeys();

};

#endif // ROCKSDBCONNECT_H
