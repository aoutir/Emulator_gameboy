#include "bus.h"
#include "error.h"
#include "bit.h"

/**
 * See bus.h
 */
int bus_remap(bus_t bus, component_t* c, addr_t offset){
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE_NON_NULL(c->mem);
	size_t size_with_offset = c->end - c->start + offset ;
	M_REQUIRE(size_with_offset <(c->mem) -> size,ERR_ADDRESS,"Size not big enough");
	size_t size_without_offset = c->end - c->start + 1 ;
	for (int i=0 ; i < size_without_offset ; i++) {
		bus[c->start +i]= &(c->mem->memory[offset+i]);
	}
	return ERR_NONE ;
}

/**
 * See bus.h
 */
int bus_forced_plug(bus_t bus, component_t* c, addr_t start, addr_t end, addr_t offset){
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE(start < end,ERR_BAD_PARAMETER,"Start should be less than end ");
	M_REQUIRE(end-start+offset < (c->mem) -> size,ERR_ADDRESS,"Not enough size");
	c->start = start;
	c->end = end ;
	M_EXIT_IF_ERR_DO_SOMETHING(bus_remap(bus,c,offset),{c->start = 0;c->end = 0;});
	return ERR_NONE ;	 
}

/**
 * See bus.h
 */
int bus_plug(bus_t bus, component_t* c, addr_t start, addr_t end){
	M_REQUIRE_NON_NULL(c);
	M_REQUIRE(start < end,ERR_BAD_PARAMETER,"Start should be less than end ");
	for(int i= start ; i <= end; ++i){
		M_REQUIRE(bus[i] == NULL , ERR_ADDRESS , "The bus at position %d is not NULL" , i);
	}
	M_EXIT_IF_ERR(bus_forced_plug(bus,c,start,end,0));
	return ERR_NONE;
}

/**
 * See bus.h
 */
int bus_unplug(bus_t bus , component_t* c){
	M_REQUIRE_NON_NULL(c);
	for(int i = c -> start  ; i <= c-> end ; i++){
		bus[i] = NULL;
	}
	c -> start = 0 ;
	c -> end = 0 ; 
	return ERR_NONE;
}

/**
 * See bus.h
 */
int bus_read( const bus_t bus , addr_t address , data_t* data) {
	M_REQUIRE_NON_NULL(data);
	if(bus[address] == NULL){
		*data = 0xFF ;	
	}else{
		*data = *bus[address];
	}
	return ERR_NONE;
}

/**
 * See bus.h
 */
int bus_read16(const bus_t bus, addr_t address, addr_t* data16){
	M_REQUIRE_NON_NULL(data16);
	if(bus[address] == NULL){
		*data16 = 0xFF ;
	}else{
		*data16 = merge8(*bus[address],*bus[address+1]);
	} 
	return ERR_NONE;
}

/**
 * See bus.h
 */
int bus_write(bus_t bus, addr_t address , data_t data){
	M_REQUIRE_NON_NULL(bus);
	M_REQUIRE_NON_NULL(bus[address]);
	*bus[address] = data ;
	return ERR_NONE;
}

/**
 * See bus.h
 */
int bus_write16(bus_t bus, addr_t address , addr_t data16){
	M_REQUIRE_NON_NULL(bus[address]);
	M_EXIT_IF_ERR(bus_write(bus,address,lsb8(data16)));
	M_EXIT_IF_ERR(bus_write(bus,(addr_t)(address+1),msb8(data16)));
	return ERR_NONE;
}