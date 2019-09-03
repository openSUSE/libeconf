# libeconf

**libeconf** is a highly flexible and configureable library to parse and
manage key=value configuration files.
It reads configuration file snippets from different directories and builds
the final configuration file for the application from it.

The first file is the vendor provided configuration file. There are
two methods of overriding this vendor settings: copy the file from
/usr/_vendordir_ to /etc and modify the settings. Alternatively, a
directory named _file_._suffix_.d/ within /etc can be created, with
drop-in files in the form _name_._suffix_. This files contain only the
changes of the specific settings the user is interested in. There can
be serveral such drop-in files, they are processed in lexicographic order
of their filename.


The first method is useful to override the complete configuration file with an
own one, the vendor supplied configuration is ignored.

So, if /etc/_example_._suffix_ exists, /usr/_vendor_/_example_._suffix_ will
not be read. The disadvantage is, that changes of the vendor configuration
file are ignored and the user has to manually merge them.

The other method will continue to use /usr/_vendor_/_example_._suffix_ as base
configuration file and merge all changes from
/etc/_example_._suffix_/*._suffix_. So the user will automatically get
improvements of the vendor, with the drawback, that they could be incompatible
with the user made changes.

The list of files and directories read if no /etc/_example_._suffix_ file
exists:

* /usr/_vendor_/_example_._suffix_
* /usr/_vendor_/_example_._suffix_.d/*._suffix_
* /etc/_example_._suffix_.d/*._suffix_

If a /etc/_example_._suffix_ files exists:
* /etc/_example_._suffix_
* /etc/_example_._suffix_.d/*._suffix_
