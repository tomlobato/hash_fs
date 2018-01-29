
#include "hash_function.h"

unsigned int hash(char *str)
{
    XXH32_state_t state32;
    XXH32_reset(&state32, 0);
    XXH32_update(&state32, str, strlen(str));    
    return XXH32_digest(&state32);
}


