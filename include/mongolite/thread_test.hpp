#pragma once
#include <string>

namespace mongolite::thread_test {


void doWork(int id, const std::string& name, double value);
void launchThreads(int thread_count);


}