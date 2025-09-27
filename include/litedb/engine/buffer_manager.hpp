#pragma once

#include "litedb/page/page.hpp"
#include "litedb/globals.hpp"
#include "litedb/engine/lru_cache.hpp"

namespace litedb::buffer_manager {


class BufferManager {
private:
    size_t capacity_;
    litedb::lru_cache::LRUCache<uint32_t, std::shared_ptr<litedb::page::Page>> cache_;

public:
    BufferManager(size_t capacity);
    ~BufferManager();

    std::shared_ptr<litedb::page::Page> getPage(size_t page_id);
};


}