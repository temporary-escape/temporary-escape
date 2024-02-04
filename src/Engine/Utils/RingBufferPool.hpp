#pragma once

#include <array>
#include <list>
#include <memory>

namespace Engine {
template <typename T, size_t ChunkSize> class RingBufferPool {
public:
    using Array = std::array<T, ChunkSize>;
    using Chunks = std::list<std::unique_ptr<Array>>;

    struct Iterator {
        Iterator() : chunks{nullptr}, ring{}, it{0} {
        }

        Iterator(Chunks& chunks, typename Chunks::iterator ring, size_t it) : chunks{&chunks}, ring{ring}, it{it} {
        }

        void next() {
            if (chunks) {
                it++;

                if (it < ChunkSize) {
                    return;
                }

                ring++;
                it = 0;

                if (ring != chunks->end()) {
                    return;
                }

                chunks = nullptr;
            }
        }

        T& get() {
            return (*ring)->at(it);
        }

        bool operator==(const Iterator& other) const {
            if (chunks == nullptr && other.chunks == nullptr) {
                return true;
            }
            return chunks == other.chunks && ring == other.ring && it == other.it;
        }

        bool operator!=(const Iterator& other) const {
            return !this->operator==(other);
        }

        operator bool() const {
            return chunks != nullptr;
        }

        Chunks* chunks;
        typename Chunks::iterator ring;
        size_t it;
    };

    RingBufferPool() {
        chunks.push_back(std::make_unique<Array>());
        head = Iterator{chunks, chunks.begin(), 0};
        tail = Iterator{chunks, chunks.begin(), 0};
    }

    Iterator allocate() {
        auto res = head;
        head.next();

        if (!head) {
            chunks.push_back(std::make_unique<Array>());
            head = Iterator{chunks, --chunks.end(), 0};
        }

        size++;
        return res;
    }

    void pop() {
        if (tail == head) {
            return;
        }

        bool pop{false};
        if (tail.it + 1 == ChunkSize) {
            pop = true;
        }

        tail.next();

        if (pop) {
            chunks.pop_front();
        }

        size--;
    }

    Iterator getHead() {
        return head;
    }

    Iterator getTail() const {
        return tail;
    }

    size_t getCapacity() const {
        return chunks.size() * ChunkSize;
    }

    size_t getSize() const {
        return size;
    }

private:
    Iterator head;
    Iterator tail;
    Chunks chunks;
    size_t size{0};
};
} // namespace Engine
