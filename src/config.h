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
  struct radix *parent;    // radix �ĥ꡼�ο�
  struct radix *left;      // radix �ĥ꡼�κ��λ�
  struct radix *right;     // radix �ĥ꡼�α��λ�
  void         *data;      // ��Ͽ����Ƥ���ǡ���
  u_int32_t     bitlen;    // �����Υӥå�Ĺ
  u_int32_t     checkbyte; // �����Υ����å��оݥӥå�
  u_int8_t      checkbit;  // �����Υ����å��оݥӥåȤΤ� 1
  u_int8_t      key[1];    // ���� (����Ĺ�ǡ���)
} radix_t;

struct radb
{
#ifdef TEST
  struct radix  queue;    // ��ǧ�ѥΡ��ɥꥹ��
  int           qcount;   // �Ρ��ɿ�
#endif // TEST
  struct radix *root;     // radix �ĥ꡼
  u_int32_t     count;    // ��Ͽ�ǡ�����
  u_int32_t     unitlen;  // ����Ĺ�����Υ���Ĺ
};

/*
 * Local Variables:
 * tab-width: 999
 * End:
 */
