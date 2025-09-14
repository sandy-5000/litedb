#include "litedb/page/page.hpp"

namespace litedb::page {


Page::Page(std::uint32_t id) : id_(id), PageHeader(data_, &mtx_) {
    std::unique_lock lock(mtx_);
    std::memset(data_, 0, litedb::constants::PAGE_SIZE);
    writeValue(0, id);
}


}