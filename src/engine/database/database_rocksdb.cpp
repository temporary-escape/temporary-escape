#include "database_rocksdb.hpp"
#include <rocksdb/db.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/slice_transform.h>
#include <rocksdb/table.h>
#include <rocksdb/utilities/optimistic_transaction_db.h>
#include <rocksdb/utilities/transaction.h>

#include <memory>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class DefaultLogger : public rocksdb::Logger {
public:
    explicit DefaultLogger(rocksdb::InfoLogLevel filterLogLevel) : filterLogLevel{filterLogLevel} {
    }

    void Logv(const rocksdb::InfoLogLevel logLevel, const char* format, va_list ap) override {
        using LogFunc = void (Engine::Logger::*)(const std::string&);

        if (logLevel < filterLogLevel) {
            return;
        }

        auto func = static_cast<LogFunc>(&Engine::Logger::debug);
        switch (logLevel) {
        case rocksdb::InfoLogLevel::DEBUG_LEVEL: {
            func = static_cast<LogFunc>(&Engine::Logger::debug);
            break;
        }
        case rocksdb::InfoLogLevel::HEADER_LEVEL: {
            func = static_cast<LogFunc>(&Engine::Logger::info);
            break;
        }
        case rocksdb::InfoLogLevel::INFO_LEVEL: {
            func = static_cast<LogFunc>(&Engine::Logger::info);
            break;
        }
        case rocksdb::InfoLogLevel::FATAL_LEVEL: {
            func = static_cast<LogFunc>(&Engine::Logger::error);
            break;
        }
        case rocksdb::InfoLogLevel::WARN_LEVEL: {
            func = static_cast<LogFunc>(&Engine::Logger::warn);
            break;
        }
        default: {
            break;
        }
        }
        char buf[2048];
        auto len = std::vsnprintf(buf, sizeof(buf), format, ap);
        while (buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
            --len;
        }
        (logger.*func)(buf);
    }

private:
    rocksdb::InfoLogLevel filterLogLevel;

    /*void Logv(const char* format, va_list ap) override {
        char buf[2048];
        std::vsnprintf(buf, sizeof(buf), format, ap);
        logger.debug(buf);
    }*/
};

DatabaseRocksDB::DatabaseRocksDB(const Path& path, const Options& options) {
    rocksdb::Options opts;
    opts.create_if_missing = true;
    const auto logLevel = options.debugLogging ? rocksdb::InfoLogLevel::DEBUG_LEVEL : rocksdb::InfoLogLevel::WARN_LEVEL;
    opts.info_log = std::make_shared<DefaultLogger>(logLevel);

    opts.compression = rocksdb::kNoCompression;
    opts.max_open_files = -1;
    opts.write_buffer_size = options.writeBufferSizeMb * 1024 * 1024; // 64MB
    opts.max_background_compactions = 2;

    // options.table_factory.reset(rocksdb::NewTotalOrderPlainTableFactory());
    /*options.prefix_extractor.reset(rocksdb::NewFixedPrefixTransform(16));

    rocksdb::BlockBasedTableOptions tableOptions{};
    tableOptions.block_cache = rocksdb::NewLRUCache(128 * 1024 * 1024);
    tableOptions.filter_policy.reset(rocksdb::NewBloomFilterPolicy(16, false));
    tableOptions.index_type = rocksdb::BlockBasedTableOptions::kHashSearch;
    tableOptions.block_size = 4 * 1024;
    options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(tableOptions));*/

    opts.OptimizeForPointLookup(options.cacheSizeMb);
    opts.prefix_extractor.reset(rocksdb::NewFixedPrefixTransform(16));

    // logger.info("Opening database: {}", path.string());

    rocksdb::OptimisticTransactionDB* txnPtr;
    const auto s = rocksdb::OptimisticTransactionDB::Open(opts, path.string(), &txnPtr);
    if (!s.ok()) {
        EXCEPTION("Failed to open database error: {}", s.ToString());
    }

    txn = std::unique_ptr<rocksdb::OptimisticTransactionDB, decltype(&deleterDb)>{txnPtr, &deleterDb};
    db = txn->GetBaseDB();
}

void DatabaseRocksDB::deleterDb(rocksdb::OptimisticTransactionDB* value) {
    value->Close();
    delete value;
}

DatabaseRocksDB::~DatabaseRocksDB() = default;

std::optional<msgpack::object_handle> DatabaseRocksDB::getRaw(const std::string_view& key) {
    rocksdb::ReadOptions options{};
    rocksdb::PinnableSlice slice;
    const auto s = db->Get(options, db->DefaultColumnFamily(), key, &slice);
    if (!s.ok()) {
        if (s.code() == rocksdb::Status::Code::kNotFound) {
            return {};
        }
        throw std::runtime_error(fmt::format("database Get() error: {}", s.ToString()));
    }

    msgpack::object_handle oh;
    msgpack::unpack(oh, slice.data(), slice.size());
    return oh;
}

std::vector<msgpack::object_handle> DatabaseRocksDB::multiGetRaw(const std::vector<std::string>& keys) {
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

    std::vector<msgpack::object_handle> handles;
    for (size_t i = 0; i < keys.size(); i++) {
        if (statuses[i].code() == rocksdb::Status::Code::kNotFound) {
            continue;
        }

        if (!statuses[i].ok()) {
            throw std::runtime_error(fmt::format("database MultiGet() error: {}", statuses[i].ToString()));
        }

        handles.emplace_back();
        msgpack::unpack(handles.back(), slices[i].data(), slices[i].size());
    }

    return handles;
}

void DatabaseRocksDB::putRaw(const std::string_view& key, const void* data, size_t size) {
    rocksdb::WriteOptions options{};
    rocksdb::Slice slice(reinterpret_cast<const char*>(data), size);
    const auto s = db->Put(options, db->DefaultColumnFamily(), key, slice);
    if (!s.ok()) {
        throw std::runtime_error(fmt::format("database Put() error: {}", s.ToString()));
    }
}

std::unique_ptr<Database::Transaction> DatabaseRocksDB::startTransaction() {
    rocksdb::WriteOptions writeOptions{};
    std::unique_ptr<rocksdb::Transaction> tx(txn->BeginTransaction(writeOptions));
    return std::make_unique<TransactionRocksDB>(std::move(tx), *db);
}

void DatabaseRocksDB::removeRaw(const std::string_view& key) {
    rocksdb::WriteOptions options{};
    const auto s = db->Delete(options, db->DefaultColumnFamily(), key);
    if (!s.ok()) {
        throw std::runtime_error(fmt::format("database Delete() error: {}", s.ToString()));
    }
}

std::unique_ptr<Database::ObjectIterator> DatabaseRocksDB::seekRaw(const std::string_view& prefix,
                                                                   const std::optional<std::string_view>& lowerBound) {
    rocksdb::ReadOptions options{};
    options.prefix_same_as_start = true;

    std::unique_ptr<rocksdb::Slice> lowerBoundSlice;
    if (lowerBound.has_value()) {
        lowerBoundSlice = std::make_unique<rocksdb::Slice>(lowerBound.value());
        options.iterate_lower_bound = lowerBoundSlice.get();
    }

    std::unique_ptr<rocksdb::Iterator> iter(db->NewIterator(options));

    return std::make_unique<ObjectIteratorRocksDB>(std::move(iter), std::string{prefix}, std::move(lowerBoundSlice));
}

DatabaseRocksDB::ObjectIteratorRocksDB::ObjectIteratorRocksDB(std::unique_ptr<rocksdb::Iterator> iter,
                                                              std::string prefix,
                                                              std::unique_ptr<rocksdb::Slice> lowerBound) :
    iter{std::move(iter)}, prefix{std::move(prefix)}, lowerBound{std::move(lowerBound)} {
    this->iter->Seek(this->prefix);
}

DatabaseRocksDB::ObjectIteratorRocksDB::~ObjectIteratorRocksDB() = default;

bool DatabaseRocksDB::ObjectIteratorRocksDB::next() {
    hasValue = false;

    while (iter && iter->Valid()) {
        if (isFirst) {
            isFirst = false;
        } else {
            iter->Next();
            if (!iter->Valid()) {
                break;
            }
        }

        if (iter->key().starts_with(prefix)) {
            iterKey = iter->key().ToString();
            oh = {};
            msgpack::unpack(oh, iter->value().data(), iter->value().size());
            hasValue = true;
            return true;
        }
    }

    return false;
}

void DatabaseRocksDB::ObjectIteratorRocksDB::close() {
    iter.reset();
}

DatabaseRocksDB::ObjectIteratorRocksDB::operator bool() const {
    return iter && hasValue;
}

const std::string& DatabaseRocksDB::ObjectIteratorRocksDB::key() const {
    if (!hasValue || !iter) {
        EXCEPTION("Database iterator has no key");
    }
    return iterKey;
}

const msgpack::object_handle& DatabaseRocksDB::ObjectIteratorRocksDB::value() const {
    if (!hasValue || !iter) {
        EXCEPTION("Database iterator has no value");
    }
    return oh;
}

std::string DatabaseRocksDB::ObjectIteratorRocksDB::valueString() const {
    if (!hasValue || !iter) {
        EXCEPTION("Database iterator has no value");
    }
    return iter->value().ToString();
}

DatabaseRocksDB::TransactionRocksDB::TransactionRocksDB(std::unique_ptr<rocksdb::Transaction> txn, rocksdb::DB& db) :
    txn{std::move(txn)}, db{db} {
}

DatabaseRocksDB::TransactionRocksDB::~TransactionRocksDB() = default;

bool DatabaseRocksDB::TransactionRocksDB::commit() {
    const auto s = txn->Commit();
    if (s.code() == rocksdb::Status::Code::kBusy) {
        return false;
    }

    if (!s.ok()) {
        throw std::runtime_error(fmt::format("database transaction commit error: {}", s.ToString()));
    }

    return true;
}

void DatabaseRocksDB::TransactionRocksDB::abort() {
    const auto s = txn->Rollback();
    if (!s.ok()) {
        throw std::runtime_error(fmt::format("database transaction abort error: {}", s.ToString()));
    }
}

std::optional<msgpack::object_handle> DatabaseRocksDB::TransactionRocksDB::getRaw(const std::string_view& key) {
    rocksdb::ReadOptions options{};
    rocksdb::PinnableSlice slice;
    const auto s = txn->GetForUpdate(options, db.DefaultColumnFamily(), key, &slice);
    if (!s.ok()) {
        if (s.code() == rocksdb::Status::Code::kNotFound) {
            return {};
        }
        throw std::runtime_error(fmt::format("Failed to get database key: {} error: {}", key, s.ToString()));
    }

    msgpack::object_handle oh;
    msgpack::unpack(oh, slice.data(), slice.size());
    return oh;
}

std::vector<msgpack::object_handle>
DatabaseRocksDB::TransactionRocksDB::multiGetRaw(const std::vector<std::string>& keys) {
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

    std::vector<msgpack::object_handle> handles;
    for (size_t i = 0; i < keys.size(); i++) {
        if (statuses[i].code() == rocksdb::Status::Code::kNotFound) {
            continue;
        }

        if (!statuses[i].ok()) {
            throw std::runtime_error(
                fmt::format("Failed to get database key: {} error: {}", keys[i], statuses[i].ToString()));
        }

        handles.emplace_back();
        msgpack::unpack(handles.back(), slices[i].data(), slices[i].size());
    }

    return handles;
}

void DatabaseRocksDB::TransactionRocksDB::putRaw(const std::string_view& key, const void* data, size_t size) {
    rocksdb::Slice slice(reinterpret_cast<const char*>(data), size);
    const auto s = txn->Put(db.DefaultColumnFamily(), key, slice);
    if (!s.ok()) {
        throw std::runtime_error(fmt::format("Failed to put database key: {} error: {}", key, s.ToString()));
    }
}

std::unique_ptr<Database::Transaction> DatabaseRocksDB::TransactionRocksDB::startTransaction() {
    throw std::runtime_error("Can not create nested transaction");
}

void DatabaseRocksDB::TransactionRocksDB::removeRaw(const std::string_view& key) {
    const auto s = txn->Delete(db.DefaultColumnFamily(), key);
    if (!s.ok()) {
        throw std::runtime_error(fmt::format("Failed to delete database key: {} error: {}", key, s.ToString()));
    }
}

std::unique_ptr<Database::ObjectIterator>
DatabaseRocksDB::TransactionRocksDB::seekRaw(const std::string_view& prefix,
                                             const std::optional<std::string_view>& lowerBound) {
    rocksdb::ReadOptions options{};
    options.prefix_same_as_start = true;

    std::unique_ptr<rocksdb::Slice> lowerBoundSlice;
    if (lowerBound.has_value()) {
        lowerBoundSlice = std::make_unique<rocksdb::Slice>(lowerBound.value());
        options.iterate_lower_bound = lowerBoundSlice.get();
    }

    std::unique_ptr<rocksdb::Iterator> iter(txn->GetIterator(options));

    return std::make_unique<ObjectIteratorRocksDB>(std::move(iter), std::string{prefix}, std::move(lowerBoundSlice));
}
