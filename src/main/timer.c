#include "timer.h"
#include "error.h"
#include "cpu-storage.c"

#define MASK_FIRST_2BITS 0x03
#define MAX_VALUE_SEC_COUNTER 0xFF
#define BIT_TAC_00 1
#define BIT_TAC_01 3
#define BIT_TAC_10 5
#define BIT_TAC_11 7


/**
 * see timer.h
 * */
int timer_init(gbtimer_t *timer, cpu_t *cpu)
{
	M_REQUIRE_NON_NULL(cpu);
	M_REQUIRE_NON_NULL(timer);
	timer -> cpu = cpu ;
	timer->counter = 0;

	return ERR_NONE;
}
/**
 * @brief return the state of timer
 * 
 * @param timer pointer to timer
 * @return state 
 * */
bit_t timer_state(gbtimer_t *timer)
{

	uint8_t TAC = cpu_read_at_idx(timer->cpu, REG_TAC);
	bit_t minuteur_act = bit_get(TAC, 2);
	bit_t counter_bit = 0;
	switch (TAC & MASK_FIRST_2BITS)
	{
	case 0:
		counter_bit = bit_get(msb8(timer->counter), BIT_TAC_00);
		break;
	case 1:
		counter_bit = bit_get(lsb8(timer->counter), BIT_TAC_01);
		break;
	case 2:
		counter_bit = bit_get(lsb8(timer->counter), BIT_TAC_10);
		break;
	case 3:
		counter_bit = bit_get(lsb8(timer->counter), BIT_TAC_11);
		break;
	}
	return counter_bit & minuteur_act;
}
/**
 * @brief increment the second timer if the state change 
 * 
 * @param timer pointer to timer 
 * @param old_state old state 
 * @return error code 
 * */
int timer_incr_if_state_change(gbtimer_t *timer, bit_t old_state)
{
	M_REQUIRE_NON_NULL(timer);
	bit_t curr_state = timer_state(timer);
	if (curr_state == 0 && old_state == 1)
	{
		uint8_t sec_counter = cpu_read_at_idx(timer->cpu, REG_TIMA);
		sec_counter += 1;
		M_EXIT_IF_ERR(cpu_write_at_idx(timer->cpu, REG_TIMA, sec_counter));
		if (sec_counter >= MAX_VALUE_SEC_COUNTER)
		{
			uint8_t TMA = cpu_read_at_idx(timer->cpu, REG_TMA);
			M_EXIT_IF_ERR(cpu_write_at_idx(timer->cpu, REG_TIMA, TMA));
			cpu_request_interrupt(timer->cpu, TIMER);
		}
	}

	return ERR_NONE;
}
/**
 * see timer.h
 * */
int timer_cycle(gbtimer_t *timer)
{
	M_REQUIRE_NON_NULL(timer);

	bit_t old_state = timer_state(timer);
	timer->counter += 4;
	M_EXIT_IF_ERR(cpu_write_at_idx(timer->cpu, REG_DIV, msb8(timer->counter)));
	M_EXIT_IF_ERR(timer_incr_if_state_change(timer, old_state));
	return ERR_NONE;
}

/**
 * see timer.h
 * */
int timer_bus_listener(gbtimer_t *timer, addr_t addr)
{
	M_REQUIRE_NON_NULL(timer);
	bit_t old_state = timer_state(timer);
	if (addr == REG_DIV)
	{
		timer->counter = 0;
		M_EXIT_IF_ERR(timer_incr_if_state_change(timer, old_state));
	}
	if (addr == REG_TAC)
	{
		M_EXIT_IF_ERR(timer_incr_if_state_change(timer, old_state));
	}
	return ERR_NONE;
}
