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
  struct radix *parent;    // radix ツリーの親
  struct radix *left;      // radix ツリーの左の子
  struct radix *right;     // radix ツリーの右の子
  void         *data;      // 登録されているデータ
  u_int32_t     bitlen;    // キーのビット長
  u_int32_t     checkbyte; // キーのチェック対象ビット
  u_int8_t      checkbit;  // キーのチェック対象ビットのみ 1
  u_int8_t      key[1];    // キー (可変長データ)
} radix_t;

struct radb
{
#ifdef TEST
  struct radix  queue;    // 確認用ノードリスト
  int           qcount;   // ノード数
#endif // TEST
  struct radix *root;     // radix ツリー
  u_int32_t     count;    // 登録データ数
  u_int32_t     unitlen;  // 固定長キーのキー長
};

/*
 * Local Variables:
 * tab-width: 999
 * End:
 */
