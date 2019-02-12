#include <cassert>
#include <cmath>
#include <copcomp/2019packet.hpp>
#include <copcomp/copcomp.hpp>
#include <iostream>

using namespace team114::copcomp;
using namespace team114::c2019::vision;
using namespace std;
int main()
{
    // std::cout << "init" << std::endl;

    Packet p;
    p.micros = 123213213;
    p.x = 123.3219;
    p.y = -323.089;

    std::cout << p.micros << p.x << p.y << std::endl;

    std::string addr("0.0.0.0");
    std::string port("5808");
    Connection one(addr, port);
    one.write_item<Packet>(p);
    std::cout << "sent" << std::endl;
}
