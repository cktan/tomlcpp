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
#ifndef TOML_HPP
#define TOML_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

struct toml_table_t;
struct toml_array_t;

namespace toml {

struct Backing;
class Array;
class Table;
using std::pair;
using std::string;
using std::vector;

/* A Timestamp value */
struct Timestamp {
  // -1 means it is not valid
  int year = -1;
  int month = -1;
  int day = -1;
  int hour = -1;
  int minute = -1;
  int second = -1;
  int millisec = -1;
  string z; // "" if no timezone
};

/* A table in toml. You can extract value/table/array using a key. */
class Table {
public:
  vector<string> keys() const;

  // get content
  pair<bool, string> getString(const string &key) const;
  pair<bool, bool> getBool(const string &key) const;
  pair<bool, int64_t> getInt(const string &key) const;
  pair<bool, double> getDouble(const string &key) const;
  pair<bool, Timestamp> getTimestamp(const string &key) const;
  std::unique_ptr<Table> getTable(const string &key) const;
  std::unique_ptr<Array> getArray(const string &key) const;

  // internal
  Table(toml_table_t *t, std::shared_ptr<Backing> backing)
      : m_table(t), m_backing(backing) {}

private:
  toml_table_t *const m_table = 0;
  std::shared_ptr<Backing> m_backing;

  Table() = delete;
};

/* An array in toml. You can extract value/table/array using an index. */
class Array {
public:
  // Content kind
  // t:table, a:array, v:value, m:mixed
  char kind() const;

  // For Value kind only, check the type of the value
  // i:int, d:double, b:bool, s:string, t:time, D: date, T:timestamp, m:mixed,
  // 0:unknown
  char type() const;

  // Return the #elements in the array
  int size() const;

  // You may have to use these methods for arrays with mixed values
  pair<bool, string> getString(int idx) const;
  pair<bool, bool> getBool(int idx) const;
  pair<bool, int64_t> getInt(int idx) const;
  pair<bool, double> getDouble(int idx) const;
  pair<bool, Timestamp> getTimestamp(int idx) const;

  std::unique_ptr<Table> getTable(int idx) const;
  std::unique_ptr<Array> getArray(int idx) const;

  // Use these methods only if you know the array has no mixed values!
  // For values, some conveniet methods to obtain vector
  std::unique_ptr<vector<string>> getStringVector() const;
  std::unique_ptr<vector<bool>> getBoolVector() const;
  std::unique_ptr<vector<int64_t>> getIntVector() const;
  std::unique_ptr<vector<double>> getDoubleVector() const;
  std::unique_ptr<vector<Timestamp>> getTimestampVector() const;

  // Obtain vectors of table or array
  std::unique_ptr<vector<Table>> getTableVector() const;
  std::unique_ptr<vector<Array>> getArrayVector() const;

  // internal
  Array(toml_array_t *a, std::shared_ptr<Backing> backing)
      : m_array(a), m_backing(backing) {}

private:
  toml_array_t *const m_array = 0;
  std::shared_ptr<Backing> m_backing;

  Array() = delete;
};

/* The main function: Parse */
struct Result {
  std::shared_ptr<Table> table;
  string errmsg;
};

Result parse(const string &conf);
Result parseFile(const string &path);
}; // namespace toml

#endif /* TOML_HPP */
