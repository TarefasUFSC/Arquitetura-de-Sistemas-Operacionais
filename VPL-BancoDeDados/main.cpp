#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include "BancoDeDados.h"

std::atomic<int> numEscritas {0};
std::atomic<int> numLeituras {0};
std::mutex screenLock;
BancoDeDados* BancoDeDados::pinstance = nullptr;

void funcaoThreadLeitora(int idLeitor){
    BancoDeDados* bd = BancoDeDados::getInstance();
    int valorLido;
    for(int n = 0; n < 5; n++){
        valorLido = bd->read();
        screenLock.lock();
        std::cout << "Leitor #" << idLeitor << " leu = " << valorLido << std::endl; 
        screenLock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1050));
        numLeituras++;
    }
}

void funcaoThreadEscritora(int idEscritor){
    BancoDeDados* bd = BancoDeDados::getInstance();
    int ret;
    for(int n = 0; n < 5; n++){
        ret = bd->increment();
        screenLock.lock();
        std::cout << "Escritor #" << idEscritor << " incrementou a variável para = " << ret << std::endl;
        screenLock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        numEscritas++;
    }
}

int main(){
    std::thread leitor1(funcaoThreadLeitora, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread leitor2(funcaoThreadLeitora, 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread leitor3(funcaoThreadLeitora, 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    std::thread escritor1(funcaoThreadEscritora, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread escritor2(funcaoThreadEscritora, 2);

    leitor1.join();
    leitor2.join();
    leitor3.join();

    escritor1.join();
    escritor2.join();

    std::cout << "Foram realizadas " << numLeituras << " leituras e " << numEscritas << " escritas." << std::endl;
}