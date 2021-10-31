#pragma once

#include "Database.hpp"

namespace Scissio {
template <typename T> class Page {
public:
    std::vector<T> results;
    uint64_t cont = 0;

    MSGPACK_DEFINE_ARRAY(results, cont);
};

template <typename T> class Repository {
public:
    explicit Repository(Database& db, const uint64_t pageLimit = 128) : db(db), pageLimit(pageLimit) {
    }

    virtual ~Repository() = default;

    T getOrThrow(const std::string& field, const uint64_t value) {
        auto res = db.get<T>(field, value);
        if (!res.has_value()) {
            throw std::out_of_range("{} row for {}={} not found", T::dbName(), field, value);
        }
        return {std::move(res.value())};
    }

    T getOrThrow(const uint64_t id) {
        return get("id", id);
    }

    std::optional<T> get(const std::string& field, const uint64_t value) {
        return db.get<T>(field, value);
    }

    std::optional<T> get(const uint64_t id) {
        return get("id", id);
    }

    template <typename... Args> std::optional<T> find(const std::string& where, Args&&... args) {
        auto results = db.select<T>(where, std::forward<Args>(args)...);
        if (results.empty()) {
            return std::nullopt;
        }
        return {std::move(results.front())};
    }

    template <typename... Args> std::vector<T> findMany(const std::string& where, Args&&... args) {
        return db.select<T>(where, std::forward<Args>(args)...);
    }

    template <typename... Args> Page<T> findPaged(const std::string& where, uint64_t cont, Args&&... args) {
        auto results =
            db.select<T>(fmt::format("WHERE {} {} id > ? ORDER BY id ASC LIMIT ?", where, where.empty() ? "" : "AND"),
                         std::forward<Args>(args)..., cont, pageLimit);

        return {results, results.empty() ? cont : results.back().id};
    }

    template <typename L, typename... Args>
    Page<Database::InnerJoin<L, T>> joinPaged(const std::string& leftField, const std::string& where, uint64_t cont,
                                              Args&&... args) {
        const auto join = db.join<L, T>(leftField, "id",
                                        fmt::format("WHERE {} {} {}.id > ? ORDER BY {}.id ASC LIMIT ?", where,
                                                    where.empty() ? "" : "AND", T::dbName(), T::dbName()),
                                        std::forward<Args>(args)..., cont, pageLimit);

        return {join, join.empty() ? cont : join.back().right.id};
    }

    template <typename D, typename... Args>
    Page<D> findPagedDto(const std::string& where, uint64_t cont, Args&&... args) {
        auto results =
            db.select<T>(fmt::format("WHERE {} {} id > ? ORDER BY id ASC LIMIT ?", where, where.empty() ? "" : "AND"),
                         std::forward<Args>(args)..., cont, pageLimit);

        std::vector<D> dtos;
        dtos.reserve(results.size());

        std::transform(results.begin(), results.end(), std::back_inserter(dtos), [](T& res) { return D::fromt(res); });

        return {dtos, results.empty() ? cont : results.back().id};
    }

    void update(T& item) {
        db.update<T>(item);
    }

    uint64_t insert(T& item) {
        return db.insert<T>(item);
    }

protected:
    Database& db;
    const uint64_t pageLimit;
};
} // namespace Scissio
