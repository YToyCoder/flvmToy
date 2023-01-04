#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctime>

// Memory Manager faster than stdlib malloc and free
class MM {
  public:
    typedef uint8_t Unit_T;
    typedef union ListNodeT {ListNodeT*_link; char user_data[1];} ListNodeT;
  private:
    ListNodeT **free_list;
    uint8_t *_MM_chunk_start;
    uint8_t *_MM_chunk_end  ;
  public:
    enum {_Max_bytes = 128};
    enum {Align = 8};
    enum {_free_list_size = 16};
    static const void *nptr;

    MM(){
      free_list = (ListNodeT **) malloc(sizeof(ListNodeT *) * 16);
      for(int i=0; i<16; i++)
        free_list[i] = 0;
      _MM_chunk_start = 0;
      _MM_chunk_end   = 0;
    }

    void *alloc(size_t size){
      void *free = get_from_free_list(size);
      // printf("alloc one\n");
      return free;
    }

    void release(void *p, size_t _n){
      ListNodeT ** the_free_list = free_list + free_list_index(_n);
      ListNodeT * _p = (ListNodeT *)p;
      _p->_link = *the_free_list;
      *the_free_list = _p;
      // printf("release one\n");
    }

    private:

    void *get_from_free_list(size_t size){
      size_t ind = free_list_index(size);
      ListNodeT **the_free_list = free_list + ind;
      if(*the_free_list == nptr)
        return fill_free_list(size);
      ListNodeT *ret= *the_free_list;
      *the_free_list = ret->_link;
      // printf("get from free list \n");
      return ret;
    }

    void *fill_free_list(size_t size){
      size_t len = Round_up(size);
      size_t nb = 10;

      uint8_t *chunk = (uint8_t *)malloc_from_chunk(len, nb);

      if(nb == 1) return chunk;

      if(chunk == 0)
        return 0;

      ListNodeT **the_free_list = free_list + free_list_index(len);
      void *res = chunk;
      ListNodeT *current = (ListNodeT *)(chunk + len);
      ListNodeT *next;
      for(int i=1; i<nb; i++){
        next = current + 1;
        current->_link = *the_free_list;
        *the_free_list = current;
        current = next;
      }
      return res;
    }

    void *malloc_from_chunk(size_t len, size_t &nb){
      size_t total_bytes_need = len * nb;
      size_t bytes_left = _MM_chunk_end - _MM_chunk_start;
      void *result;

      if(bytes_left >= total_bytes_need){
        result = _MM_chunk_start;
        _MM_chunk_start += total_bytes_need;
        return result;
      }else if(bytes_left >= len){
        result = _MM_chunk_start;
        nb = bytes_left / len;
        _MM_chunk_start += len * nb;
        return result;
      }else {
        if(bytes_left > 0){
          ListNodeT **the_free_list = free_list + free_list_index(bytes_left);
          ((ListNodeT *) _MM_chunk_start)->_link = *the_free_list;
          *the_free_list = (ListNodeT *)_MM_chunk_start;
        }
        return malloc(total_bytes_need);
      }
    }

    size_t free_list_index(size_t size){
      return ((size + Align - 1) / Align - 1);
    }

    size_t Round_up(size_t need){
      return (need + (size_t) Align - 1) & ~((size_t)Align - 1);
    }

};

const void *MM::nptr = 0;
MM mm;

size_t size = 18;

class A {
  char *p;
  size_t s;
  public:
  A(size_t n){
    // p = (char *)mm.alloc(size);
    p = (char *)mm.alloc(s = (n % 2 == 0 ? size : size * 3));
    for(int i=0; i<size; i++){
      p[i] = 'A';
    }
  }

  void increase(){
    mm.release(p, s);
    p = (char *)mm.alloc(size * 2);
  }
  ~A(){
    mm.release(p, size * 2);
  }
};

class B {
  char *p;
  size_t s;
  public:
    B(int n){
      p = (char *)malloc(s = (n % 2 == 0 ? size : size * 3));
      for(int i=0; i<size; i++){
        p[i] = 'A';
      }
    }

    void increase(){
      free(p);
      p = (char *)malloc(size * 2);
    }
    ~B(){
      free(p);
    }
};

void print_time(){
  time_t t = time(nullptr);
  // printf(ctime(&t));
  printf("%d\n",t);
}

int main(int argc, char const *argv[])
{
  // int n = 1000000;
  // print_time();
  // for(int i=0; i<n; i++){
  //   A a(i);
  //   a.increase();
  // }
  // print_time();
  // for(int i=0; i<n; i++){
  //   B b(i);
  //   b.increase();
  // }
  // print_time();
  return 0;
}
