#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool listo = false;

void consumidor() {
    std::unique_lock<std::mutex> lock(mtx);
    // Espera hasta que 'listo' sea true
    cv.wait(lock, [] { return listo; }); 
    std::cout << "Consumidor: Dato recibido y procesado\n";
}

void productor() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        listo = true;
    }
    cv.notify_one(); // Avisar al consumidor
}

int main() {
    std::thread t1(consumidor);
    std::thread t2(productor);

    t1.join();
    t2.join();
    return 0;
}