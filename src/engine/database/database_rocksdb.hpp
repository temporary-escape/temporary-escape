#pragma once

#include "../utils/path.hpp"
#include "database.hpp"

namespace rocksdb {
class DB;
class OptimisticTransactionDB;
class Transaction;
class Slice;
class Iterator;
} // namespace rocksdb

namespace Engine {
class DatabaseRocksDB : public Database {
public:
    struct Options {
        size_t writeBufferSizeMb{64};
        size_t cacheSizeMb{256};
        bool debugLogging{false};
    };

    class ObjectIteratorRocksDB : public ObjectIterator {
    public:
        explicit ObjectIteratorRocksDB(std::unique_ptr<rocksdb::Iterator> iter, std::string prefix,
                                       std::unique_ptr<rocksdb::Slice> lowerBound);
        ~ObjectIteratorRocksDB();

        bool next() override;
        void close() override;
        operator bool() const override;
        [[nodiscard]] const std::string& key() const override;
        [[nodiscard]] const msgpack::object_handle& value() const override;
        [[nodiscard]] std::string valueString() const override;

    private:
        std::unique_ptr<rocksdb::Iterator> iter;
        std::string prefix;
        std::unique_ptr<rocksdb::Slice> lowerBound;
        msgpack::object_handle oh;
        std::string iterKey;
        bool hasValue{false};
        bool isFirst{true};
    };

    class TransactionRocksDB : public Transaction {
    public:
        explicit TransactionRocksDB(std::unique_ptr<rocksdb::Transaction> txn, rocksdb::DB& db);
        ~TransactionRocksDB();

        bool commit() override;
        void abort() override;

        std::optional<msgpack::object_handle> getRaw(const std::string_view& key) override;
        std::vector<msgpack::object_handle> multiGetRaw(const std::vector<std::string>& keys) override;
        void putRaw(const std::string_view& key, const void* data, size_t size) override;
        std::unique_ptr<Transaction> startTransaction() override;
        void removeRaw(const std::string_view& key) override;
        std::unique_ptr<ObjectIterator> seekRaw(const std::string_view& prefix,
                                                const std::optional<std::string_view>& lowerBound) override;

    private:
        std::unique_ptr<rocksdb::Transaction> txn;
        rocksdb::DB& db;
    };

    explicit DatabaseRocksDB(const Path& path, const Options& options);
    virtual ~DatabaseRocksDB();

    std::optional<msgpack::object_handle> getRaw(const std::string_view& key) override;
    std::vector<msgpack::object_handle> multiGetRaw(const std::vector<std::string>& keys) override;
    void putRaw(const std::string_view& key, const void* data, size_t size) override;
    std::unique_ptr<Transaction> startTransaction() override;
    void removeRaw(const std::string_view& key) override;
    std::unique_ptr<ObjectIterator> seekRaw(const std::string_view& prefix,
                                            const std::optional<std::string_view>& lowerBound) override;

private:
    static void deleterDb(rocksdb::OptimisticTransactionDB* value);

    rocksdb::DB* db{nullptr};
    std::unique_ptr<rocksdb::OptimisticTransactionDB, decltype(&deleterDb)> txn{nullptr, &deleterDb};
};
} // namespace Engine
