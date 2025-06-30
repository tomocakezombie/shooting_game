#include <string.h>
#include <unistd.h>

#include "snet.h"           // ���а��̿��饤�֥��
#include "tagGame.h"        // �����ä��⥸�塼��

#define PORT       10000    // �ǥե���ȤΥ����С�¦�ݡ����ֹ�
#define HOST_LEN   64       // �ۥ���̾�κ���Ĺ
#define MY_CHARA   'o'      // ��ʬ��ɽ������饯��
#define MY_SX      77       // ��ʬ�γ��� X ��ɸ
#define MY_SY      1        // ��ʬ�γ��� Y ��ɸ
#define IT_CHARA   'x'      // ����ɽ������饯��
#define IT_SX      1        // ���γ��� X ��ɸ
#define IT_SY      1        // ���γ��� Y ��ɸ

int main(int argc, char *argv[]) 
{ 
  char     serverName[HOST_LEN];    // �����С��Υۥ���̾
  int      s;                       // ���饤����ȤȤβ����ѥǥ�����ץ�
  TagGame *game;                    // �����ä�������

  // �����ä�������ν����
  game = initTagGame(MY_CHARA, MY_SX, MY_SY, IT_CHARA, IT_SX, IT_SY);

  // �����ǻ��ꤵ�줿�ۥ���̾�򥵡��ФȤ���
  // �⤷�������ʤ���м�ʬ���Ȥ򥵡��С��Ȥ��Ʋ��ꤷ�������������ߤ�
  if (argc == 2) 
    strcpy(serverName, argv[1]);
  else
    gethostname(serverName, HOST_LEN);

  // �����С���������롣����Υ����С��λ���Υݡ��Ȥ���³�����,�����С�
  // �Ȳ��ä��뤿��Υǥ�����ץ����֤�
  s = setupClient(serverName, PORT);

  // �����ä�������ν���
  setupTagGame(game, s);

  // �����ä�������γ���
  playClientTagGame(game);

  // �����ä�������θ����
  destroyTagGame(game);

  return 0;
} 
