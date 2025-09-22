#include "litedb/page/page_header.hpp"

namespace litedb::page {


PageHeader::PageHeader(uint8_t* data) : data_(data) {}

// ===== Accessors =====
std::uint32_t PageHeader::getId() const { return readValue<std::uint32_t>(PAGE_ID_OFFSET); }
void PageHeader::setId(std::uint32_t t) { writeValue<std::uint32_t>(PAGE_ID_OFFSET, t); }

std::uint8_t PageHeader::getType() const { return readValue<std::uint8_t>(PAGE_TYPE_OFFSET); }
void PageHeader::setType(std::uint8_t t) { writeValue<std::uint8_t>(PAGE_TYPE_OFFSET, t); }

std::uint8_t PageHeader::getFlag() const { return readValue<std::uint8_t>(PAGE_FLAG_OFFSET); }
void PageHeader::setFlag(std::uint8_t f) { writeValue<std::uint8_t>(PAGE_FLAG_OFFSET, f); }

std::uint16_t PageHeader::getFreeSpace() const { return readValue<std::uint16_t>(FREE_SPACE_OFFSET); }
void PageHeader::setFreeSpace(std::uint16_t f) { writeValue<std::uint16_t>(FREE_SPACE_OFFSET, f); }

std::uint64_t PageHeader::getChecksum() const { return readValue<std::uint64_t>(CHECKSUM_OFFSET); }
void PageHeader::setChecksum(std::uint64_t c) { writeValue<std::uint64_t>(CHECKSUM_OFFSET, c); }

std::uint16_t PageHeader::getRecordCount() const { return readValue<std::uint16_t>(RECORD_COUNT_OFFSET); }
void PageHeader::setRecordCount(std::uint16_t c) { writeValue<std::uint16_t>(RECORD_COUNT_OFFSET, c); }

std::uint16_t PageHeader::getSlotsDir() const { return readValue<std::uint16_t>(SLOTS_DIR_OFFSET); }
void PageHeader::setSlotsDir(std::uint16_t d) { writeValue<std::uint16_t>(SLOTS_DIR_OFFSET, d); }

std::uint32_t PageHeader::getNextPage() const { return readValue<std::uint32_t>(NEXT_PAGE_OFFSET); }
void PageHeader::setNextPage(std::uint32_t p) { writeValue<std::uint32_t>(NEXT_PAGE_OFFSET, p); }

std::uint64_t PageHeader::getLSN() const { return readValue<std::uint64_t>(LSN_OFFSET); }
void PageHeader::setLSN(std::uint64_t lsn) { writeValue<std::uint64_t>(LSN_OFFSET, lsn); }

std::uint64_t PageHeader::getMaxKey() const { return readValue<std::uint64_t>(MAX_KEY_OFFSET); }
void PageHeader::setMaxKey(std::uint64_t key) { writeValue<std::uint64_t>(MAX_KEY_OFFSET, key); }

std::uint32_t PageHeader::getPossibleParent() const { return readValue<std::uint32_t>(P_PARENT_OFFSET); }
void PageHeader::setPossibleParent(std::uint32_t p) { writeValue<std::uint32_t>(P_PARENT_OFFSET, p); }

std::uint32_t PageHeader::getLeftMostChild() const { return readValue<std::uint32_t>(LEFT_MOST_CHILD); }
void PageHeader::setLeftMostChild(std::uint32_t k) { writeValue<std::uint32_t>(LEFT_MOST_CHILD, k); }

void PageHeader::printHeader() const {
    std::cout << "------ Page header ---"
              << " (" << getId() << ")"
              << std::endl;
    for (int i = 0; i < PAGE_HEADER_SIZE; ++i) {
        std::printf("%02X ", data_[i]);
        if ((i + 1) % 8 == 0) std::cout << std::endl;
    }
}


}
