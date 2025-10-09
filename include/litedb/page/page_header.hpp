#pragma once

#include <mutex>
#include <shared_mutex>
#include <iostream>

#include "litedb/constants.hpp"

namespace litedb::page {

constexpr uint32_t INVALID_PAGE_ID        = 0xFFFFFFFF;

constexpr std::size_t PAGE_HEADER_SIZE    = litedb::constants::PAGE_HEADER_SIZE;

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
constexpr std::size_t LEFT_MOST_CHILD     = 44;

class PageHeader {
protected:
    uint8_t* data_;

public:
    template<typename T>
    T read_value(std::size_t offset) const;

    template<typename T>
    void write_value(std::size_t offset, const T value);

    PageHeader(uint8_t* data);

    std::uint32_t get_id() const;
    void set_id(std::uint32_t);

    std::uint8_t get_type() const;
    void set_type(std::uint8_t);

    std::uint8_t get_flag() const;
    void set_flag(std::uint8_t);

    std::uint16_t get_free_space() const;
    void set_free_space(std::uint16_t);

    std::uint64_t get_checksum() const;
    void set_checksum(std::uint64_t);

    std::uint16_t get_record_count() const;
    void set_record_count(std::uint16_t);

    std::uint16_t get_free_space_offset() const;
    void set_free_space_offset(std::uint16_t);

    std::uint32_t get_next_page() const;
    void set_next_page(std::uint32_t);

    std::uint64_t get_lsn() const;
    void set_lsn(std::uint64_t);

    std::uint64_t get_max_key() const;
    void set_max_key(std::uint64_t);

    std::uint32_t get_possible_parent() const;
    void set_possible_parent(std::uint32_t);

    std::uint32_t get_leftmost_child() const;
    void set_leftmost_child(std::uint32_t);

    void print_header() const;
};

template <typename T>
inline T PageHeader::read_value(std::size_t offset) const {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    return *reinterpret_cast<const T*>(data_ + offset);
}

template <typename T>
inline void PageHeader::write_value(std::size_t offset, const T value) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    *reinterpret_cast<T*>(data_ + offset) = value;
}


}
