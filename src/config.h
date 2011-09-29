/****************************************************************
 * configuration for radb
 ****************************************************************/

#include "mkopt.h"
#include "../include/radb.h"

typedef struct radix
{
#ifdef TEST
  struct radb  *db;
  struct radix *next;
  struct radix *prev;
  int           check;
#endif // TEST
  struct radix *parent;    // radix $B%D%j!<$N?F(B
  struct radix *left;      // radix $B%D%j!<$N:8$N;R(B
  struct radix *right;     // radix $B%D%j!<$N1&$N;R(B
  void         *data;      // $BEPO?$5$l$F$$$k%G!<%?(B
  u_int32_t     bitlen;    // $B%-!<$N%S%C%HD9(B
  u_int32_t     checkbyte; // $B%-!<$N%A%'%C%/BP>]%S%C%H(B
  u_int8_t      checkbit;  // $B%-!<$N%A%'%C%/BP>]%S%C%H$N$_(B 1
  u_int8_t      key[1];    // $B%-!<(B ($B2DJQD9%G!<%?(B)
} radix_t;

struct radb
{
#ifdef TEST
  struct radix  queue;    // $B3NG'MQ%N!<%I%j%9%H(B
  int           qcount;   // $B%N!<%I?t(B
#endif // TEST
  struct radix *root;     // radix $B%D%j!<(B
  u_int32_t     count;    // $BEPO?%G!<%??t(B
  u_int32_t     unitlen;  // $B8GDjD9%-!<$N%-!<D9(B
};

/*
 * Local Variables:
 * tab-width: 999
 * End:
 */
