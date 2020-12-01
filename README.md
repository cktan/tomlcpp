# tomlcpp
TOML C++ Library

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
	port = 80
	peers = ["alpha", "beta", "gamma"]
```

Steps for getting values:

1. Call toml::parseFile on a toml file
2. Get the top-level table
3. Get values from the top-level table
4. Examine the values

```
// parse a file containing toml data
auto result = toml::parseFile("sample.toml");
if (!result.table) {
    handle_error(result.errmsg);
}

// get the top level table
auto server = result.table.getTable("server");
if (!server) {
    handle_error("missing table [server]");
}

// get value from the table
auto host = server->getString("host");
if (!host.first) {
    handle_error("missing or bad host entry");
}
auto port = server->getInt("port");
if (!port.first) {
   handle_error("missing or bad port entry");
}
auto peers = server->getArray("peers");
if (!peers) {
    handle_error("missing or bad peers entry");
}
auto pv = peers->getStringVector();
if (!pv) {
    handle_error("bad peers entry");
}

// examine the values
cout << "server.host is " << host.second << "\n";
cout << "server.port is " << port.second << "\n";
cout << "server.peers is [";
for (auto& p : *pv) {
    cout << p << " ";
}
cout << "]\n";
```

### Parsing

To parse a toml text or file, simply call `toml::parse(text)` or `toml::parseFile(path)`. 
The return value is a `Result` struct. On success, the `Result.table` will have a non-NULL 
pointer to the toml table content. On failure, the `Result.table` will be NULL, and `Result.errmsg` 
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
