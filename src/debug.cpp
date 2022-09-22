#include <cstdio>

#include "config.h"
#include "debug.h"

void debug_message(char* fmt, ...)
{
	if (!config.showDebugMessages())
		return;

	va_list ap;
	char msg[1000];

	va_start(ap, fmt);
	vsnprintf(msg, (sizeof(msg)/sizeof(*msg))-1, fmt, ap);
	printf("%s", msg);
}
