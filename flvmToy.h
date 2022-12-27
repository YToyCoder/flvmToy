#ifndef flvm_toy
#define flvm_toy
#include "jni.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
JNIEXPORT jint JNICALL  Java_com_fsilence_flToy_Main_run(JNIEnv *env, jobject obj);

#ifdef __cplusplus
}
#endif

namespace flvm {
  namespace core {
    template <typename E>
    class FlStack{
      private:
        E * elements;
        size_t capability; // max size
        size_t pos;
      public:
        FlStack(size_t max_size){
          // static_assert( max_size >= 0);
          capability = max_size;
          pos = -1;
        }

        E pop(){
          // static_assert( pos >= 0);
          return elements[pos--];
        }

        void push(const E el){
          // static_assert( capability > pos + 1 );
          elements[++pos] = el;
        }

        bool isEmpty(){
          return pos >= 0;
        }
    };
  }
}

#endif