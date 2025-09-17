#pragma once

#include "litedb/buffer_manager.hpp"
#include "litedb/page_manager.hpp"


namespace litedb::engine {

extern litedb::buffer_manager::BufferManager* buffer_manager;
extern litedb::page_manager::PageManager* page_manager;

}
