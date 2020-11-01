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
#include <memory>
#include <vector>
#include "tomlcpp.hpp"
#include "toml.h"
#include <string.h>
#include <fstream>

using namespace toml;
using std::string;
using std::vector;

/**
 *  Keep track of memory to be freed when all references
 *  to the tree returned by toml::parse is no longer reachable.
 */
struct toml::Backing {
	char* ptr;
	toml_table_t* root;
	Backing(char* p) : ptr(p) {}
	~Backing() {
		free(ptr);
		if (root) toml_free(root);
	}
};

std::pair<bool, string> Value::toString() const
{
	string str;
	char* s;
	bool ok = (0 == toml_rtos(m_raw, &s));
	if (ok) {
		str = s;
		free(s);
	}
	return {ok, str};
}

std::pair<bool, bool> Value::toBool() const
{
	int b;
	bool ok = (0 == toml_rtob(m_raw, &b));
	return {ok, !!b};
}

std::pair<bool, int64_t> Value::toInt() const
{
	int64_t i;
	bool ok = (0 == toml_rtoi(m_raw, &i));
	return {ok, i};
}

std::pair<bool, double> Value::toDouble() const
{
	double d;
	bool ok = (0 == toml_rtod(m_raw, &d));
	return {ok, d};
}


std::pair<bool, Timestamp> Value::toTimestamp() const
{
	toml_timestamp_t ts;
	Timestamp ret;
	bool ok = (0 == toml_rtots(m_raw, &ts));
	if (ok) {
		ret.year = (ts.year ? *ts.year : -1);
		ret.month = (ts.month ? *ts.month : -1);
		ret.day = (ts.day ? *ts.day : -1);
		ret.hour = (ts.hour ? *ts.hour : -1);
		ret.second = (ts.second ? *ts.second : -1);
		ret.millisec = (ts.millisec ? *ts.millisec : -1);
		ret.z = ts.z ? string(ts.z) : "";
	}
	return {ok, ret};
}

std::pair<bool, std::string> Table::getString(const string& key) const
{
	string str;
	bool ok = 0;
	const char* raw = toml_raw_in(m_table, key.c_str());
	if (raw) {
		char* s;
		ok = (0 == toml_rtos(raw, &s));
		if (ok) {
			str = s;
			free(s);
		}
	}
	return {ok, str};
}

std::pair<bool, bool> Table::getBool(const string& key) const
{
	int b = 0;
	bool ok = 0;
	const char* raw = toml_raw_in(m_table, key.c_str());
	if (raw) {
		ok = (0 == toml_rtob(raw, &b));
	}
	return {ok, !!b};
	
}

std::pair<bool, int64_t> Table::getInt(const string& key) const
{
	int64_t i = 0;
	bool ok = 0;
	const char* raw = toml_raw_in(m_table, key.c_str());
	if (raw) {
		ok = (0 == toml_rtoi(raw, &i));
	}
	return {ok, i};
	
}

std::pair<bool, double> Table::getDouble(const string& key) const
{
	double d = 0;
	bool ok = 0;
	const char* raw = toml_raw_in(m_table, key.c_str());
	if (raw) {
		ok = (0 == toml_rtod(raw, &d));
	}
	return {ok, d};
}

std::pair<bool, Timestamp> Table::getTimestamp(const string& key) const
{
	Timestamp ret;
	bool ok = 0;
	const char* raw = toml_raw_in(m_table, key.c_str());
	if (raw) {
		toml_timestamp_t ts;
		ok = (0 == toml_rtots(raw, &ts));
		if (ok) {
			ret.year = (ts.year ? *ts.year : -1);
			ret.month = (ts.month ? *ts.month : -1);
			ret.day = (ts.day ? *ts.day : -1);
			ret.hour = (ts.hour ? *ts.hour : -1);
			ret.second = (ts.second ? *ts.second : -1);
			ret.millisec = (ts.millisec ? *ts.millisec : -1);
			ret.z = ts.z ? string(ts.z) : "";
		}
	}
	return {ok, ret};
}


std::unique_ptr<Value> Table::getValue(const string& key) const
{
	toml_raw_t r = toml_raw_in(m_table, key.c_str());
	if (!r)
		return 0;
	
	auto ret = std::make_unique<Value>(r, m_backing);
	return ret;
}


std::unique_ptr<Array> Table::getArray(const string& key) const
{
	toml_array_t* a = toml_array_in(m_table, key.c_str());
	if (!a)
		return 0;

	auto ret = std::make_unique<Array>(a, m_backing);
	return ret;
}

std::unique_ptr<Table> Table::getTable(const string& key) const
{
	toml_table_t* t = toml_table_in(m_table, key.c_str());
	if (!t)
		return 0;

	auto ret = std::make_unique<Table>(t, m_backing);
	return ret;
}

vector<string> Table::keys() const
{
	vector<string> vec;
	for (int i = 0; ; i++) {
		const char* k = toml_key_in(m_table, i);
		if (!k) break;
		vec.push_back(k);
	}
	return vec;
}


char Array::kind() const
{
	return toml_array_kind(m_array);
}

char Array::type() const
{
	return toml_array_type(m_array);
}

std::unique_ptr<Value> Array::getValue(int idx) const
{
	toml_raw_t r = toml_raw_at(m_array, idx);
	if (!r)
		return 0;
	
	auto ret = std::make_unique<Value>(r, m_backing);
	return ret;
}

std::unique_ptr<Array> Array::getArray(int idx) const
{
	toml_array_t* a = toml_array_at(m_array, idx);
	if (!a)
		return 0;

	auto ret = std::make_unique<Array>(a, m_backing);
	return ret;
}

std::unique_ptr<Table> Array::getTable(int idx) const
{
	toml_table_t* t = toml_table_at(m_array, idx);
	if (!t)
		return 0;

	auto ret = std::make_unique<Table>(t, m_backing);
	return ret;
}

std::unique_ptr< vector<string> > Array::getStringVector() const
{
	int top = toml_array_nelem(m_array);
	if (top < 0) return 0;
	
	auto ret = std::make_unique< vector<string> >(top);
	for (int i = 0; i < top; i++) {
		toml_raw_t r = toml_raw_at(m_array, i);
		if (!r) return 0;

		char* s;
		if (toml_rtos(r, &s)) return 0;

		string v(s);
		free(s);

		ret->push_back(v);
	}

	return ret;
}


std::unique_ptr< vector<bool> > Array::getBoolVector() const
{
	int top = toml_array_nelem(m_array);
	if (top < 0) return 0;
	
	auto ret = std::make_unique< vector<bool> >(top);
	for (int i = 0; i < top; i++) {
		toml_raw_t r = toml_raw_at(m_array, i);
		if (!r) return 0;

		int v;
		if (toml_rtob(r, &v)) return 0;

		ret->push_back(!!v);
	}

	return ret;
}


std::unique_ptr< vector<int64_t> > Array::getIntVector() const
{
	int top = toml_array_nelem(m_array);
	if (top < 0) return 0;
	
	auto ret = std::make_unique< vector<int64_t> >(top);
	for (int i = 0; i < top; i++) {
		toml_raw_t r = toml_raw_at(m_array, i);
		if (!r) return 0;

		int64_t v;
		if (toml_rtoi(r, &v)) return 0;

		ret->push_back(v);
	}

	return ret;
}


std::unique_ptr< vector<Timestamp> > Array::getTimestampVector() const
{
	int top = toml_array_nelem(m_array);
	if (top < 0) return 0;
	
	auto ret = std::make_unique< vector<Timestamp> >(top);
	for (int i = 0; i < top; i++) {
		toml_raw_t r = toml_raw_at(m_array, i);
		if (!r) return 0;

		toml_timestamp_t ts;
		if (toml_rtots(r, &ts)) return 0;

		Timestamp v;
		v.year = (ts.year ? *ts.year : -1);
		v.month = (ts.month ? *ts.month : -1);
		v.day = (ts.day ? *ts.day : -1);
		v.hour = (ts.hour ? *ts.hour : -1);
		v.second = (ts.second ? *ts.second : -1);
		v.millisec = (ts.millisec ? *ts.millisec : -1);
		v.z = ts.z ? string(ts.z) : "";

		ret->push_back(v);
	}

	return ret;
}


std::unique_ptr< vector<double> > Array::getDoubleVector() const
{
	int top = toml_array_nelem(m_array);
	if (top < 0) return 0;
	
	auto ret = std::make_unique< vector<double> >(top);
	for (int i = 0; i < top; i++) {
		toml_raw_t r = toml_raw_at(m_array, i);
		if (!r) return 0;

		double v;
		if (toml_rtod(r, &v)) return 0;

		ret->push_back(v);
	}

	return ret;
}


int toml::Array::getSize() const
{
	return toml_array_nelem(m_array);
}
	

toml::ParserResult toml::parse(const string& conf)
{
	toml::ParserResult ret;
	char errbuf[200];
	char* s = strdup(conf.c_str());
	if (!s) {
		ret.errmsg = "out of memory";
		return ret;
	}
	auto backing = std::make_shared<Backing>(s);
	
	toml_table_t* t = toml_parse(s, errbuf, sizeof(errbuf));
	if (t) {
		ret.table = std::make_shared<Table>(t, backing);
		backing->root = t;
	} else {
		ret.errmsg = (*errbuf) ? string(errbuf) : "unknown error";
	}
	return ret;
}


toml::ParserResult toml::parseFile(const string& path)
{
	toml::ParserResult ret;
	std::ifstream stream(path);
	if (!stream) {
		ret.errmsg = strerror(errno);
		return ret;
	}
	std::string conf(std::istreambuf_iterator<char>{stream}, {});
	return toml::parse(conf);
}

