/****************************************************************
 * configuration for radb
 ****************************************************************/

#include "../include/radb.h"

struct radb
{
	struct radix *root;     // radix $B%D%j!<(B
  u_int32_t     count;    // $BEPO?%G!<%??t(B
  u_int32_t     unitlen;  // $B8GDjD9%-!<$N%-!<D9(B
};

struct radix
{
  struct radix *parent;    // radix $B%D%j!<$N?F(B
  struct radix *left;      // radix $B%D%j!<$N:8$N;R(B
  struct radix *right;     // radix $B%D%j!<$N1&$N;R(B
  void         *data;      // $BEPO?$5$l$F$$$k%G!<%?(B
  u_int32_t     bitlen;    // $B%-!<$N%S%C%HD9(B
  u_int32_t     checkbyte; // $B%-!<$N%A%'%C%/BP>]%S%C%H(B
  u_int8_t      checkbit;  // $B%-!<$N%A%'%C%/BP>]%S%C%H$N$_(B 1
  u_int8_t      key[1];    // $B%-!<(B ($B2DJQD9%G!<%?(B)
};

typedef struct radix radix_t;

/*
 * Local Variables:
 * tab-width: 999
 * End:
 */
