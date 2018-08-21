#include "mem_sim.h"
// global arr to know which frame is free
int free_frames[MEMORY_SIZE/PAGE_SIZE] = {0};
fifo *head;


struct sim_database* init_system(char exe_file_name[], char swap_file_name[], int text_size,int data_bss_size, int heap_stack_size) // initializing the system according to the demands on the drill page.
{
	if(exe_file_name == NULL)
		return NULL;
	struct sim_database *mem_sim;
	mem_sim = (struct sim_database*) malloc(sizeof(struct sim_database)); // allocating memory for the struct.
	assert(mem_sim);

	mem_sim->swapfile_fd = open(swap_file_name,O_CREAT|O_TRUNC|O_RDWR,S_IRUSR|S_IWUSR);
	mem_sim->program_fd = open(exe_file_name,O_RDONLY);
	if(mem_sim->swapfile_fd < 0 || mem_sim->program_fd < 0) // if failed to open files for some reason we exits
		error("open file failed",mem_sim);
	mem_sim->text_size = text_size;
	mem_sim->data_bss_size = data_bss_size;
	mem_sim->heap_stack_size = heap_stack_size;
	int actual_text_size,actual_data_size,actual_heap_stack_size;  // 3 vars for the correct amount of papes for each section
	actual_text_size = text_size/PAGE_SIZE;
	actual_data_size = data_bss_size/PAGE_SIZE;
	actual_heap_stack_size = heap_stack_size/PAGE_SIZE;

	
	if(text_size%PAGE_SIZE != 0)    // if ( size % PAGE_SIZE ) is NOT 0 than we need one more page for this section.
		actual_text_size += 1;
	if(data_bss_size%PAGE_SIZE != 0)
		actual_data_size += 1;
	if(heap_stack_size%PAGE_SIZE != 0)
		actual_heap_stack_size +=1;

	int total_size = actual_text_size+actual_data_size+actual_heap_stack_size; // total amount of pages to init 
	for (int i = 0; i < NUM_OF_PAGES && i < total_size; i++)
	{
		if(i<actual_text_size) // if were in the text section than p is 0
			mem_sim->page_table[i].P = 0;
		else if(i<actual_text_size + actual_data_size + actual_heap_stack_size) // else p is 1 
			mem_sim->page_table[i].P = 1;

		mem_sim->page_table[i].frame = -1; // initializing according to demands 
		mem_sim->page_table[i].V = 0;
		mem_sim->page_table[i].D = 0;
	}

	for (int i = 0; i < MEMORY_SIZE; i++) // initalizing the main memory to 0 
		mem_sim->main_memory[i] = '0';

	for (int i = 0; i < MEMORY_SIZE/PAGE_SIZE; ++i) // free_frames is a private arr to know which frame is free , initialized to -1 at first.
		free_frames[i] = -1;

	return mem_sim;
}
char load(struct sim_database* mem_sim,int address) // loading whats in the address to the main memory and returning the value thats in this address 
{
	int physical_address = 0;
	int status=0; 
	int offset = address % PAGE_SIZE;        // calculating the offset and the page_number 
	int page_number = address / PAGE_SIZE;
	int frame;
	int total_size = mem_sim->text_size+mem_sim->data_bss_size+mem_sim->heap_stack_size;
	if(address < 0 || address >= total_size) // if address is invalid return \0
		return '\0';
	if(mem_sim->page_table[page_number].V == 1) // if valid == 1 the wanted page is in main memory just take it 
	{
		physical_address = mem_sim->page_table[page_number].frame*PAGE_SIZE + offset;
		return mem_sim->main_memory[physical_address];
	}
	else
	{
		if( !ram_is_full(mem_sim) )
		{
			frame = locate_free_frame(mem_sim); // locating a free frame because ram is not full.
			if(mem_sim->page_table[page_number].P == 0 ) // if p == 0 we need to take it from exe file.
			{
				source_to_frame(mem_sim,page_number,frame,mem_sim->program_fd); 
				update_page_table(mem_sim,page_number,1,0,frame); // updating the current page info .
			}
			else // p == 1 
			{
				if(mem_sim->page_table[page_number].D == 0) 
				{
					int position = page_belong_to(mem_sim,address); // finding in which section the page is at .
					if( position == DATA_BSS ) 
						source_to_frame(mem_sim,page_number,frame,mem_sim->program_fd); // taking the wanted page from the exe file
					else if( position == HEAP_STACK ) // heap_stack page means new_page on load --> init new page with d =0 
						init_new_page(mem_sim,page_number,frame);
					// finally
					update_page_table(mem_sim,page_number,1,0,frame); // updating anyway the page info 
				}
				else // D == 1 ;
				{
					source_to_frame(mem_sim,page_number,frame,mem_sim->swapfile_fd); // because d is 1 than we take it from swap file
					update_page_table(mem_sim,page_number,1,1,frame); // updating 
				}
			}
		}
		else // else means ram is full 
		{
			frame = remove_from_fifo(); // ram is full than we need to free one frame
			int page_removed = free_frames[frame]; // in each cell we stored the page number that seats on that frame so this is the page_to_remove
			if(mem_sim->page_table[page_removed].D == 1) // if the page_reomved is dirty than we transfer it to swap file and updating
			{
				frame_to_swap(mem_sim,page_removed,frame);
				update_page_table(mem_sim,page_removed,0,1,-1);
			}
			else // if not dirty than we just update the current info  
				update_page_table(mem_sim,page_removed,0,0,-1);

			free_frames[frame] = -1 ; // the frame is now free 
			load(mem_sim,address); // recursivley calling load but now its going to go to the ram is not full section .
		}
		
	}
	physical_address = mem_sim->page_table[page_number].frame*PAGE_SIZE + offset; // eventually calculating the physical address and returning it.
	return mem_sim->main_memory[physical_address];
}
void store(struct sim_database * mem_sim, int address, char value) // storing value into the right address 
{
	int physical_address = 0;
	int status=0; 
	int offset = address % PAGE_SIZE;         // calculating offset and page_number.
	int page_number = address / PAGE_SIZE;
	int frame;
	int total_size = mem_sim->text_size+mem_sim->data_bss_size+mem_sim->heap_stack_size;
	if(address < 0 || address >= total_size) // if address is invalid return
		return;
	if( page_belong_to(mem_sim,address) == TEXT) // if address is in text section than we cannot write on it. return 
		return;

	if(mem_sim->page_table[page_number].V == 1) // if valid == 1 the wanted page that is in main memory just store it 
	{
		physical_address = mem_sim->page_table[page_number].frame*PAGE_SIZE + offset;
		mem_sim->main_memory[physical_address] = value;
		update_page_table(mem_sim,page_number,1,1,mem_sim->page_table[page_number].frame); // updating d == 1
		return;
	}
	else // if not in memory
	{
		if( !ram_is_full(mem_sim) )
		{
			frame = locate_free_frame(mem_sim); // locating a free frame because ram is NOT full 
			if(mem_sim->page_table[page_number].D == 0)
			{
				int position = page_belong_to(mem_sim,address); 
				if( position == DATA_BSS ) 
					source_to_frame(mem_sim,page_number,frame,mem_sim->program_fd);	// d is 0 && DATA_BSS so we take it from exe file.		
				else if(position == HEAP_STACK ) // heap_stack page means new_page on store --> init new page
					init_new_page(mem_sim,frame,page_number);
			}
			else // D == 1 ;
				source_to_frame(mem_sim,page_number,frame,mem_sim->swapfile_fd); // if d is equal to 1 we take the wanted page from the swap file

		}
		else // else means ram is full 
		{
			frame = remove_from_fifo(); // we need to free a spot for the new page 
			int page_removed = free_frames[frame]; // this is the page we need to remove from the main memory
			if(mem_sim->page_table[page_removed].D == 1)
				frame_to_swap(mem_sim,page_removed,frame); // if dirty we remove the page to swap file otherwise we can overide 
			free_frames[frame] = -1 ; // now the frame is "free"
			update_page_table(mem_sim,page_removed,0,1,-1); // updating the page info 
			store(mem_sim,address,value); // recursivley calling store but now its gonna go to the ram NOT full part.
		}
		
	}
	update_page_table(mem_sim,page_number,1,1,frame); // eventually updating the new page info
	physical_address = mem_sim->page_table[page_number].frame*PAGE_SIZE + offset; 
	mem_sim->main_memory[physical_address] = value; // storing value in the physical address

	return;	
}
void frame_to_swap(struct sim_database * mem_sim,int page_number,int frame) // private function to remove a frame to the swap file .
{
	int status = 0;
	char str[PAGE_SIZE+1]; // page size + 1 for the 5 chars and '\0'
	for (int i = frame*PAGE_SIZE,j=0; i < (frame*PAGE_SIZE)+PAGE_SIZE; ++i) // copying the wanted page into the str.
		strncpy(&str[j++],&mem_sim->main_memory[i],1);
	status = lseek(mem_sim->swapfile_fd,page_number*PAGE_SIZE,SEEK_SET); // moving the lsink to the wanted location
	if(status != page_number*PAGE_SIZE)
		error("lseek failed",mem_sim);
	status = write(mem_sim->swapfile_fd,str,PAGE_SIZE); // writing PAGE_SIZE bytes to swap file 
	if(status != PAGE_SIZE)
		error("Error on write",mem_sim);
	return;
}
void source_to_frame(struct sim_database * mem_sim,int page_number,int frame,int source_fd) // sourde_fd can be the exe file or swap file.
{
	int status = 0;
	char str[PAGE_SIZE];
	status = lseek(source_fd,page_number*PAGE_SIZE,SEEK_SET); // moving the lsink to the wanted location
	if(status != page_number*PAGE_SIZE)
		error("lseek failed",mem_sim);
	status = read(source_fd,str,PAGE_SIZE); // reading PAGE_SIZE bytes
	if(status != PAGE_SIZE)
		error("Error on read",mem_sim);

	for (int i = frame*PAGE_SIZE,j=0; i < (frame*PAGE_SIZE)+PAGE_SIZE; ++i) // copying from str to main memory 
		strncpy(&mem_sim->main_memory[i],&str[j++],1);
	free_frames[frame] = page_number ; // updating the current frame to the correct page_number
	add_to_fifo(frame); // adding the new page to the queue
}
void update_page_table(struct sim_database* mem_sim,int page,int v,int d,int frame) // a simple function to update the page info
{
	mem_sim->page_table[page].V = v;
	mem_sim->page_table[page].D = d;
	mem_sim->page_table[page].frame = frame;
}

int locate_free_frame(struct sim_database* mem_sim) // locating free frames , if -1 its free otherwise we store the page_number that is in the frame.
{
	for (int i = 0; i < MEMORY_SIZE/PAGE_SIZE; i++)
		if(free_frames[i] == -1)
			return i;
		printf("couldnt locate free frame");
		return -1;	
}
void print_free_frames() // printing the free_frame array
{
		printf("Free frames:");
		for (int i = 0; i < MEMORY_SIZE/PAGE_SIZE; ++i)
		{
			printf("%d," ,free_frames[i]);
		}
		printf("\n");
}
void print_swap(struct sim_database* mem_sim) // asaf method 
{
	char str[PAGE_SIZE];
	int i,status;
	printf("\n Swap memory\n");
	status = lseek(mem_sim->swapfile_fd,0,SEEK_SET);
	while(read(mem_sim->swapfile_fd,str,PAGE_SIZE) == PAGE_SIZE)
	{
		for (int i = 0; i < PAGE_SIZE; ++i)
		{
			printf("[%c]\t", str[i]);
		}
		printf("\n");
	}
}
bool ram_is_full(struct sim_database* mem_sim) // private function to check if ram is full
{
		for (int i = 0; i < MEMORY_SIZE/PAGE_SIZE; ++i) // if free_frames[i] is equal to -1 than the main memory is not full 
			if(free_frames[i] == -1)
				return false;
			return true;
}
void print_page_table(struct sim_database* mem_sim) // assaf method 
{
			int i;
			printf("\n page table \n");
			printf("Valid\t Dirty\t Permission\t Frame\n");
			for (int i = 0; i < NUM_OF_PAGES; ++i)
			{
				printf("%d-[%d]\t[%d]\t[%d]\t\t[%d]\n",i,mem_sim->page_table[i].V,mem_sim->page_table[i].D,
					mem_sim->page_table[i].P,mem_sim->page_table[i].frame);
			}
}
void print_memory(struct sim_database * mem_sim) // assaf method 
{
	int i;
	printf("\n Physical memory\n");
	for (i = 0; i < MEMORY_SIZE; ++i)
		printf("[%c]\n", mem_sim->main_memory[i]);
}
void clear_system(struct sim_database * mem_sim) // clears the system before exiting 
{
	if(!mem_sim->program_fd) // if opened than close
		close(mem_sim->program_fd);
	if(!mem_sim->swapfile_fd) // if opened than close
		close(mem_sim->swapfile_fd);

	free_queue(); // private function to free the queue 
	free(mem_sim); // freeing the struct sim_database
}
void error(char *msg,struct sim_database* mem_sim) // private error to print messages 
{
	fprintf(stderr,"%s\n",msg);
	clear_system(mem_sim); // before exiting we clear the system
	exit(0);
}
void add_to_fifo(int frame) // adding the frame into the queue
{
	if(head == NULL) // if queue is empty than we add to head 
	{
		head = (fifo*)malloc(sizeof(fifo));
		head->frame = frame;
		head->next = NULL;
		return;
	}

	fifo *new = (fifo*)malloc(sizeof(fifo)); // otherwise allocating a new struct
	fifo *temp=head;
	while(temp->next != NULL) // iterating to the end of the queue (working in FIFO)
		temp = temp->next;

	temp->next = new; // adding the new page to the end of the queue 
	new->frame = frame;
	new->next = NULL;
	return;
}
int remove_from_fifo() // removing the first frame that entered the queue 
{
	fifo * fifo_temp = head; // remove from fifo removing the head (enters first)  and poiting the head.next
	int temp = head->frame;
	head = head->next;
	free(fifo_temp);
	return temp;
}
void print_fifo() // private function to print the queue
{
	fifo * temp = head ;
	printf("FIFO : ");
	while(temp != NULL)
	{
		printf("%d-->", temp->frame);
		temp = temp->next;
	}
	printf("\n");
}
int page_belong_to(struct sim_database* mem_sim, int slot) // finding in which sectiong the slot is at .
{
	int text_start = 0;
	int text_end = mem_sim->text_size-1;
	int data_bss_start = mem_sim->text_size;
	int data_bss_end = data_bss_start + mem_sim->data_bss_size-1;
	int heap_stack_start = mem_sim->text_size+mem_sim->data_bss_size;
	int heap_stack_end = heap_stack_start + mem_sim->heap_stack_size-1;

					// calculating each section where it starts and where it ends 	
					// returning according to the DEFINE we did in the header file
	if(slot < text_start || slot > heap_stack_end) 
		return -1;

	if(slot >= text_start && slot <= text_end)
		return TEXT;

	if(slot >= data_bss_start && slot <= data_bss_end)
		return DATA_BSS;

	if(slot >= heap_stack_start && slot <= heap_stack_end)
		return HEAP_STACK;

}
void init_new_page(struct sim_database* mem_sim, int frame,int page_number) // inits a new page (simulates malloc)
{
	for (int i = frame*PAGE_SIZE; i < (frame*PAGE_SIZE)+PAGE_SIZE; ++i)  // creating a PAGE_SIZE of '\0'
		mem_sim->main_memory[i] = '\0';
	add_to_fifo(frame); // adding the current frame to fifo 
	free_frames[frame] = page_number;
}
void free_queue()// private function that iterated through the queue and free each one of its members 
{								
	struct fifo* temp = head; 
	if(!temp)
		return;

	else if(temp->next == NULL)
		free(head);
	
	else
	{
		struct fifo* temp2 = head->next;
		while(temp2->next != NULL)
		{
			free(temp);
			temp = temp2;
			temp2 = temp2->next;
		}
		free(temp);
		free(temp2);
	}
}