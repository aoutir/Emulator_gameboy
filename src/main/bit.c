#include "bit.h"
#include <stdio.h> 


/**
 * See bit.h
 */
uint8_t lsb4(uint8_t value){
	return value & 0xf;
}

/**
 * See bit.h
 */
uint8_t msb4(uint8_t value){
	return value >> 4;
}

/**
 * See bit.h
 */
uint8_t lsb8(uint16_t value){
	return value & 0xFF;
}

/**
 * See bit.h
 */
uint8_t msb8(uint16_t value){
	return value >> 8 ;
}

/**
 * See bit.h
 */
uint16_t merge8(uint8_t v1, uint8_t v2){
	return (v2 << 8) | (v1) ;
}

/**
 * See bit.h
 */
uint8_t merge4(uint8_t v1, uint8_t v2){
	return ((v2 & 0xf) << 4 ) | (v1 & 0xf);
}

/**
 * See bit.h
 */
bit_t bit_get(uint8_t value, int index){
	return (value >> CLAMP07(index)) & 1;
}

/**
 * See bit.h
 */
void bit_set(uint8_t* value, int index){
	if(value != NULL){
	 *value = (1 << CLAMP07(index)) |*value; 
	}
}

/**
 * See bit.h
 */
void bit_unset(uint8_t* value, int index){
	if(value != NULL){
	 *value = *value&(~(1 << CLAMP07(index)));
	}
}

/**
 * See bit.h
 */
void bit_rotate(uint8_t* value, rot_dir_t dir, int d){
	if(value != NULL) {
		int dc = CLAMP07(d);
		if(dir == LEFT){
		uint8_t lsb = *value >> (8 - dc);
		uint8_t msb = *value << dc;
		*value = msb | lsb ;
		} else if(dir == RIGHT){
		uint8_t lsb = *value >> dc;
		uint8_t msb = *value << (8-dc);
		*value = msb | lsb ;
		}		
	}
}

/**
 * See bit.h
 */
void bit_edit(uint8_t* value, int index, uint8_t v){
	if(value != NULL){
		if(v == 0){
			bit_unset(value,index);
		}else{
			bit_set(value,index);
		}
	}
}

