#pragma once

#include <mutex>
#include <shared_mutex>
#include <iostream>

#include "litedb/constants.hpp"

namespace litedb::page {


constexpr std::size_t PAGE_HEADER_SIZE = litedb::constants::PAGE_HEADER_SIZE;

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

// 8 Bytes
constexpr std::size_t P_PARENT_OFFSET     = 40;
constexpr std::size_t LOWEST_KEY          = 44;

class PageHeader {
protected:
    uint8_t* data_;
    mutable std::shared_mutex* mtx_;

    template<typename T>
    T readValue(std::size_t offset) const;

    template<typename T>
    void writeValue(std::size_t offset, const T value);

public:
    PageHeader(uint8_t* data, std::shared_mutex* mtx);

    std::uint32_t getId() const;

    std::uint8_t getType() const;
    void setType(std::uint8_t);

    std::uint8_t getFlag() const;
    void setFlag(std::uint8_t);

    std::uint16_t getFreeSpace() const;
    void setFreeSpace(std::uint16_t);

    std::uint64_t getChecksum() const;
    void setChecksum(std::uint64_t);

    std::uint16_t getRecordCount() const;
    void setRecordCount(std::uint16_t);

    std::uint16_t getSlotsDir() const;
    void setSlotsDir(std::uint16_t);

    std::uint32_t getNextPage() const;
    void setNextPage(std::uint32_t);

    std::uint64_t getLSN() const;
    void setLSN(std::uint64_t);

    std::uint64_t getMaxKey() const;
    void setMaxKey(std::uint64_t);

    std::uint32_t getPossibleParent() const;
    void setPossibleParent(std::uint32_t);

    std::uint32_t getLowestKey() const;
    void setLowestKey(std::uint32_t);

    void printHeader() const;
};

template <typename T>
inline T PageHeader::readValue(std::size_t offset) const {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    // std::shared_lock lock(*mtx_);
    return *reinterpret_cast<const T*>(data_ + offset);
}

template <typename T>
inline void PageHeader::writeValue(std::size_t offset, const T value) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    // std::unique_lock lock(*mtx_);
    *reinterpret_cast<T*>(data_ + offset) = value;
}


}
