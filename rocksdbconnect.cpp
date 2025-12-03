#include "RocksDBConnect.h"
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <atomic>
#include <mutex>
#include <memory>

#pragma comment(lib, "Rpcrt4.lib")

static std::string dbFilename = ".\\static\\RocksDBData";

// 静态成员变量初始化
rocksdb::DB* RocksDBConnect::globalDB_ = nullptr;
std::atomic<int> RocksDBConnect::refCount_(0);
std::mutex RocksDBConnect::initMutex_;

RocksDBConnect::RocksDBConnect() : db_(nullptr)
{
    acquireDB();
}

void RocksDBConnect::acquireDB()
{
    std::lock_guard<std::mutex> lock(initMutex_);

    // 如果全局连接尚未建立，创建它
    if (globalDB_ == nullptr) {
        rocksdb::Options options;
        options.create_if_missing = true;
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();

        rocksdb::Status status = rocksdb::DB::Open(options, dbFilename, &globalDB_);
        if (!status.ok()) {
            std::ostringstream oss;
            oss << "打开RocksDB数据库失败: " << status.ToString();
            qDebug()<<(oss.str().c_str());
            throw std::runtime_error(oss.str());
        }
        qDebug()<<("RocksDB数据库已成功打开");
    }

    // 增加引用计数并获取连接
    db_ = globalDB_;
    refCount_.fetch_add(1, std::memory_order_relaxed);
}

void RocksDBConnect::releaseDB()
{
    if (db_ == nullptr) return;

    std::lock_guard<std::mutex> lock(initMutex_);

    // 减少引用计数
    int currentCount = refCount_.fetch_sub(1, std::memory_order_relaxed);
    int newCount = currentCount - 1;

    // 如果是最后一个引用，关闭数据库
    if (newCount == 0 && globalDB_ != nullptr) {
        delete globalDB_;
        globalDB_ = nullptr;
        qDebug()<<("RocksDB数据库已关闭（无活跃引用）");
    }

    db_ = nullptr;
}

void RocksDBConnect::setDBFilePath(std::string filename)
{
    std::lock_guard<std::mutex> lock(initMutex_);
    if (globalDB_ != nullptr) {
        throw std::runtime_error("数据库已打开，无法更改路径");
    }
    dbFilename = filename;
}

RocksDBConnect::~RocksDBConnect()
{
    releaseDB();
}

// 禁止拷贝
//RocksDBConnect::RocksDBConnect(const RocksDBConnect&) = delete;
//RocksDBConnect& RocksDBConnect::operator=(const RocksDBConnect&) = delete;

// 允许移动
RocksDBConnect::RocksDBConnect(RocksDBConnect&& other) noexcept : db_(other.db_)
{
    other.db_ = nullptr;
}

RocksDBConnect& RocksDBConnect::operator=(RocksDBConnect&& other) noexcept
{
    if (this != &other) {
        releaseDB();  // 释放当前持有的连接
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

// 检查是否还有活跃连接
bool RocksDBConnect::hasActiveConnections()
{
    return refCount_.load(std::memory_order_relaxed) > 0;
}

// 关闭所有连接（强制关闭，不管引用计数）
void RocksDBConnect::forceCloseAllConnections()
{
    std::lock_guard<std::mutex> lock(initMutex_);

    if (globalDB_ != nullptr) {
        // 警告：这会强制关闭所有连接，可能导致正在使用连接的实例出现问题
        delete globalDB_;
        globalDB_ = nullptr;
        refCount_.store(0, std::memory_order_relaxed);
        qDebug()<<("RocksDB连接已被强制关闭");
    }
}

// 其他数据库操作方法
void RocksDBConnect::Set(std::string key1, std::string value)
{
    if (db_ == nullptr) {
        throw std::runtime_error("数据库连接无效");
    }

    rocksdb::Status status = db_->Put(rocksdb::WriteOptions(), key1, value);
    if (!status.ok()) {
        std::ostringstream oss;
        oss << "写入RocksDB失败 (key: " << key1 << "): " << status.ToString();
        qDebug()<<(oss.str().c_str());
        throw std::runtime_error(oss.str());
    }
}

void RocksDBConnect::Set(std::string key1, std::string key2, std::string value)
{
    if (db_ == nullptr) {
        throw std::runtime_error("数据库连接无效");
    }

    std::string key = key1 + "|+|" + key2;
    rocksdb::Status status = db_->Put(rocksdb::WriteOptions(), key, value);
    if (!status.ok()) {
        std::ostringstream oss;
        oss << "写入RocksDB失败 (key: " << key << "): " << status.ToString();
        qDebug()<<(oss.str().c_str());
        throw std::runtime_error(oss.str());
    }
}

void RocksDBConnect::Set(std::string key1, std::string key2, std::string key3, std::string value)
{
    if (db_ == nullptr) {
        throw std::runtime_error("数据库连接无效");
    }

    std::string key = key1 + "|+|" + key2 + "|+|" + key3;
    rocksdb::Status status = db_->Put(rocksdb::WriteOptions(), key, value);
    if (!status.ok()) {
        std::ostringstream oss;
        oss << "写入RocksDB失败 (key: " << key << "): " << status.ToString();
        qDebug()<<(oss.str().c_str());
        throw std::runtime_error(oss.str());
    }
}

std::string RocksDBConnect::Get(std::string key1)
{
    if (db_ == nullptr) {
        throw std::runtime_error("数据库连接无效");
    }

    std::string rValue;
    rocksdb::Status s = db_->Get(rocksdb::ReadOptions(), key1, &rValue);
    if (s.ok()) {
        return rValue;
    }
    else if (s.IsNotFound()) {
        std::ostringstream oss;
        oss << "键不存在: " << key1;
        qDebug()<<(oss.str().c_str());
        throw std::out_of_range(oss.str());
    }
    else {
        std::ostringstream oss;
        oss << "从RocksDB读取失败 (key: " << key1 << "): " << s.ToString();
        qDebug()<<(oss.str().c_str());
        throw std::runtime_error(oss.str());
    }
}

std::string RocksDBConnect::Get(std::string key1, std::string key2)
{
    if (db_ == nullptr) {
        throw std::runtime_error("数据库连接无效");
    }

    std::string key = key1 + "|+|" + key2;
    std::string value;
    rocksdb::Status status = db_->Get(rocksdb::ReadOptions(), key, &value);

    if (status.ok()) {
        return value;
    }
    else if (status.IsNotFound()) {
        std::ostringstream oss;
        oss << "键不存在: " << key;
        qDebug()<<(oss.str().c_str());
        throw std::out_of_range(oss.str());
    }
    else {
        std::ostringstream oss;
        oss << "从RocksDB读取失败 (key: " << key << "): " << status.ToString();
        qDebug()<<(oss.str().c_str());
        throw std::runtime_error(oss.str());
    }
}

std::string RocksDBConnect::Get(std::string key1, std::string key2, std::string key3)
{
    if (db_ == nullptr) {
        throw std::runtime_error("数据库连接无效");
    }

    std::string key = key1 + "|+|" + key2 + "|+|" + key3;
    std::string value;
    rocksdb::Status status = db_->Get(rocksdb::ReadOptions(), key, &value);

    if (status.ok()) {
        return value;
    }
    else if (status.IsNotFound()) {
        std::ostringstream oss;
        oss << "键不存在: " << key;
        qDebug()<<(oss.str().c_str());
        throw std::out_of_range(oss.str());
    }
    else {
        std::ostringstream oss;
        oss << "从RocksDB读取失败 (key: " << key << "): " << status.ToString();
        qDebug()<<(oss.str().c_str());
        throw std::runtime_error(oss.str());
    }
}

std::vector<std::string> RocksDBConnect::getAllKeys()
{
    std::vector<std::string> result;
    if (db_ == nullptr) {
        throw std::runtime_error("数据库连接无效");
    }
    auto it = db_->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        result.push_back(it->key().ToString());
    }
    delete it;
    return result;
}
