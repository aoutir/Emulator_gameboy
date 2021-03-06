#pragma once

/**
 * @file gameboy.h
 * @brief Gameboy Header for GameBoy Emulator
 *
 * @author C. Hölzl, EPFL
 * @date 2019
 */

#include <stdint.h>
#include <stdlib.h>

#include "bus.h"
#include "component.h"
#include "cpu.h"
#include "util.h"
#include "cartridge.h"
#include "timer.h"
#include "joypad.h"
#include "lcdc.h"
#ifdef __cplusplus
extern "C" {
#endif

#define GB_NB_COMPONENTS 6

/**
 * @brief Game Boy data structure.
 *        Regroups everything needed to simulate the Game Boy.
 */
struct gameboy_t {
	bus_t bus;
	cpu_t cpu;
	uint64_t cycles;
	gbtimer_t timer;
	cartridge_t cartridge;
	component_t components[GB_NB_COMPONENTS];
	uint8_t nb_components;
	component_t bootrom ;
	bit_t boot ;
	lcdc_t screen;
	joypad_t pad;
	
	};

// Number of Game Boy cycles per second (= 2^20)
#define GB_CYCLES_PER_S  (((uint64_t) 1) << 20)

/**
 * @brief Creates a gameboy
 *
 * @param gameboy pointer to gameboy to create
 */
int gameboy_create(gameboy_t* gameboy, const char* filename);

/**
 * @brief Destroys a gameboy
 *
 * @param gameboy pointer to gameboy to destroy
 */
void gameboy_free(gameboy_t* gameboy);

/**
 * @brief Runs a gamefor for/until a given cycle
 */
int gameboy_run_until(gameboy_t* gameboy, uint64_t cycle);

/**
 * @brief Adresses of the GameBoy
 *
 */
#define MEM_SIZE(X) (X ## _END - X ## _START + 1)

#define BOOT_ROM_START   0x0000
#define BOOT_ROM_END     0x00FF

#define VIDEO_RAM_START  0x8000
#define VIDEO_RAM_END    0x9FFF

#define EXTERN_RAM_START 0xA000
#define EXTERN_RAM_END   0xBFFF

#define WORK_RAM_START   0xC000
#define WORK_RAM_END     0xDFFF

#define ECHO_RAM_START   0xE000
#define ECHO_RAM_END     0xFDFF

#define GRAPH_RAM_START  0xFE00
#define GRAPH_RAM_END    0xFE9F

#define USELESS_START    0xFEA0
#define USELESS_END      0xFEFF

#define REGISTERS_START  0xFF00
#define REGISTERS_END    0xFF7F


// Memory-mapped "IO" registers
#define REGS_START      0xFF00
#define BLARGG_REG      0xFF01

#define REGS_LCDC_START 0xFF40
#define REGS_LCDC_END   0xFF4C
#define REG_BOOT_ROM_DISABLE  0xFF50


// Indices of each component in the gameboy array of components

#define WORK_RAM_INDEX 0
#define EXTERN_RAM_INDEX 1
#define VIDEO_RAM_INDEX 2
#define GRAPH_RAM_INDEX 3
#define USELESS_INDEX 4
#define REGISTERS_INDEX 5
#define BOOT_ROM_INDEX 6


#ifdef __cplusplus
}
#endif
