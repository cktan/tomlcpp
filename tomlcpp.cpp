#include <memory>
#include <vector>
#include "tomlcpp.hpp"
#include "toml.h"
#include <string.h>

using namespace toml;
using std::string;

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
	char* s;
	if (toml_rtos(m_raw, &s))
		return std::make_pair(false, "");

	string ss(s);
	free(s);
	return std::make_pair(true, ss);
}

std::pair<bool, bool> Value::toBool() const
{
	int b;
	bool ok = (0 == toml_rtob(m_raw, &b));
	return std::make_pair(ok, !!b);
}

std::pair<bool, int64_t> Value::toInt() const
{
	int64_t i;
	bool ok = (0 == toml_rtoi(m_raw, &i));
	return std::make_pair(ok, i);
}

std::pair<bool, double> Value::toDouble() const
{
	double d;
	bool ok = (0 == toml_rtod(m_raw, &d));
	return std::make_pair(ok, d);
}


std::pair<bool, Timestamp> Value::toTimestamp() const
{
	Timestamp ret;
	toml_timestamp_t ts;
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
	return std::make_pair(ok, ret);
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

std::vector<string> Table::keys() const
{
	std::vector<string> vec;
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
