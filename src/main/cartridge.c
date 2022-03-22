#include "cartridge.h"
#include "error.h"
#include <stdio.h>

/**
 * see cartridge.h
 **/
int cartridge_init_from_file(component_t* c, const char* filename){
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(filename);
	
	
	FILE* entree = NULL;
	entree = fopen(filename,"rb");
	if(entree == NULL){
		return ERR_IO;
	}
	fseek(entree,0L,SEEK_END);
	long int size = ftell(entree);
	if(c->mem->size > size){
		fclose(entree);
		return ERR_IO;
	}
	fseek(entree,0L,SEEK_SET);
	size_t succ = fread(c-> mem->memory,1,size, entree);
	fclose(entree);
	if(succ != size){
		return ERR_IO;
	}
	
	if(c->mem->memory[CARTRIDGE_TYPE_ADDR] != 0 ){
		return ERR_NOT_IMPLEMENTED;
	}
	
	return ERR_NONE ;
}

/**
 * see cartridge.h
 **/
int cartridge_init(cartridge_t* ct, const char* filename){
	M_REQUIRE_NON_NULL(ct);
	M_REQUIRE_NON_NULL(filename);
	M_EXIT_IF_ERR(component_create(&ct->c,BANK_ROM_SIZE));
	M_EXIT_IF_ERR(cartridge_init_from_file(&ct -> c,filename));
	return ERR_NONE;
}

/**
 * see cartridge.h
 **/
int cartridge_plug(cartridge_t* ct, bus_t bus){
	M_REQUIRE_NON_NULL(ct);
	M_REQUIRE_NON_NULL(ct->c.mem);
	M_REQUIRE_NON_NULL(bus);
	
	M_EXIT_IF_ERR(bus_forced_plug(bus,&ct->c,BANK_ROM0_START,BANK_ROM1_END,0));
	return ERR_NONE;
}

/**
 * see cartridge.h
 **/
void cartridge_free(cartridge_t* ct){
	if(ct != NULL){
		component_free(&ct->c);
	}
}
