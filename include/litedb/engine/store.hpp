#pragma once

#include "litedb/engine/root_manager.hpp"
#include "litedb/engine/buffer_manager.hpp"

namespace litedb::engine {

extern root_manager::RootManager* root_manager_;
extern buffer_manager::BufferManager* buffer_manager_;

}
