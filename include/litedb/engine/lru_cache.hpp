#pragma once

#include <cstddef>
#include <type_traits>
#include <list>
#include <unordered_map>
#include <optional>
#include <algorithm>

namespace litedb::lru_cache {


template<typename K, typename T>
requires std::is_scalar_v<K>
class LRUCache {
private:
    using ListIt = typename std::list<std::pair<K, T>>::iterator;

    std::unordered_map<K, ListIt> map_;
    std::list<std::pair<K, T>> list_;

    size_t capacity_ = 10;

public:
    LRUCache() {};
    void set_capacity(size_t capacity);
    size_t getCapacity();

    void set(K key, T record);
    std::optional<T> get(K key);

    std::list<std::pair<K, T>>& get_list();

};

template<typename K, typename T>
requires std::is_scalar_v<K>
void LRUCache<K, T>::set(K key, T record) {
    auto p = map_.find(key);
    if (map_.end() != p) {
        list_.splice(list_.begin(), list_, p->second);
        p->second->second = record;
        return;
    }
    if (list_.size() == capacity_) {
        auto last = list_.back();
        map_.erase(last.first);
        list_.pop_back();
    }
    list_.push_front({key, record});
    map_[key] = list_.begin();
}

template<typename K, typename T>
requires std::is_scalar_v<K>
std::optional<T> LRUCache<K, T>::get(K key) {
    auto p = map_.find(key);
    if (map_.end() == p) {
        return std::nullopt;
    }
    list_.splice(list_.begin(), list_, p->second);
    return p->second->second;
}

template<typename K, typename T>
requires std::is_scalar_v<K>
void LRUCache<K, T>::set_capacity(size_t capacity) {
    capacity_ = std::max((size_t)10, capacity);
    while (list_.size() > capacity_) {
        auto last = list_.back();
        map_.erase(last.first);
        list_.pop_back();
    }
}

template<typename K, typename T>
requires std::is_scalar_v<K>
size_t LRUCache<K, T>::getCapacity() {
    return capacity_;
}

template<typename K, typename T>
requires std::is_scalar_v<K>
std::list<std::pair<K, T>>& LRUCache<K, T>::get_list() {
    return list_;
}


}