#include <cassert>
#include <cmath>
#include <copcomp/2019packet.hpp>
#include <copcomp/copcomp.hpp>
#include <cstdlib>
#include <iostream>
#include <thread>

using namespace team114::copcomp;
using namespace team114::c2019::vision;
using namespace std;
int main()
{
    std::string addr("0.0.0.0");
    std::string port("5808");
    Connection one(addr, port);
    uint64_t micros = 100000;
    srand(123);
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        Packet p;
        p.micros = ++micros;
        p.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 100.0));
        p.y = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 100.0));
        one.write_item<Packet>(p);
        std::cout << "sent Packet {\n\tmicros: " << p.micros << "\n\tx: " << p.x << "\n\ty: " << p.y << "\n}" << std::endl;
    }
}
