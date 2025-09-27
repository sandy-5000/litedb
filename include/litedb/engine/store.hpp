#pragma once

#include "litedb/engine/root_manager.hpp"
#include "litedb/engine/buffer_manager.hpp"


namespace litedb::engine {


extern litedb::root_manager::RootManager* root_manager;
extern litedb::buffer_manager::BufferManager* buffer_manager;


}
