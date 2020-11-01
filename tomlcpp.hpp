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

struct toml_table_t;
struct toml_array_t;

namespace toml {

	struct Backing;
	class Value;
	class Array;
	class Table;

	/* A Timestamp value */
	struct Timestamp {
		// -1 means it is not valid
		int year, month, day, hour, minute, second, millisec;
		std::string z;			// "" if no timezone
	};

	/* A value in toml. Can be string/bool/int/double or timestamp. */
	class Value {
		public:
		// extract string, bool, etc from Value.
		std::pair<bool, std::string> toString() const;
		std::pair<bool, bool> toBool() const;
		std::pair<bool, int64_t> toInt() const;
		std::pair<bool, double> toDouble() const;
		std::pair<bool, Timestamp> toTimestamp() const;

		Value(const char* raw, std::shared_ptr<Backing> backing)
			: m_raw(raw), m_backing(backing) {}

		private:
		const char* m_raw;
		std::shared_ptr<Backing> m_backing;
		
		Value() = delete;
		Value(Value&) = delete;
		Value& operator=(Value&) = delete;
	};


	/* A table in toml. You can extract value/table/array using a key. */
	class Table {
		public:
		std::vector<std::string> keys() const;

		std::unique_ptr<Value> getValue(const std::string& key) const;
		std::unique_ptr<Table> getTable(const std::string& key) const;
		std::unique_ptr<Array> getArray(const std::string& key) const;

		Table(toml_table_t* t, std::shared_ptr<Backing> backing) : m_table(t), m_backing(backing) {}
		
		private:
		toml_table_t* const m_table = 0;
		std::shared_ptr<Backing> m_backing;
		
		Table() = delete;
		Table(Table&) = delete;
		Table& operator=(Table&) = delete;
	};


	/* An array in toml. You can extract value/table/array using an index. */
	class Array {
		public:

		// Content kind
		// t:table, a:array, v:value
		char kind() const;

		// For Value kind only, check the type of the value
		// i:int, d:double, b:bool, s:string, t:time, D: date, T:timestamp, 0:unknown
		char type() const;
		
		std::unique_ptr<Value> getValue(int idx) const;
		std::unique_ptr<Table> getTable(int idx) const;
		std::unique_ptr<Array> getArray(int idx) const;

		Array(toml_array_t* a, std::shared_ptr<Backing> backing) : m_array(a), m_backing(backing) {}
		
		private:
		toml_array_t* const m_array = 0;
		std::shared_ptr<Backing> m_backing;
		
		Array() = delete;
		Array(Array&) = delete;
		Array& operator=(Array&) = delete;
	};


	struct ParserResult {
		std::shared_ptr<Table> table;
		std::string errmsg;
	};

	ParserResult parse(const std::string& conf);
};


#endif /* TOML_HPP */
