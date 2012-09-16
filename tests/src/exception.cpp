#include <iostream>

int main()
{
        try
        {
                throw 5;
        }
        catch (int e)
        {
                std::cout << "Exception: " << e << std::endl;
        }
}

