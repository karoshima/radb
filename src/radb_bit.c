/****************************************************************
 *
 ****************************************************************/

#include <stdlib.h>  // calloc(), free()
#include <string.h>  // memcpy()
#include <errno.h>   // errno

#include "config.h"

#ifdef TEST
#include <stdio.h>   // printf()
#include <assert.h>  // assert()
#endif // TEST


/****************************************************************/

/****************
 * 先頭から何 bit 目が不一致なのか
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
 * struct radix 獲得/解放
 ****************/
static struct radix *
radix_alloc(radb_t *db,
            void   *key,
            size_t  bitlen)
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
  memcpy(node->key, key, (bitlen+7)/8);

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
void
radb_verify(FILE   *fp,
            radb_t *db)
{
  struct radix *node, *path;
  int count;
  int idx;
  char buf[10];

  static const char *bits[] = {
    " 00000000", " 00000001", " 00000010", " 00000011",
    " 00000100", " 00000101", " 00000110", " 00000111",
    " 00001000", " 00001001", " 00001010", " 00001011",
    " 00001100", " 00001101", " 00001110", " 00001111",
    " 00010000", " 00010001", " 00010010", " 00010011",
    " 00010100", " 00010101", " 00010110", " 00010111",
    " 00011000", " 00011001", " 00011010", " 00011011",
    " 00011100", " 00011101", " 00011110", " 00011111",
    " 00100000", " 00100001", " 00100010", " 00100011",
    " 00100100", " 00100101", " 00100110", " 00100111",
    " 00101000", " 00101001", " 00101010", " 00101011",
    " 00101100", " 00101101", " 00101110", " 00101111",
    " 00110000", " 00110001", " 00110010", " 00110011",
    " 00110100", " 00110101", " 00110110", " 00110111",
    " 00111000", " 00111001", " 00111010", " 00111011",
    " 00111100", " 00111101", " 00111110", " 00111111",
    " 01000000", " 01000001", " 01000010", " 01000011",
    " 01000100", " 01000101", " 01000110", " 01000111",
    " 01001000", " 01001001", " 01001010", " 01001011",
    " 01001100", " 01001101", " 01001110", " 01001111",
    " 01010000", " 01010001", " 01010010", " 01010011",
    " 01010100", " 01010101", " 01010110", " 01010111",
    " 01011000", " 01011001", " 01011010", " 01011011",
    " 01011100", " 01011101", " 01011110", " 01011111",
    " 01100000", " 01100001", " 01100010", " 01100011",
    " 01100100", " 01100101", " 01100110", " 01100111",
    " 01101000", " 01101001", " 01101010", " 01101011",
    " 01101100", " 01101101", " 01101110", " 01101111",
    " 01110000", " 01110001", " 01110010", " 01110011",
    " 01110100", " 01110101", " 01110110", " 01110111",
    " 01111000", " 01111001", " 01111010", " 01111011",
    " 01111100", " 01111101", " 01111110", " 01111111",
    " 10000000", " 10000001", " 10000010", " 10000011",
    " 10000100", " 10000101", " 10000110", " 10000111",
    " 10001000", " 10001001", " 10001010", " 10001011",
    " 10001100", " 10001101", " 10001110", " 10001111",
    " 10010000", " 10010001", " 10010010", " 10010011",
    " 10010100", " 10010101", " 10010110", " 10010111",
    " 10011000", " 10011001", " 10011010", " 10011011",
    " 10011100", " 10011101", " 10011110", " 10011111",
    " 10100000", " 10100001", " 10100010", " 10100011",
    " 10100100", " 10100101", " 10100110", " 10100111",
    " 10101000", " 10101001", " 10101010", " 10101011",
    " 10101100", " 10101101", " 10101110", " 10101111",
    " 10110000", " 10110001", " 10110010", " 10110011",
    " 10110100", " 10110101", " 10110110", " 10110111",
    " 10111000", " 10111001", " 10111010", " 10111011",
    " 10111100", " 10111101", " 10111110", " 10111111",
    " 11000000", " 11000001", " 11000010", " 11000011",
    " 11000100", " 11000101", " 11000110", " 11000111",
    " 11001000", " 11001001", " 11001010", " 11001011",
    " 11001100", " 11001101", " 11001110", " 11001111",
    " 11010000", " 11010001", " 11010010", " 11010011",
    " 11010100", " 11010101", " 11010110", " 11010111",
    " 11011000", " 11011001", " 11011010", " 11011011",
    " 11011100", " 11011101", " 11011110", " 11011111",
    " 11100000", " 11100001", " 11100010", " 11100011",
    " 11100100", " 11100101", " 11100110", " 11100111",
    " 11101000", " 11101001", " 11101010", " 11101011",
    " 11101100", " 11101101", " 11101110", " 11101111",
    " 11110000", " 11110001", " 11110010", " 11110011",
    " 11110100", " 11110101", " 11110110", " 11110111",
    " 11111000", " 11111001", " 11111010", " 11111011",
    " 11111100", " 11111101", " 11111110", " 11111111" };

  if (fp != NULL)
    {
      fprintf(fp, "[radb %p]\n", db);
      if (db == NULL)
        return;
      fprintf(fp, "    %p->root =    %p\n", db, db->root);
      fprintf(fp, "    %p->count =   %u\n", db, db->count);
      fprintf(fp, "    %p->unitlen = %u\n", db, db->unitlen);
    }
  else
    {
      if (db == NULL)
        return;
    }

  for (node = db->queue.next;
       node != &(db->queue);
       node = node->next)
    {
      assert(node->next->prev == node);  // 確認データそのものの確認
      assert(node->prev->next == node);
      node->check = 'X';                 // 各ノードは「未チェック」
    }

  fprintf(fp, "[radixtree]\n");
  count = 0;
  node = db->root;
  while (node != NULL)
    {
      switch (node->check)
        {
        case 'X':
          if (node->left != NULL) // 左部分木を先にチェックする
            {
              assert(node->left->parent == node);
              node->check = 'l';
              node = node->left;
              break;
            }
          // fall through
        case 'l':
          if (fp != NULL) // このノードを表示する
            {
              path = db->root;      // root から辿り直して、枝を表示する
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
              fprintf(fp, "+-[%p]", node);  // ノードを表示する
              for (idx = 0;  idx < node->checkbyte; ++idx) // キーを表示する
                fprintf(fp, "%s", bits[node->key[idx]]);
              memcpy(buf, bits[node->key[idx]], 8);
              switch (node->checkbit)
                {
                case 0x80: buf[0] = 0; break;
                case 0x40: buf[2] = 0; break;
                case 0x20: buf[3] = 0; break;
                case 0x10: buf[4] = 0; break;
                case 0x08: buf[5] = 0; break;
                case 0x04: buf[6] = 0; break;
                case 0x02: buf[7] = 0; break;
                case 0x01: buf[8] = 0; break;
                default:
                  assert(node->checkbit == 0x80
                         || node->checkbit == 0x40
                         || node->checkbit == 0x20
                         || node->checkbit == 0x10
                         || node->checkbit == 0x08
                         || node->checkbit == 0x04
                         || node->checkbit == 0x02
                         || node->checkbit == 0x01);
                  break;
                }
              fprintf(fp, "%s/%u", buf, node->bitlen);
              if (node->data != NULL)
                {
                  fprintf(fp, " => %p", node->data);
                  ++count;
                }
              fprintf(fp, "\n");
            }

          if (node->right != NULL) // 右の部分木をチェックする
            {
              assert(node->right->parent == node);
              node->check = 'r';
              node = node->right;
              break;
            }
          // fall through
        case 'r':
          node->check = 0;        // チェック完了。親に戻す。
          node = node->parent;
          break;
        default:                  // チェック処理の障害
          if (fp != NULL)
            fprintf(fp, "ILLEGAL CHECK [%p]->check = %d\n", node, node->check);
          assert(node->check == 'X' || node->check == 'l' || node->check == 'r');
          break;
        }
    }
  assert(db->count == count);

  for (node = db->queue.next;
       node != &(db->queue);
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
  db->queue.next = db->queue.prev = &(db->queue);
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

      if (prev == node->parent) // 上から降りてきた
        {
          if (node->left != NULL)
            next = node->left;  // 左ツリーを解放する
          else if (node->right != NULL)
            next = node->right; // 右ツリーを解放する
          else
            goto goup; // 末端
        }
      else if (prev == node->left) // 左から戻ってきた
        {
          if (node->right != NULL)
            next = node->right; // 右ツリーを解放する
          else
            goto goup; // 末端
        }
      else // 右から戻ってきた
        {
        goup: // 末端だから当該 node を解放して上に戻る
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

  // パラメタチェック
  if (db == NULL)
    return EINVAL;
  if (bitlen < 0)
    return EINVAL;
  if (bitlen > 0 && key == NULL)
    return EINVAL;

  // 登録場所の確認と登録
  node = db->root;
  if (node == NULL)  // 最初の一個だから、db->root に置く
    {
      new = radix_alloc(db, key, bitlen);
      if (new == NULL)
        return ENOMEM;
      new->parent = NULL;
      new->left =   NULL;
      new->right =  NULL;
      new->data =   data;
      db->root =    new;
    }
  else  // radix ツリー内の登録位置を探す
    {
      // ビットだけ見てツリーを降りる
      // (あとでキー全体をチェックして巻き戻す)
      while (node != NULL && node->bitlen <= bitlen)
        {
          parent = node;
          node = (key[parent->checkbyte] & parent->checkbit) == 0
            ? parent->left : parent->right;
        }
      // キー全体をチェックして巻き戻す
      diffbit = differentbit(key, parent->key, parent->bitlen);
      if (diffbit > parent->bitlen)
        diffbit = parent->bitlen;
      while (parent != NULL && parent->bitlen >= diffbit)
        {
          node = parent;
          parent = node->parent;
        }
      // これによる結果は以下のとおりとなる
      // node:    登録位置近傍のノード
      // parent:  近傍ノード node の親
      // diffbit: 近傍ノード node と新規登録キーの食い違い位置 (単位bit)

      // 登録位置の確認およびツリーへの組み込み
      if (diffbit == bitlen && diffbit == node->bitlen)
        {
          // ここがまさにデータ登録箇所
          // node をそのまま使う
          if (node->data != NULL)
            return EEXIST;  // 重複登録不可
          new = node;
          new->data = data;
        }
      else
        {
          new = radix_alloc(db, key, bitlen);
          if (new == NULL)
            return ENOMEM;
          new->data = data;

          // 新規登録データのための新規ノード new を
          // ここで radix ツリーに組み込む
          if (diffbit == node->bitlen) // new は node の初出の子
            {
              if ((key[node->checkbyte] & node->checkbit) == 0)
                node->left =  new;
              else
                node->right = new;

              new->parent = node;
              new->left =   NULL;
              new->right =  NULL;
            }
          else if (diffbit == bitlen) // new は node の親
            {
              if (parent == NULL)
                db->root =      new;
              else if (parent->left == node)
                parent->left =  new;
              else // parent->right == node
                parent->right = new;

              new->parent = parent;
              if ((node->key[new->checkbyte] & new->checkbit) == 0)
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
          else // new は node の兄弟
            {
              // 分岐のための中間ノードが必要になる
              branch = radix_alloc(db, key, diffbit);
              if (branch == NULL)
                {
                  free(new);
                  return ENOMEM;
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
