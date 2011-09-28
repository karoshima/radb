/****************************************************************
 * configuration for radb
 ****************************************************************/

#include "../include/radb.h"

struct radb
{
	struct radix *root;     // radix ツリー
  u_int32_t     count;    // 登録データ数
  u_int32_t     unitlen;  // 固定長キーのキー長
};

struct radix
{
  struct radix *parent;    // radix ツリーの親
  struct radix *left;      // radix ツリーの左の子
  struct radix *right;     // radix ツリーの右の子
  void         *data;      // 登録されているデータ
  u_int32_t     bitlen;    // キーのビット長
  u_int32_t     checkbyte; // キーのチェック対象ビット
  u_int8_t      checkbit;  // キーのチェック対象ビットのみ 1
  u_int8_t      key[1];    // キー (可変長データ)
};

typedef struct radix radix_t;

/*
 * Local Variables:
 * tab-width: 999
 * End:
 */
