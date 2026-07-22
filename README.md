# libeconf

**libeconf** is a highly flexible and configurable library to parse and
manage key=value configuration files.
It reads configuration file snippets from different directories and builds
the final configuration file for the application from it.

The first file is the vendor provided configuration file places in /usr/\<vendordir\>/\<project\>.
Optionally, /run/\<project\> is also supported for ephemeral overrides.

*Defining \<project\> sub directory is optional.*

There are two methods of overriding this vendor settings: Copy the file from
/usr/\<vendordir\>/\<project\> to /etc/\<project\> and modify the settings (see *Example 1*).

Alternatively, a directory named \<file\>.\<suffix\>.d/ within /etc/\<project\> can be created,
with drop-in files in the form \<name\>.\<suffix\> (see *Example 2*).

These files contain only the changes of the specific settings the user is
interested in.
There can be several such drop-in files, they are processed in
lexicographic order of their filename.

The first method is useful to override the complete configuration file with an
own one, the vendor supplied configuration is ignored.

So, if /etc/\<project\>/\<example\>.\<suffix\> exists, /usr/\<vendor\>/\<project\>/\<example\>.\<suffix\>
and /run/\<project\>/\<example\>.\<suffix\> will not be read.
The disadvantage is, that changes of the vendor configuration file, due e.g.
an package update, are ignored and the user has to manually merge them.

The other method will continue to use /usr/\<vendor\>/\<project\>/\<example\>.\<suffix\> as base
configuration file and merge all changes from /etc/\<project\>/\<example\>.\<suffix\>.d/*.\<suffix\>.
So the user will automatically get improvements of the vendor, with the drawback,
that they could be incompatible with the user made changes.

If there is a file with the same name in /usr/\<vendor\>/\<project\>/\<example\>.\<suffix\>.d/ and
in /etc/\<project\>/\<example\>.\<suffix\>.d/*.\<suffix\>., the file in /usr/\<project\>/\<vendor\>/\<example\>.\<suffix\>.d/
will be completely ignored.

To disable a configuration file supplied by the vendor, the recommended way is to place
a symlink to /dev/null in the configuration directory in /etc/\<project\>/, with the same filename
as the vendor configuration file.

Optionally, schemes with only drop-ins and without a ‘main’ configuration file will be supported too. In such
schemes many drop-ins are loaded from a common directory in each hierarchy.
For example, /usr/lib/\<project\>.d/*, /run/\<project\>.d/* and /etc/\<project\>.d/c.conf are all loaded and parsed
in this scheme.

**Example 1**

If a /etc/\<example\>.\<suffix\> file exists:

* /etc/\<example\>.\<suffix\>
* /usr/\<vendor\>/\<project\>/\<example\>.\<suffix\>.d/*.\<suffix\>
* /run/\<project\>/\<example\>.\<suffix\>.d/*.\<suffix\>
* /etc/\<project\>/\<example\>.\<suffix\>.d/*.\<suffix\>

**Example 2**

The list of files and directories read if **no** /etc/\<example\>.\<suffix\> file
exists:

* /usr/\<vendor\>/\<project\>/\<example\>.\<suffix\> if no /run/\<project\>/\<example\>.\<suffix\> exist
* /usr/_vendor_/\<project\>/\<example\>.\<suffix\>.d/*.\<suffix\>
* /run/\<project\>/\<example\>.\<suffix\>.d/*.\<suffix\>
* /etc/\<project\>/\<example\>.\<suffix\>.d/*.\<suffix\>

The libeconf library fulfills all requirements defined by the **Linux Userspace API (UAPI) Group**
chapter "Configuration Files Specification".
See: :https://uapi-group.org/specifications/specs/configuration_files_specification/

## API

The API is written in plain C. The description can be found here: https://opensuse.github.io/libeconf/

## Bindings for other languages

- [Python](https://github.com/openSUSE/libeconf/blob/v0.7.0/bindings/python3/) ([Documentation](https://github.com/openSUSE/libeconf/blob/v0.6.0/bindings/python3/docs/python-libeconf.3)
- [C#](https://github.com/openSUSE/libeconf/blob/v0.7.0/bindings/csharp/) ([Documentation](https://github.com/openSUSE/libeconf/blob/v0.6.0/bindings/csharp/docs/README.md))
