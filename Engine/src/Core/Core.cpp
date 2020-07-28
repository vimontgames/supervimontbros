#include "Precomp.h"
#include "Core.h"

#define USE_COUT
#ifdef USE_COUT
#include <iostream>
#endif

void debugPrint(const char * _format, ...)
{
	va_list args;
	va_start(args, _format);

	#ifdef USE_COUT
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), _format, args);
	perror(buffer);
	std::cout << buffer << std::flush;
	#else
	vfprintf(stderr, _format, args);
	#endif

	va_end(args);
}