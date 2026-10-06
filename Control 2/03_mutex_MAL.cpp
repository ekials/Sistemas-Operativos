#include <iostream>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <chrono>

// EJEMPLO MALO: si hay throw entre lock y unlock -> deadlock
std::mutex mtx;

void pause_thread(int id, int segs) {
    std::this_thread::sleep_for(std::chrono::seconds(segs));
    try {
        mtx.lock();
        throw std::logic_error("error");   // NUNCA llega al unlock
        std::cout << "yo soy " << id << " pause of " << segs << " seconds ended\n";
        mtx.unlock();
    }
    catch (std::logic_error&) {
        std::cout << "error atrapado pero NUNCA se hizo unlock -> deadlock\n";
        // mtx queda bloqueado para siempre
    }
}

int main() {
    std::cout << "Spawning threads (este ejemplo puede colgarse)...\n";
    for (int i = 0; i < 3; i++) {
        std::thread(pause_thread, i, 1).detach();
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Si llegaste aqui, algunos hilos quedaron bloqueados\n";
    return 0;
}
