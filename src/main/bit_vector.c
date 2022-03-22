#include "bit_vector.h"
#include "error.h"

/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_create(size_t size, bit_t value)
{
	if (size == 0 || size == SIZE_MAX)
	{
		return NULL;
	}

	bit_vector_t *result = malloc(sizeof(bit_vector_t));
	if (result == NULL)
	{
		return NULL;
	}
	int n = size / 32;
	result->size = size;
	int res = size % 32;
	result->allocated = n;

	if (res == 0)
	{
		result->content = calloc(n, sizeof(uint32_t));
	}
	else
	{
		result->allocated += 1;
		result->content = calloc(n + 1, sizeof(uint32_t));
	}
	if (result->content != NULL)
	{
		if (value == 1)
		{
			for (int i = 0; i < n; i++)
			{
				result->content[i] = 0xffffffff;
			}
			if (res != 0)
			{
				result->content[n] = 0xffffffff >> (32 - res);
			}
		}
		return result;
	}
	else
	{
		result->size = 0;
		result->allocated = 0;
		return NULL;
	}
}
/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_cpy(const bit_vector_t *pbv)
{
	if (pbv == NULL)
	{
		return NULL;
	}
	bit_vector_t *result = malloc(sizeof(bit_vector_t));
	if (result == NULL)
	{
		return NULL;
	}

	result->size = pbv->size;
	result->allocated = pbv->allocated;
	result->content = calloc(result->allocated, sizeof(uint32_t));

	if (result->content != NULL)
	{
		for (int i = 0; i < result->allocated; i++)
		{
			result->content[i] = pbv->content[i];
		}
	}
	else
	{
		free(result);
		result = NULL;
	}
	return result;
}

/**
 * see bit_vector.h
 **/

bit_t bit_vector_get(const bit_vector_t *pbv, size_t index)
{
	if (pbv == NULL || index >= pbv->size)
	{
		return 0;
	}
	int indexContent = index / 32;
	int indexInElem = index % 32;
	int mask = 1 << indexInElem;
	if ((pbv->content[indexContent] & mask) == mask)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_not(bit_vector_t *pbv)
{
	if (pbv == NULL)
	{
		return NULL;
	}
	int res = pbv->size % 32;
	for (int i = 0; i < pbv->allocated; i++)
	{
		if (i == pbv->allocated - 1 && res != 0)
		{
			int mask = 0xffffffff >> (32 - res);
			pbv->content[i] = pbv->content[i] ^ mask;
		}
		else
		{
			pbv->content[i] = ~(pbv->content[i]);
		}
	}
	return pbv;
}

/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_and(bit_vector_t *pbv1, const bit_vector_t *pbv2)
{
	if (pbv1 == NULL || pbv2 == NULL || pbv1->size != pbv2->size)
	{
		return NULL;
	}
	for (int i = 0; i < pbv1->allocated; i++)
	{
		pbv1->content[i] = pbv1->content[i] & pbv2->content[i];
	}
	return pbv1;
}

/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_or(bit_vector_t *pbv1, const bit_vector_t *pbv2)
{
	if (pbv1 == NULL || pbv2 == NULL || pbv1->size != pbv2->size)
	{
		return NULL;
	}
	for (int i = 0; i < pbv1->allocated; i++)
	{
		pbv1->content[i] = pbv1->content[i] | pbv2->content[i];
	}
	return pbv1;
}

/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_xor(bit_vector_t *pbv1, const bit_vector_t *pbv2)
{
	if (pbv1 == NULL || pbv2 == NULL || pbv1->size != pbv2->size)
	{
		return NULL;
	}
	for (int i = 0; i < pbv1->allocated; i++)
	{
		pbv1->content[i] = pbv1->content[i] ^ pbv2->content[i];
	}
	return pbv1;
}

/**
 * see bit_vector.h
 **/

int32_t generic_extractor(const bit_vector_t *pbv, int type, int64_t index, size_t size)
{
	if (index % 32 == 0)
	{
		if (pbv->size % 32 == 0 || type == TYPE_ZERO)
		{
			if ((index < 0 || index > pbv->size - 1) && type == TYPE_ZERO)
			{
				return 0;
			}
			else
			{
				if (type == TYPE_ZERO)
				{
					return pbv->content[index / 32];
				}
				else
				{
					int index_non_neg = index < 0 ? -index / 32 : index / 32;
					return pbv->content[index_non_neg % pbv->allocated];
				}
			}
		}
		else
		{
			int index_non_neg = index < 0 ? -index : index;
			uint32_t the_res = bit_vector_get(pbv, (index_non_neg + 31) % pbv->size);
			int iterator = 1;
			while (iterator < size)
			{
				the_res = (the_res << 1) | bit_vector_get(pbv, (index_non_neg + 31 - iterator) % pbv->size);
				++iterator;
			}
			return the_res;
		}
	}
	else
	{
		int64_t index_of_first_elem = 0;
		int64_t index_of_second_elem = 0;
		if (index < 0)
		{
			index_of_first_elem = (index / 32 - 1) * 32;
			index_of_second_elem = (index / 32) * 32;
		}
		else
		{
			index_of_first_elem = (index / 32) * 32;
			index_of_second_elem = (index / 32 + 1) * 32;
		}
		uint32_t first_elem = generic_extractor(pbv, type, index_of_first_elem, size);
		uint32_t second_elem = generic_extractor(pbv, type, index_of_second_elem, size);

		int64_t index_abs = (index % 32);
		if (index < 0)
		{
			index_abs = 32 + index;
		}

		uint32_t p1 = (first_elem >> index_abs);
		uint32_t p2 = (second_elem << (32 - index_abs));
		uint32_t ret_val = p2 | p1;
		return ret_val;
	}
}

/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_extract_zero_ext(const bit_vector_t *pbv, int64_t index, size_t size)
{
	if (size == 0)
	{
		return NULL;
	}
	if (pbv == NULL)
	{
		return bit_vector_create(size, 0);
	}
	bit_vector_t *result = bit_vector_create(size, 0);

	for (int i = 0; i < result->allocated; ++i)
	{
		result->content[i] = generic_extractor(pbv, TYPE_ZERO, index + 32 * i, size);
	}

	return result;
}

/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_extract_wrap_ext(const bit_vector_t *pbv, int64_t index, size_t size)
{
	if (pbv == NULL || size == 0)
	{
		return NULL;
	}
	bit_vector_t *result = bit_vector_create(size, 0);

	for (int i = 0; i < result->allocated; ++i)
	{
		result->content[i] = generic_extractor(pbv, TYPE_WRAP, index + 32 * i, size);
	}

	return result;
}

/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_shift(const bit_vector_t *pbv, int64_t shift)
{
	if (pbv == NULL)
	{
		return NULL;
	}
	return bit_vector_extract_zero_ext(pbv, -shift, pbv->size);
}

/**
 * see bit_vector.h
 **/

bit_vector_t *bit_vector_join(const bit_vector_t *pbv1, const bit_vector_t *pbv2, int64_t shift)
{
	if (pbv1 == NULL || pbv2 == NULL || pbv1->size != pbv2->size || shift < 0 || shift > pbv1->size)
	{
		return NULL;
	}
	bit_vector_t *result = bit_vector_create(pbv1->size, 0);
	for (size_t i = 0; i < pbv1->size; ++i)
	{
		if (i < shift)
		{
			result->content[i / 32] = result->content[i / 32] | (bit_vector_get(pbv1, i) << i);
		}
		else
		{
			result->content[i / 32] = result->content[i / 32] | (bit_vector_get(pbv2, i) << i);
		}
	}
	return result;
}

/**
 * see bit_vector.h
 **/

int bit_vector_print(const bit_vector_t *pbv)
{
	if (pbv == NULL)
	{
		return 0;
	}
	for (size_t i = 0; i < pbv->allocated; ++i)
	{
		printf("%d", pbv->content[pbv->allocated - i]);
	}
	return pbv->size;
}

/**
 * see bit_vector.h
 **/

int bit_vector_println(const char *prefix, const bit_vector_t *pbv)
{
	if (prefix != NULL)
	{
		fputs(prefix, stdout);
	}
	int result = bit_vector_print(pbv);
	fputs("\n", stdout);
	return result;
}

/**
 * see bit_vector.h
 **/

void bit_vector_free(bit_vector_t **pbv)
{
	if (pbv != NULL)
	{
		free(*pbv);
		pbv = NULL;
	}
}
