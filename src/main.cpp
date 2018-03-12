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

int json2au(int argc, char **argv);

namespace {

constexpr const char *AU_VERSION = "0.1";

constexpr int AU_FORMAT_VERSION = 1; // TODO extract

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
    << "   cat      Decode listed files to stdout\n"
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

int todo(int, char **) {
  std::cout << "not yet implemented." << std::endl; // TODO
  return 1;
}


int stats(int argc, char **argv) {
  struct DictDumpHandler {
    std::vector<char> str_;

    DictDumpHandler() {
      str_.reserve(1<<16);
  }

    void onRecordStart(size_t) {}

    void onValue(size_t, size_t len, FileByteSource &source) {
      source.skip(len);
    }

    void onDictClear() {
      cout << "\n\nDictionary cleared ***********************\n\n\n";
    }

    void onDictAddStart(size_t) {
      std::cout << "\n";
    }

    void onStringStart(size_t len) {
      str_.clear();
      str_.reserve(len);
    }

    void onStringEnd() {
      std::cout << std::string_view(str_.data(), str_.size()) << "\n";
    }

    void onStringFragment(std::string_view frag) {
      str_.insert(str_.end(), frag.data(), frag.data()+frag.size());
    }
  };

  DictDumpHandler handler;
  if (argc == 0) {
    AuDecoder("-").decode(handler);
  } else {
    for (int i = 0; i < argc; i++) {
      std::string filename(argv[i]);
      AuDecoder(filename).decode(handler);
    }
  }
  return 0;
}

int grep(int argc, char **argv) {
  Dictionary dictionary;
  JsonHandler jsonHandler(dictionary);
  GrepHandler<JsonHandler> grepHandler(
      dictionary, jsonHandler, 60047061870829655ull);
  RecordHandler<decltype(grepHandler)> recordHandler(dictionary, grepHandler);

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
  commands["cat"] = cat;
  commands["tail"] = todo;
  commands["grep"] = grep;
  commands["enc"] = todo;
  commands["json2au"] = json2au;
  commands["stats"] = stats;

  std::string cmd(argv[1]);
  auto it = commands.find(cmd);
  if (it == commands.end()) {
    std::cerr << "Unknown option or command: " << cmd << std::endl;
    return usage(std::cerr);
  }

  return it->second(argc-2, argv+2);
}

