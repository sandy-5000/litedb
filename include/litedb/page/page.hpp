#include <cstdint>
#include <shared_mutex>
#include <vector>
#include <mutex>
#include <cstring>

#include "litedb/constants.hpp"
#include "litedb/page/page_header.hpp"

namespace litedb::page {


class Page : public PageHeader {
private:
    std::uint32_t id_;
    uint8_t data_[litedb::constants::PAGE_SIZE];
    mutable std::shared_mutex mtx_;

public:
    Page(std::uint32_t id);

    inline void lockShared()   const { mtx_.lock_shared(); }
    inline void unlockShared() const { mtx_.unlock_shared(); }
    inline void lockUnique()   const { mtx_.lock(); }
    inline void unlockUnique() const { mtx_.unlock(); }
};


}