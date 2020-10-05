//
// Created by root on 8/10/20.
//


#include "custom_unistd.h"
#include <stdio.h>
#include <stdint.h>

void setup_fences(struct memblock_t* mem )
{
    for(int i = 0;i < FENCE ;i++)
    {
       mem->tab_fence[i] = 4;
    }
}
int check_fences()
{
    struct memblock_t* mem = mm_handler.head;
    while(mem)
    {
        for(int i = 0; i < FENCE;i++)
        {
            if(mem->tab_fence[i]!= 4)
            {
                return -1;
            }
        }
        mem = mem->next;
    }
    return 0;
}

int sum_control(struct memblock_t* mem)
{
    int sum = 0;
    sum = mem->size + mem->real_size + mem->free_flag ;
    for(int i = 0; i < FENCE;i++)
    {
        sum += mem->tab_fence[i];
    }
    return sum;
}

int heap_setup(void)
{
    //Ten zabieg daje nam początek bloku pamięci
    // wkaznik na poczatkowy plotek
    if(setup_used == 1)
        return SETUP_USED;
    else {
        pthread_mutexattr_init(&Attr);
        pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&Mutex, &Attr);

        pthread_mutex_lock(&Mutex);
        struct memblock_t *begin = (struct memblock_t *) custom_sbrk(0);
        begin->free_flag = NOT_FREE;
        printf("begin %x\n", begin);
        int c = custom_sbrk(PAGE_SIZE);
        //jesli nie udalo nam sie uzyskac od systemu bloku pamieci to sbrk zwraca -1
        if (c == -1) {
            return -1;
        }
        mm_handler.memanager_flag = OK;

        // wskaznik na koncowy plotek

        struct memblock_t *end = (struct memblock_t *) (custom_sbrk(0) - sizeof(struct memblock_t));
        printf("end %x\n", end);

        end->free_flag = NOT_FREE;

        mm_handler.head = begin;
        mm_handler.tail = end;
        begin->prev = NULL;
        end->next = NULL;

        begin->size = 0;
        begin->real_size = 0;
        end->size = 0;
        end->real_size = 0;

        struct memblock_t *pointer = (begin + 1);

        pointer->real_size = PAGE_SIZE - 3 * (sizeof(struct memblock_t));
        pointer->size = PAGE_SIZE - 3 * (sizeof(struct memblock_t));


        pointer->pointer_data = begin + 2;
        pointer->free_flag = FREE;
        begin->next = pointer;
        pointer->prev = begin;
        end->prev = pointer;
        pointer->next = end;
        for (int i = 0; i < FENCE; i++)
        {
            mm_handler.head->tab_fence[i] = 4;
            mm_handler.tail->tab_fence[i] = 4;
            pointer->tab_fence[i] = 4;
        }
        pointer->control_sum = sum_control(pointer);
        mm_handler.tail->control_sum = sum_control(mm_handler.tail);
        mm_handler.head->control_sum = sum_control(mm_handler.head);
        setup_used = 1;
        pthread_mutex_unlock(&Mutex);

    }

    return 0;
}

void* print_all_memblock_info(struct memmanager_handler_t* handler)
{
    struct memblock_t* head = handler->head;
    long int sum = 0;

    while(head)
    {
        printf("## Block: %p Next: %p Prev: %x Pointer: %p  Size:  %d Real size: %d Sum control %d ",head, head->next,head->prev,head->pointer_data,head->size,head->real_size,head->control_sum);
        sum += head->real_size + sizeof(struct memblock_t);
        if(head->free_flag == FREE)
        {
            printf("Free: YES FENCES:");
        }
        else
            printf("Free : NO FENCES: ");
        for(int i = 0; i < FENCE; i++)
        {
            printf("%d",head->tab_fence[i]);
        }
        printf(" ##\n");
        head = head->next;
    }
    printf("###SUMA : %ld\n",sum);
}


struct memblock_t* find_free_memblock(int size)
{

    assert(size > 0 && "zly size");
    assert(mm_handler.memanager_flag == OK);

    struct memblock_t* head = mm_handler.head;

    while(head)
    {
        if(head->free_flag == FREE && head->size >= size)
        {
            return head;
        }
        head = head->next;
    }

    return NULL;

}


void join_blocks()
{
    struct memblock_t* ptr = mm_handler.head;
    check_list();
    int flag = NOT_JOINED;
    while( ptr != mm_handler.tail)
    {
        assert(ptr && "ptr null");
        if(ptr->free_flag == FREE && ptr->next->free_flag == FREE)
        {
            flag = JOINED;
            ptr->size += ptr->next->size + sizeof(struct memblock_t);
            ptr->real_size +=ptr->next->size + sizeof(struct memblock_t);
            ptr->next->next->prev = ptr;
            ptr->next = ptr->next->next;

        }
        ptr = ptr->next;
    }
    if(flag == JOINED)
    {
        join_blocks();
    }
}

//do usuniecia
void check_list()
{
    assert(mm_handler.tail != NULL);
    assert(mm_handler.head != NULL && "head jest nullem");
    struct memblock_t* ptr = mm_handler.head;
    while( ptr != mm_handler.tail)
    {
        if ( ptr != mm_handler.head ){
            assert(ptr == ptr->prev->next);
        }
        if ( ptr != mm_handler.tail ){
            assert(ptr == ptr->next->prev);
        }
        ptr = ptr->next;
    }

}
void heap_split(struct memblock_t* mem, int size)
{
    assert(mem && "mem jest nullem");
    //printf("MEM BEGIN %p\n",mem);
    if(mem->real_size >=(size + sizeof(struct memblock_t) + SIZE_OF_WORD))
    {
        struct memblock_t* new_memblock = (intptr_t )mem + (intptr_t )size + (intptr_t )sizeof(struct memblock_t);


        new_memblock->size = mem->real_size - size - sizeof(struct memblock_t);
        new_memblock->real_size = mem->real_size - size - sizeof(struct memblock_t);

        new_memblock->pointer_data = new_memblock + 1;
        new_memblock->free_flag = FREE;
        new_memblock->prev = mem;
        new_memblock->next = mem->next;

        //plotki i sumy
        setup_fences(new_memblock);
        new_memblock->control_sum = sum_control(new_memblock);


        mem->next->prev = new_memblock;

        mem->size = size;
        mem->real_size = size;
        mem->free_flag = NOT_FREE;
        mem->next = new_memblock;
        mem->control_sum = sum_control(mem);
    }

   else
        return;
}

int heap_extension(int size)
{
    //metadane sa dodawane do zadanego rozmiaru
    size = (size+sizeof(struct memblock_t));
    int s = size/ PAGE_SIZE;
    int s_modulo = size % PAGE_SIZE;
    if(s_modulo != 0)
    {
        s += 1;
    }
    int c = custom_sbrk(s*PAGE_SIZE);
    if(c == -1)
        return LACK_OF_MEM;

  struct memblock_t* new_end = (struct memblock_t*)custom_sbrk(0) - 1;
  new_end->next = NULL;
  new_end->prev = mm_handler.tail;
  mm_handler.tail->next = new_end;
  new_end->free_flag = NOT_FREE;
  mm_handler.tail->free_flag = FREE;
  mm_handler.tail->size = s*PAGE_SIZE - sizeof(struct memblock_t);
    mm_handler.tail->real_size = s*PAGE_SIZE - sizeof(struct memblock_t);
  mm_handler.tail = new_end;
  mm_handler.tail->size = 0;
    mm_handler.tail->real_size = 0;
    //plotki i sumy
    setup_fences(new_end);
    new_end->control_sum = sum_control(new_end);
    // łaczenie wolnych blokow
    join_blocks();
    return SUCESS_SBRK;

}
void* heap_malloc(size_t count)
{
    pthread_mutex_lock(&Mutex);
    //udalo sie i przydzielilismy bufor i zwracamy wskaznik na niego
    struct memblock_t* mem = find_free_memblock(count);
    if(mem == NULL)
    {
        // nie udalo sie i odwolujemt sie do sbrk
        int check = heap_extension(count);
        if(check == LACK_OF_MEM)
        {
            printf("LACK OF MEMORY");
            return NULL;
        }
            //nie udalo sie NULL
        else
            mem = find_free_memblock(count);
    }

    heap_split(mem, count);

    mem->free_flag = NOT_FREE;
    pthread_mutex_unlock(&Mutex);
    return mem->pointer_data;

}



int clear_space(int8_t* pvalue, unsigned int num)
{
    unsigned int i = 0;
    while(i!=num)
    {
        *(pvalue + i) = 0;
        i++;
    }
}

// void* calloc (size_t num, size_t size);
// num  Number of elements to allocate.
//size Size of each element.
void* heap_calloc(size_t number, size_t size)
{
    pthread_mutex_lock(&Mutex);
    void* wsk = heap_malloc(size*number);
    clear_space(wsk,number*size);
   pthread_mutex_unlock(&Mutex);
    return wsk;
}

void heap_free(void* memblock)
{
    pthread_mutex_lock(&Mutex);
    assert(memblock && "memblock null!");
    struct memblock_t*  wsk = (struct memblock_t*)memblock - 1;
    wsk->free_flag = FREE;
    join_blocks();
    wsk->real_size = wsk->size;
    return_area();
    wsk->control_sum = sum_control(wsk);
    pthread_mutex_unlock(&Mutex);
}

void* heap_realloc(void* memblock, size_t size)
{
    pthread_mutex_lock(&Mutex);
    assert(size >= 0 && "zly input");
    assert(memblock && "memblock null!!!!");
    if(size == 0)
    {
        heap_free(memblock);
    }
    struct memblock_t* wsk_mem = (struct memblock_t* )memblock - 1;
    size_t c = wsk_mem->size;
    //jesli jednak nie zmieniamy rozmiaru
    if(size == wsk_mem->size)
        return wsk_mem->pointer_data;
    if(size > wsk_mem->size) {
        //1.czy nastepny blok wolny i czy starczy nam tego co chcielismy rozszwrzyc
        if (wsk_mem->next->free_flag == FREE && (wsk_mem->size + wsk_mem->next->size + sizeof(struct memblock_t)) >= size) {
            //lacze recznie poniewaz w join blocks nie podaje parametrow a moj block realokowany nie jest free wiec funkcja by go nie wychwycila
            wsk_mem->size += wsk_mem->next->size + sizeof(struct memblock_t);
            wsk_mem->next->next->prev = wsk_mem;
            wsk_mem->next = wsk_mem->next->next;
            if (wsk_mem->size  > size)
            {
                heap_split(wsk_mem,size);
            }
            return wsk_mem - 1;
        }
        //2. nie starczy nam miejsca albo nie jest wolny ten blok ale znajduemy wystarczajaco duzy blok ale znajdujemy taki duzy blok i kopiujemy dane i zwracamy wskaznik a poprzednie miejsce ktore uzywalismy robimy na free
        else
        {
            struct memblock_t* new = heap_malloc(size);
            memcpy(new,wsk_mem->pointer_data,wsk_mem->size);
            heap_free(wsk_mem->pointer_data);
            wsk_mem->control_sum = sum_control(wsk_mem);
            return new;
        }
    }
    //chcemy zmniejszyc zaalokowany blok
    else
    {
        // zmniejszamy i zostanie miejsca chociaz na strukture + sizeofword
        if((wsk_mem->size - size) >= (sizeof(struct memblock_t) + SIZE_OF_WORD))
        {
            heap_split(wsk_mem,size);

        }
        // zmniejszamy i nie zostanie miejsca nawet na strukture
        else
        {
            wsk_mem->size = size;
        }

    }
    pthread_mutex_unlock(&Mutex);

}

/*W powyższych przykładach symbole __FILE__ oraz __LINE__ zastępowane są przez preprocesor języka
        C/C++ odpowiednio nazwą pliku (np. "test.c" typu const char*) oraz numerem linii (np. 42 typu int)*/
void* heap_malloc_debug(size_t count, int fileline, const char* filename)
{
    pthread_mutex_lock(&Mutex);
    assert(fileline >= 0 && "bad fileline");
    assert(filename && "filename is null");
    struct memblock_t* wsk_mem = heap_malloc(count) - sizeof(struct memblock_t);
    assert(wsk_mem && "BŁĄD");
    wsk_mem->fileline = fileline;
    wsk_mem->filename = filename;
    pthread_mutex_unlock(&Mutex);
    return wsk_mem->pointer_data;

}

void* heap_calloc_debug(size_t number, size_t size, int fileline,const char* filename)
{
    pthread_mutex_lock(&Mutex);
    assert(fileline >= 0 && "bad fileline");
    assert(filename && "filename is null");
    struct memblock_t* wsk_mem = heap_calloc(number,size) - sizeof(struct memblock_t);
    assert(wsk_mem && "BŁĄD");
    wsk_mem->fileline = fileline;
    wsk_mem->filename = filename;
    pthread_mutex_unlock(&Mutex);
    return wsk_mem->pointer_data;
}
void* heap_realloc_debug(void* memblock, size_t size, int fileline,const char* filename)
{
    pthread_mutex_lock(&Mutex);
    assert(fileline >= 0 && "bad fileline");
    assert(filename && "filename is null");
    void* mem =  heap_realloc(memblock,size);
    struct memblock_t* wsk_mem = mem - sizeof(struct memblock_t);
    assert(wsk_mem && "BŁĄD");
    wsk_mem->pointer_data = mem;
    wsk_mem->fileline = fileline;
    wsk_mem->filename = filename;
    pthread_mutex_unlock(&Mutex);
    return wsk_mem->pointer_data;
}

void* find_free_aligned(size_t count)
{
    assert(count > 0 && "zly size");
    assert(mm_handler.memanager_flag == OK);

    struct memblock_t* head = mm_handler.head;
    int c = 0;

   while(head)
   {
       if(head->free_flag == FREE)
       {
           if(head->size >= count)
               if(IS_PAGE_ALIGNED(head->pointer_data))
               {
                   return head;
               }
       }

       head = head->next;
   }
   head = mm_handler.head;
   struct memblock_t* prev_block = head;
   while(head)
   {
       if(head->free_flag == FREE)
       {
           if(head->size >= count+PAGE_SIZE)
           {
               intptr_t p1 = head;
               struct memblock_t* p1_next = head->next;
               long int n_of_pages = p1/PAGE_SIZE;
               //p1 adres na metadane taki ze pointer bedzie wqyrownany do strony
               p1 = n_of_pages*PAGE_SIZE + PAGE_SIZE- sizeof(struct memblock_t);
               head = p1;
               if (IS_PAGE_ALIGNED(head->pointer_data)) {
                   prev_block->next = head;
                   head->prev =  prev_block;
                   head->next = p1_next;
                   p1_next->prev = head;
                  // prev_block->real_size += head - (prev_block-1) + prev_block->size;
                 prev_block->real_size = ((long long int)head - (long long int)head->prev) -sizeof(struct memblock_t);
                 head->pointer_data = head + 1;
                 head->size = count;
                 head->real_size = ((long long int)head->next - (long long int)head) - sizeof(struct memblock_t);
                   return head;
               }
           }
       }
       prev_block = head;
       head = head->next;

   }
   return_area();
    return NULL;
}


int heap_extension_aligned(int size)
{
    //metadane sa dodawane do zadanego rozmiaru
    size = (size+sizeof(struct memblock_t));
    int s = size/ PAGE_SIZE;
    int s_modulo = size % PAGE_SIZE;
    if(s_modulo != 0)
    {
        s += 1;
    }
    // dodajemy strone ze wzgledu na wyrownanie
    s ++;
    int c = custom_sbrk(s*PAGE_SIZE);
    if(c == -1)
        return LACK_OF_MEM;

    struct memblock_t* new_end = (struct memblock_t*)custom_sbrk(0) - 1;
    setup_fences(new_end);
    new_end->next = NULL;
    new_end->prev = mm_handler.tail;
    mm_handler.tail->next = new_end;
    new_end->free_flag = NOT_FREE;
    new_end->control_sum = sum_control(new_end);
    mm_handler.tail->free_flag = FREE;
    mm_handler.tail->size = s*PAGE_SIZE - sizeof(struct memblock_t);
    mm_handler.tail->real_size = s*PAGE_SIZE - sizeof(struct memblock_t);
    mm_handler.tail = new_end;
    mm_handler.tail->size = 0;
    mm_handler.tail->real_size = 0;
    // łaczenie wolnych blokow
    join_blocks();
    return SUCESS_SBRK;

}
void* heap_malloc_aligned(size_t count)
{
    pthread_mutex_lock(&Mutex);
    //trywialny przypadek
    struct memblock_t* wsk =  find_free_aligned(count);
    if(wsk == NULL)
    {
        heap_extension_aligned(count);
         wsk =  find_free_aligned(count);
        if(wsk == NULL)
        {
            return NULL;
        }
    }

    wsk->free_flag = NOT_FREE;
    //robimy split po prawej stronie
    heap_split(wsk, count);
    //robimy split po lewej stronie
    heap_split(wsk->prev,wsk->prev->size);
    pthread_mutex_unlock(&Mutex);
    return wsk->pointer_data;

}

void* heap_calloc_aligned(size_t number, size_t size)
{
    pthread_mutex_lock(&Mutex);
    void* wsk = heap_malloc_aligned(size*number);
    struct memblock_t* wsk_mem = wsk + sizeof(struct memblock_t);
    if(wsk == NULL)
        return NULL;
    clear_space(wsk,number*size);
    pthread_mutex_unlock(&Mutex);
    return wsk;
}

void* heap_realloc_aligned(void* memblock, size_t size)
{
    pthread_mutex_lock(&Mutex);
    void* mem = heap_malloc_aligned(size);
    struct memblock_t* new = mem + sizeof(struct memblock_t);
    memcpy(new,memblock,size);
    heap_free(memblock);
    pthread_mutex_unlock(&Mutex);
    return mem;
}

void* heap_malloc_aligned_debug(size_t count, int fileline,const char* filename)
{
    pthread_mutex_lock(&Mutex);
    assert(fileline >= 0 && "bad fileline");
    assert(filename && "filename is null");
    struct memblock_t* wsk_mem = heap_malloc_aligned(count) - sizeof(struct memblock_t);
    assert(wsk_mem && "BŁĄD");
    wsk_mem->fileline = fileline;
    wsk_mem->filename = filename;
    pthread_mutex_unlock(&Mutex);
    return wsk_mem->pointer_data;
}
void* heap_realloc_aligned_debug(void* memblock, size_t size, int fileline,const char* filename)
{
    pthread_mutex_lock(&Mutex);
    assert(fileline >= 0 && "bad fileline");
    assert(filename && "filename is null");
    struct memblock_t* wsk_mem = heap_realloc_aligned(memblock,size) - sizeof(struct memblock_t);
    assert(wsk_mem && "BŁĄD");
    wsk_mem->fileline = fileline;
    wsk_mem->filename = filename;
    pthread_mutex_unlock(&Mutex);
    return wsk_mem->pointer_data;
}
void* heap_calloc_aligned_debug(size_t number, size_t size, int fileline,const char* filename)
{
    pthread_mutex_lock(&Mutex);
    assert(fileline >= 0 && "bad fileline");
    assert(filename && "filename is null");
    struct memblock_t* wsk_mem = heap_calloc_aligned(number,size) - sizeof(struct memblock_t);
    assert(wsk_mem && "BŁĄD");
    wsk_mem->fileline = fileline;
    wsk_mem->filename = filename;
    pthread_mutex_unlock(&Mutex);
    return wsk_mem->pointer_data;
}


size_t heap_get_used_space(void)
{
    struct memblock_t* head= mm_handler.head;
    // nie moze byc ujemny
    size_t size = 0;
    while(head)
    {
        size += sizeof(struct memblock_t);
        if(head->free_flag == NOT_FREE)
            size += head->size;
        head = head->next;
    }
    return size;
}
size_t heap_get_largest_used_block_size(void)
{
    size_t max = 0;
    struct memblock_t* head = mm_handler.head;
    while(head)
    {
        if(head->free_flag == NOT_FREE && head->size > max)
        {
            max = head->size;
        }
        head = head->next;
    }
    return max;

}
uint64_t heap_get_used_blocks_count(void)
{
    uint64_t size = 0;
    struct memblock_t* head = mm_handler.head;
    while(head)
    {
        if(head->free_flag == NOT_FREE )
        {
           size++;
        }
        head = head->next;
    }
    //poniewz head i tail sa notfree a ich nie bierzemy pod uwage
    return size-2;
}
size_t heap_get_free_space(void)
{
    size_t  size = 0;
    struct memblock_t* head = mm_handler.head;
    while(head)
    {
        if(head->free_flag == FREE )
        {
            size+=head->size;
        }
        head = head->next;
    }
    return size;
}
size_t heap_get_largest_free_area(void)
{
    size_t max = 0;
    struct memblock_t* head = mm_handler.head;
    while(head)
    {
        if(head->free_flag == FREE && head->size > max)
        {
            max = head->size;
        }
        head = head->next;
    }
    return max;
}

uint64_t heap_get_free_gaps_count(void)
{
    uint64_t size = 0;
    struct memblock_t* head = mm_handler.head;
    while(head)
    {
        if(head->free_flag == FREE)
            size++;
        head = head->next;
    }
    return size;
}

enum pointer_type_t get_pointer_type(const const void* pointer)
{
    if(pointer == NULL)
        return pointer_null;
    if(pointer < mm_handler.head || pointer > mm_handler.tail)
    {
        if(pointer < mm_handler.head)
            head = 1;
        else
            head = -1;
        return pointer_out_of_heap;
    }
    struct memblock_t* head = mm_handler.head;
    while(head)
    {

        if(pointer >= head && pointer < (head + 1))
        {
            return pointer_control_block;
        }

        if( head->pointer_data == pointer )
        {
            return pointer_valid;
        }

        if( head->pointer_data <= pointer && pointer < head->next && head->free_flag == FREE)
        {
            return pointer_unallocated;
        }

        if( head->pointer_data <= pointer && pointer < head->next)
        {
            return pointer_inside_data_block;
        }

        head = head->next;
    }


}

void* heap_get_data_block_start(const void* pointer)
{
    if(get_pointer_type(pointer) != pointer_inside_data_block && get_pointer_type(pointer) != pointer_valid)
        return NULL;
    if(get_pointer_type(pointer) == pointer_valid)
        return pointer;
    struct memblock_t*head = mm_handler.head;
    while(head)
    {
        if( head->pointer_data <= pointer && pointer < head->next)
        {
            return head->pointer_data;
        }
        head = head->next;
    }

}

size_t heap_get_block_size(const const void* memblock)
{
    if(get_pointer_type(memblock) != pointer_valid)
    {
        return 0;
    }
    struct memblock_t* ptr = memblock - sizeof(struct memblock_t);
    return ptr->size;int heap_validate(void);
}


void return_area(void)
{
    if(mm_handler.tail->prev->free_flag == FREE && mm_handler.tail->prev->size >= PAGE_SIZE)
    {
        struct memblock_t* before_tail = mm_handler.tail->prev;
        int count_of_pages = before_tail->size/PAGE_SIZE;
        int new_size = before_tail->size - PAGE_SIZE*count_of_pages;
        if(new_size >= SIZE_OF_WORD)
        {
            before_tail->size = new_size;
            before_tail->real_size = new_size;
        }
        else
        {
            before_tail->size = 0;
            before_tail->real_size = before_tail->size - PAGE_SIZE*count_of_pages;

        }
       void* me =  custom_sbrk(0);
        int c = custom_sbrk(-count_of_pages*PAGE_SIZE);
        void* lol = custom_sbrk(0);
        before_tail->next = custom_sbrk(0)-sizeof(struct memblock_t);
        mm_handler.tail = custom_sbrk(0)-sizeof(struct memblock_t);
        mm_handler.tail->next = NULL;
        mm_handler.tail->size = 0;
        mm_handler.tail->real_size = 0;
        mm_handler.tail->free_flag = NOT_FREE;
        mm_handler.tail->prev = before_tail;
    }

}

int heap_validate(void)
{
    pthread_mutex_lock(&Mutex);
    struct memblock_t*head = mm_handler.head;
    if(mm_handler.tail == NULL || mm_handler.head == NULL)
    {
        return HEAD_TAIL_NULL;
    }
    while(head)
    {

       if((head != mm_handler.tail && head != mm_handler.head))
        {
           if(head + 1 != head->pointer_data)
            return BAD_POINTERS ;
        }
        if((head != mm_handler.tail && head != mm_handler.head))
        {
            if((head->pointer_data + head->real_size != head->next))
            return BAD_POINTERS;
        }
        if(head->control_sum != sum_control(head))
        {
            printf(" jest: %d powinno być %d \n",head->control_sum,sum_control(head));
            return BAD_CONTROL_SUM;
        }
        if(head->free_flag != FREE && head->free_flag != NOT_FREE)
        {
            return WRONG_FLAG;
        }
        if(check_fences()!= 0)
        {
            return BAD_FENCES;
        }

        if(head->real_size < head->size)
        {
            return WRONG_SIZE;
        }

        if ( head != mm_handler.head )
        {
            if(head != head->prev->next)
                return WRONG_LIST;
        }
        if ( head != mm_handler.tail )
        {
            if(head != head->next->prev)
                return WRONG_LIST;
        }

        head = head->next;

    }
    pthread_mutex_unlock(&Mutex);
    return EVRY_OK;
}

void heap_dump_debug_information(void)
{
    struct memblock_t* head = mm_handler.head;
    long int sum = 0;

    while(head)
    {
        printf("## Block: %p Pointer to data: %p  Size:  %d   Name of file:  %s Num of line: %d ## \n",head,head->pointer_data,head->size,head->filename,head->fileline);
        sum += head->real_size + sizeof(struct memblock_t);

        head = head->next;
    }

    printf("##Size of HEAP: %ld   NOT_FREE BYTES: %d FREE BYTES : %d  THE BIGGEST FREE BLOCK SIZE : %d ##\n",sum,heap_get_used_space(),heap_get_free_space(),heap_get_largest_free_area());
}
