#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "tomlcpp.hpp"

using std::cerr;
using std::cout;

void fatal(std::string msg)
{
	cerr << "FATAL: " << msg << "\n";
	exit(1);
}

int main()
{
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
	bool ok;
	std::string host;
	std::tie(ok, host) = server->getString("host");
	if (!ok) {
		fatal("missing or bad host entry");
	}

	auto portArray = server->getArray("port");
	if (!portArray) {
		fatal("missing 'port' array");
	}

	auto port = portArray->getIntVector();
	if (!port) {
		fatal("unable to extract int vector from 'port'");
	}

	// 4. examine the values
	cout << "host: " << host << "\n";
	cout << "port: ";
	for (auto p : *port) {
		cout << p << " ";
	}
	cout << "\n";

	return 0;
}
