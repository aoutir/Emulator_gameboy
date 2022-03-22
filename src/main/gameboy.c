#include "gameboy.h"
#include "error.h"
#include "bootrom.h"

#ifdef BLARGG
        static int blargg_bus_listener(gameboy_t * gameboy, addr_t addr)
        {	M_REQUIRE_NON_NULL(gameboy);
            if (addr == BLARGG_REG)
            {
                data_t data;
                M_EXIT_IF_ERR(bus_read(gameboy->bus, addr, &data));
                printf("%c", data);
            }
            return ERR_NONE;
        }
#endif

/**
 * @brief Helper to create a component , add it to the gameboy's array , and plug it to the bus
 * @param n_gameboy pointer to gameboy
 * @param index_in_array index of the component
 * @param start starting index 
 * @param end ending index
 * */
int add_component(gameboy_t *n_gameboy, int index_in_array, size_t size, addr_t start, addr_t end)
{
	M_EXIT_IF_ERR(component_create(&(n_gameboy->components[index_in_array]), size));
	M_EXIT_IF_ERR(bus_plug(n_gameboy->bus, &n_gameboy->components[index_in_array], start,end));
	++n_gameboy->nb_components;
    return ERR_NONE;
}

/**
 * see gameboy.h
 **/

int gameboy_create(gameboy_t *gameboy, const char *filename)
{
	M_REQUIRE_NON_NULL(gameboy);
	zero_init_var(gameboy->bus);
	gameboy->cycles = 1;
	gameboy->nb_components = 0;

	//initialize cpu 
	M_EXIT_IF_ERR_DO_SOMETHING(cpu_init(&gameboy->cpu),gameboy_free(gameboy));
      
    
	//-----------------------------------------WORK_RAM---------------------------------------------------
	M_EXIT_IF_ERR_DO_SOMETHING(add_component(gameboy, WORK_RAM_INDEX, MEM_SIZE(WORK_RAM), WORK_RAM_START, WORK_RAM_END),gameboy_free(gameboy));

	//-----------------------------------------ECHO_RAM---------------------------------------------------
	component_t echo_ram;
	
	//Sharing the two components WORK_RAM and ECHO_RAM memory
	M_EXIT_IF_ERR_DO_SOMETHING(component_shared(&echo_ram, &gameboy->components[WORK_RAM_INDEX]),gameboy_free(gameboy));
	

    // Pluging it to the bus
	M_EXIT_IF_ERR_DO_SOMETHING(bus_plug(gameboy->bus, &echo_ram, ECHO_RAM_START, ECHO_RAM_END),gameboy_free(gameboy));

	//-----------------------------------------EXTERN_RAM---------------------------------------------------
	M_EXIT_IF_ERR_DO_SOMETHING(add_component(gameboy, EXTERN_RAM_INDEX, MEM_SIZE(EXTERN_RAM), EXTERN_RAM_START, EXTERN_RAM_END),gameboy_free(gameboy));

	//-----------------------------------------VIDEO_RAM---------------------------------------------------
	M_EXIT_IF_ERR_DO_SOMETHING(add_component(gameboy, VIDEO_RAM_INDEX, MEM_SIZE(VIDEO_RAM), VIDEO_RAM_START, VIDEO_RAM_END),gameboy_free(gameboy));

	//-----------------------------------------GRAPH_RAM---------------------------------------------------
	M_EXIT_IF_ERR_DO_SOMETHING(add_component(gameboy, GRAPH_RAM_INDEX, MEM_SIZE(GRAPH_RAM), GRAPH_RAM_START, GRAPH_RAM_END),gameboy_free(gameboy));

	//-----------------------------------------USELESS---------------------------------------------------
	M_EXIT_IF_ERR_DO_SOMETHING(add_component(gameboy, USELESS_INDEX, MEM_SIZE(USELESS), USELESS_START, USELESS_END),gameboy_free(gameboy));

	//-----------------------------------------REGISTERS-------------------------------------------------
	M_EXIT_IF_ERR_DO_SOMETHING(add_component(gameboy, REGISTERS_INDEX, MEM_SIZE(REGISTERS), REGISTERS_START, REGISTERS_END),gameboy_free(gameboy));
	
     //initialize the cartridge
	M_EXIT_IF_ERR_DO_SOMETHING(cartridge_init(&(gameboy->cartridge), filename),gameboy_free(gameboy));

	//plug the cartridge
	M_EXIT_IF_ERR_DO_SOMETHING(cartridge_plug(&(gameboy->cartridge), gameboy->bus),gameboy_free(gameboy));

	//-----------------------------------------BOOT_ROM---------------------------------------------------
	gameboy->boot = 1;
    M_EXIT_IF_ERR_DO_SOMETHING(bootrom_init(&gameboy->bootrom),gameboy_free(gameboy));
	M_EXIT_IF_ERR_DO_SOMETHING(bootrom_plug(&gameboy->bootrom, gameboy->bus),gameboy_free(gameboy));

    //initialize the timer
	M_EXIT_IF_ERR_DO_SOMETHING(timer_init(&gameboy->timer, &gameboy->cpu),gameboy_free(gameboy));

	//plug the bus to the cpu
	M_EXIT_IF_ERR_DO_SOMETHING(cpu_plug(&gameboy->cpu, &gameboy->bus),gameboy_free(gameboy));

	
	//intialize lcdc
	M_EXIT_IF_ERR_DO_SOMETHING(lcdc_init(gameboy),gameboy_free(gameboy));

	//initialize joypad and plug 
     M_EXIT_IF_ERR_DO_SOMETHING(joypad_init_and_plug(&gameboy->pad,&gameboy->cpu),gameboy_free(gameboy));
    //plug l'ecran 
	 M_EXIT_IF_ERR_DO_SOMETHING(lcdc_plug(&gameboy->screen,gameboy->bus),gameboy_free(gameboy));

   
	return ERR_NONE;
}

/**
 * see gameboy.h
 **/
void gameboy_free(gameboy_t *gameboy)
{
	if (gameboy != NULL)
	{
		for (int i = 0; i < GB_NB_COMPONENTS; ++i)
		{
			bus_unplug(gameboy->bus, &gameboy->components[i]);
			component_free(&gameboy->components[i]);
		}
	gameboy->nb_components = 0;
	gameboy->timer.counter = 0;
	bus_unplug(gameboy->bus,&(gameboy->cartridge.c));
	cartridge_free(&(gameboy->cartridge));
	component_free(&gameboy->bootrom);
	component_t echo_ram = {NULL,ECHO_RAM_START,ECHO_RAM_END};
	bus_unplug(gameboy->bus,&(echo_ram));
    lcdc_free(&gameboy->screen);
	cpu_free(&(gameboy->cpu));
    
	}
}

/**
 * see gameboy.h
 **/
int gameboy_run_until(gameboy_t *gameboy, uint64_t cycle)
{
	M_REQUIRE_NON_NULL(gameboy);
	while(gameboy->cycles <= cycle)
	{	
		M_EXIT_IF_ERR(lcdc_cycle(&gameboy->screen,gameboy->cycles ));
		M_EXIT_IF_ERR(timer_cycle(&gameboy->timer));
		M_EXIT_IF_ERR(cpu_cycle(&gameboy->cpu));
		
		
		M_EXIT_IF_ERR(timer_bus_listener(&gameboy->timer, gameboy->cpu.write_listener));
		M_EXIT_IF_ERR(bootrom_bus_listener(gameboy, gameboy->cpu.write_listener));
		M_EXIT_IF_ERR(joypad_bus_listener(&gameboy->pad,gameboy->cpu.write_listener));
		M_EXIT_IF_ERR(lcdc_bus_listener(&gameboy->screen,gameboy->cpu.write_listener));

		#ifdef BLARGG
		M_EXIT_IF_ERR(blargg_bus_listener(gameboy,gameboy->cpu.write_listener));
		#endif;
		
		
		++gameboy->cycles;
		
	}
	
	return ERR_NONE;
}
