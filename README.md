# tomlcpp
TOML in C++; v1.0 compliant.

This is a C++ wrapper around the C library available here: https://github.com/cktan/tomlc99.

## Usage

First, include the necessary header file:

```
#include "tomlcpp.hpp"
```

Here is a simple example that parses this config file:

```
[server]
	host = "example.com"
	port = [ 8080, 8181, 8282 ]
```

Steps for getting values:

1. Call toml::parseFile on a toml file
2. Get the top-level table
3. Get values from the top-level table
4. Examine the values

```
// 1. parse a file containing toml data
auto result = toml::parseFile("sample.toml");
if (!result.table) {
    handle_error(result.errmsg);
}

// 2. get the top level table
auto server = result.table.getTable("server");
if (!server) {
    handle_error("missing table [server]");
}

// 3. get values from the table
auto host = server->getString("host");
if (!host.first) {
    handle_error("missing or bad host entry");
}
auto portArray = server->getArray("port");
if (!portArray) {
   handle_error("missing or bad port entry");
}
auto port = portArray->getIntVector();
if (!port) {
    handle_error("bad port entry");
}

// 4. examine the values
cout << "server.host is " << host.second << "\n";
cout << "server.port is [";
for (auto& p : *port) {
    cout << p << " ";
}
cout << "]\n";
```

### Parsing

To parse a toml text or file, invoke `toml::parse(text)` or `toml::parseFile(path)`. 
The return value is a `Result` struct. On success, the `Result.table` will have a non-NULL 
pointer to the toml table content. Otherwise, the `Result.table` will be NULL, and `Result.errmsg` 
stores a string describing the error.

### Traversing Table

Toml tables are key-value maps. 

#### Keys

The method `Table::keys()` returns a vector of keys.

#### Content

To extract values in a Table, call the `Table::getXXXX(key)` methods and supply the key:

```
Table::getString(key)
Table::getBool(key)
Table::getInt(key)
Table::getDouble(key)
Table::getTimestamp(key)
```

These methods return a C++ `pair`, in which `pair.first` is a success indicator, and `pair.second` is the result value.

To access table or array in a Table, use these methods which return a `unique_ptr` to an Array or Table:

```
Table::getTable(key)
Table::getArray(key)
```

### Traversing Array

To extract the primitive content of a toml::Array, call one of these methods:

```
Array::getStringVector()
Array::getBoolVector()
Array::getIntVector()
Array::getDoubleVector()
Array::getTimestampVector()
Array::getArrayVector()
Array::getTableVector()
```

These methods return `unique_ptr` to a C++ `vector`.

## Building and installing

A normal *make* suffices. You can also simply include the
`toml.c`, `toml.h`, `tomlcpp.cpp`, `tomlcpp.hpp` files in your project.

Invoking `make install` will install the header file in
/usr/local/include and library files in /usr/local/lib.

Alternatively, specify `make install prefix=/a/file/path` to install into
/a/file/path/include and /a/file/path/lib/.
