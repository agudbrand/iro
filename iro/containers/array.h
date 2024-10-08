/*
 *  Growing array
 */

#ifndef _iro_array_h
#define _iro_array_h

#include "../common.h"
#include "../memory/memory.h"
#include "../memory/allocator.h"

#include "ArrayFuncs.h"

#include "../traits/nil.h"
#include "../traits/move.h"
#include "../traits/container.h"
#include "../traits/iterable.h"

#include "new"
#include "assert.h"

namespace iro
{

template<typename T>
struct Array 
{
  struct Header
  {
    s32 len = 0;
    s32 space = 0;
    mem::Allocator* allocator = nullptr;
  };

  T* arr = nullptr;

  static Header* getHeader(T* ptr) { return (Header*)ptr - 1; }
  static s32& len(T* ptr) { return getHeader(ptr)->len; }
  static s32& space(T* ptr) { return getHeader(ptr)->space; }
  static mem::Allocator* allocator(T* ptr) 
    { return getHeader(ptr)->allocator; }

  static Array<T> fromOpaquePointer(T* ptr) { return Array<T>{ptr}; }

  static Array<T> create(mem::Allocator* allocator)
  {
    return create(8, allocator);
  }

  /* --------------------------------------------------------------------------
   */ 
  b8 init(s32 init_space = 8, mem::Allocator* allocator = &mem::stl_allocator)
  {
    init_space = (8 > init_space ? 8 : init_space);

    Header* header = 
      (Header*)allocator->allocate(sizeof(Header) + init_space * sizeof(T));

    header->space = init_space;
    header->len = 0;
    header->allocator = allocator;

    arr = (T*)(header + 1);

    return true;
  }

  /* --------------------------------------------------------------------------
   */ 
  static Array<T> create(
      s32 init_space = 8, 
      mem::Allocator* allocator = &mem::stl_allocator)
  {

    Array<T> out = {};
    
    if (!out.init(init_space, allocator))
      return nil;

    return out;
  }

  /* --------------------------------------------------------------------------
   */ 
  void destroy()
  {
    if (isnil(*this))
      return;
    clear();
    mem::Allocator* a = allocator();
    a->free(getHeader());
    *this = nil;
  }

  /* --------------------------------------------------------------------------
   */ 
  __attribute__((noinline))
  Header* getHeader();
  __attribute__((noinline))
  s32& len();
  __attribute__((noinline))
  s32& space();
  __attribute__((noinline))
  mem::Allocator* allocator();

  b8 isEmpty() { return len() == 0; }

  /* --------------------------------------------------------------------------
   */ 
  T* push()
  {
    growIfNeeded(1);

    return array::push(arr, &len());

    // len() += 1;
    // return new (arr + len() - 1) T; 
  }

  void push(const T& x)
  {
    growIfNeeded(1);

    array::push(arr, &len(), x);

    // arr[len()] = x;
    // len() += 1;
  }

  /* --------------------------------------------------------------------------
   */ 
  void pop()
  {
    assert(len() != 0);
    array::pop(arr, &len());

    // len() -= 1;
  }

  /* --------------------------------------------------------------------------
   */ 
  void insert(s32 idx, const T& x)
  {
    assert(idx >= 0 && idx <= len());
    growIfNeeded(1);

    array::insert(arr, &len(), idx, x);

    // if (!len()) 
    //   push(x);
    // else
    // {
    //   mem::move(arr + idx + 1, arr + idx, sizeof(T) * (len() - idx));
    //   len() += 1;
    //   arr[idx] = x;
    // }
  }

  T* insert(s32 idx)
  {
    assert(idx >= 0 && idx <= len());
    growIfNeeded(1);

    return array::insert(arr, &len(), idx);

    // if (idx == len())
    //   return push();

    // growIfNeeded(1);
    // mem::move(arr + idx + 1, arr + idx, sizeof(T) * (len() - idx));
    // len() += 1;
    // return new (arr + idx) T;
  }

  /* --------------------------------------------------------------------------
   */ 
  void remove(T* x)
  {
    return remove(x - arr);
  }

  void remove(s32 idx)
  {
    assert(idx >= 0 && idx < len());
    array::remove(arr, &len(), idx);
    // mem::move(arr + idx + 1, arr + idx, sizeof(T) * (len() - idx));
    // len() -= 1;
  }

  /* --------------------------------------------------------------------------
   */ 
  void clear()
  {
    array::clear(arr, &len());

    // for (s32 i = 0; i < len(); i++)
    //   (arr + i)->~T();
    // len() = 0;
  }


  /* --------------------------------------------------------------------------
   */ 
  T& operator [](s64 i) { assert(i >=0 && i < len()); return arr[i]; }

  T* begin() { return arr; }
  T* end()   { return arr + len(); }

  T* first() { return begin(); } 
  T* last() { return end() - 1; }

  /* --------------------------------------------------------------------------
   */ 
  void growIfNeeded(s32 new_elements)
  {
    Header* header = getHeader();

    if (header->len + new_elements <= header->space)
      return;

    while (header->len + new_elements > header->space)
      header->space *= 2;

    header = 
      (Header*)header->allocator->reallocate(
          header, sizeof(Header) + header->space * sizeof(T));
    arr = (T*)(header + 1);
  }
};

// NOTE(sushi) these are defined out-of-line to prevent the compiler from
//             for inlining them to help debugging.

/* ----------------------------------------------------------------------------
 */ 
template<typename T>
Array<T>::Header* Array<T>::getHeader()
{
  return (Header*)arr - 1;
}

/* ----------------------------------------------------------------------------
 */ 
template<typename T>
s32& Array<T>::len()
{
  return getHeader()->len;
}

/* ----------------------------------------------------------------------------
 */ 
template<typename T>
s32& Array<T>::space() 
{
  return getHeader()->space;
}

/* ----------------------------------------------------------------------------
 */ 
template<typename T>
mem::Allocator* Array<T>::allocator()
{
  return getHeader()->allocator;
}

}

template<typename X>
struct NilValue<iro::Array<X>>
{
  constexpr static const iro::Array<X> Value = {nullptr};
  inline static b8 isNil(const iro::Array<X>& x) { return x.arr == nullptr; }
};

template<typename T>
struct MoveTrait<iro::Array<T>>
{
  inline static void doMove(iro::Array<T>& from, iro::Array<T>& to)
  {
    iro::mem::copy(&to, &from, sizeof(iro::Array<T>));
  }
};

DefineExpandableContainerT(iro::Array, { self->push(value); return true; });
DefineReducibleContainerT(iro::Array, { self->remove(x); return true; });
DefineIndexableContainerT(iro::Array, { return &self->arr[idx]; });

DefineIterableT(iro::Array, { s32 idx = 0; }, 
    { 
      if (idx >= self->len())
        return nullptr;
      auto* out = &self->arr[idx]; 
      idx += 1; 
      return out; 
    });

#endif
