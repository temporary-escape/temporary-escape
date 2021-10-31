#include "Database.hpp"
#include "../Utils/Log.hpp"

#include <rocksdb/db.h>
#include <rocksdb/utilities/optimistic_transaction_db.h>
#include <rocksdb/utilities/transaction.h>

#define CMP "Database"

using namespace Scissio;

struct Database::Data {
    rocksdb::DB* db{nullptr};
    rocksdb::OptimisticTransactionDB* txn;
};

Database::Database(const Path& path) : data(std::make_unique<Data>()) {
    rocksdb::Options options;
    options.create_if_missing = true;

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

bool Database::get(const std::string& key, const std::function<void(const char*, size_t)>& fn) {
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

bool Database::update(
    const std::string& key,
    const std::function<void(const char*, size_t, const std::function<void(const char*, size_t)>&)>& fn) {
    rocksdb::WriteOptions writeOptions{};

    while (true) {
        std::shared_ptr<rocksdb::Transaction> txn(data->txn->BeginTransaction(writeOptions));

        rocksdb::ReadOptions readOptions{};
        rocksdb::PinnableSlice readSlice;
        auto s = txn->GetForUpdate(readOptions, data->db->DefaultColumnFamily(), key, &readSlice);

        if (!s.ok()) {
            if (s.code() == rocksdb::Status::Code::kNotFound) {
                return false;
            }
            EXCEPTION("Failed to get database key: {} error: {}", key, s.ToString());
        }

        auto inserted = false;

        const auto callback = [&](const char* rawData, const size_t size) {
            rocksdb::Slice putSlice(rawData, size);
            auto ss = data->db->Put(writeOptions, data->db->DefaultColumnFamily(), key, putSlice);
            if (!ss.ok()) {
                EXCEPTION("Failed to put database key: {} error: {}", key, s.ToString());
            }

            inserted = true;
        };

        fn(readSlice.data(), readSlice.size(), callback);

        if (inserted) {
            s = txn->Commit();
            if (!s.ok()) {
                EXCEPTION("Failed to commit database key: {} error: {}", key, s.ToString());
            }
        }
        return true;
    }
}

void Database::put(const std::string& key, const char* rawData, size_t size) {
    rocksdb::WriteOptions options{};
    rocksdb::Slice slice(rawData, size);
    const auto s = data->db->Put(options, data->db->DefaultColumnFamily(), key, slice);
    if (!s.ok()) {
        EXCEPTION("Failed to put database key: {} error: {}", key, s.ToString());
    }
}

void Database::remove(const std::string& key) {
    rocksdb::WriteOptions options{};
    const auto s = data->db->Delete(options, data->db->DefaultColumnFamily(), key);
    if (!s.ok()) {
        EXCEPTION("Failed to delete database key: {} error: {}", key, s.ToString());
    }
}
