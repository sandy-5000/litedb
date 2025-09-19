#include "litedb/buffer_manager.hpp"

namespace litedb::buffer_manager {


BufferManager::BufferManager() {
    root.forceRead(0);
    uint32_t page_id = root.getLeftMostChild();
    if (0 != page_id) {
        top = new litedb::page::Page();
        top->read(page_id);
    }
}

litedb::page::Page* BufferManager::getRoot() {
    return &root;
}

litedb::page::Page* BufferManager::getPage(size_t page_id) {
    auto page = &pages_[page_id % litedb::constants::BUFFER_MANAGER_SIZE];
    int32_t byte_count = page->read(page_id);
    return page;
}

litedb::page::Page* BufferManager::getEmptyPage(size_t page_id) {
    auto page = &pages_[page_id % litedb::constants::BUFFER_MANAGER_SIZE];
    return page;
}

uint32_t BufferManager::getFreePage() {
    std::unique_lock lock(mtx_);
    if (nullptr == top) {
        return litedb::g::pages_count++;
    }
    uint32_t page_id = top->getId();
    uint32_t next_page = top->getNextPage();
    if (0 == next_page) {
        delete top;
        top = nullptr;
    } else {
        top->read(next_page);
    }
    root.setLeftMostChild(next_page);
    return page_id;
}

void BufferManager::addFreePage(uint32_t page_id) {
    std::unique_lock lock(mtx_);
    uint32_t p_page_id = root.getLeftMostChild();
    if (0 == p_page_id) {
        top = new litedb::page::Page(page_id);
    } else {
        top->read(page_id);
    }
    top->setNextPage(p_page_id);
    top->write();
    root.setLeftMostChild(page_id);
}


}
