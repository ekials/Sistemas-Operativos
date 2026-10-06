#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
/*
programa donde N hilos incrementen un contador global o 
modifiquen un arreglo compartido sin que haya condiciones de carrera 
(data race) ni pérdidas de datos.
*/
std::mutex mtx;
int contador_compartido = 0;

void incrementar(int iteraciones) {
    for (int i = 0; i < iteraciones; ++i) {
        std::lock_guard<std::mutex> lock(mtx); // Proteger sección crítica
        contador_compartido++;
    }
}

int main() {
    std::vector<std::thread> hilos;
    for (int i = 0; i < 5; ++i)
        hilos.push_back(std::thread(incrementar, 1000));

    for (auto& h : hilos)
        h.join(); // Esperar a todos los hilos

    std::cout << "Resultado final: " << contador_compartido << "\n";
    return 0;
}