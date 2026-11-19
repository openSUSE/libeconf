# `python-libeconf`
`python-libeconf` is a Python library which offers Python bindings for `libeconf`. It is memory safe and uses `ctypes` to talk to the native library. The core library has no external dependencies besides the native binary for `libeconf`.

libeconf is a highly flexible and configurable library to parse and manage key=value configuration files. It reads configuration file snippets from different directories and builds the final configuration file for the application from it.

# Usage
All functions live in the `econf` module and are named the same as their C counterpart, e.g. the C function `econf_readFile` becomes `econf.read_file` in Python. Functions which work on a parsed configuration take an `EconfFile` object, which is the Python equivalent of the C `econf_file` struct and is freed automatically at the end of its lifetime.

You can find more information about each function [here](python-libeconf.3).

## Install
> **WARNING:** The recommended way to install this project is to get it via your distro package manager.

You can install this project with pip too:
```shell
cd bindings/python3
pip install .
```
and then import it into your python project with
```python
import econf
```

## Example
```python
>>> import econf
>>> file = econf.read_file("examples/etc/example/example.conf", "=", "#")
>>> econf.get_groups(file)
['Another Group', 'First Group', 'Group']
>>> econf.get_keys(file, "Group")
['Bla', 'Welcome[la]', 'Welcome']
>>> econf.get_int_value(file, "Group", "Bla")
12311
>>> econf.set_int_value(file, "Group", "Bla", 5)
>>> econf.get_int_value(file, "Group", "Bla")
5
>>> econf.write_file(file, "examples/etc/example", "example_new.conf")
>>>
```

# Building from Source
To build from source, open up the `bindings/python3` folder and run `pip install .`. You will also need to build `libeconf` so the native shared library (`libeconf.so`) is available to be loaded by `ctypes`.
