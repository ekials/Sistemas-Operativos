#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
int current_turn = 0;
bool ready = false;

void print_id(int id) {
    std::unique_lock<std::mutex> lck(mtx);
    // Espera a que alguien diga "go"
    while (!ready)
        cv.wait(lck);
    // Espera a que sea mi turno
    while (current_turn != id)
        cv.wait(lck);

    std::cout << "thread " << id << "\n";
    current_turn++;
    cv.notify_all();
}

void go() {
    std::unique_lock<std::mutex> lck(mtx);
    ready = true;
    cv.notify_all();
}

int main() {
    std::thread threads[10];
    for (int i = 0; i < 10; ++i)
        threads[i] = std::thread(print_id, i);

    std::cout << "10 threads ready to race...\n";
    go();

    for (auto& th : threads)
        th.join();

    return 0;
}
