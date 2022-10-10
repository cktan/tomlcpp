# tomlcpp
TOML in C++; v1.0 compliant.

This is a C++ wrapper around the C library available here: https://github.com/cktan/tomlc99.

* Compatible with [TOML v1.0.0](https://toml.io/en/v1.0.0).
* Tested with multiple test suites, including
[BurntSushi/toml-test](https://github.com/BurntSushi/toml-test) and
[iarna/toml-spec-tests](https://github.com/iarna/toml-spec-tests).
* Does not throw C++ exceptions.
* Provides very simple and intuitive interface.


## Usage

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

```c++
#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "tomlcpp.hpp"

using std::cerr;
using std::cout;

void error(std::string msg)
{
    cerr << "ERROR: " << msg << "\n";
    exit(1);
}

int main()
{
    // 1. parse file
    auto res = toml::parseFile("sample.toml");
    if (!res.table) {
        error("cannot parse file: " + res.errmsg);
    }

    // 2. get top level table
    auto server = res.table->getTable("server");
    if (!server) {
        error("missing [server]");
    }

    // 3. extract values from the top level table
    auto [ ok, host ] = server->getString("host");
    if (!ok) {
        fatal("missing or bad host entry");
    }

    auto portArray = server->getArray("port");
    if (!portArray) {
        fatal("missing 'port' array");
    }

    // 4. examine the values
    cout << "host: " << host << "\n";
    cout << "port: ";
    for (int i = 0; ; i++) {
        auto p = portArray->getInt(i);
        if (!p.first) break;

        cout << p.second << " ";
    }
    cout << "\n";

    return 0;
}
```

### Parsing

To parse a toml text or file, invoke `toml::parse(text)` or `toml::parseFile(path)`.
The return value is a `Result` struct. On success, the `Result.table` will have a non-NULL
pointer to the toml table content. Otherwise, the `Result.table` will be NULL, and `Result.errmsg`
stores a string describing the error.

### Traversing table

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

To access table or array in a Table, use these methods which return a `unique_ptr` to a Table or Array:

```
Table::getTable(key)
Table::getArray(key)
```

### Traversing array

Similarly, to extract the primitive content of a toml::Array, call one of these methods:

```
Array::getString(idx)
Array::getBool(idx)
Array::getInt(idx)
Array::getDouble(idx)
Array::getTimestamp(idx)
Array::getArray(idx)
Array::getTable(idx)
```


## Building and installing

### Make
A normal *make* suffices. You can also simply include the
`toml.c`, `toml.h`, `tomlcpp.cpp`, `tomlcpp.hpp` files in your project.

Invoking `make install` will install the header and library files into
/usr/local/{include,lib}.

Alternatively, specify `make install prefix=/a/file/path` to install into
/a/file/path/{include,lib}.

### Meson
`meson` is a modern build tool that a separate build tree and has cleaner directives for dependency declaration (amongst many other features). To build with `meson`, you will need to have both `meson` and `ninja` installed.

You will then specify a build tree where the output goes, as the command line option to meson

```
$ meson ./build
The Meson build system
Version: 0.63.3
Source dir: /code/tomlcpp
Build dir: /code/tomlcpp/build
Build type: native build
Project name: tomlcpp
Project version: undefined
C compiler for the host machine: cc (clang 11.0.0 "Apple clang version 11.0.0 (clang-1100.0.33.17)")
C linker for the host machine: cc ld64 530
C++ compiler for the host machine: c++ (clang 11.0.0 "Apple clang version 11.0.0 (clang-1100.0.33.17)")
C++ linker for the host machine: c++ ld64 530
Host machine cpu family: x86_64
Host machine cpu: x86_64
Build targets in project: 3

Found ninja-1.10.2 at /usr/local/bin/ninja
$ cd build
$ ninja
[1/8] Compiling C object libtomlcpp.dylib.p/toml.c.o
[2/8] Compiling C++ object toml_sample.p/toml_sample.cpp.o
[3/8] Compiling C++ object toml_json.p/toml_json.cpp.o
[4/8] Compiling C++ object libtomlcpp.dylib.p/tomlcpp.cpp.o
[5/8] Linking target libtomlcpp.dylib
[6/8] Generating symbol file libtomlcpp.dylib.p/libtomlcpp.dylib.symbols
[7/8] Linking target toml_sample
[8/8] Linking target toml_json
```
