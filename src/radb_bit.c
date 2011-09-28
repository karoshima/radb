/****************************************************************
 *
 ****************************************************************/

#ifdef TEST
#include <stdio.h>   // printf()
#include <assert.h>  // assert()
#endif // TEST

#include <stdlib.h>  // calloc(), free()
#include <string.h>  // memcpy()
#include <errno.h>   // errno

#include "config.h"


/****************************************************************/

/****************
 * $B@hF,$+$i2?(B bit $BL\$,IT0lCW$J$N$+(B
 ****************/
static unsigned int
differentbit(void *bit1,
             void *bit2,
             int bitlen)
{
  u_int8_t *key1 = bit1;
  u_int8_t *key2 = bit2;
  int bytelen = (bitlen + 7) / 8;
  int pos;

  static u_int32_t tbl[] = {
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  for (pos = 0; pos < bytelen;  ++pos)
      if (key1[pos] != key2[pos])
        break;
  pos = pos*8 + tbl[key1[pos] ^ key2[pos]];
  if (pos > bitlen)
    pos = bitlen;
  return pos;
}

/****************
 * struct radix $B3MF@(B/$B2rJ|(B
 ****************/
static struct radix *
radix_alloc(struct radb *db,
            void        *key,
            size_t       bitlen)
{
  struct radix *node = malloc(sizeof(*node) + (bitlen/8));
  if (node == NULL)
    return NULL;

#ifdef TEST
  node->db = db;
  node->next = &(db->queue);
  node->prev = db->queue.prev;
  node->next->prev = node->prev->next = node;
  ++(db->qcount);
#endif // TEST

  // node->parent = undef;
  // node->left =   undef;
  // node->right =  undef;
  // node->data =   undef;
  node->bitlen =    bitlen;
  node->checkbyte = bitlen/8;
  node->checkbit =  (u_int8_t)0x80 >> (bitlen%8);
  memcpy(new->key, key, (bitlen+7)/8);

  return node;
}
static void
radix_free(struct radix *node)
{
#ifdef TEST
  node->next->prev = node->prev;
  node->prev->next = node->next;
  --(node->db->qcount);
#endif // TEST
  free(node);
}

/****************************************************************/
#ifdef TEST
static void
radix_verify(FILE         *fp,
             struct radix *node,
             int          *count)
{
  struct radix *path;

}

void
radb_verify(FILE        *fp,
            struct radb *db)
{
  struct radix *node, *path;
  int count;

  for (node = db->queue.next;
       node != &(db->queue);
       node = node->next)
    {
      assert(node->next->prev == node);  // $B3NG'%G!<%?$=$N$b$N$N3NG'(B
      assert(node->prev->next == node);
      node->check = 'X';                 // $B3F%N!<%I$O!VL$%A%'%C%/!W(B
    }

  count = 0;
  node = db->root;
  while (node != NULL)
    {
      switch (node->check)
        {
        case 'X':
          if (node->left != NULL) // $B:8ItJ,LZ$r@h$K%A%'%C%/$9$k(B
            {
              assert(node->left->parent == node);
              node->check = 'l';
              node = node->left;
              break;
            }
          // fall through
        case 'l':
          if (fp != NULL) // $B$3$N%N!<%I$rI=<($9$k(B
            {
              path = db->root;      // root $B$+$iC)$jD>$7$F!";^$rI=<($9$k(B
              while (path != node)
                {
                  if (path->parent == NULL)
                    fprintf(fp, "  ");
                  else if (path->parent->check == path->check)
                    fprintf(fp, "  ");
                  else
                    fprintf(fp, "| ");
                  path = (path->check == 'l') ? path->left : path->right;
                }
              fprintf(fp, "+-[%p]", node);  // $B%N!<%I$rI=<($9$k(B
              if (node->data != NULL)
                {
                  fprintf(fp, " => %p", node->data);
                  ++count;
                }
              fprintf(fp, "\n");
            }

          if (node->right != NULL) // $B1&$NItJ,LZ$r%A%'%C%/$9$k(B
            {
              assert(node->right->parent == node);
              node->check = 'r';
              node = node->right;
              break;
            }
          // fall through
        case 'r':
          node->check = 0;        // $B%A%'%C%/40N;!#?F$KLa$9!#(B
          node = node->parent;
          break;
        default:                  // $B%A%'%C%/=hM}$N>c32(B
          if (fp != NULL)
            printf(fp, "ILLEGAL CHECK [%p]->check = %d\n", node, node->check);
          assert(node->check == 'X' || node->check == 'l' || node->check == 'r');
          break;
        }
    }
  assert(db->count == count);

  for (node = db->queue.next;
       node != &(db->queueu);
       node = node->next)
    assert(node->check == 0);
}
#endif // TEST

/****************************************************************/

/****************
 ****************/
int
radb_bit_init(radb_t **dbp)
{
  radb_t *db;

  if (dbp == NULL)
    return EINVAL;
  db = calloc(1, sizeof(radb_t));
  if (db == NULL)
    return ENOMEM;
#ifdef TEST
  db->queue.next = db->queue.predv = &(db->queue);
#endif
  *dbp = db;
  return 0;
}

/****************
 ****************/
int
radb_bit_destroy(radb_t **dbp,
                 void (*callback)(void *data,
                                  void *arg),
                 void *arg)
{
  radb_t *db;
  radb_t *db = *dbp;
  radix_t *node = db->root;
  radix_t *prev, *next;

  if (dbp == NULL)
    return EINVAL;
  db = *dbp;
  if (db == NULL)
    return 0;

  *dbp = NULL;
  node = db->root;
  prev = NULL;

  while (node != NULL)
    {
      if (node->data != NULL && callback != NULL)
        callback(node->data, arg);

      if (prev == node->parent) // $B>e$+$i9_$j$F$-$?(B
        {
          if (node->left != NULL)
            next = node->left;  // $B:8%D%j!<$r2rJ|$9$k(B
          else if (node->right != NULL)
            next = node->right; // $B1&%D%j!<$r2rJ|$9$k(B
          else
            goto goup; // $BKvC<(B
        }
      else if (prev == node->left) // $B:8$+$iLa$C$F$-$?(B
        {
          if (node->right != NULL)
            next = node->right; // $B1&%D%j!<$r2rJ|$9$k(B
          else
            goto goup; // $BKvC<(B
        }
      else // $B1&$+$iLa$C$F$-$?(B
        {
        goup: // $BKvC<$@$+$iEv3:(B node $B$r2rJ|$7$F>e$KLa$k(B
          next = node->parent;
          radix_free(node);
        }
      prev = node;
      node = next;
    }

  free(db);
  return 0;
}

/****************
 ****************/
int
radb_bit_add(radb_t *db,
             void *vkey,
             size_t bitlen,
             void *data)
{
  radix_t *new, *node, *parent, *branch;
  u_int8_t *key = vkey;
  unsigned int diffbit;

  // $B%Q%i%a%?%A%'%C%/(B
  if (db == NULL)
    return EINVAL;
  if (bitlen < 0)
    return EINVAL;
  if (bitlen > 0 && key == NULL)
    return EINVAL;

  // $BEPO?>l=j$N3NG'$HEPO?(B
  node = db->root;
  if (node == NULL)  // $B:G=i$N0l8D$@$+$i!"(Bdb->root $B$KCV$/(B
    {
      new = radix_alloc(key, bitlen);
      if (new == NULL)
        return ENOMEM;
      new->parent =    NULL;
      new->left =      NULL;
      new->right =     NULL;
      new->data =      data;
      db->root = new;
    }
  else  // radix $B%D%j!<Fb$NEPO?0LCV$rC5$9(B
    {
      // $B%S%C%H$@$18+$F%D%j!<$r9_$j$k(B
      // ($B$"$H$G%-!<A4BN$r%A%'%C%/$7$F4,$-La$9(B)
      while (node != NULL && node->bitlen <= bitlen)
        {
          parent = node;
          node = (key[parent->checkbyte] & parent->checkbit) == 0
            ? parent->left : parent->right;
        }
      // $B%-!<A4BN$r%A%'%C%/$7$F4,$-La$9(B
      diffbit = differentbit(key, parent->key, parent->bitlen);
      if (diffbit > parent->bitlen)
        diffbit = parent->bitlen;
      while (parent != NULL && parent->bitlen > diffbit)
        {
          node = parent;
          parent = node->parent;
        }
      // $B$3$l$K$h$k7k2L$O0J2<$N$H$*$j$H$J$k(B
      // node:    $BEPO?0LCV6aK5$N%N!<%I(B
      // parent:  $B6aK5%N!<%I(B node $B$N?F(B
      // diffbit: $B6aK5%N!<%I(B node $B$H?75,EPO?%-!<$N?)$$0c$$0LCV(B ($BC10L(Bbit)

      // $BEPO?0LCV$N3NG'$*$h$S%D%j!<$X$NAH$_9~$_(B
      if (diffbit == bitlen && diffbit == node->bitlen)
        {
          // $B$3$3$,$^$5$K%G!<%?EPO?2U=j(B
          // node $B$r$=$N$^$^;H$&(B
          if (node->data != NULL)
            return EEXIST;  // $B=EJ#EPO?IT2D(B
          new = node;
          new->data = data;
        }
      else
        {
          new = radix_alloc(key, bitlen);
          if (new == NULL)
            return ENOMEM;
          new->data =      data;

          // $B?75,EPO?%G!<%?$N$?$a$N?75,%N!<%I(B new $B$r(B
          // $B$3$3$G(B radix $B%D%j!<$KAH$_9~$`(B
          if (diffbit == node->bitlen) // new $B$O(B node $B$N=i=P$N;R(B
            {
              if ((key[node->checkbyte] & node->checkbit) == 0)
                node->left =  new;
              else
                node->right = new;

              new->parent = node;
              new->left =   NULL;
              new->right =  NULL;
            }
          else if (diffbit == bitlen) // new $B$O(B node $B$N?F(B
            {
              if (parent == NULL)
                db->root =      new;
              else if (parent->left == node)
                parent->left =  new;
              else // parent->right == node
                parent->right = new;

              new->parent = parent;
              if ((node->key[checkbyte] & checkbit) == 0)
                {
                  new->left =  node;
                  new->right = NULL;
                }
              else
                {
                  new->left =  NULL;
                  new->right = node;
                }

              node->parent = new;
            }
          else // new $B$O(B node $B$N7;Do(B
            {
              // $BJ,4t$N$?$a$NCf4V%N!<%I$,I,MW$K$J$k(B
              branch = radix_alloc(key, diffbit);
              if (branch == NULL)
                {
                  free(new);
                  return NULL;
                }
              branch->data =      NULL;

              if (parent == NULL)
                db->root =      branch;
              else if (parent->left == node)
                parent->left =  branch;
              else // parent->right == node
                parent->right = branch;

              branch->parent = parent;
              if ((key[branch->checkbyte] & branch->checkbit) == 0)
                {
                  branch->left =  new;
                  branch->right = node;
                }
              else
                {
                  branch->left =  node;
                  branch->right = new;
                }

              node->parent = branch;

              new->parent = branch;
              new->left =   NULL;
              new->right =  NULL;
            }
        }
    }
  ++db->count;
  return 0;
}

/*
 * Local Variables:
 * tab-width: 999
 * End:
 */
