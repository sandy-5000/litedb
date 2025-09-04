#pragma once
#include <cstddef>
#include <iostream>

namespace mongolite {


inline constexpr std::size_t PAGE_SIZE = 4096;

inline void print_page_size() {
    std::cout << "Page size = " << PAGE_SIZE << " bytes\n";
}


}
