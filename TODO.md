# libeconf TODO

* Implement/Fix parts marked with TODO or XXX
* Reformat the code (should have uniform coding style)
* Complete NULL value and error checking
* Rework internal data structure for better group and key handling:
  * adding new groups and keys
  * faster searching
  * deletion of groups and keys
* Rework function to write the configuration to a file
* Implement a function to write the changes back into the source file
* Support for comments: comments should be preserved when rewriting a file
* Support writing files into a '.d' directory.
* Implement econf_setOpt function to modify behavior of loading a
  configuration file for special cases
* Add more to the list ...

