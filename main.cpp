#include "server.hpp"

using namespace std;

int main()
{
    hashd::Server s;
    s.run(11011);

    return 0;
}
