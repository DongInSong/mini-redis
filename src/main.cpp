#include "network/server.hpp"
#include <iostream>

int main()
{
    try
    {
        // Create and run the server on port 6379
        mini_redis::server s(6379);
        std::cout << "Mini-Redis server started on port 6379" << std::endl;
        s.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}