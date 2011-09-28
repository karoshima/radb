/****************************************************************
 * radix database
 * database for variable length bits, chars, 
 *    and fixed length variables u_int32_t
 * Copyright: Hiroaki KASHIMA <hiro@karoshima.org>
 ****************************************************************/
#include <sys/types.h>

/****************************************************************
 * database for variable length bits.
 ****************************************************************/

/****************
 * database structure
 * alloc this with radb_bit_init()
 * free with radb_bit_destroy()
 ****************/
typedef struct radb_bit radb_bit_t;

/****************
 ****************/
int radb_bit_init(radb_t **db);

/****************
 * destroy all the data
 * this needs a callback function to free data
 ****************/
int radb_bit_destroy(radb_t **db, void (*callback)(void *data, void *arg), void *arg);

/****************
 * add an data with key(bitmap), and the length of bits
 ****************/
int radb_bit_add(radb_t *db, void *key, size_t bitlen, void *data);

/****************
 * remove data with key(bitmap), and the length of bits
 ****************/
int radb_bit_del(radb_t *db, void *key, size_t bitlen, void *data);

/****************
 * search with longest-match
 * this returns the data
 ****************/
void *radb_bit_match_longest(radb_t *db, void *key, size_t bitlen);

/****************
 * data count
 ****************/
int radb_bit_count(radb_t *db);

/****************
 * walk all the data
 * this needs a callback function
 * to break walking, return non-0 from the callback function
 ****************/
int radb_bit_walk(radb_t *db, int (*callback)(void *data, void *arg), void *arg);

/****************
 * walk the data included in key/bitlen.
 * to break walking, return non-0 from the callback function
 ****************/
int radb_bit_walk_longer(radb_t *db, void *key, size_t bitlen, int (*callback)(void *data, void *arg), void *arg);

/****************
 * walk up from lonegest-match data to shortest-match data
 * to break walking, return non-0 from the callback function
 ****************/
int radb_bit_walk_shorter(radb_t *db, void *key, size_t bitlen, int (*callback)(void *data, void *arg), void *arg);

/****************
 * 
 ****************/
void *radb_bit_getnext(radb_t *db, void *key, size_t bitlen);

/*
 * Local Variables:
 * tab-width: 999
 * End:
 */
