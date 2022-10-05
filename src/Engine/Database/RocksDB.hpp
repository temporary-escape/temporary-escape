#pragma once

#include "Database.hpp"

#ifdef __linux__
#define NO_ADDRESS_SAFETY_ANALYSIS __attribute__((no_sanitize_address))
#else
#define NO_ADDRESS_SAFETY_ANALYSIS
#endif

namespace rocksdb {
class DB;
class OptimisticTransactionDB;
} // namespace rocksdb

namespace Engine {
class ENGINE_API RocksDB : public TransactionalDatabase {
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
};
} // namespace Engine
