#pragma once             
#include "memoryHandler.h"     

#define dataFlags (PTE_READABLE | PTE_WRITEABLE)
#define bssFlags (PTE_READABLE | PTE_WRITEABLE)
#define rodataFlags (PTE_READABLE)
#define textFlags (PTE_READABLE | PTE_EXECUTEABLE)