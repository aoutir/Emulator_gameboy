#include "memory.h"
#include "error.h"
#include <stdlib.h>

/**
 * See memory.h
 */
int mem_create(memory_t *mem, size_t size)
{
	M_REQUIRE_NON_NULL(mem);
	M_REQUIRE(size != 0, ERR_BAD_PARAMETER, "Size should not be zero ");
	// Creating a new temporary memory_t structure where we will allocate memory
	// to prevent problems occuring during the dynamic allocation
	memory_t tmpMem = {NULL, 0};
	M_EXIT_IF_NULL(tmpMem.memory = (data_t *)calloc(size, sizeof(data_t)), size * sizeof(data_t));
	tmpMem.size = size;
	// Now that everything went alright , we can modify our original parameter mem
	*mem = tmpMem;
	return ERR_NONE;
}

/**
 * See memory.h
 */
void mem_free(memory_t *mem)
{
	if (mem != NULL)
	{
		free(mem->memory);
		mem->memory = NULL;
		mem->size = 0;
	}
}
