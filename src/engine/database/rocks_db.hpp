#pragma once

#include "database.hpp"

#ifdef __linux__
#define NO_ADDRESS_SAFETY_ANALYSIS __attribute__((no_sanitize_address))
#else
#define NO_ADDRESS_SAFETY_ANALYSIS
#endif

namespace rocksdb {
class DB;
class OptimisticTransactionDB;
class Transaction;
class Slice;
class Iterator;
} // namespace rocksdb

namespace Engine {
class ENGINE_API RocksDBBackend : public DatabaseBackend {
public:
    class Transaction : public DatabaseBackendTransaction {
    public:
        explicit Transaction(std::unique_ptr<rocksdb::Transaction> txn, rocksdb::DB& db);
        ~Transaction() override;

        bool commit();
        std::optional<msgpack::object_handle> get(const std::string_view& key) override;
        std::vector<msgpack::object_handle> multiGet(const std::vector<std::string>& keys) override;
        void put(const std::string_view& key, const void* data, size_t size) override;
        void remove(const std::string_view& key) override;
        std::optional<msgpack::object_handle> getForUpdate(const std::string_view& key) override;
        std::unique_ptr<DatabaseBackendIterator> seek(const std::string_view& prefix,
                                                      const std::optional<std::string_view>& lowerBound) override;

    private:
        std::unique_ptr<rocksdb::Transaction> txn;
        rocksdb::DB& db;
    };

    class Iterator : public DatabaseBackendIterator {
    public:
        explicit Iterator(std::unique_ptr<rocksdb::Iterator> iter, std::string prefix,
                          std::unique_ptr<rocksdb::Slice> lowerBound);

        bool next() override;
        operator bool() const override;
        const std::string& key() const override;
        const msgpack::object_handle& value() const override;
        std::string valueRaw() const override;

    private:
        std::unique_ptr<rocksdb::Iterator> iter;
        std::string prefix;
        std::unique_ptr<rocksdb::Slice> lowerBound;
        msgpack::object_handle oh;
        std::string iterKey;
        bool hasValue{false};
        bool isFirst{true};
    };

    explicit NO_ADDRESS_SAFETY_ANALYSIS RocksDBBackend(const Path& path);
    ~RocksDBBackend() override;

    std::optional<msgpack::object_handle> get(const std::string_view& key) override;
    std::vector<msgpack::object_handle> multiGet(const std::vector<std::string>& keys) override;
    void put(const std::string_view& key, const void* data, size_t size) override;
    void remove(const std::string_view& key) override;
    bool transaction(const std::function<bool(DatabaseBackendTransaction&)>& callback, bool retry) override;
    std::unique_ptr<DatabaseBackendIterator> seek(const std::string_view& prefix,
                                                  const std::optional<std::string_view>& lowerBound) override;

private:
    std::shared_ptr<rocksdb::DB> db;
    std::shared_ptr<rocksdb::OptimisticTransactionDB> txn;
};

class ENGINE_API RocksDB : public Database {
public:
    explicit RocksDB(const Path& path) : Database{backend}, backend{path} {
    }
    ~RocksDB() override = default;

private:
    RocksDBBackend backend;
};

/*class ENGINE_API RocksDB : public Database {
public:
    explicit NO_ADDRESS_SAFETY_ANALYSIS RocksDB(const Path& path);
    virtual ~RocksDB();

    bool transaction(const std::function<bool(Transaction&)>& callback, bool retry = true) override;

private:
    void internalGet(const std::string& key, const Callback& callback) override;
    void internalMultiGet(const std::vector<std::string>& keys, const Callback& callback) override;
    void internalPut(const std::string& key, const Span<char>& data) override;
    void internalRemove(const std::string& key) override;
    std::unique_ptr<InternalIterator> internalSeek(const std::string& key,
                                                   const std::optional<std::string>& lowerBound) override;

    std::shared_ptr<rocksdb::DB> db;
    std::shared_ptr<rocksdb::OptimisticTransactionDB> txn;
};*/
} // namespace Engine
