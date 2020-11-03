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
using std::pair;

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

static void print(const string& s)
{
	cout << "{\"type\":\"string\",\"value\":\"";
	print_escape_string(s);
	cout << "\"}";
	return;
}

static void print(bool b)
{
	cout << "{\"type\":\"bool\",\"value\":\"" << b << "\"}";
}

static void print(int64_t i)
{
	cout << "{\"type\":\"integer\",\"value\":\"" << i << "\"}";
}

static void print(double d)
{
	cout << "{\"type\":\"float\",\"value\":\"" << d << "\"}";
}

static void print(const toml::Timestamp& t)
{
	if (t.year != -1 && t.hour != -1) {
		cout << "{\"type\":\"datetime\",\"value\":\"";
		cout << z4(t.year);
		cout << "-" << z2(t.month);
		cout << "-" << z2(t.day);
		cout << "T" << z2(t.hour);
		cout << ":" << z2(t.minute);
		cout << ":" << z2(t.second);
		if (t.millisec != -1) 
			cout << "." << t.millisec;
		cout << t.z << "\"}";
		return;
	}
	
	if (t.year != -1) {
		cout << "{\"type\":\"date\",\"value\":\"";
		cout << z4(t.year);
		cout << "-" << z2(t.month);
		cout << "-" << z2(t.day);
		cout << "\"}";
		return;
	}
	
	if (t.hour != -1) {
		cout << "{\"type\":\"time\",\"value\":\"";
		cout << z2(t.hour);
		cout << ":" << z2(t.minute);
		cout << ":" << z2(t.second);
		if (t.millisec != -1) 
			cout << "." << t.millisec;
		cout << "\"}";
		return;
	}
}

static bool print(pair<bool, string> v)
{
	if (!v.first) return 0;
	print(v.second);
	return true;
}

static bool print(pair<bool, bool> v)
{
	if (!v.first) return 0;
	print(v.second);
	return true;
}

static bool print(pair<bool, int64_t> v)
{
	if (!v.first) return 0;
	print(v.second);
	return true;
}

static bool print(pair<bool, double> v)
{
	if (!v.first) return 0;
	print(v.second);
	return true;
}

static bool print(pair<bool, toml::Timestamp> v)
{
	if (!v.first) return 0;
	print(v.second);
	return true;
}


static void print(const toml::Array& arr);
static void print(const toml::Table& curtab)
{
	bool first = true;
	cout << "{";
	for (auto& key : curtab.keys()) {
		cout << (first ? "" : ",");
		first = false;
		
		cout << "\"";
		print_escape_string(key);
		cout << "\":";
		
		/*
		auto v = curtab.getValue(key);
		if (v) {
			print(*v);
			continue;
		}
		*/

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

		if (print(curtab.getString(key)))
			continue;

		if (print(curtab.getInt(key)))
			continue;

		if (print(curtab.getBool(key)))
			continue;

		if (print(curtab.getDouble(key)))
			continue;

		if (print(curtab.getTimestamp(key)))
			continue;
		
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


	if (curarr.kind() == 'a') {
		cout << "{\"type\":\"array\",\"value\":[";
		for (int i = 0; ; i++) {
			auto a = curarr.getArray(i);
			if (!a) break;
			if (i) cout << ",";
			print(*a);
		}
		cout << "]}";
		return;
	}

	
	cout << "{\"type\":\"array\",\"value\":[";
	{
		auto v = curarr.getStringVector();
		if (v) {
			bool first = 1;
			for (const auto& s : *v) {
				cout << (first ? "" : ",");
				print(s);
				first = 0;
			}
			cout << "]}";
			return;
		}
	}

	{
		auto v = curarr.getIntVector();
		if (v) {
			bool first = 1;
			for (auto s : *v) {
				cout << (first ? "" : ",");
				print(s);
				first = 0;
			}
			cout << "]}";
			return;
		}
	}

	{
		auto v = curarr.getBoolVector();
		if (v) {
			bool first = 1;
			for (const auto&& s : *v) {
				cout << (first ? "" : ",");
				print(s);
				first = 0;
			}
			cout << "]}";
			return;
		}
	}

		
	{
		auto v = curarr.getDoubleVector();
		if (v) {
			bool first = 1;
			for (auto s : *v) {
				cout << (first ? "" : ",");
				print(s);
				first = 0;
			}
			cout << "]}";
			return;
		}
	}

	{
		auto v = curarr.getTimestampVector();
		if (v) {
			bool first = 1;
			for (const auto& s : *v) {
				cout << (first ? "" : ",");
				print(s);
				first = 0;
			}
			cout << "]}";
			return;
		}
	}

	abort();
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
