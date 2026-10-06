#include <iostream>
#include <thread>
#include <chrono>

void pause_thread(int id, int segs) 
{
    std::this_thread::sleep_for(std::chrono::seconds(segs));
    
    // std::cout es un recurso compartido (seccion critica). 
    // Sin std::mutex, las impresiones de varios hilos pueden entrelazarse.
    std::cout << "yo soy " << id << " - pause of " << segs << " seconds ended\n"; 
}

int main() 
{
    std::cout << "Spawning and detaching 5 threads...\n";
    
    for (int i = 0; i < 5; i++) {
        // Sintaxis: std::thread(funcion_a_ejecutar, arg1, arg2).detach()
        // .detach() independiza el hilo: al terminar se libera solo.
        std::thread(pause_thread, i, 2).detach();
    }
    
    std::cout << "Done spawning threads.\n";
    std::cout << "(main pausa 4 segundos para dar tiempo a los hilos)\n";
    
    // IMPORTANTE: Si el hilo main llega a 'return 0' y finaliza, 
    // el proceso entero se destruye y mata a todos los hilos detached activos.
    std::this_thread::sleep_for(std::chrono::seconds(4));
    
    std::cout << "Main termina\n";
    return 0;
}