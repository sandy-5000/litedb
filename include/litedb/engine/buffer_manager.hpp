#pragma once

#include <unordered_map>
#include <memory>

#include "litedb/page/page.hpp"
#include "litedb/globals.hpp"
#include "litedb/engine/lru_cache.hpp"

namespace litedb::engine::buffer_manager {

class MiniBuffer {
private:
    std::string buffer_id_;
    size_t capacity_;
    litedb::lru_cache::LRUCache<uint32_t, std::shared_ptr<litedb::page::Page>> cache_;

public:
    MiniBuffer(const std::string& buffer_id, size_t capacity);
    ~MiniBuffer();

    std::shared_ptr<litedb::page::Page> getPage(size_t page_id);
    void setCapacity(size_t capacity);
};

class BufferManager {
private:
    std::unordered_map<std::string, std::shared_ptr<MiniBuffer>> buffers_;

public:
    BufferManager();
    ~BufferManager();

    std::optional<std::shared_ptr<MiniBuffer>> getBuffer(const std::string& buffer_id);
    std::shared_ptr<MiniBuffer> getBuffer(const std::string& buffer_id, size_t capacity);
    void setBuffer(const std::string& buffer_id, size_t capacity);
    bool removeBuffer(const std::string& buffer_id);

};

}