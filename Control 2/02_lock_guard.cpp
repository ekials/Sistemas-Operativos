#include <iostream>
#include <thread>
#include <mutex>
#include <stdexcept>

std::mutex mtx;

void print_even(int x) {
    if (x % 2 == 0)
        std::cout << x << " is even\n";
    else
        throw std::logic_error("not even");
}

void print_thread_id(int id) {
    try {
        // lock_guard: se desbloquea SOLO al salir del scope (incluso con throw)
        std::lock_guard<std::mutex> lck(mtx);
        print_even(id);
    }
    catch (std::logic_error&) {
        std::cout << "[exception caught]\n";
    }
}

int main() {
    std::thread threads[10];
    for (int i = 0; i < 10; ++i)
        threads[i] = std::thread(print_thread_id, i + 1);

    for (auto& th : threads)
        th.join();

    return 0;
}
