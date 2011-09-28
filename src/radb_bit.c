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
      assert(node->next->prev == node);  // 確認データそのものの確認
      assert(node->prev->next == node);
      node->check = 'X';                 // 各ノードは「未チェック」
    }

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
      new = radix_alloc(key, bitlen);
      if (new == NULL)
        return ENOMEM;
      new->parent =    NULL;
      new->left =      NULL;
      new->right =     NULL;
      new->data =      data;
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
          new = radix_alloc(key, bitlen);
          if (new == NULL)
            return ENOMEM;
          new->data =      data;

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
