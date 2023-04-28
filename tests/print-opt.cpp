#include <OptParser.hpp>

using namespace std;
using namespace optp;

int main(int argc, char *argv[])
{
  OptParser opt;

  opt.addOption("a", "long-a", OptParser::OptType::value, false, "option a");
  opt.addOption("b", "long-b", OptParser::OptType::trigger, false, "option b");
  cout << opt << endl;

  return EXIT_SUCCESS;
}