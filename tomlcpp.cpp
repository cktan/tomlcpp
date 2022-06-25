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
#include "tomlcpp.hpp"
#include "toml.h"
#include <cstring>
#include <fstream>

using namespace toml;
using std::pair;
using std::string;
using std::vector;

static void *toml_mymalloc(size_t sz) { return new char[sz]; }

static void toml_myfree(void *p) {
  if (p) {
    char *pp = (char *)p;
    delete[] pp;
  }
}

/**
 *  Keep track of memory to be freed when all references
 *  to the tree returned by toml::parse is no longer reachable.
 */
struct toml::Backing {
  char *ptr = 0;
  toml_table_t *root = 0;
  Backing(const string &conf) {
    ptr = new char[conf.length() + 1];
    strcpy(ptr, conf.c_str());
  }
  ~Backing() {
    if (ptr)
      delete[] ptr;
    if (root)
      toml_free(root);
  }
};

pair<bool, string> Table::getString(const string &key) const {
  string str;
  toml_datum_t p = toml_string_in(m_table, key.c_str());
  if (p.ok) {
    str = p.u.s;
    toml_myfree(p.u.s);
  }
  return {p.ok, str};
}

pair<bool, bool> Table::getBool(const string &key) const {
  toml_datum_t p = toml_bool_in(m_table, key.c_str());
  return {p.ok, !!p.u.b};
}

pair<bool, int64_t> Table::getInt(const string &key) const {
  toml_datum_t p = toml_int_in(m_table, key.c_str());
  return {p.ok, p.u.i};
}

pair<bool, double> Table::getDouble(const string &key) const {
  toml_datum_t p = toml_double_in(m_table, key.c_str());
  return {p.ok, p.u.d};
}

pair<bool, Timestamp> Table::getTimestamp(const string &key) const {
  Timestamp ret;
  toml_datum_t p = toml_timestamp_in(m_table, key.c_str());
  if (p.ok) {
    toml_timestamp_t &ts = *p.u.ts;
    ret.year = (ts.year ? *ts.year : -1);
    ret.month = (ts.month ? *ts.month : -1);
    ret.day = (ts.day ? *ts.day : -1);
    ret.hour = (ts.hour ? *ts.hour : -1);
    ret.minute = (ts.minute ? *ts.minute : -1);
    ret.second = (ts.second ? *ts.second : -1);
    ret.millisec = (ts.millisec ? *ts.millisec : -1);
    ret.z = ts.z ? string(ts.z) : "";
    toml_myfree(p.u.ts);
  }
  return {p.ok, ret};
}

std::unique_ptr<Array> Table::getArray(const string &key) const {
  toml_array_t *a = toml_array_in(m_table, key.c_str());
  if (!a)
    return 0;

  auto ret = std::make_unique<Array>(a, m_backing);
  return ret;
}

std::unique_ptr<Table> Table::getTable(const string &key) const {
  toml_table_t *t = toml_table_in(m_table, key.c_str());
  if (!t)
    return 0;

  auto ret = std::make_unique<Table>(t, m_backing);
  return ret;
}

vector<string> Table::keys() const {
  vector<string> vec;
  for (int i = 0;; i++) {
    const char *k = toml_key_in(m_table, i);
    if (!k)
      break;
    vec.push_back(k);
  }
  return vec;
}

char Array::kind() const { return toml_array_kind(m_array); }

char Array::type() const { return toml_array_type(m_array); }

pair<bool, string> Array::getString(int idx) const {
  string str;
  toml_datum_t p = toml_string_at(m_array, idx);
  if (p.ok) {
    str = p.u.s;
    toml_myfree(p.u.s);
  }
  return {p.ok, str};
}

pair<bool, bool> Array::getBool(int idx) const {
  toml_datum_t p = toml_bool_at(m_array, idx);
  return {p.ok, !!p.u.b};
}

pair<bool, int64_t> Array::getInt(int idx) const {
  toml_datum_t p = toml_int_at(m_array, idx);
  return {p.ok, p.u.i};
}

pair<bool, double> Array::getDouble(int idx) const {
  toml_datum_t p = toml_double_at(m_array, idx);
  return {p.ok, p.u.d};
}

pair<bool, Timestamp> Array::getTimestamp(int idx) const {
  Timestamp ret;
  toml_datum_t p = toml_timestamp_at(m_array, idx);
  if (p.ok) {
    toml_timestamp_t &ts = *p.u.ts;
    ret.year = (ts.year ? *ts.year : -1);
    ret.month = (ts.month ? *ts.month : -1);
    ret.day = (ts.day ? *ts.day : -1);
    ret.hour = (ts.hour ? *ts.hour : -1);
    ret.second = (ts.second ? *ts.second : -1);
    ret.millisec = (ts.millisec ? *ts.millisec : -1);
    ret.z = ts.z ? string(ts.z) : "";
    toml_myfree(p.u.ts);
  }
  return {p.ok, ret};
}

std::unique_ptr<Array> Array::getArray(int idx) const {
  toml_array_t *a = toml_array_at(m_array, idx);
  if (!a)
    return 0;

  auto ret = std::make_unique<Array>(a, m_backing);
  return ret;
}

std::unique_ptr<Table> Array::getTable(int idx) const {
  toml_table_t *t = toml_table_at(m_array, idx);
  if (!t)
    return 0;

  auto ret = std::make_unique<Table>(t, m_backing);
  return ret;
}

std::unique_ptr<vector<Array>> Array::getArrayVector() const {
  int top = toml_array_nelem(m_array);
  if (top < 0)
    return 0;

  auto ret = std::make_unique<vector<Array>>();
  ret->reserve(top);
  for (int i = 0; i < top; i++) {
    toml_array_t *a = toml_array_at(m_array, i);
    if (!a)
      return 0;

    ret->push_back(Array(a, m_backing));
  }

  return ret;
}

std::unique_ptr<vector<Table>> Array::getTableVector() const {
  int top = toml_array_nelem(m_array);
  if (top < 0)
    return 0;

  auto ret = std::make_unique<vector<Table>>();
  ret->reserve(top);
  for (int i = 0; i < top; i++) {
    toml_table_t *t = toml_table_at(m_array, i);
    if (!t)
      return 0;

    ret->push_back(Table(t, m_backing));
  }

  return ret;
}

std::unique_ptr<vector<string>> Array::getStringVector() const {
  int top = toml_array_nelem(m_array);
  if (top < 0)
    return 0;

  auto ret = std::make_unique<vector<string>>();
  ret->reserve(top);
  for (int i = 0; i < top; i++) {
    toml_datum_t p = toml_string_at(m_array, i);
    if (!p.ok)
      return 0;
    ret->push_back(p.u.s);
    toml_myfree(p.u.s);
  }

  return ret;
}

std::unique_ptr<vector<bool>> Array::getBoolVector() const {
  int top = toml_array_nelem(m_array);
  if (top < 0)
    return 0;

  auto ret = std::make_unique<vector<bool>>();
  ret->reserve(top);
  for (int i = 0; i < top; i++) {
    toml_datum_t p = toml_bool_at(m_array, i);
    if (!p.ok)
      return 0;
    ret->push_back(!!p.u.b);
  }

  return ret;
}

std::unique_ptr<vector<int64_t>> Array::getIntVector() const {
  int top = toml_array_nelem(m_array);
  if (top < 0)
    return 0;

  auto ret = std::make_unique<vector<int64_t>>();
  ret->reserve(top);
  for (int i = 0; i < top; i++) {
    toml_datum_t p = toml_int_at(m_array, i);
    if (!p.ok)
      return 0;
    ret->push_back(p.u.i);
  }

  return ret;
}

std::unique_ptr<vector<Timestamp>> Array::getTimestampVector() const {
  int top = toml_array_nelem(m_array);
  if (top < 0)
    return 0;

  auto ret = std::make_unique<vector<Timestamp>>();
  ret->reserve(top);
  for (int i = 0; i < top; i++) {
    toml_datum_t p = toml_timestamp_at(m_array, i);
    if (!p.ok)
      return 0;

    toml_timestamp_t &ts = *p.u.ts;
    Timestamp v;
    v.year = (ts.year ? *ts.year : -1);
    v.month = (ts.month ? *ts.month : -1);
    v.day = (ts.day ? *ts.day : -1);
    v.hour = (ts.hour ? *ts.hour : -1);
    v.second = (ts.second ? *ts.second : -1);
    v.millisec = (ts.millisec ? *ts.millisec : -1);
    v.z = ts.z ? string(ts.z) : "";
    toml_myfree(p.u.ts);

    ret->push_back(v);
  }

  return ret;
}

std::unique_ptr<vector<double>> Array::getDoubleVector() const {
  int top = toml_array_nelem(m_array);
  if (top < 0)
    return 0;

  auto ret = std::make_unique<vector<double>>();
  ret->reserve(top);
  for (int i = 0; i < top; i++) {
    toml_datum_t p = toml_double_at(m_array, i);
    if (!p.ok)
      return 0;
    ret->push_back(p.u.d);
  }

  return ret;
}

int toml::Array::size() const { return toml_array_nelem(m_array); }

toml::Result toml::parse(const string &conf) {
  toml::Result ret;
  char errbuf[200];
  auto backing = std::make_shared<Backing>(conf);

  toml_set_memutil(toml_mymalloc, toml_myfree);
  toml_table_t *t = toml_parse(backing->ptr, errbuf, sizeof(errbuf));
  if (t) {
    ret.table = std::make_shared<Table>(t, backing);
    backing->root = t;
  } else {
    ret.errmsg = (*errbuf) ? string(errbuf) : "unknown error";
  }
  return ret;
}

toml::Result toml::parseFile(const string &path) {
  toml::Result ret;
  std::ifstream stream(path);
  if (!stream) {
    ret.errmsg = strerror(errno);
    return ret;
  }
  string conf(std::istreambuf_iterator<char>{stream}, {});
  return toml::parse(conf);
}
