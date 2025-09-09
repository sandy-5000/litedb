#pragma once
#include <cstddef>
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include <cstring>
#include <vector>
#include "constants.hpp"

namespace litedb::page {

constexpr std::size_t PAGE_HEADER_SIZE = 64;

// 8 Bytes
constexpr std::size_t PAGE_ID_OFFSET      = 0;
constexpr std::size_t PAGE_TYPE_OFFSET    = 4;
constexpr std::size_t PAGE_FLAG_OFFSET    = 5;
constexpr std::size_t FREE_SPACE_OFFSET   = 6;

// 8 Bytes
constexpr std::size_t CHECKSUM_OFFSET     = 8;

// 8 Bytes
constexpr std::size_t RECORD_COUNT_OFFSET = 16;
constexpr std::size_t SLOTS_DIR_OFFSET    = 18;
constexpr std::size_t NEXT_PAGE_OFFSET    = 20;

// 8 Bytes
constexpr std::size_t LSN_OFFSET          = 24;

// 8 Bytes
constexpr std::size_t MAX_KEY_OFFSET      = 32;

// 8 Bytes (4 + 4 empty)
constexpr std::size_t P_PARENT_OFFSET     = 40;

class Page {
private:
    std::size_t id_;
    char data_[litedb::constants::PAGE_SIZE];
    mutable std::shared_mutex mtx_;

    template<typename T>
    T get(std::size_t offset) const;

    template<typename T>
    void set(std::size_t offset, const T& value);

public:
    Page(std::uint32_t id);

    std::vector<char> getBytes(std::size_t offset, std::size_t len);
    void setBytes(std::size_t offset, const std::vector<char>& data);

    std::uint32_t getId() const;

    std::uint8_t getType() const;
    void setType(std::uint8_t type);

    std::uint8_t getFlag() const;
    void setFlag(std::uint8_t flag);

    std::uint16_t getFreeSpace() const;
    void setFreeSpace(std::uint16_t free);

    std::uint64_t getChecksum() const;
    void setChecksum(std::uint64_t checksum);

    std::uint16_t getRecordCount() const;
    void setRecordCount(std::uint16_t count);

    std::uint16_t getSlotsDir() const;
    void setSlotsDir(std::uint16_t dir);

    std::uint32_t getNextPage() const;
    void setNextPage(std::uint32_t page);

    std::uint64_t getLSN() const;
    void setLSN(std::uint64_t lsn);

    std::uint64_t getMaxKey() const;
    void setMaxKey(std::uint64_t key);

    std::uint32_t getPossibleParent() const;
    void setPossibleParent(std::uint32_t ptr);
};

template<typename T>
T Page::get(std::size_t offset) const {
    if (offset + sizeof(T) > litedb::constants::PAGE_SIZE) {
        throw std::out_of_range("Read exceeds page size");
    }
    std::shared_lock lock(mtx_);
    T value;
    std::memcpy(&value, data_ + offset, sizeof(T));
    return value;
}

template<typename T>
void Page::set(std::size_t offset, const T& value) {
    if (offset + sizeof(T) > litedb::constants::PAGE_SIZE) {
        throw std::out_of_range("Write exceeds page size");
    }
    std::unique_lock lock(mtx_);
    std::memcpy(data_ + offset, &value, sizeof(T));
}

}
