/****************************************************************
 * test suite for radb.a
 ****************************************************************/

#include "../include/radb.h"
#include <stdio.h>

void radb_verify(FILE *, radb_t *);

static char ptr[4];

/****************************************************************
 * initialization
 ****************************************************************/
void
sut_init(void)
{
  int i;
  for (i = i;  i < sizeof(ptr);  ++i)
    ptr[i] = i;
}

/****************************************************************
 * test about radb_bit_init()
 ****************************************************************/
void
sut_bit_init(void)
{
  radb_t *db;

  printf("\n%s\n", __FUNCTION__);
  radb_bit_init(&db);
  radb_verify(stdout, db);
  radb_bit_destroy(&db, NULL, NULL);
  radb_verify(stdout, db);
}

/****************************************************************
 * test about radb_bit_add()
 ****************************************************************/
void
sut_bit_add()
{
  radb_t *db;

  printf("\n%s\n", __FUNCTION__);
  radb_bit_init(&db);
  radb_verify(stdout, db);
  radb_bit_add(db, "0", 8, &ptr[0]);
  radb_verify(stdout, db);
  radb_bit_add(db, "00", 12, &ptr[1]);
  radb_verify(stdout, db);
  radb_bit_add(db, "²µ", 12, &ptr[2]);
  radb_verify(stdout, db);
  radb_bit_destroy(&db, NULL, NULL);
  radb_verify(stdout, db);
}

/****************************************************************
 * test for radb_bit_*
 ****************************************************************/
void
sut_bit(void)
{
  sut_init();
  sut_bit_init();
  sut_bit_add();
}

/****************************************************************/

int
main(void)
{
  sut_bit();
  return 0;
}

// Local Variables:
// tab-width: 999
// End:
 
