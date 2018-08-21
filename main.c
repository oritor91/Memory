#include "mem_sim.h"

int main()
{
	struct sim_database * mem_sim = init_system("exe_file","swap_file",1,1,1);

	char val;

	
	printf("LOADING 64\n");
	val = load (mem_sim , 64);
	printf("Char [%c] LOADED FROM 64\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);
	
	printf("LOADING 66\n");
	val = load (mem_sim , 66);
	printf("Char [%c] LOADED FROM 66\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);
	
	printf("LOADING 2\n");
	val = load (mem_sim , 2);
	printf("Char [%c] LOADED FROM 2\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);
	
	printf("STORING 'X' to 98\n");
	store(mem_sim , 98,'X');
	print_memory(mem_sim);
	print_swap(mem_sim);
	
	printf("LOADING 16\n");
	val = load (mem_sim ,16);
	printf("Char [%c] LOADED FROM 16\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);
	
	printf("LOADING 70\n");
	val = load (mem_sim ,70);
	printf("Char [%c] LOADED FROM 70\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);
	
	printf("STORING 'Y' to 32\n");
	store(mem_sim ,32,'Y');
	print_memory(mem_sim);
	print_swap(mem_sim);
	
	printf("STORING 'Z' to 15\n");
	store (mem_sim ,15,'Z');
	print_memory(mem_sim);
	print_swap(mem_sim);
	
	printf("LOADING 23\n");
	val = load (mem_sim ,23);
	printf("Char [%c] LOADED FROM 23\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);

	printf("LOADING 23\n");
	val = load (mem_sim ,23);
	printf("Char [%c] LOADED FROM 23\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);

	printf("LOADING 0\n");
	val = load (mem_sim ,0);
	printf("Char [%c] LOADED FROM 0\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);

	printf("LOADING 5\n");
	val = load (mem_sim ,5);
	printf("Char [%c] LOADED FROM 5\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);
	
	printf("LOADING 98\n");
	val = load (mem_sim ,98);
	printf("Char [%c] LOADED FROM 98\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);

	printf("STORING '~' to 99\n");
	store (mem_sim ,99,'~');
	print_memory(mem_sim);
	print_swap(mem_sim);

	printf("LOADING 98\n");
	val = load (mem_sim ,98);
	printf("Char [%c] LOADED FROM 98\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);


	printf("LOADING 10\n");
	val = load (mem_sim ,10);
	printf("Char [%c] LOADED FROM 10\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);

	printf("LOADING 45\n");
	val = load (mem_sim ,45);
	printf("Char [%c] LOADED FROM 45\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);

	printf("LOADING 50\n");
	val = load (mem_sim ,50);
	printf("Char [%c] LOADED FROM 50\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);

	printf("LOADING 55\n");
	val = load (mem_sim ,55);
	printf("Char [%c] LOADED FROM 55\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);

	printf("LOADING 98\n");
	val = load (mem_sim ,98);
	printf("Char [%c] LOADED FROM 98\n", val);
	print_memory(mem_sim);
	print_swap(mem_sim);




	print_page_table(mem_sim);


	clear_system(mem_sim); 
}