#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "tomlcpp.hpp"

using std::cerr;
using std::cout;

std::shared_ptr<toml::Table> load()
{
	toml::Result res = toml::parseFile("sample.toml");
	if (!res.table) {
		cerr << "ERROR: " << res.errmsg << "\n";
		exit(1);
	}
	return res.table;
}

int main()
{
	auto table = load();
	auto server = table->getTable("server");
	if (!server) {
		cerr << "ERROR: missing [server]\n";
		exit(1);
	}

	auto portArray = server->getArray("port");
	if (!portArray) {
		cerr << "ERROR: missing \"port\" array\n";
		exit(1);
	}

	auto port = portArray->getIntVector();
	if (!port) {
		cerr << "ERROR: unable to extract int vector from moreport\n";
		exit(1);
	}

	for (auto p : *port) {
		cout << p << "\n";
	}

	return 0;
}
