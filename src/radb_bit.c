/****************************************************************
 *
 ****************************************************************/

#include <stdlib.h>  // calloc(), free()
#include <string.h>  // memcpy()
#include <errno.h>   // errno

#include "config.h"


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
          free(node);
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
      new = malloc(sizeof(*node) + (bitlen/8));
      if (new == NULL)
        return ENOMEM;
      new->parent =    NULL;
      new->left =      NULL;
      new->right =     NULL;
      new->data =      data;
      new->bitlen =    bitlen;
      new->checkbyte = bitlen/8;
      new->checkbit =  (u_int8_t)0x80 >> (bitlen % 8);
      new->key[new->checkbyte] = 0;
      memcpy(new->key, key, (bitlen+7)/8);
      db->root = new;
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
      while (parent != NULL && parent->bitlen > diffbit)
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
          new = malloc(sizeof(*node) + (bitlen/8));
          if (new == NULL)
            return ENOMEM;
          new->data =      data;
          new->bitlen =    bitlen;
          new->checkbyte = bitlen / 8;
          new->checkbit =  (u_int8_t)0x80 >> (bitlen % 8);
          memcpy(new->key, key, (bitlen+7)/8);

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
          else // new は node の兄弟
            {
              // 分岐のための中間ノードが必要になる
              branch = malloc(sizeof(*branch) + (diffbit/8));
              if (branch == NULL)
                {
                  free(new);
                  return NULL;
                }
              branch->data =      NULL;
              branch->bitlen =    diffbit;
              branch->checkbyte = diffbit/8;
              branch->checkbit =  (u_int8_t)0x80 >> (diffbit % 8);
              memcpy(branch->key, key, (diffbit+7)/8);

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
