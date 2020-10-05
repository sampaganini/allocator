//
// Created by root on 8/10/20.
//

#ifndef ALLOCATOR_CUSTOM_UNISTD_H
#define ALLOCATOR_CUSTOM_UNISTD_H


#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include  <pthread.h>

// czy wolny blok
#define FREE 0
#define NOT_FREE 1

#define PAGE_SIZE       4096    // Długość strony w bajtach
#define OK 2
#define NOT_OK 3
#define LACK_OF_MEM 4
#define SUCESS_SBRK 5
//laczone
#define JOINED 1
#define NOT_JOINED 0
// From https://ftp.samba.org/pub/unpacked/junkcode/alloc_mmap/alloc_mmap.c
#define IS_PAGE_ALIGNED(ptr) (((PAGE_SIZE-1) & (intptr_t)ptr) == 0)

#define SIZE_OF_WORD 0
//rozmiar tablicy do plotkow
#define FENCE 8

//deffinicje do validate:
#define HEAD_TAIL_NULL -1
#define WRONG_LIST -2
#define BAD_POINTERS -3
#define BAD_CONTROL_SUM -4
#define BAD_FENCES -5
#define WRONG_FLAG -6
#define WRONG_SIZE -7
#define EVRY_OK 0


//deklaracja do setup
#define SETUP_USED 6

struct memblock_t
{
    int tab_fence[8];
    struct memblock_t* next;
    struct memblock_t* prev;
    void* pointer_data;
    long int size;
    long int real_size;
    int free_flag;
    int fileline;
    const char* filename;
    int control_sum;
};

struct memmanager_handler_t
{
    struct memblock_t* head;
    struct memblock_t* tail;
    int memanager_flag;
}mm_handler;

enum pointer_type_t
{
    pointer_null,
    pointer_out_of_heap,
    pointer_control_block,
    pointer_inside_data_block,
    pointer_unallocated,
    pointer_valid
};

//do usuniecia
int head;

// WIELOWĄTKOWOŚĆ :

pthread_mutex_t Mutex;
pthread_mutexattr_t Attr;
int setup_used;

// otrzymane:
void* custom_sbrk(intptr_t delta);


// zwracanie pamieci do systemu:
void return_area(void);
void setup_fences(struct memblock_t* mem );
int  check_fences();
int sum_control(struct memblock_t* mem);


void* print_all_memblock_info(struct memmanager_handler_t*  handler);
int heap_setup(void);
struct memblock_t* find_free_memblock(int size);
void* heap_malloc(size_t count);
void heap_split(struct memblock_t* mem, int size);
void check_list();
void join_blocks();
void* heap_calloc(size_t number, size_t size);
int clear_space(int8_t* pvalue, unsigned int num);
void heap_free(void* memblock);
void* heap_realloc(void* memblock, size_t size);

// funkcje debugujace
void* heap_malloc_debug(size_t count, int fileline, const char* filename);
void* heap_calloc_debug(size_t number, size_t size, int fileline,const char* filename);
void* heap_realloc_debug(void* memblock, size_t size, int fileline,const char* filename);

//funckje aligned
void* heap_malloc_aligned(size_t count);
int heap_extension_aligned(int size);
void* heap_calloc_aligned(size_t number, size_t size);
void* heap_realloc_aligned(void* memblock, size_t size);

//funkcje aligned debugujace:
void* heap_malloc_aligned_debug(size_t count, int fileline,
                                const char* filename);
void* heap_realloc_aligned_debug(void* memblock, size_t size, int fileline,
                                 const char* filename);
void* heap_calloc_aligned_debug(size_t number, size_t size, int fileline,
                         const char* filename);

//funkcje analizujace:
size_t heap_get_used_space(void);
size_t heap_get_largest_used_block_size(void);
uint64_t heap_get_used_blocks_count(void);
size_t heap_get_free_space(void);
size_t heap_get_largest_free_area(void);
uint64_t heap_get_free_gaps_count(void);

//funkcje z pointertype:
enum pointer_type_t get_pointer_type(const const void* pointer);
void* heap_get_data_block_start(const void* pointer);
size_t heap_get_block_size(const const void* memblock);
int heap_validate(void);




void heap_dump_debug_information(void);

#if defined(sbrk)
#undef sbrk
#endif

#if defined(brk)
#undef brk
#endif


#define sbrk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), (void*)-1)
#define brk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), -1)
#endif //MY_ALLOCATOR_SO_CUSTOM_UNISTD_H
