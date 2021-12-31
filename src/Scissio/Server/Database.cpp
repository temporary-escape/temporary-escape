#include "Database.hpp"
#include "../Utils/Log.hpp"

#include <rocksdb/db.h>
#include <rocksdb/utilities/optimistic_transaction_db.h>
#include <rocksdb/utilities/transaction.h>

#define CMP "Database"

using namespace Scissio;

struct Database::Data {
    rocksdb::DB* db{nullptr};
    rocksdb::OptimisticTransactionDB* txn{nullptr};
};

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

class TransactionWrapper : public Transaction {
public:
    explicit TransactionWrapper(std::shared_ptr<rocksdb::Transaction> txn, rocksdb::DB* db)
        : txn(std::move(txn)), db(db) {
    }

    ~TransactionWrapper() override = default;

private:
    bool getForUpdateInternal(const std::string& key, const std::function<void(const char*, size_t)>& fn) override {
        rocksdb::ReadOptions options{};
        rocksdb::PinnableSlice slice;
        const auto s = txn->GetForUpdate(options, db->DefaultColumnFamily(), key, &slice);
        if (!s.ok()) {
            if (s.code() == rocksdb::Status::Code::kNotFound) {
                return false;
            }
            EXCEPTION("Failed to get database key: {} error: {}", key, s.ToString());
        }

        fn(slice.data(), slice.size());

        return true;
    }

    bool getInternal(const std::string& key, const std::function<void(const char*, size_t)>& fn) override {
        rocksdb::ReadOptions options{};
        rocksdb::PinnableSlice slice;
        const auto s = txn->Get(options, db->DefaultColumnFamily(), key, &slice);
        if (!s.ok()) {
            if (s.code() == rocksdb::Status::Code::kNotFound) {
                return false;
            }
            EXCEPTION("Failed to get database key: {} error: {}", key, s.ToString());
        }

        fn(slice.data(), slice.size());

        return true;
    }

    void multiGetInternal(const std::vector<std::string>& keys,
                          const std::function<void(const std::string&, const char*, size_t)>& fn) override {
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
        txn->MultiGet(options, db->DefaultColumnFamily(), keys.size(), keysSlice.data(), slices.data(),
                      statuses.data());

        for (size_t i = 0; i < keys.size(); i++) {
            if (statuses[i].ok()) {
                fn(keys[i], slices[i].data(), slices[i].size());
            }
        }
    }

    void seekInternal(const std::string& prefix,
                      const std::function<bool(const std::string&, const char*, size_t)>& fn) override {
        rocksdb::ReadOptions options{};
        options.prefix_same_as_start = true;
        std::shared_ptr<rocksdb::Iterator> iter(txn->GetIterator(options));
        iter->Seek(prefix);

        while (iter->Valid()) {
            if (iter->key().starts_with(prefix)) {
                if (!fn(iter->key().ToString(), iter->value().data(), iter->value().size())) {
                    break;
                }
            }
            iter->Next();
        }
    }

    void nextInternal(const std::string& prefix, const std::string& start,
                      const std::function<bool(const std::string&, const char*, size_t)>& fn) override {
        rocksdb::ReadOptions options{};
        rocksdb::Slice lowerBound(start);
        options.prefix_same_as_start = true;
        options.iterate_lower_bound = &lowerBound;

        std::shared_ptr<rocksdb::Iterator> iter(txn->GetIterator(options));
        iter->Seek(prefix);

        while (iter->Valid()) {
            if (iter->key().starts_with(prefix)) {
                if (!fn(iter->key().ToString(), iter->value().data(), iter->value().size())) {
                    break;
                }
            }
            iter->Next();
        }
    }

    void putInternal(const std::string& key, const char* rawData, size_t size) override {
        rocksdb::WriteOptions options{};
        rocksdb::Slice slice(rawData, size);
        const auto s = txn->Put(db->DefaultColumnFamily(), key, slice);
        if (!s.ok()) {
            EXCEPTION("Failed to put database key: {} error: {}", key, s.ToString());
        }
    }

    void removeInternal(const std::string& key) override {
        rocksdb::WriteOptions options{};
        const auto s = txn->Delete(db->DefaultColumnFamily(), key);
        if (!s.ok()) {
            EXCEPTION("Failed to delete database key: {} error: {}", key, s.ToString());
        }
    }

    void removeByPrefixInternal(const std::string& prefix) override {
        rocksdb::ReadOptions options{};
        options.prefix_same_as_start = true;
        std::shared_ptr<rocksdb::Iterator> iter(txn->GetIterator(options));
        iter->Seek(prefix);

        while (iter->Valid()) {
            if (iter->key().starts_with(prefix)) {
                const auto s = txn->Delete(iter->key());
                if (!s.ok()) {
                    EXCEPTION("Failed to delete database key: {} error: {}", iter->key().ToString(), s.ToString());
                }
            }
            iter->Next();
        }
    }

    std::shared_ptr<rocksdb::Transaction> txn;
    rocksdb::DB* db;
};

Database::Database(const Path& path) : data(std::make_unique<Data>()) {
    rocksdb::Options options;
    options.create_if_missing = true;
    options.info_log = std::make_shared<DefaultLogger>();

    Log::i(CMP, "Opening database: {}", path.string());
    const auto s = rocksdb::OptimisticTransactionDB::Open(options, path.string(), &data->txn);
    if (!s.ok()) {
        EXCEPTION("Failed to open database error: {}", s.ToString());
    }

    data->db = data->txn->GetBaseDB();
}

Database::~Database() {
    if (data && data->db) {
        Log::i(CMP, "Closing database");
        const auto s = data->db->Close();
        if (!s.ok()) {
            Log::e(CMP, "Failed to close database error: {}", s.ToString());
        }
        data.reset();
    }
}

bool Database::getInternal(const std::string& key, const std::function<void(const char*, size_t)>& fn) {
    rocksdb::ReadOptions options{};
    rocksdb::PinnableSlice slice;
    const auto s = data->db->Get(options, data->db->DefaultColumnFamily(), key, &slice);
    if (!s.ok()) {
        if (s.code() == rocksdb::Status::Code::kNotFound) {
            return false;
        }
        EXCEPTION("Failed to get database key: {} error: {}", key, s.ToString());
    }

    fn(slice.data(), slice.size());

    return true;
}

void Database::multiGetInternal(const std::vector<std::string>& keys,
                                const std::function<void(const std::string&, const char*, size_t)>& fn) {
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
    data->db->MultiGet(options, data->db->DefaultColumnFamily(), keys.size(), keysSlice.data(), slices.data(),
                       statuses.data());

    for (size_t i = 0; i < keys.size(); i++) {
        if (statuses[i].ok()) {
            fn(keys[i], slices[i].data(), slices[i].size());
        }
    }
}

void Database::seekInternal(const std::string& prefix,
                            const std::function<bool(const std::string&, const char*, size_t)>& fn) {
    rocksdb::ReadOptions options{};
    options.prefix_same_as_start = true;
    std::shared_ptr<rocksdb::Iterator> iter(data->db->NewIterator(options));
    iter->Seek(prefix);

    while (iter->Valid()) {
        if (iter->key().starts_with(prefix)) {
            if (!fn(iter->key().ToString(), iter->value().data(), iter->value().size())) {
                break;
            }
        }
        iter->Next();
    }
}

void Database::nextInternal(const std::string& prefix, const std::string& start,
                            const std::function<bool(const std::string&, const char*, size_t)>& fn) {
    rocksdb::ReadOptions options{};
    rocksdb::Slice lowerBound(start);
    options.prefix_same_as_start = true;
    options.iterate_lower_bound = &lowerBound;

    std::shared_ptr<rocksdb::Iterator> iter(data->db->NewIterator(options));
    iter->Seek(prefix);

    while (iter->Valid()) {
        if (iter->key().starts_with(prefix)) {
            if (!fn(iter->key().ToString(), iter->value().data(), iter->value().size())) {
                break;
            }
        }
        iter->Next();
    }
}

bool Database::transaction(const std::function<bool(Transaction&)>& fn, bool retry) {
    while (true) {
        rocksdb::WriteOptions writeOptions{};
        std::shared_ptr<rocksdb::Transaction> txn(data->txn->BeginTransaction(writeOptions));

        TransactionWrapper wrapper(txn, data->db);

        if (fn(wrapper)) {
            const auto s = txn->Commit();
            if (s.code() == rocksdb::Status::Code::kBusy) {
                if (retry) {
                    continue;
                }
                return false;
            }

            if (!s.ok()) {
                EXCEPTION("Failed to commit transaction error: {}", s.ToString());
            }

            return true;
        }

        return false;
    }
}

void Database::putInternal(const std::string& key, const char* rawData, size_t size) {
    rocksdb::WriteOptions options{};
    rocksdb::Slice slice(rawData, size);
    const auto s = data->db->Put(options, data->db->DefaultColumnFamily(), key, slice);
    if (!s.ok()) {
        EXCEPTION("Failed to put database key: {} error: {}", key, s.ToString());
    }
}

void Database::removeInternal(const std::string& key) {
    rocksdb::WriteOptions options{};
    const auto s = data->db->Delete(options, data->db->DefaultColumnFamily(), key);
    if (!s.ok()) {
        EXCEPTION("Failed to delete database key: {} error: {}", key, s.ToString());
    }
}

void Database::removeByPrefixInternal(const std::string& prefix) {
    rocksdb::ReadOptions options{};
    rocksdb::WriteOptions writeOptions{};
    options.prefix_same_as_start = true;
    std::shared_ptr<rocksdb::Iterator> iter(data->db->NewIterator(options));
    iter->Seek(prefix);

    while (iter->Valid()) {
        if (iter->key().starts_with(prefix)) {
            const auto s = data->db->Delete(writeOptions, iter->key());
            if (!s.ok()) {
                EXCEPTION("Failed to delete database key: {} error: {}", iter->key().ToString(), s.ToString());
            }
        }
        iter->Next();
    }
}
