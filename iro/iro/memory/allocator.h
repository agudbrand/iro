/*
 *  Allocator interface.
 */ 

#ifndef _iro_allocator_h
#define _iro_allocator_h

#include "../common.h"

namespace iro::mem
{

struct Allocator
{
  virtual void* allocate(u64 size) = 0;
  virtual void* reallocate(void* ptr, u64 size) = 0;
  virtual void  free(void* ptr) = 0;

  template<typename T>
  T* allocate(u64 count = 1) { return (T*)allocate(sizeof(T) * count); }

  template<typename T, typename... ConstructorArg>
  T* construct(ConstructorArg... args) 
  {  
    return new (allocate<T>()) T(args...);
  }

  template<typename T>
  void deconstruct(T* ptr)
  {
    ptr->~T();
    free(ptr);
  }
};

// Standard allocator.
// Currently this is the default allocator in use throughout iro,
// but later on when we implement our own memory backend we'll
// probably want to use that as default instead.
struct STLAllocator : Allocator
{
  void* allocate(u64 size) override;
  void* reallocate(void* ptr, u64 size) override;
  void  free(void* ptr) override;
};

extern STLAllocator stl_allocator;

}

#endif // _iro_allocator_h
