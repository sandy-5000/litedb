#include "litedb/tables/find.hpp"

namespace litedb::table::utils {

// ------------------------------------------------------------------------
uint16_t find::key_internal(
    std::shared_ptr<litedb::page::Page> page, std::string &key
) {
    if (!page) {
        throw std::invalid_argument("Page is null");
    }

    uint8_t type = page->get_type() & 0xC0;
    if (0xC0 != type) {
        throw std::runtime_error("Invalid page type in key_internal");
    }

    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    uint16_t record_count = page->get_record_count();
    uint16_t low = 0, high = record_count;

    while (low < high) {
        uint16_t mid = low + (high - low) / 2;

        uint16_t record_offset = slot_ptr[mid];
        int8_t cmp = compare::key(
            reinterpret_cast<const uint8_t*>(key.c_str()), 0, 0,
            page->data_ + record_offset, 4, 0
        );

        if (cmp == -1) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    return low;
}

// ------------------------------------------------------------------------
uint16_t find::key_leaf(
    std::shared_ptr<litedb::page::Page> page, std::string &key
) {
    if (!page) {
        throw std::invalid_argument("Page is null");
    }

    uint8_t type = page->get_type() & 0xC0;
    if (0x80 != type) {
        throw std::runtime_error("Invalid page type in key_leaf");
    }

    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    uint16_t record_count = page->get_record_count();
    uint16_t low = 0, high = record_count;

    while (low < high) {
        uint16_t mid = low + (high - low) / 2;

        uint16_t record_offset = slot_ptr[mid];
        int8_t cmp = compare::key(
            reinterpret_cast<const uint8_t*>(key.c_str()), 0, 0,
            page->data_ + record_offset, 0, 0
        );

        if (cmp == -1) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    return low;
}

// ------------------------------------------------------------------------
uint16_t find::data_internal(
    std::shared_ptr<litedb::page::Page> page, uint64_t key
) {
    if (!page) {
        throw std::invalid_argument("Page is null");
    }

    uint8_t type = page->get_type() & 0xC0;
    if (0x40 != type) {
        throw std::runtime_error("Invalid page type in data_internal");
    }

    slot_data_internal* slot_ptr = reinterpret_cast<slot_data_internal*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    uint16_t record_count = page->get_record_count();
    uint16_t low = 0, high = record_count;
    bool is_equal = false;

    while (low < high) {
        uint16_t mid = (low + high) / 2;
        slot_data_internal* mid_ptr = slot_ptr + mid;

        if (key > mid_ptr->key) {
            low = mid + 1;
        } else {
            is_equal = (key == mid_ptr->key);
            high = mid;
        }
    }

    return low;
}

// ------------------------------------------------------------------------
uint16_t find::data_leaf(
    std::shared_ptr<litedb::page::Page> page, uint64_t key
) {
    if (!page) {
        throw std::invalid_argument("Page is null");
    }

    uint8_t type = page->get_type() & 0xC0;
    if (0x00 != type) {
        throw std::runtime_error("Invalid page type in data_leaf");
    }

    slot_data_leaf* slot_ptr = reinterpret_cast<slot_data_leaf*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    uint16_t record_count = page->get_record_count();
    int16_t low = 0, high = record_count;
    bool is_equal = false;

    while (low < high) {
        int16_t mid = (low + high) / 2;
        slot_data_leaf* mid_ptr = slot_ptr + mid;

        if (key > mid_ptr->key) {
            low = mid + 1;
        } else {
            is_equal = (key == mid_ptr->key);
            high = mid;
        }
    }

    return low;
}


}