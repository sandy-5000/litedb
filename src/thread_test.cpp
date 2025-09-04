#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

namespace mongolite::thread_test {


void doWork(int id, const std::string& name, double value) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    long long cnt = 0, n = 1e8;
    for (int i = 0; i < n; ++i) {
        cnt += i;
    }
    std::cout << "Count: " << cnt << std::endl;
    std::cout << "Thread " << id << " (" << name << ") finished with value [" << value << "]" << "\n";
}

void launchThreads(int thread_count) {
    std::vector<std::jthread> threads;

    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(doWork, i, "Task" + std::to_string(i), i * 3.14);
    }

    std::cout << "All threads launched\n";
}


}