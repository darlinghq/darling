#include <iostream>
#include <stdexcept>
#include <string>

#include <iostream>

void g(int n)
{
  if (n == 17) {
    std::cerr << "Throwing an exception" << std::endl;
    throw std::invalid_argument("17 is a bad number");
  }
  std::cerr << "Not throwing an exception" << std::endl;
}

int do_test_2(int n)
{
  std::cerr << "Entering do_test_2\n";
  try {
    g(n);
  }
  catch(const std::invalid_argument& e) {
    std::cerr << "Caught an exception: " << e.what() << std::endl;
    return 1;
  }
  catch(...) {
    std::cerr << "Caught an unexpected exception" << std::endl;
    return 2;
  }

  std::cerr << "Normal exit from do_test_2\n";
  return 0;
}

extern "C" int do_test(int n);

int main()
{
  std::cerr << "About to call do_test" << std::endl;
  int result = do_test(16);
  std::cerr << "Return value is " << result << std::endl;

  std::cerr << "About to call do_test_2" << std::endl;
  int result2 = do_test_2(17);
  std::cerr << "Return value is " << result2 << std::endl;
}
