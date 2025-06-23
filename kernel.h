#pragma once             
#include "memoryHandler.h"     

#define dataFlags (PTEReadable | PTEWriteable)
#define bssFlags (PTEReadable | PTEWriteable)
#define rodataFlags (PTEReadable)
#define textFlags (PTEReadable | PTEExecutable)