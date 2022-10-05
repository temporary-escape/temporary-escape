#include "RocksDB.hpp"
#include <functional>
#include <rocksdb/db.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/table.h>
#include <rocksdb/utilities/optimistic_transaction_db.h>
#include <rocksdb/utilities/transaction.h>
#include <string_view>

#define CMP "RocksDB"

using namespace Engine;

class DefaultLogger : public rocksdb::Logger {
public:
    DefaultLogger() = default;

    void Logv(const rocksdb::InfoLogLevel logLevel, const char* format, va_list ap) override {
        using LogFunc = void (*)(const std::string&, const std::string&);
        auto func = static_cast<LogFunc>(&Log::d);
        switch (logLevel) {
        case rocksdb::InfoLogLevel::DEBUG_LEVEL: {
            func = static_cast<LogFunc>(&Log::e);
            break;
        }
        case rocksdb::InfoLogLevel::HEADER_LEVEL: {
            func = static_cast<LogFunc>(&Log::i);
            break;
        }
        case rocksdb::InfoLogLevel::INFO_LEVEL: {
            func = static_cast<LogFunc>(&Log::i);
            break;
        }
        case rocksdb::InfoLogLevel::FATAL_LEVEL: {
            func = static_cast<LogFunc>(&Log::e);
            break;
        }
        case rocksdb::InfoLogLevel::WARN_LEVEL: {
            func = static_cast<LogFunc>(&Log::w);
            break;
        }
        default: {
            break;
        }
        }
        char buf[2048];
        std::vsnprintf(buf, sizeof(buf), format, ap);
        func(CMP, buf);
    }

    void Logv(const char* format, va_list ap) override {
        char buf[2048];
        std::vsnprintf(buf, sizeof(buf), format, ap);
        Log::d(CMP, buf);
    }
};

class RocksDBIterator : public Database::InternalIterator {
public:
    explicit RocksDBIterator(rocksdb::DB& db, std::string prefix, const std::optional<std::string>& lowerBound) :
        prefix(std::move(prefix)) {

        rocksdb::ReadOptions options{};
        options.prefix_same_as_start = true;
        if (lowerBound.has_value()) {
            this->lowerBound = rocksdb::Slice(lowerBound.value());
            options.iterate_lower_bound = &this->lowerBound;
        }
        iter.reset(db.NewIterator(options));
        iter->Seek(this->prefix);
    }

    bool next(const Database::Callback& callback) override {
        while (iter->Valid()) {
            if (iter->key().starts_with(prefix)) {
                callback(iter->key().ToString(), {iter->value().data(), iter->value().size()});
                iter->Next();
                return true;
            }
            iter->Next();
        }

        return false;
    }

private:
    std::unique_ptr<rocksdb::Iterator> iter;
    std::string prefix;
    rocksdb::Slice lowerBound;
    bool first{true};
};

class RocksDBTransaction : public Transaction {
public:
    explicit RocksDBTransaction(std::unique_ptr<rocksdb::Transaction> txn, rocksdb::DB& db) :
        txn(std::move(txn)), db(db) {
    }

    virtual ~RocksDBTransaction() = default;

    rocksdb::Status commit() {
        return txn->Commit();
    }

private:
    void internalGetForUpdate(const std::string& key, const Callback& callback) override {
        rocksdb::ReadOptions options{};
        rocksdb::PinnableSlice slice;
        const auto s = txn->GetForUpdate(options, db.DefaultColumnFamily(), key, &slice);
        if (!s.ok()) {
            if (s.code() == rocksdb::Status::Code::kNotFound) {
                return;
            }
            EXCEPTION("Failed to get database key: {} error: {}", key, s.ToString());
        }

        callback(key, {slice.data(), slice.size()});
    }

    void internalGet(const std::string& key, const Callback& callback) override {
        rocksdb::ReadOptions options{};
        rocksdb::PinnableSlice slice;
        const auto s = txn->Get(options, db.DefaultColumnFamily(), key, &slice);
        if (!s.ok()) {
            if (s.code() == rocksdb::Status::Code::kNotFound) {
                return;
            }
            EXCEPTION("Failed to get database key: {} error: {}", key, s.ToString());
        }

        callback(key, {slice.data(), slice.size()});
    }

    void internalMultiGet(const std::vector<std::string>& keys, const Callback& callback) override {
        rocksdb::ReadOptions options{};
        std::vector<rocksdb::Slice> keysSlice;
        keysSlice.reserve(keys.size());
        for (const auto& key : keys) {
            keysSlice.emplace_back(key.data(), key.size());
        }
        std::vector<rocksdb::PinnableSlice> slices;
        std::vector<rocksdb::Status> statuses;
        slices.resize(keys.size());
        statuses.resize(keys.size());
        txn->MultiGet(options, db.DefaultColumnFamily(), keys.size(), keysSlice.data(), slices.data(), statuses.data());

        for (size_t i = 0; i < keys.size(); i++) {
            if (statuses[i].ok()) {
                callback(keys[i], {slices[i].data(), slices[i].size()});
            }
        }
    }

    void internalPut(const std::string& key, const Span<char>& data) override {
        rocksdb::Slice slice(data.data(), data.size());
        const auto s = txn->Put(db.DefaultColumnFamily(), key, slice);
        if (!s.ok()) {
            EXCEPTION("Failed to put database key: {} error: {}", key, s.ToString());
        }
    }

    void internalRemove(const std::string& key) override {
        const auto s = txn->Delete(db.DefaultColumnFamily(), key);
        if (!s.ok()) {
            EXCEPTION("Failed to delete database key: {} error: {}", key, s.ToString());
        }
    }

    std::unique_ptr<InternalIterator> internalSeek(const std::string& key,
                                                   const std::optional<std::string>& lowerBound) override {
        return std::unique_ptr<InternalIterator>();
    }

    std::unique_ptr<rocksdb::Transaction> txn;
    rocksdb::DB& db;
};

RocksDB::RocksDB(const Path& path) {
    rocksdb::Options options;
    options.create_if_missing = true;
    options.info_log_level = rocksdb::InfoLogLevel::WARN_LEVEL;
    options.info_log = std::make_shared<DefaultLogger>();

    rocksdb::BlockBasedTableOptions tableOptions;
    tableOptions.block_cache = rocksdb::NewLRUCache(100 * 1048576);

    tableOptions.filter_policy.reset(
        rocksdb::NewBloomFilterPolicy(10 /* bits_per_key */, false /* use_block_based_builder*/));

    options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(tableOptions));

    Log::i(CMP, "Opening database: {}", path.string());

    rocksdb::OptimisticTransactionDB* txnPtr;
    const auto s = rocksdb::OptimisticTransactionDB::Open(options, path.string(), &txnPtr);
    if (!s.ok()) {
        EXCEPTION("Failed to open database error: {}", s.ToString());
    }

    txn = std::shared_ptr<rocksdb::OptimisticTransactionDB>(txnPtr, [](rocksdb::OptimisticTransactionDB* ptr) {
        ptr->Close();
        delete ptr;
    });

    db = std::shared_ptr<rocksdb::DB>(txn->GetBaseDB(), [](rocksdb::DB* ptr) {});
}

RocksDB::~RocksDB() = default;

void RocksDB::internalGet(const std::string& key, const Callback& callback) {
    rocksdb::ReadOptions options{};
    rocksdb::PinnableSlice slice;
    const auto s = db->Get(options, db->DefaultColumnFamily(), key, &slice);
    if (!s.ok()) {
        if (s.code() == rocksdb::Status::Code::kNotFound) {
            return;
        }
        EXCEPTION("Failed to get database key: {} error: {}", key, s.ToString());
    }

    callback(key, {slice.data(), slice.size()});
}

void RocksDB::internalPut(const std::string& key, const Span<char>& data) {
    rocksdb::WriteOptions options{};
    rocksdb::Slice slice(data.data(), data.size());
    const auto s = db->Put(options, db->DefaultColumnFamily(), key, slice);
    if (!s.ok()) {
        EXCEPTION("Failed to put database key: {} error: {}", key, s.ToString());
    }
}

void RocksDB::internalRemove(const std::string& key) {
    rocksdb::WriteOptions options{};
    const auto s = db->Delete(options, db->DefaultColumnFamily(), key);
    if (!s.ok()) {
        EXCEPTION("Failed to delete database key: {} error: {}", key, s.ToString());
    }
}

void RocksDB::internalMultiGet(const std::vector<std::string>& keys, const Database::Callback& callback) {
    rocksdb::ReadOptions options{};
    std::vector<rocksdb::Slice> keysSlice;
    keysSlice.reserve(keys.size());
    for (const auto& key : keys) {
        keysSlice.emplace_back(key.data(), key.size());
    }
    std::vector<rocksdb::PinnableSlice> slices;
    std::vector<rocksdb::Status> statuses;
    slices.resize(keys.size());
    statuses.resize(keys.size());
    db->MultiGet(options, db->DefaultColumnFamily(), keys.size(), keysSlice.data(), slices.data(), statuses.data());

    for (size_t i = 0; i < keys.size(); i++) {
        if (statuses[i].ok()) {
            callback(keys[i], {slices[i].data(), slices[i].size()});
        }
    }
}

std::unique_ptr<Database::InternalIterator> RocksDB::internalSeek(const std::string& key,
                                                                  const std::optional<std::string>& lowerBound) {
    return std::unique_ptr<Database::InternalIterator>(new RocksDBIterator(*db, key, lowerBound));
}

bool RocksDB::transaction(const std::function<bool(Transaction&)>& callback, bool retry) {
    while (true) {
        rocksdb::WriteOptions writeOptions{};
        std::unique_ptr<rocksdb::Transaction> tx(txn->BeginTransaction(writeOptions));

        RocksDBTransaction wrapper(std::move(tx), *db);

        if (callback(wrapper)) {
            const auto s = wrapper.commit();
            if (s.code() == rocksdb::Status::Code::kBusy) {
                if (retry) {
                    continue;
                }
                return false;
            }

            if (!s.ok()) {
                EXCEPTION("Failed to commit database transaction error: {}", s.ToString());
            }

            return true;
        }

        return false;
    }
}
