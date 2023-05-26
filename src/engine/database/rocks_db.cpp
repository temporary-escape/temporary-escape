#include "rocks_db.hpp"
#include <functional>
#include <rocksdb/db.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/slice_transform.h>
#include <rocksdb/table.h>
#include <rocksdb/utilities/optimistic_transaction_db.h>
#include <rocksdb/utilities/transaction.h>
#include <string_view>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class DefaultLogger : public rocksdb::Logger {
public:
    DefaultLogger() = default;

    void Logv(const rocksdb::InfoLogLevel logLevel, const char* format, va_list ap) override {
        using LogFunc = void (Engine::Logger::*)(const std::string&);
        auto func = static_cast<LogFunc>(&Engine::Logger::debug);
        switch (logLevel) {
        case rocksdb::InfoLogLevel::DEBUG_LEVEL: {
            func = static_cast<LogFunc>(&Engine::Logger::error);
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
        std::vsnprintf(buf, sizeof(buf), format, ap);
        (logger.*func)(buf);
    }

    void Logv(const char* format, va_list ap) override {
        char buf[2048];
        std::vsnprintf(buf, sizeof(buf), format, ap);
        logger.debug(buf);
    }
};

RocksDBBackend::Transaction::Transaction(std::unique_ptr<rocksdb::Transaction> txn, rocksdb::DB& db) :
    txn(std::move(txn)), db(db) {
}

RocksDBBackend::Transaction::~Transaction() = default;

std::optional<msgpack::object_handle> RocksDBBackend::Transaction::get(const std::string_view& key) {
    rocksdb::ReadOptions options{};
    rocksdb::PinnableSlice slice;
    const auto s = txn->Get(options, db.DefaultColumnFamily(), key, &slice);
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

std::vector<msgpack::object_handle> RocksDBBackend::Transaction::multiGet(const std::vector<std::string>& keys) {
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

void RocksDBBackend::Transaction::put(const std::string_view& key, const void* data, size_t size) {
    rocksdb::Slice slice(reinterpret_cast<const char*>(data), size);
    const auto s = txn->Put(db.DefaultColumnFamily(), key, slice);
    if (!s.ok()) {
        throw std::runtime_error(fmt::format("Failed to put database key: {} error: {}", key, s.ToString()));
    }
}

void RocksDBBackend::Transaction::remove(const std::string_view& key) {
    const auto s = txn->Delete(db.DefaultColumnFamily(), key);
    if (!s.ok()) {
        throw std::runtime_error(fmt::format("Failed to delete database key: {} error: {}", key, s.ToString()));
    }
}

std::optional<msgpack::object_handle> RocksDBBackend::Transaction::getForUpdate(const std::string_view& key) {
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

std::unique_ptr<DatabaseBackendIterator>
RocksDBBackend::Transaction::seek(const std::string_view& prefix, const std::optional<std::string_view>& lowerBound) {
    rocksdb::ReadOptions options{};
    options.prefix_same_as_start = true;

    std::unique_ptr<rocksdb::Slice> lowerBoundSlice;
    if (lowerBound.has_value()) {
        lowerBoundSlice = std::make_unique<rocksdb::Slice>(lowerBound.value());
        options.iterate_lower_bound = lowerBoundSlice.get();
    }

    std::unique_ptr<rocksdb::Iterator> iter(txn->GetIterator(options));

    return std::unique_ptr<DatabaseBackendIterator>{
        new Iterator{std::move(iter), std::string{prefix}, std::move(lowerBoundSlice)}};
}

bool RocksDBBackend::Transaction::commit() {
    const auto s = txn->Commit();
    if (s.code() == rocksdb::Status::Code::kBusy) {
        return false;
    }

    if (!s.ok()) {
        throw std::runtime_error(fmt::format("Failed to commit database transaction error: {}", s.ToString()));
    }

    return true;
}

RocksDBBackend::Iterator::Iterator(std::unique_ptr<rocksdb::Iterator> iter, std::string prefix,
                                   std::unique_ptr<rocksdb::Slice> lowerBound) :
    iter{std::move(iter)}, prefix{std::move(prefix)}, lowerBound{std::move(lowerBound)} {
    this->iter->Seek(this->prefix);
}

RocksDBBackend::Iterator::operator bool() const {
    return hasValue;
}

bool RocksDBBackend::Iterator::next() {
    hasValue = false;

    while (iter->Valid()) {
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

const std::string& RocksDBBackend::Iterator::key() const {
    if (!hasValue) {
        throw std::runtime_error("Iterator has no value");
    }
    return iterKey;
}

const msgpack::object_handle& RocksDBBackend::Iterator::value() const {
    if (!hasValue) {
        throw std::runtime_error("Iterator has no value");
    }
    return oh;
}

std::string RocksDBBackend::Iterator::valueRaw() const {
    if (!hasValue) {
        throw std::runtime_error("Iterator has no value");
    }
    return iter->value().ToString();
}

RocksDBBackend::RocksDBBackend(const Path& path) {
    rocksdb::Options options;
    options.create_if_missing = true;
    options.info_log_level = rocksdb::InfoLogLevel::WARN_LEVEL;
    options.info_log = std::make_shared<DefaultLogger>();

    // options.table_factory.reset(rocksdb::NewTotalOrderPlainTableFactory());
    options.prefix_extractor.reset(rocksdb::NewFixedPrefixTransform(16));

    rocksdb::BlockBasedTableOptions tableOptions{};
    tableOptions.block_cache = rocksdb::NewLRUCache(128 * 1024 * 1024);
    tableOptions.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10, false));
    options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(tableOptions));

    logger.info("Opening database: {}", path.string());

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

RocksDBBackend::~RocksDBBackend() = default;

std::optional<msgpack::object_handle> RocksDBBackend::get(const std::string_view& key) {
    rocksdb::ReadOptions options{};
    rocksdb::PinnableSlice slice;
    const auto s = db->Get(options, db->DefaultColumnFamily(), key, &slice);
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

std::vector<msgpack::object_handle> RocksDBBackend::multiGet(const std::vector<std::string>& keys) {
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
            throw std::runtime_error(
                fmt::format("Failed to get database key: {} error: {}", keys[i], statuses[i].ToString()));
        }

        handles.emplace_back();
        msgpack::unpack(handles.back(), slices[i].data(), slices[i].size());
    }

    return handles;
}

void RocksDBBackend::put(const std::string_view& key, const void* data, size_t size) {
    rocksdb::WriteOptions options{};
    rocksdb::Slice slice(reinterpret_cast<const char*>(data), size);
    const auto s = db->Put(options, db->DefaultColumnFamily(), key, slice);
    if (!s.ok()) {
        throw std::runtime_error(fmt::format("Failed to put database key: {} error: {}", key, s.ToString()));
    }
}

void RocksDBBackend::remove(const std::string_view& key) {
    rocksdb::WriteOptions options{};
    const auto s = db->Delete(options, db->DefaultColumnFamily(), key);
    if (!s.ok()) {
        throw std::runtime_error(fmt::format("Failed to delete database key: {} error: {}", key, s.ToString()));
    }
}

bool RocksDBBackend::transaction(const std::function<bool(DatabaseBackendTransaction&)>& callback, bool retry) {
    while (true) {
        rocksdb::WriteOptions writeOptions{};
        std::unique_ptr<rocksdb::Transaction> tx(txn->BeginTransaction(writeOptions));

        RocksDBBackend::Transaction wrapper(std::move(tx), *db);

        if (callback(wrapper)) {
            if (!wrapper.commit()) {
                if (retry) {
                    continue;
                } else {
                    return false;
                }
            }

            return true;
        }

        return false;
    }
}

std::unique_ptr<DatabaseBackendIterator> RocksDBBackend::seek(const std::string_view& prefix,
                                                              const std::optional<std::string_view>& lowerBound) {
    rocksdb::ReadOptions options{};
    options.prefix_same_as_start = true;

    std::unique_ptr<rocksdb::Slice> lowerBoundSlice;
    if (lowerBound.has_value()) {
        lowerBoundSlice = std::make_unique<rocksdb::Slice>(lowerBound.value());
        options.iterate_lower_bound = lowerBoundSlice.get();
    }

    std::unique_ptr<rocksdb::Iterator> iter(db->NewIterator(options));

    return std::unique_ptr<DatabaseBackendIterator>{
        new Iterator{std::move(iter), std::string{prefix}, std::move(lowerBoundSlice)}};
}
