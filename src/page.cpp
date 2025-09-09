#include "litedb/page.hpp"

namespace litedb::page {

Page::Page(std::uint32_t id) : id_(id) {
    std::unique_lock lock(mtx_);
    std::memset(data_, 0, litedb::constants::PAGE_SIZE);
    set<std::uint32_t>(PAGE_ID_OFFSET, id);
}

std::vector<char> Page::getBytes(std::size_t offset, std::size_t len) {
    if (offset + len > litedb::constants::PAGE_SIZE) {
        throw std::out_of_range("Read exceeds page size");
    }
    std::shared_lock lock(mtx_);
    return std::vector<char>(data_ + offset, data_ + offset + len);
}

void Page::setBytes(std::size_t offset, const std::vector<char>& input) {
    if (offset + input.size() > litedb::constants::PAGE_SIZE) {
        throw std::out_of_range("Write exceeds page size");
    }
    std::unique_lock lock(mtx_);
    std::memcpy(data_ + offset, input.data(), input.size());
}

std::uint32_t Page::getId() const {
    return get<std::uint32_t>(PAGE_ID_OFFSET);
}

// ---- Type ----
std::uint8_t Page::getType() const {
    return get<std::uint8_t>(PAGE_TYPE_OFFSET);
}

void Page::setType(std::uint8_t type) {
    set<std::uint8_t>(PAGE_TYPE_OFFSET, type);
}

// ---- Flag ----
std::uint8_t Page::getFlag() const {
    return get<std::uint8_t>(PAGE_FLAG_OFFSET);
}

void Page::setFlag(std::uint8_t flag) {
    set<std::uint8_t>(PAGE_FLAG_OFFSET, flag);
}

// ---- Free Space ----
std::uint16_t Page::getFreeSpace() const {
    return get<std::uint16_t>(FREE_SPACE_OFFSET);
}

void Page::setFreeSpace(std::uint16_t free) {
    set<std::uint16_t>(FREE_SPACE_OFFSET, free);
}

// ---- Checksum ----
std::uint64_t Page::getChecksum() const {
    return get<std::uint64_t>(CHECKSUM_OFFSET);
}

void Page::setChecksum(std::uint64_t checksum) {
    set<std::uint64_t>(CHECKSUM_OFFSET, checksum);
}

// ---- Record Count ----
std::uint16_t Page::getRecordCount() const {
    return get<std::uint16_t>(RECORD_COUNT_OFFSET);
}

void Page::setRecordCount(std::uint16_t count) {
    set<std::uint16_t>(RECORD_COUNT_OFFSET, count);
}

// ---- Slots Directory ----
std::uint16_t Page::getSlotsDir() const {
    return get<std::uint16_t>(SLOTS_DIR_OFFSET);
}

void Page::setSlotsDir(std::uint16_t dir) {
    set<std::uint16_t>(SLOTS_DIR_OFFSET, dir);
}

// ---- Next Page ----
std::uint32_t Page::getNextPage() const {
    return get<std::uint32_t>(NEXT_PAGE_OFFSET);
}

void Page::setNextPage(std::uint32_t page) {
    set<std::uint32_t>(NEXT_PAGE_OFFSET, page);
}

// ---- LSN ----
std::uint64_t Page::getLSN() const {
    return get<std::uint64_t>(LSN_OFFSET);
}

void Page::setLSN(std::uint64_t lsn) {
    set<std::uint64_t>(LSN_OFFSET, lsn);
}

// ---- Max Key ----
std::uint64_t Page::getMaxKey() const {
    return get<std::uint64_t>(MAX_KEY_OFFSET);
}

void Page::setMaxKey(std::uint64_t key) {
    set<std::uint64_t>(MAX_KEY_OFFSET, key);
}

// ---- Possible Parent ----
std::uint32_t Page::getPossibleParent() const {
    return get<std::uint32_t>(P_PARENT_OFFSET);
}

void Page::setPossibleParent(std::uint32_t ptr) {
    set<std::uint32_t>(P_PARENT_OFFSET, ptr);
}

}