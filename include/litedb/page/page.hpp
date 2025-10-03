#pragma once
#include <cstdint>
#include <shared_mutex>
#include <vector>
#include <mutex>
#include <cstring>

#include "litedb/constants.hpp"
#include "litedb/page/page_header.hpp"
#include "litedb/page/page_io.hpp"

namespace litedb::page {


class Page : public PageHeader {
private:
    uint32_t id_;
    mutable std::shared_mutex mtx_;
    bool dirty_ = false;

public:
    uint8_t data_[litedb::constants::PAGE_SIZE];
    Page();
    ~Page();

    inline void lockShared()   const { mtx_.lock_shared(); }
    inline void unlockShared() const { mtx_.unlock_shared(); }
    inline void lockUnique()   const { mtx_.lock(); }
    inline void unlockUnique() const { mtx_.unlock(); }

    uint32_t id() const;
    void readEmpty(uint32_t page_id);
    ssize_t read(uint32_t page_id);
    ssize_t forceRead(uint32_t page_id);
    ssize_t write();

    void setDirty();
    bool isDirty();
    bool isEmpty();
    void clear();
};


}