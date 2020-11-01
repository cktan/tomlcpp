/*
  MIT License
  
  Copyright (c) 2017 - 2019 CK Tan
  https://github.com/cktan/tomlc99
  
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
	
	class Value;
	class Array;
	class Table;

	struct Timestamp {
		// -1 means it is not valid
		int year, month, day, hour, minute, second, millisec;
		std::string z;
	};

	class Value {
		public:
		Value(const char* raw) : m_raw(raw) {}

		std::pair<bool, std::string> toString() const;
		std::pair<bool, bool> toBool() const;
		std::pair<bool, int64_t> toInt() const;
		std::pair<bool, double> toDouble() const;
		std::pair<bool, Timestamp> toTimestamp() const;

		// do not use
		void __set_root(std::shared_ptr<Table> root) { m_root = root; }
		
		protected:
		std::shared_ptr<Table> m_root;
		const char* m_raw = 0;
	};


	class Table {
		public:
		Table() = delete;
		Table(toml_table_t* t) : m_table(t) {}
		Table(Table&) = delete;
		Table& operator=(Table&) = delete;
		~Table();

		std::vector<std::string> keys() const;

		std::unique_ptr<Value> getValue(const std::string& key) const;
		std::unique_ptr<Table> getTable(const std::string& key) const;
		std::unique_ptr<Array> getArray(const std::string& key) const;

		// do not use
		void __set_root(std::shared_ptr<Table> root) { m_root = root; }
		void __set_backing(char* s) { m_backing = s; }
		
		private:
		std::shared_ptr<Table> m_root;
		toml_table_t* const m_table = 0;
		char* m_backing = 0;		// this will only be set in the root Table
	};


	class Array {
		public:
		Array(toml_array_t* a) : m_array(a) {}
		Array() = delete;
		Array(Array&) = delete;
		Array& operator=(Array&) = delete;
		~Array() {}

		// Content kind
		// t:table, a:array, v:value
		char kind() const;

		// For Value kind only, check the type of the value
		// i:int, d:double, b:bool, s:string, t:time, D: date, T:timestamp, 0:unknown
		char type() const;
		
		std::unique_ptr<Value> getValue(int idx) const;
		std::unique_ptr<Table> getTable(int idx) const;
		std::unique_ptr<Array> getArray(int idx) const;

		// do not use
		void __set_root(std::shared_ptr<Table> root) { m_root = root; }
		
		private:
		std::shared_ptr<Table> m_root;
		toml_array_t* const m_array = 0;
	};


	struct ParserResult {
		std::shared_ptr<Table> table;
		std::string errmsg;
	};

	ParserResult parse(const std::string& conf);
};


#endif /* TOML_HPP */
