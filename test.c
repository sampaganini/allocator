//
// Created by root on 8/27/20.
//


#include "test.h"
#include "stdio.h"

//TESTY DANE

void test_too_large()
{
    // inicjalizacja sterty
    int status = heap_setup();
    assert(status == 0);

// parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    void *p1 = heap_malloc(8 * 1024 * 1024); // 8MB
    void *p2 = heap_malloc(8 * 1024 * 1024); // 8MB
    void *p3 = heap_malloc(8 * 1024 * 1024); // 8MB
    void *p4 = heap_malloc(45 * 1024 * 1024); // 45MB
    assert(p1 != NULL); // malloc musi się udać
    assert(p2 != NULL); // malloc musi się udać
    assert(p3 != NULL); // malloc musi się udać
    assert(p4 != NULL); // nie ma prawa zadziałać
// Ostatnia alokacja, na 45MB nie może się powieść,
// ponieważ sterta nie może być aż tak
// wielka (brak pamięci w systemie operacyjnym).

    status = heap_validate();
    assert(status == 0); // sterta nie może być uszkodzona

// zaalokowano 3 bloki
    assert(heap_get_used_blocks_count() == 3);

// zajęto 24MB sterty; te 2000 bajtów powinno
// wystarczyć na wewnętrzne struktury sterty
    assert(
            heap_get_used_space() >= 24 * 1024 * 1024 &&
            heap_get_used_space() <= 24 * 1024 * 1024 + 2000
    );

// zwolnij pamięć
    heap_free(p1);
    heap_free(p2);
    heap_free(p3);

// wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

// już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);

}

// MOJE TESTY :

//Test ma na celu przetestowanie funkcji malloc
void test_malloc()
{
    //inicjalizacja sterty;
    int status = heap_setup();
    assert(status == 0);

// parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    void *p1 = heap_malloc(8 * 1024 * 1024); // 8MB
    void *p2 = heap_malloc(8 * 1024 * 1024); // 8MB
    void *p3 = heap_malloc(8 * 1024 * 1024); // 8MB

    assert(p1 != NULL); // malloc musi się udać
    assert(p2 != NULL); // malloc musi się udać
    assert(p3 != NULL); // malloc musi się udać


    status = heap_validate();
    assert(status == 0); // sterta nie może być uszkodzona

// zaalokowano 3 bloki
    assert(heap_get_used_blocks_count() == 3);

// zajęto 24MB sterty; te 2000 bajtów powinno
// wystarczyć na wewnętrzne struktury sterty
    assert(
            heap_get_used_space() >= 24 * 1024 * 1024 &&
            heap_get_used_space() <= 24 * 1024 * 1024 + 2000
    );

// zwolnij pamięć
    heap_free(p1);
    heap_free(p2);
    heap_free(p3);

// wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

// już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);

}


// TEST ma na celu przetestowanie funkcji calloc min czy zeruje alokowana pamiec.
void test_calloc()
{
    // inicjalizacja sterty :
    int status = heap_setup();
    assert(status == 0);

// parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    // int uzyty aby swobodnie poruszac sie w petli i moc przypisac inty
    int *p1 = heap_calloc(8*1024,4);
    int *p2 = heap_calloc(8*1024 ,4);
    int  *p3 = heap_calloc(8*1024,4);


    assert(p1 != NULL); // calloc musi się udać
    assert(p2 != NULL); // calloc musi się udać
    assert(p3 != NULL); // calloc musi się udać

// sprawdzenie funkcją validate
    status = heap_validate();
    assert(status == 0);

// wskazniki na wczesniej zaalokowane bloki callocem
    int* wsk_p1 = p1;
    int* wsk_p2 = p2;
    int* wsk_p3 = p3;

// wpisanie danych w bloki pamieci:

    for(int i = 0; i < 8*1024;i++)
    {
        *(p1+i) = i;
        *(p2+i) = i;
        *(p3+i) = i;
    }

    assert(heap_get_used_blocks_count() == 3);

// zwalniamy pamiec
    heap_free(p1);
    heap_free(p2);
    heap_free(p3);

    // ponownie wywolujemy calloc
     p1 = heap_calloc(8*1024,4);
     p2 = heap_calloc(8*1024 ,4);
      p3 = heap_calloc(8*1024,4);

      // sprawdzamy czy calloc zeruje pamiec
    for(int i = 0; i < 8*1024;i++)
    {
        assert(*(p1+i) == 0);
        assert(*(p2+i) == 0);
        assert(*(p3+i) == 0);
    }

    // porownanie adresow p1 starego z p1 nowym
    assert(wsk_p1 == p1);
    assert(wsk_p2 == p2);
    assert(wsk_p3 == p3);

    heap_free(p1);
    heap_free(p2);
    heap_free(p3);

// wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

// już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);

}

//test w celu sprawdze ia funkcji free

void test_free()
{
    // inicjalizacja sterty
    int status = heap_setup();
    assert(status == 0);


    size_t free_bytes = heap_get_free_space();

 // alokujemy pamiec
    int *p1 = heap_calloc(8*1024,4);
    int* p2 = heap_calloc(8*1024 ,4);
    int  *p3 = heap_calloc(8*1024,4);
    int  *p4 = heap_calloc(8*1024,4);

// sprawdzam poprawnosc sterty
    status = heap_validate();
    assert(status == 0);

// zwalniam jeden blok
    heap_free(p2);
// sprawdzam czy teraz jest -1 zajetych blokow
    assert(heap_get_used_blocks_count() == 3);
    // sprawdzam czy przestrzen sie zwolnila
    assert(heap_get_largest_free_area()== (8*1024*4));

    // zwalniam kolejny blok
    heap_free(p3);

    // sprawdzam czy wolne bloki się polaczyly i mammy jeden duzy blok wolny 2*size
    assert(heap_get_largest_free_area()== 2*(8*1024*4)+ sizeof(struct memblock_t));

    // zwalniam reszte blokow
    heap_free(p1);
    heap_free(p4);

// sprawdezam czt wszystko wrocilo do normy
    int c = heap_get_free_space();
    assert(heap_get_free_space() == free_bytes);
}


// test sprawdzajacy dzialanie wszystkich funkcji typu debug
void test_debug()
{
    // inicjalizacja sterty
    int status = heap_setup();
    assert(status == 0);
    size_t free_bytes = heap_get_free_space();

// wszystko powinno zadzialac
    void *p1 = heap_malloc_debug(8,1,"file1");
    void* p2 = heap_calloc_debug(8,4,2,"file2");
    void* p3 = heap_malloc_debug(4,1,"file3");


    status = heap_validate();
    assert(status == 0);

    assert(heap_get_used_blocks_count() == 3);

    //sprawdzamy dzialanie realloc debug
    p3 = heap_realloc_debug(p3,8,1,"file3");

    // nadal powinny byc 3 bloki
    assert(heap_get_used_blocks_count() == 3);
    //zwalniamy wszystko
    heap_free(p1);
    heap_free(p2);
    heap_free(p3);

    // nie powinno zadzialac, nie podajemy prawidlowych parametrow !

    heap_malloc_debug(4,-1,NULL);

    assert(heap_get_free_space() == free_bytes);
}

// test funkcji realloc min czy gdy ma wystarczajacy blok pamieci za soba do rozszerzenia to wykorzysta to, a jesli nie to znajdzie nowa lokalizacje oraz czy dane zostana przeniesione

void test_realloc()
{
    // inicjalizacja sterty
    int status = heap_setup();
    assert(status == 0);

    size_t free_bytes = heap_get_free_space();

    // alokujemy mallocem
    int* p1 = heap_malloc(2*4);

    status = heap_validate();
    assert(status == 0);

    // zapisujemy dane
    int* p1_help = p1;
    *(p1) = 1;
    *(p1+1) = 2;

    assert(heap_get_used_blocks_count() == 1);

    //realokujemy pamiec
    p1 = heap_realloc(p1,8);

    // sprawdzamy czy dane sie zgadzaja
    assert(*(p1) == 1);
    assert(*(p1+1) == 2);

    assert(heap_get_used_blocks_count() == 1);

//sprawdzamy czy wskaznik zmienil swoje miejsce nie, poniewaz blok  mial miejsce na rozszerzenie sie
    assert(p1_help == p1);


//  alokujemy drugi blok ktory ,,ograniczy,, miejsce na rozszerzanie sie pierwszemu co spowoduje ze przy wywolaniu realloc wskaznik zmieni swoje miejsce
    void*p2 = heap_malloc(16);

    p1 = heap_realloc(p1,16);


   status = heap_validate();
    assert(status == 0);


// sprawdzamy czy blok zmirnil miejsce tak jak zakladalismy
    assert(p1 != p1_help);

    // sprawdzamy czy dane ,,przeniosly sie''
    assert(*(p1) == 1);
    assert(*(p1+1) == 2);

     heap_free(p1);
     heap_free(p2);

     //sprawdzamy czy wszystko wrocilo do ,,normy''
    assert(heap_get_free_space() == free_bytes);

}

//celem tego testu jest sprawdzenie czy funkcjee malloc,calloc, realloc zwracaja wskazniki dobrego typu
void is_valid()
{
    // inicjalizacja sterty
    int status = heap_setup();
    assert(status == 0);
    size_t free_bytes = heap_get_free_space();

    void* p1 = heap_malloc(4);
    void* p2 = heap_calloc(2,4);

    // sprawdzamy czy zwroce wskazniki sa dobrego typu czyli wskazuja na blok danych
    assert(get_pointer_type(p1) == pointer_valid);
    assert(get_pointer_type(p2) == pointer_valid);

    status = heap_validate();
    assert(status == 0);

// sprawdzamy czy zwroce wskazniki sa dobrego typu czyli wskazuja na blok danych
    p1 = heap_realloc(p1,8);

    assert(get_pointer_type(p1) == pointer_valid);

    status = heap_validate();
    assert(status == 0);


    heap_free(p1);
    heap_free(p2);

    //sprawdzamy czy wszystko wrocilo do ,,normy''
    assert(heap_get_free_space() == free_bytes);


}

// test sprawdzajacy poprawnosc zainicjalizowanej sterrty
void setup_check()
{
    int status = heap_setup();
    assert(status == 0);

    // sprawdzamy czy tail lub head nie sa nullami
    assert(mm_handler.tail && "tail is null");
    assert(mm_handler.head && "head is null");
// sprawdzamy poprawnosc flag taila oraz heada

    assert(mm_handler.tail->free_flag == NOT_FREE);
    assert(mm_handler.head->free_flag == NOT_FREE);

    // sprawdzamy poprawnosc wielkosci pozostalej wolnej przestrzeni sterty
    assert(heap_get_free_space() == PAGE_SIZE - 3*(sizeof(struct memblock_t)));

    status = heap_validate();
    assert(status == 0);

}


void too_big_wanted()
{
    int status = heap_setup();
    assert(status == 0);


    void* p1 = heap_malloc(67108864);

// zazadalismy za duzo wiec test sie nie wykona
    assert(p1 != NULL && "too big block wanted");

}

void test_calloc_aligned()
{
    // inicjalizacja sterty :
    int status = heap_setup();
    assert(status == 0);

// parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    // int uzyty aby swobodnie poruszac sie w petli i moc przypisac inty
    int *p1 = heap_calloc_aligned(5,4);

    assert(p1 != NULL); // calloc musi się udać

    status = heap_validate();
    assert(status == 0);
    heap_dump_debug_information();

    for(int i = 0;i < 5;i++)
    {
        printf("%d ",*(p1+i));
    }

// zwalniamy pamiec
    heap_free(p1);

// wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

// już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);

}


void test_malloc_aligned()
{
    //inicjalizacja sterty;
    int status = heap_setup();
    assert(status == 0);

// parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    void *p1 = heap_malloc_aligned(8 * 1024 * 1024); // 8MB

    heap_dump_debug_information();
    status = heap_validate();
    assert(status == 0); // sterta nie może być uszkodzona

    heap_free(p1);

// wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

// już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);

}


