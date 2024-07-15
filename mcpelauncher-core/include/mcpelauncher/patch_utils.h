#pragma once

#include <cstdlib>
#include <cstring>
#include <unordered_map>


#define OnceToken() (void*)+[](void *args) \
  { \
    void *idx = __builtin_return_address(0) - 19; \
    Log::info("PUOnce", "--> Called %p", idx); \
    const auto meta = once_state[idx]; \
    memcpy(meta.patchOff, meta.backup, 5); \
    once_state.erase(idx); \
    reinterpret_cast<void (*)(void *, void *)>(meta.caller)(meta.patchOff, \
                                                            args);\
    Log::info("PUOnce", "<-- Called %p", idx); \
  };
typedef struct
{
  void *patchOff;
  void *caller;
  char backup[5];
} Meta;

extern std::unordered_map<void *, Meta> once_state;


class PatchUtils
{
 private:
  static const char *TAG;

 public:
  class VtableReplaceHelper
  {
   private:
    void *lib;
    void **vtable;
    void **referenceVtable;

   public:
    VtableReplaceHelper(void *lib, void **vtable, void **referenceVtable)
        : lib(lib), vtable(vtable), referenceVtable(referenceVtable)
    {
    }

    void replace(void *sym, void *replacement);

    void replace(const char *name, void *replacement);

    template <typename T>
    void replace(void *sym, T replacement)
    {
      replace(sym, memberFuncCast(replacement));
    }

    template <typename T>
    void replace(const char *name, T replacement)
    {
      replace(name, memberFuncCast(replacement));
    }
  };

  static void patchCallInstruction(void *patchOff, void *func, bool jump,
                                   char *backup = nullptr);

  static void once(void* Token, void *patchOff, void* caller);


  static size_t getVtableSize(void **vtable);

  template <typename T>
  static void *memberFuncCast(T func)
  {
    union
    {
      T func;
      void *ptr;
    } u;
    u.func = func;
    return u.ptr;
  }
};
