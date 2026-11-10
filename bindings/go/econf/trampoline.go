package econf

/*
#include <stdbool.h>

extern bool goEconfCallback(char *filename, void *data);

bool econf_callback_trampoline(const char *filename, const void *data) {
	return goEconfCallback((char *)filename, (void *)data);
}
*/
import "C"
