# tomlcpp
TOML C++ Library

This is a C++ wrapper around the C library available here: https://github.com/cktan/tomlc99.

## Usage

Here is a simple example that parses this config file:

```
[server]
	host = "example.com"
	port = 80
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

// examine the values
cout << "server.host is " << host.second << "\n";
cout << "server.port is " << port.second << "\n";


```
