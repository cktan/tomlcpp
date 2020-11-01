# tomlcpp
TOML CPP Library

This is a C++ wrapper around the C library available here: https://github.com/cktan/tomlc99.

## Usage

Here is a simple example that parses this config file:

```
[server]
	host = "example.com"
	port = 80
```

Steps for getting values:

1. Read the file into a string
2. Call toml::parse on the string
3. Get the top-level table
4. Get values from the top-level table
5. Examine the values

```
// parse a string containing toml data
auto result = toml::parse(str);
if (!result.table) {
    handle_error(result.errmsg);
}

// get the top level table
auto server = result.table.getTable("server");
if (!server) {
    handle_error("missing table [server]");
}

// get value from the table
auto hostval = server->getValue("host");
auto host = hostval ? hostval->toString() : pair<bool, string>();
if (!host.first) {
    handle_error("missing or bad host entry");
}
auto portval = server.getValue("port");
auto port = portval ? portval->toInt() : pair<bool, int64_t>();
if (!port.first) {
   handle_error("missing or bad port entry");
}

// examine the values
cout << "server.host is " << host.second << "\n";
cout << "server.port is " << port.second << "\n";


```