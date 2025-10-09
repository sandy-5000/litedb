#include "litedb/engine/buffer_manager.hpp"

namespace litedb::buffer_manager {


MiniBuffer::MiniBuffer(const std::string& buffer_id, size_t capacity) : buffer_id_(buffer_id), capacity_(capacity) {
    cache_.set_capacity(capacity);
}

MiniBuffer::~MiniBuffer() {
    auto page_list = cache_.get_list();
    for (auto page : page_list) {
        try {
            page.second->write();
        } catch (const std::exception& e) {
            std::cerr << "[MiniBuffer] " << buffer_id_ << " Failed to write buffer page-" << page.first << ": " << e.what() << "\n";
        }
    }
}

std::shared_ptr<litedb::page::Page> MiniBuffer::get_page(size_t page_id) {
    auto cached_page = cache_.get(page_id);
    if (cached_page) {
        return *cached_page;
    }
    std::shared_ptr<litedb::page::Page> p = std::make_shared<litedb::page::Page>();
    cache_.set(page_id, p);
    return p;
}

void MiniBuffer::set_capacity(size_t capacity) {
    capacity_ = capacity;
    cache_.set_capacity(capacity);
}


BufferManager::BufferManager() {
    set_buffer("root", 10);
}

BufferManager::~BufferManager() = default;


std::optional<std::shared_ptr<MiniBuffer>> BufferManager::get_buffer(const std::string& buffer_id) {
    auto it = buffers_.find(buffer_id);
    if (buffers_.end() == it) {
        return std::nullopt;
    }
    return it->second;
}

std::shared_ptr<MiniBuffer> BufferManager::get_buffer(const std::string& buffer_id, size_t capacity) {
    auto it = buffers_.find(buffer_id);
    if (buffers_.end() != it) {
        return it->second;
    }
    std::shared_ptr<MiniBuffer> buffer = std::make_shared<MiniBuffer>(buffer_id, capacity);
    buffers_[buffer_id] = buffer;
    return buffer;
}

void BufferManager::set_buffer(const std::string& buffer_id, size_t capacity) {
    auto it = buffers_.find(buffer_id);
    if (buffers_.end() != it) {
        it->second->set_capacity(capacity);
        return;
    }
    std::shared_ptr<MiniBuffer> buffer = std::make_shared<MiniBuffer>(buffer_id, capacity);
    buffers_[buffer_id] = buffer;
}

bool BufferManager::remove_buffer(const std::string& buffer_id) {
    auto it = buffers_.find(buffer_id);
    if (buffers_.end() == it) {
        return false;
    }
    buffers_.erase(it);
    return true;
}



}
