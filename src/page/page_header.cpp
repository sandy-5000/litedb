#include "litedb/page/page_header.hpp"

namespace litedb::page {


PageHeader::PageHeader(uint8_t* data) : data_(data) {}

// ===== Accessors =====
std::uint32_t PageHeader::get_id() const { return read_value<std::uint32_t>(PAGE_ID_OFFSET); }
void PageHeader::set_id(std::uint32_t t) { write_value<std::uint32_t>(PAGE_ID_OFFSET, t); }

std::uint8_t PageHeader::get_type() const { return read_value<std::uint8_t>(PAGE_TYPE_OFFSET); }
void PageHeader::set_type(std::uint8_t t) { write_value<std::uint8_t>(PAGE_TYPE_OFFSET, t); }

std::uint8_t PageHeader::get_flag() const { return read_value<std::uint8_t>(PAGE_FLAG_OFFSET); }
void PageHeader::set_flag(std::uint8_t f) { write_value<std::uint8_t>(PAGE_FLAG_OFFSET, f); }

std::uint16_t PageHeader::get_free_space() const { return read_value<std::uint16_t>(FREE_SPACE_OFFSET); }
void PageHeader::set_free_space(std::uint16_t f) { write_value<std::uint16_t>(FREE_SPACE_OFFSET, f); }

std::uint64_t PageHeader::get_checksum() const { return read_value<std::uint64_t>(CHECKSUM_OFFSET); }
void PageHeader::set_checksum(std::uint64_t c) { write_value<std::uint64_t>(CHECKSUM_OFFSET, c); }

std::uint16_t PageHeader::get_record_count() const { return read_value<std::uint16_t>(RECORD_COUNT_OFFSET); }
void PageHeader::set_record_count(std::uint16_t c) { write_value<std::uint16_t>(RECORD_COUNT_OFFSET, c); }

std::uint16_t PageHeader::get_free_space_offset() const { return read_value<std::uint16_t>(SLOTS_DIR_OFFSET); }
void PageHeader::set_free_space_offset(std::uint16_t d) { write_value<std::uint16_t>(SLOTS_DIR_OFFSET, d); }

std::uint32_t PageHeader::get_next_page() const { return read_value<std::uint32_t>(NEXT_PAGE_OFFSET); }
void PageHeader::set_next_page(std::uint32_t p) { write_value<std::uint32_t>(NEXT_PAGE_OFFSET, p); }

std::uint64_t PageHeader::get_lsn() const { return read_value<std::uint64_t>(LSN_OFFSET); }
void PageHeader::set_lsn(std::uint64_t lsn) { write_value<std::uint64_t>(LSN_OFFSET, lsn); }

std::uint64_t PageHeader::get_max_key() const { return read_value<std::uint64_t>(MAX_KEY_OFFSET); }
void PageHeader::set_max_key(std::uint64_t key) { write_value<std::uint64_t>(MAX_KEY_OFFSET, key); }

std::uint32_t PageHeader::get_possible_parent() const { return read_value<std::uint32_t>(P_PARENT_OFFSET); }
void PageHeader::set_possible_parent(std::uint32_t p) { write_value<std::uint32_t>(P_PARENT_OFFSET, p); }

std::uint32_t PageHeader::get_leftmost_child() const { return read_value<std::uint32_t>(LEFT_MOST_CHILD); }
void PageHeader::set_leftmost_child(std::uint32_t k) { write_value<std::uint32_t>(LEFT_MOST_CHILD, k); }

void PageHeader::print_header() const {
    std::cout << "------ Page header ---"
              << " (" << get_id() << ")"
              << std::endl;
    for (int i = 0; i < PAGE_HEADER_SIZE; ++i) {
        std::printf("%02X ", data_[i]);
        if ((i + 1) % 8 == 0) std::cout << std::endl;
    }
}


}
