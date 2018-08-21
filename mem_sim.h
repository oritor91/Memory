
// *************** //
//  writing by    //
//  Ori Tor      //
//  305706699   // 
// *********** // 

#ifndef MEM_SIM_H
#define MEM_SIM_H 

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#define PAGE_SIZE 5
#define NUM_OF_PAGES 25
#define MEMORY_SIZE 20

#define TEXT 0         // define for the page_belong_to function 
#define DATA_BSS 1     
#define HEAP_STACK 2


typedef struct page_descriptor
{
unsigned int V; // valid
unsigned int D; // dirty
unsigned int P; // permission
int frame; //the number of a frame if in case it is page-mapped
} page_descriptor;

struct sim_database
{
	page_descriptor page_table[NUM_OF_PAGES]; //pointer to page table
	int swapfile_fd; //swap file swapfile_fd
	int program_fd; //executable file fd
	char main_memory[MEMORY_SIZE];
	int text_size;
	int data_bss_size;
	int heap_stack_size;
};

typedef struct fifo
{
	int frame;
	struct fifo* next;
}fifo;

void source_to_frame(struct sim_database *,int,int,int);                          
void update_page_table(struct sim_database*,int ,int ,int ,int);
void error(char *,struct sim_database*);
bool ram_is_full(struct sim_database*);
int locate_free_frame(struct sim_database*);
struct sim_database* init_system(char *, char*, int, int, int);
void print_page_table(struct sim_database*);                                         // ************************  //
char load(struct sim_database*,int);												//    FUNCTION DECLERATION   //
void print_memory(struct sim_database *);                                          // ************************  //
void print_swap(struct sim_database *);
void print_page_table(struct sim_database *);
void clear_system(struct sim_database *);
void print_free_frames();
void add_to_fifo(int);
int remove_from_fifo();
void print_fifo();
int page_belong_to(struct sim_database*,int);
void bring_from_swap(struct sim_database*,int,int,int);
void init_new_page(struct sim_database*,int,int);
void frame_to_swap(struct sim_database *,int,int);
void change_d(struct sim_database *);
void store(struct sim_database *, int, char);
void free_queue();

#endif