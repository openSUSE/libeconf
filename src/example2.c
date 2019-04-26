#include <libeconf.h>

int main() {
  merge_files("example/etc/example.conf.d", "example.ini", "example/etc/example", "example/usr/share/defaults", '=', '#');
}
