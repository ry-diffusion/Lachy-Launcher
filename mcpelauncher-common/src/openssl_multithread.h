#pragma once

#include <pthread.h>

#include <mutex>
#include <vector>

struct OpenSSLMultithreadHelper
{
 private:
  struct PThreadMutex
  {
    pthread_mutex_t mutex;

    PThreadMutex()
    {
      mutex = PTHREAD_MUTEX_INITIALIZER;
      pthread_mutex_init(&mutex, nullptr);
    }
    ~PThreadMutex()
    {
      pthread_mutex_destroy(&mutex);
    }
  };

  static OpenSSLMultithreadHelper instance;

  std::vector<PThreadMutex> mutexes;

 public:
  OpenSSLMultithreadHelper();
};