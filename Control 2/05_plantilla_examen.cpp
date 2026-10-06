#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

std::mutex mtx;
std::condition_variable cv;

// ---------- Cambia esto segun el enunciado ----------
void trabajador(int id) {
    // Si necesitas seccion critica:
    {
        std::lock_guard<std::mutex> lck(mtx);
        std::cout << "Hilo " << id << " en seccion critica\n";
    }
    // trabajo fuera del lock...
}

int main() {
    const int N = 5;
    std::vector<std::thread> hilos;

    for (int i = 0; i < N; ++i)
        hilos.emplace_back(trabajador, i);

    for (auto& h : hilos)
        h.join();

    return 0;
}
