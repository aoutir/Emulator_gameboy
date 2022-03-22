#include "alu.h"
#include "error.h"

#define INDEX_FANION_N 6
#define INDEX_FANION_H 5

/**
 * See alu.h
 */
flag_bit_t get_flag(flags_t flags, flag_bit_t flag)
{
	if (flag != FLAG_Z && flag != FLAG_N && flag != FLAG_H && flag != FLAG_C)
	{
		return 0;
	}
	return flags & flag;
}

/**
 * See alu.h
 */
void set_flag(flags_t *flags, flag_bit_t flag)
{
	if (flags != NULL)
	{
		if (flag == FLAG_Z || flag == FLAG_N || flag == FLAG_H || flag == FLAG_C)
		{
			*flags = *flags | flag;
		}
	}
}

/**
 * See alu.h
 */
int alu_add8(alu_output_t *result, uint8_t x, uint8_t y, bit_t c0)
{	
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(c0 <= 1, ERR_BAD_PARAMETER, "carry should be 0 or 1 ");
	uint8_t v1 = lsb4(x) + lsb4(y) + c0;
	uint8_t v2 = msb4(x) + msb4(y) + msb4(v1);
	(result->value) = merge4(v1, v2);
	if (result->value == 0)
	{
		set_Z(&(result->flags));
	}
	// A half carry should be set every time v1's value exceeds 4 bits
	if (bit_get(v1, 4))
	{
		set_H(&(result->flags));
	}
	// A carry should be set every time v2's value exceeds 4 bits
	if (bit_get(v2, 4))
	{
		set_C(&(result->flags));
	}
	return ERR_NONE;
		
}

/**
 * See alu.h
 */
int alu_sub8(alu_output_t *result, uint8_t x, uint8_t y, bit_t b0)
{
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(b0 <= 1, ERR_BAD_PARAMETER, "carry should be 0 or 1 ");
	uint8_t v1 = lsb4(x) - lsb4(y) - b0;
	uint8_t v2 = msb4(x) - msb4(y) - bit_get(v1, 4);
	result->value = merge4(v1, v2);
	set_N(&(result->flags));
	if (result->value == 0)
	{
		set_Z(&(result->flags));
	}
	if (bit_get(v1, 4))
	{
		set_H(&(result->flags));
	}
	if (bit_get(v2, 4))
	{
		set_C(&(result->flags));
	}
	return ERR_NONE;
}

/**
 * See alu.h
 */
int alu_add16_low(alu_output_t *result, uint16_t x, uint16_t y)
{
	M_REQUIRE_NON_NULL(result);
	M_EXIT_IF_ERR(alu_add8(result, lsb8(x), lsb8(y), 0));

	// Forcing Z Flag to be 0 in case the addition overflowed and alu_add8 set the flag to 1 while it shouldn't be
	result->flags = result->flags & (~FLAG_Z);
	uint8_t v1 = 0;
	if (get_C(result->flags) == FLAG_C)
	{
		v1 = msb8(x) + msb8(y) + msb8(result->value) + 1;
	}
	else
	{
		v1 = msb8(x) + msb8(y) + msb8(result->value);
	}
	result->value = merge8(lsb8(result->value), v1);
	if (result->value == 0)
	{
		set_Z(&(result->flags));
	}
	return ERR_NONE;
}

/**
 * See alu.h
 */
int alu_add16_high(alu_output_t *result, uint16_t x, uint16_t y)
{
	M_REQUIRE_NON_NULL(result);
	uint16_t v1 = lsb8(x) + lsb8(y);
	M_EXIT_IF_ERR(alu_add8(result, msb8(x), msb8(y), msb8(v1)));

	// Forcing Z Flag to be 0 in case the addition overflowed and alu_add8 set the flag to 1 while it shouldn't be
	result->flags = result->flags & (~FLAG_Z);
	result->value = merge8(v1, lsb8(result->value));
	if (result->value == 0)
	{
		set_Z(&(result->flags));
	}
	return ERR_NONE;
}

/**
 * See alu.h
 */
int alu_shift(alu_output_t *result, uint8_t x, rot_dir_t dir)
{
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(dir <= RIGHT, ERR_BAD_PARAMETER, "dir should be left(0) or right(1) ");
	if (dir)
	{
		bit_t b0 = bit_get(x, 0);
		if (b0 == 1)
		{
			set_C(&(result->flags));
		}
		result->value = x >> 1;
	}
	else
	{
		bit_t b7 = bit_get(x, 7);
		if (b7 == 1)
		{
			set_C(&(result->flags));
		}
		x = x << 1;
		result->value = x;
	}
	if (result->value == 0)
	{
		set_Z(&(result->flags));
	}
	bit_unset(&(result->flags), INDEX_FANION_N);
	bit_unset(&(result->flags), INDEX_FANION_H);
	return ERR_NONE;
}

/**
 * See alu.h
 */
int alu_shiftR_A(alu_output_t *result, uint8_t x)
{
	M_REQUIRE_NON_NULL(result);
	bit_t b7 = bit_get(x, 7);
	bit_t b0 = bit_get(x, 0);
	result->value = x >> 1;
	uint8_t tmp = result->value;
	if (b7 == 1)
	{
		bit_set(&tmp, 7);
		result->value = tmp;
	}
	if (b0 == 1)
	{
		set_C(&result->flags);
	}
	if (result->value == 0)
	{
		set_Z(&(result->flags));
	}
	bit_unset(&(result->flags), INDEX_FANION_N);
	bit_unset(&(result->flags), INDEX_FANION_H);
	return ERR_NONE;
}

/**
 * See alu.h
 */
int alu_rotate(alu_output_t *result, uint8_t x, rot_dir_t dir)
{
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(dir <= RIGHT, ERR_BAD_PARAMETER, "dir should be left(0) or right(1) ");
	bit_t c = 0;
	if (dir == RIGHT)
	{
		c = bit_get(x, 0);
		uint8_t lsb = x >> 1;
		uint8_t msb = x << 7;
		result->value = msb | lsb;
	}
	else
	{
		c = bit_get(x, 7);
		uint8_t msb = x << 1;
		result->value = msb | c;
	}

	//flags
	if (c == 1)
	{
		set_C(&(result->flags));
	}
	if (result->value == 0)
	{
		set_Z(&(result->flags));
	}
	bit_unset(&(result->flags), INDEX_FANION_N);
	bit_unset(&(result->flags), INDEX_FANION_H);
	return ERR_NONE;
}

/**
 * See alu.h
 */
int alu_carry_rotate(alu_output_t *result, uint8_t x, rot_dir_t dir, flags_t flags)
{
	M_REQUIRE_NON_NULL(result);
	M_REQUIRE(dir <= RIGHT, ERR_BAD_PARAMETER, "dir should be left(0) or right(1) ");

	bit_t c = get_C(flags) >> 4;
	result->flags = 0x00;
	alu_shift(result, x, dir);
	result->flags = get_C(result->flags);

	if (dir == LEFT)
	{
		uint8_t msb = result->value;
		result->value = msb | c;
	}
	else
	{
		uint8_t lsb = result->value;
		result->value = c << 7 | lsb;
	}
	if (result->value == 0)
	{
		set_flag(&(result->flags), FLAG_Z);
	}
	bit_unset(&(result->flags), INDEX_FANION_N);
	bit_unset(&(result->flags), INDEX_FANION_H);

	return ERR_NONE;
}
