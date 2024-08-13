# libeconf

**libeconf** is a highly flexible and configurable library to parse and
manage key=value configuration files.
It reads configuration file snippets from different directories and builds
the final configuration file for the application from it.

The first file is the vendor provided configuration file places in /usr/_vendordir_/_project_.
Optionally, /run/_project_ is also supported for ephemeral overrides.

*Defining _project_ sub directory is optional.*

There are two methods of overriding this vendor settings: Copy the file from
/usr/_vendordir_/_project_ to /etc/_project_ and modify the settings (see *Example 1*).

Alternatively, a directory named _file_._suffix_.d/ within /etc/_project_ can be created,
with drop-in files in the form _name_._suffix_ (see *Example 2*).

These files contain only the changes of the specific settings the user is
interested in.
There can be several such drop-in files, they are processed in
lexicographic order of their filename.

The first method is useful to override the complete configuration file with an
own one, the vendor supplied configuration is ignored.

So, if /etc/_project_/_example_._suffix_ exists, /usr/_vendor_/_project_/_example_._suffix_
and /run/_project_/_example_._suffix_ will not be read.
The disadvantage is, that changes of the vendor configuration file, due e.g.
an package update, are ignored and the user has to manually merge them.

The other method will continue to use /usr/_vendor_/_project_/_example_._suffix_ as base
configuration file and merge all changes from /etc/_project_/_example_._suffix_.d/*._suffix_.
So the user will automatically get improvements of the vendor, with the drawback,
that they could be incompatible with the user made changes.

If there is a file with the same name in /usr/_vendor_/_project_/_example_._suffix_.d/ and
in /etc/_project_/_example_._suffix_.d/*._suffix_., the file in /usr/_project_/_vendor_/_example_._suffix_.d/
will completely ignored.

To disable a configuration file supplied by the vendor, the recommended way is to place
a symlink to /dev/null in the configuration directory in /etc/_project_/, with the same filename
as the vendor configuration file.

Optionally, schemes with only drop-ins and without a ‘main’ configuration file will be supported too. In such
schemes many drop-ins are loaded from a common directory in each hierarchy.
For example, /usr/lib/_project_.d/*, /run/_project_.d/* and /etc/_project_.d/c.conf are all loaded and parsed
in this scheme.

**Example 1**

If a /etc/_example_._suffix_ file exists:

* /etc/_example_._suffix_
* /usr/_vendor_/_project_/_example_._suffix_.d/*._suffix_
* /run/_project_/_example_._suffix_.d/*._suffix_
* /etc/_project_/_example_._suffix_.d/*._suffix_

**Example 2**

The list of files and directories read if **no** /etc/_example_._suffix_ file
exists:

* /usr/_vendor_/_project_/_example_._suffix_ if no /run/_project_/_example_._suffix_ exist
* /usr/_vendor_/_project_/_example_._suffix_.d/*._suffix_
* /run/_project_/_example_._suffix_.d/*._suffix_
* /etc/_project_/_example_._suffix_.d/*._suffix_

The libeconf library fulfills all requirements defined by the **Linux Userspace API (UAPI) Group**
chapter "Configuration Files Specification".
See: :https://uapi-group.org/specifications/specs/configuration_files_specification/

## API

The API is written in plain C. The description can be found here: https://opensuse.github.io/libeconf/

## Bindings for other languages

- [Python](https://github.com/openSUSE/libeconf/blob/v0.7.0/bindings/python3/) ([Documentation](https://github.com/openSUSE/libeconf/blob/v0.6.0/bindings/python3/docs/python-libeconf.3)
- [C#](https://github.com/openSUSE/libeconf/blob/v0.7.0/bindings/csharp/) ([Documentation](https://github.com/openSUSE/libeconf/blob/v0.6.0/bindings/csharp/docs/README.md))
