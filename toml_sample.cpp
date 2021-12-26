#include "tomlcpp.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using std::cerr;
using std::cout;

void fatal(std::string msg) {
  cerr << "FATAL: " << msg << "\n";
  exit(1);
}

int main() {
  // 1. parse file
  auto res = toml::parseFile("sample.toml");
  if (!res.table) {
    fatal("cannot parse file: " + res.errmsg);
  }

  // 2. get top level table
  auto server = res.table->getTable("server");
  if (!server) {
    fatal("missing [server]");
  }

  // 3. extract values from the top level table
  auto[ok, host] = server->getString("host");
  if (!ok) {
    fatal("missing or bad host entry");
  }

  auto portArray = server->getArray("port");
  if (!portArray) {
    fatal("missing 'port' array");
  }

  // 4. examine the values
  cout << "host: " << host << "\n";
  cout << "port: ";
  for (int i = 0;; i++) {
    auto p = portArray->getInt(i);
    if (!p.first)
      break;

    cout << p.second << " ";
  }
  cout << "\n";

  return 0;
}
