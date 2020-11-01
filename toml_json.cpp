/*
  MIT License

  Copyright (c) 2020 CK Tan
  https://github.com/cktan/tomlcpp

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <memory>
#include <iostream>
#include <vector>
#include <iterator>
#include "tomlcpp.hpp"

using std::cout;
using std::cerr;
using std::string;

static void print_escape_string(const string& str)
{
	for (char ch : str) {
		switch (ch) {
		case '\b': cout << "\\b"; break;
		case '\t': cout << "\\t"; break;
		case '\n': cout << "\\n"; break;
		case '\f': cout << "\\f"; break;
		case '\r': cout << "\\r"; break;
		case '"':  cout << "\\\""; break;
		case '\\': cout << "\\\\"; break;
		default: cout << ch; break;
		}
	}
}


static string z4(int i)
{
	char buf[10];
	sprintf(buf, "%04d", i);
	return buf;
}

static string z2(int i)
{
	char buf[10];
	sprintf(buf, "%02d", i);
	return buf;
}


static void print(const toml::Value& v)
{
	{
		auto s = v.toString();
		if (s) {
			cout << "{\"type\":\"string\",\"value\":\"";
			print_escape_string(*s);
			cout << "\"}";
			return;
		}
	}

	{
		auto i = v.toInt();
		if (i) {
			cout << "{\"type\":\"integer\",\"value\":\"";
			cout << *i;
			cout << "\"}";
			return;
		}
	}

	{
		auto b = v.toBool();
		if (b) {
			cout << "{\"type\":\"bool\",\"value\":\"" << *b << "\"}";
			return;
		}
	}

	{
		auto d = v.toDouble();
		if (d) {
			cout << "{\"type\":\"float\",\"value\":\"" << *d << "\"}";
			return;
		}
	}

	{
		auto ts = v.toTimestamp();
		if (ts) {
			if (ts->year != -1 && ts->hour != -1) {
				cout << "{\"type\":\"datetime\",\"value\":\"";
				cout << z4(ts->year);
				cout << "-" << z2(ts->month);
				cout << "-" << z2(ts->day);
				cout << "T" << z2(ts->hour);
				cout << ":" << z2(ts->minute);
				cout << ":" << z2(ts->second);
				if (ts->millisec != -1) 
					cout << "." << ts->millisec;
				cout << ts->z << "\"}";
				return;
			}

			if (ts->year != -1) {
				cout << "{\"type\":\"date\",\"value\":\"";
				cout << z4(ts->year);
				cout << "-" << z2(ts->month);
				cout << "-" << z2(ts->day);
				cout << "\"}";
				return;
			}

			if (ts->hour != -1) {
				cout << "{\"type\":\"time\",\"value\":\"";
				cout << z2(ts->hour);
				cout << ":" << z2(ts->minute);
				cout << ":" << z2(ts->second);
				if (ts->millisec != -1) 
					cout << "." << ts->millisec;
				cout << "\"}";
				return;
			}
		}
	}

	cerr << "unknown type\n";
	exit(1);
}


static void print(const toml::Array& arr);
static void print(const toml::Table& curtab)
{
	bool first = true;
	cout << "{";
	for (auto& key : curtab.keys()) {
		const char* sep = first ? "" : ",";
		first = false;

		cout << sep << "\"";
		print_escape_string(key);
		cout << "\":";

		auto v = curtab.getValue(key);
		if (v) {
			print(*v);
			continue;
		}

		auto a = curtab.getArray(key);
		if (a) {
			print(*a);
			continue;
		}

		auto t = curtab.getTable(key);
		if (t) {
			print(*t);
			continue;
		}
		
		abort();
	}
	cout << "}";
}


static void print(const toml::Array& curarr)
{
	if (curarr.kind() == 't') {
		cout << "[";
		for (int i = 0; ; i++) {
			auto tab = curarr.getTable(i);
			if (!tab) break;
			if (i) cout << ",";
			print(*tab);
		}
		cout << "]";
		return;
	}
	
	cout << "{\"type\":\"array\",\"value\":[";
	switch (curarr.kind()) {

	case 'v':
		for (int i = 0; ; i++) {
			auto v = curarr.getValue(i);
			if (!v) break;
			if (i) cout << ",";
			print(*v);
		}
		break;

	case 'a': 
		for (int i = 0; ; i++) {
			auto a = curarr.getArray(i);
			if (!a) break;
			if (i) cout << ",";
			print(*a);
		}
		break;

	default:
		break;
	}
	cout << "]}";
}



static void cat(std::istream& stream)
{
	std::string str(std::istreambuf_iterator<char>{stream}, {});
	/*
	cout << "---------------\n";
	cout << str << "\n";
	cout << "---------------\n";
	*/
	auto result = toml::parse(str);
	if (!result.table) {
		cerr << "ERROR: " << result.errmsg << '\n';
		exit(1);
	}

	print(*result.table);
	cout << '\n';
}


int main(int argc, const char* argv[])
{
	int i;
	if (argc == 1) {
		cat(std::cin);
	} else {
		for (i = 1; i < argc; i++) {
			std::ifstream stream(argv[i]);
			if (!stream) {
				cerr << "ERROR: cannot open " << argv[i] << ":" << strerror(errno) << "\n";
				exit(1);
			}
			cat(stream);
		}
	}
	return 0;
}
