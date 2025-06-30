#include "snet.h"           // ���а��̿��饤�֥��
#include "tagGame.h"        // �����ä��⥸�塼��

#define PORT       10000    // �ǥե���ȤΥ����С�¦�ݡ����ֹ�
#define MY_CHARA   'o'      // ��ʬ��ɽ������饯��
#define MY_SX      1        // ��ʬ�γ��� X ��ɸ
#define MY_SY      1        // ��ʬ�γ��� Y ��ɸ
#define IT_CHARA   'x'      // ����ɽ������饯��
#define IT_SX      77       // ���γ��� X ��ɸ
#define IT_SY      1        // ���γ��� Y ��ɸ

int main(int argc, char *argv[]) 
{ 
  int      s;       // ���饤����ȤȤβ����ѥǥ�����ץ�
  TagGame *game;    // �����ä�������

  // �����ä�������ν����
  game = initTagGame(MY_CHARA, MY_SX, MY_SY, IT_CHARA, IT_SX, IT_SY);

  // �����С���������롣���饤����Ȥ�����Υݡ��Ȥ���³�����,
  // ���饤����ȤȲ��ä��뤿��Υǥ�����ץ����֤�
  s = setupServer(PORT);

  // �����ä�������ν���
  setupTagGame(game, s);

  // �����ä�������γ���
  playServerTagGame(game);

  // ���饤����Ȥ������Ǥ��ʤ��� "can't bind" �ˤʤ�Τǡ������Ԥ�
  sleep(1);

  // �����ä�������θ����
  destroyTagGame(game);

  return 0;
}
