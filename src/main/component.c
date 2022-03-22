#include "component.h"
#include "error.h"
#include "util.h"
#include <stdlib.h>

/**
 * See component.h
 */
int component_create(component_t* c, size_t mem_size){
	M_REQUIRE_NON_NULL(c);
	// Creating a new temporary component_t where we will allocate memory
	// to prevent problems occuring during the dynamic allocation 
	component_t tmpC = {NULL,0,0};
	if(mem_size != 0){
		M_EXIT_IF_NULL(tmpC.mem = (memory_t*)calloc(1,sizeof(memory_t)), sizeof(memory_t));
		int err = mem_create(tmpC.mem,mem_size);
		if(err != ERR_NONE){
			free(tmpC.mem);
			return err;
		}
	}
	// Now that everything went alright , we can modify our original component 
	*c = tmpC;
	return ERR_NONE ;
}

/**
 * See component.h
 */
void component_free(component_t* c){
	if(c != NULL ){
	mem_free(c -> mem);
	free(c->mem);
	c-> mem = NULL;
	c->start = 0 ;
	c->end = 0 ;
	}
}

/**
 * See component.h
 */
int component_shared(component_t* c, component_t* c_old){
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(c_old);
	M_REQUIRE_NON_NULL(c_old->mem);
	M_REQUIRE_NON_NULL(c_old->mem->memory);
	
	c->start = 0 ;
	c->end = 0 ;
	c-> mem = c_old -> mem ;
	return ERR_NONE;
}


