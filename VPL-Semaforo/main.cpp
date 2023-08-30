#include <iostream>
#include <string>
#include "MySemaphore.h"

AbstractSemaphore* s = new MySemaphore(2);
std::mutex mp;

void func(int id){
    s->acquire();
	using namespace std::literals;
    mp.lock();
    std::cout << id << " Entrando na região crítica!" << std::endl;
    mp.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    mp.lock();
	std::cout << id << " Saindo da região crítica!" << std::endl;
    mp.unlock();
    s->release();
}

int main(void){
    std::thread t1(func, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t2(func, 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t3(func, 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t4(func, 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t5(func, 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t6(func, 6);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t7(func, 7);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t8(func, 8);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t9(func, 9);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t10(func, 10);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t11(func, 11);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t12(func, 12);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t13(func, 13);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t14(func, 14);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t15(func, 15);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t16(func, 16);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t17(func, 17);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t18(func, 18);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t19(func, 19);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::thread t20(func, 20);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    t8.join();
    t9.join();
    t10.join();
    t11.join();
    t12.join();
    t13.join();
    t14.join();
    t15.join();
    t16.join();
    t17.join();
    t18.join();
    t19.join();
    t20.join();
}


