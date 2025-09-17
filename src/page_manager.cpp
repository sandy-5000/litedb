#include "litedb/page_manager.hpp"

namespace litedb::page_manager {


PageManager::PageManager() {
    root.read(1);
    if (1 != root.getPossibleParent()) {
        top.read(root.getPossibleParent());
    }
}

uint32_t PageManager::popFreePage() {
    litedb::page::Page* page = &root;
    if (static_cast<uint32_t>(1) != page->getPossibleParent()) {
        page = &top;
    } else {
        if (static_cast<uint16_t>(0) == page->getRecordCount()) {
            return litedb::g::pages_count;
        }
    }
    uint32_t offset = litedb::page::PAGE_HEADER_SIZE + page->getRecordCount() * 2 - 2;
    return offset;
}

bool PageManager::pushFreePage() {
    litedb::page::Page* page = &root;
    if (static_cast<uint32_t>(1) != page->getPossibleParent()) {
        page = &top;
    }
    uint32_t offset = litedb::page::PAGE_HEADER_SIZE + page->getRecordCount() * 2 - 2;
    return true;
}


}