#include <stdio.h>
#include "kv.h"

#ifdef DEBUG

int main()
{
  initialize();
  for (int i = 0; i < 4096; ++i) {
    char key[128];
    sprintf(key, "aaaaaaa_%d\x05", i);
    store_data(key, "test", 4);
  }
  clear_db();
}

#endif
