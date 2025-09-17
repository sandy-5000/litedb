#pragma once

#include <cstdint>
#include <vector>

#include "litedb/page/page.hpp"
#include "litedb/globals.hpp"

namespace litedb::page_manager {


class PageManager {
private:
    litedb::page::Page root;
    litedb::page::Page top;

public:
    PageManager();

    uint32_t popFreePage();
    bool pushFreePage();
};


}
