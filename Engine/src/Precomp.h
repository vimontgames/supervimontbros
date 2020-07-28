#include <SFML/Graphics.hpp>

struct IUnknown; // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here" when using /permissive-
#include <windows.h>

#include <cstdint>
#include <stdio.h>
#include <assert.h> 
#include <stdarg.h>

#include "Core/Core.h"