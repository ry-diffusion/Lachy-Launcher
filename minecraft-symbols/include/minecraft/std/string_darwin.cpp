#include <iostream>

#include "string.h"

mcpe::string* mcpe::string::empty;

mcpe::string::_Rep* mcpe::string::createRep(const char* data, size_t length,
                                            size_t capacity)
{
  if (capacity < length) capacity = length;
  _Rep* rp = (_Rep*)malloc(sizeof(mcpe::string::_Rep) + capacity + 1);
  rp->length = length;
  rp->capacity = capacity;
  rp->refcount = -1;
  char* rp_data = (char*)(rp + 1);
  if (data != nullptr) memcpy((void*)rp_data, data, length);
  rp_data[length] = '\0';
  return rp;
}

mcpe::string::string()
{
  ptr = empty->ptr;
}
mcpe::string::string(const char* str, size_t len)
{
  if (len == 0)
  {
    ptr = empty->ptr;
  }
  else
  {
    initRep(createRep(str, len));
  }
}
mcpe::string::string(const string& str)
{
  if (str.ptr == empty->ptr)
  {
    ptr = empty->ptr;
  }
  else
  {
    initRep(str.getRep());
  }
}
mcpe::string::~string()
{
  if (ptr == empty->ptr) return;
  getRep()->removeRef();
}

void mcpe::string::resize(std::size_t size)
{
  if (length() == size) return;
  if (size <= getRep()->capacity)
  {
    getRep()->length = size;
  }
  else
  {
    _Rep* newRep = createRep(nullptr, size, size);
    memcpy(newRep + 1, c_str(), length() + 1);
    assignRep(newRep);
  }
}

size_t mcpe::string::length() const
{
  if (ptr == empty->ptr) return 0;
  return getRep()->length;
}

mcpe::string& mcpe::string::operator=(const mcpe::string& str)
{
  assignRep(str.getRep());
  return *this;
}

const char* mcpe::string::c_str() const
{
  return (char*)ptr;
}

std::ostream& operator<<(std::ostream& os, const mcpe::string& obj)
{
  os << obj.std();
  return os;
}
