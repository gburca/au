#include "main.h"
#include "au/AuEncoder.h"
#include "au/AuDecoder.h"
#include "JsonHandler.h"
#include "GrepHandler.h"

#include <cmath>
#include <cstdio>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace rapidjson;

namespace {

int version(int, char **) {
  std::cout << "au version " << AU_VERSION
    << " (format version " << AU_FORMAT_VERSION << ")" << std::endl;
  return 0;
}

int usage(std::ostream &os) {
  os << "usage: au [--version] [--help] <command> [args]" << std::endl;
  return 1;
}

int help(int, char **) {
  usage(std::cout);
  std::cout << "\nCommands:\n"
    << "   canned   Dump a canned snippet\n"
    << "   cat      Decode listed files to stdout (au2json)\n"
    << "   tail     Decode and/or follow file\n"
    << "   grep     Find records matching pattern\n"
    << "   enc      Encode listed files to stdout\n"
    << "   json2au  <json_file> <au_file> [count]\n"
    << "            Encode json to au (either file can be '-')\n"
    << "            Optionally stops after count lines have been encoded\n"
    << "   stats    Display file statistics\n";
  return 0;
}

int cat(int argc, char **argv) {
  argc -= 2; argv += 2;
  Dictionary dictionary;
  JsonHandler valueHandler(dictionary);
  RecordHandler<JsonHandler> recordHandler(dictionary, valueHandler);

  if (argc == 0) {
    AuDecoder("-").decode(recordHandler);
  } else {
    for (int i = 0; i < argc; i++) {
      std::string filename(argv[i]);
      AuDecoder(filename).decode(recordHandler);
    }
  }
  return 0;
}

}

int main(int argc, char **argv) {
  if (argc < 2) {
    help(0, nullptr);
    return 1;
  }

  std::unordered_map<std::string, std::function<int(int, char **)>> commands;
  commands["--version"] = version;
  commands["--help"] = help;
  commands["canned"] = canned;
  commands["cat"] = cat;
  commands["tail"] = tail;
  commands["grep"] = grep;
  commands["enc"] = json2au;
  commands["json2au"] = json2au;
  commands["stats"] = stats;

  std::string cmd(argv[1]);
  auto it = commands.find(cmd);
  if (it == commands.end()) {
    std::cerr << "Unknown option or command: " << cmd << std::endl;
    return usage(std::cerr);
  }

  return it->second(argc, argv);
}

