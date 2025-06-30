#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "tagGame.h"           // 鬼ごっこモジュールヘッダファイル

#define LOGICWIN_SX     2
#define LOGICWIN_SY     1

#define MAINWIN_LINES   20    // メインウィンドウの高さ
#define MAINWIN_COLUMS  100    // メインウィンドウの横幅
#define MAINWIN_SX      2     // メインウィンドウの左上X座標
#define MAINWIN_SY      1     // メインウィンドウの左上Y座標

#define SUBWIN_LINES    20    // サブウィンドウの高さ
#define SUBWIN_COLUMS   100    // サブウィンドウの横幅
#define SUBWIN_SX       2     // サブウィンドウの左上X座標
#define SUBWIN_SY       22    // サブウィンドウの左上Y座標 (メインウィンドウの下に配置)

#define STATUSWIN_LIENS  5
#define STATUSWIN_COLUMS 50    // サブウィンドウの横幅
#define STATUSWIN_SX     2     // サブウィンドウの左上X座標
#define STATUSWIN_SY     44    // サブウィンドウの左上Y座標 (メインウィンドウの下に配置)

#define RESUWIN_LINES   100
#define RESUWIN_COLUMS  100
#define RESUWIN_SX      2
#define RESUWIN_SY      1

#define MOVE_UP         'i'    // 上に移動するキー
#define MOVE_LEFT       'j'    // 左に移動するキー
#define MOVE_DOWN       'k'    // 下に移動するキー
#define MOVE_RIGHT      'l'    // 右に移動するキー
#define ENTER           '\n'
#define SPACE           ' '

#define OBJECT_COLOR    100

// サーバーが送信するメッセージの最大長さ
#define SERVER_MSG_LEN   (16 + 16 + 1 + 16 + 16)

// クライアントが送信するメッセージの最大長さ
#define CLIENT_MSG_LEN   (4 + 1)

//--------------------------------------------------------------------
//  鬼ごっこゲームモジュール内部で使用する構造体の定義
//--------------------------------------------------------------------

// サーバーに届く入力データ
typedef struct {
  int myKey;                   // ユーザが押しているキー
  int itKey;                   // 相手の押しているキー
  int quit;                    // ゲームを終了させるメッセージが届いた場合に TRUE
} ServerInputData;

// クライアントに届く入力データ
typedef struct {
  // 自分に関するデータ
  int myKey;                   // ユーザが押しているキー
  int myhitpoint;              // 自分のヒットポイント
  int myX;                     // 自分の X 座標
  int myY;                     // 自分の Y 座標
  bool myAnother;              // 自分のいる世界を示す 0:メイン画面の世界 1:サブ画面の世界
  int mybulletX;               // 自分の弾の X 座標 
  int mybulletY;               // 自分の弾の Y 座標
  int mybullet_remain;         // 自分の弾の生存時間
  bool mybullet_Another;       // 自分の弾がある世界を示す 0:メイン画面の世界 1:サブ画面の世界
  
  // 相手に関するデータ
  int ithitpoint;              // 相手のヒットポイント
  int itX;                     // 相手の X 座標
  int itY;                     // 相手の Y 座標
  bool itAnother;              // 相手のいる世界を示す 0:メイン画面の世界 1:サブ画面の世界
  int itbulletX;               // 相手の弾の X 座標
  int itbulletY;               // 相手の弾の Y 座標
  bool itbullet_Another;       // 相手の弾がある世界を示す 0:メイン画面の世界 1:サブ画面の世界
  int itbullet_remain;         // 相手の弾の生存時間

  // その他のデータ
  int quit;                    // ゲームを終了させるメッセージが届いた場合に TRUE
} ClientInputData;

//--------------------------------------------------------------------
//  鬼ごっこゲームモジュール内部で使用する関数のプロトタイプ宣言
//--------------------------------------------------------------------
static void getServerInputData(TagGame *game, ServerInputData *serverData);
static void getClientInputData(TagGame *game, ClientInputData *clientData);
static void updatePlayerStatus(TagGame *game, ServerInputData *serverData);
static void copyGameState(TagGame *game, ClientInputData *clientData);
static void printGame(TagGame *game);
static void sendGameInfo(TagGame *game);
static void sendMyPressedKey(TagGame *game, ClientInputData *clietData);
static void die();
static int** readmap(char *FileName, TagGame* game, int *MAX_X, int *MAX_Y);
static char** readresult(char *FileName, TagGame* game, int *MAX_X, int *MAX_Y);
static void showresult(bool isWin, TagGame* game);
static void updatemyBulletStatus(TagGame *game, ServerInputData *serverData);
static void updateitBulletStatus(TagGame *game, ServerInputData *serverData);


//--------------------------------------------------------------------
//  外部に公開する関数の定義
//--------------------------------------------------------------------

/*
 * 鬼ごっこゲームの初期化
 * 引数 :
 *   myChara - 自分を表すキャラクタ
 *   mySX    - 自分の開始 X 座標
 *   mySY    - 自分の開始 Y 座標
 *   itChara - 相手を表すキャラクタ
 *   itSX    - 相手の開始 X 座標
 *   itSY    - 相手の開始 Y 座標
 * 戻値 :
 *   鬼ごっこゲームオブジェクトへのポインタ
 */
TagGame* initTagGame(char myChara, int mySX, int mySY,
                     char itChara, int itSX, int itSY)
{
  TagGame* game = (TagGame *)malloc(sizeof(TagGame));

  // すべてのメンバを 0 で初期化
  bzero(game, sizeof(TagGame));

  //
  // ゲームの論理的データの初期化
  //
  game->my.chara = myChara;
  game->my.x     = mySX;
  game->my.y     = mySY;
  game->it.chara = itChara;
  game->it.x     = itSX;
  game->it.y     = itSY;
  game->my.another_world = false;
  game->it.another_world = false;
  game->MAP1_WARP_TARGET_X = -1;
  game->MAP1_WARP_TARGET_Y = -1;
  game->MAP2_WARP_TARGET_X = -1;
  game->MAP2_WARP_TARGET_Y = -1;
  game->MAP1_MAX_X = 0;
  game->MAP1_MAX_Y = 0;
  game->MAP2_MAX_X = 0;
  game->MAP2_MAX_Y = 0;
  game->my.hitpoint = 5;
  game->it.hitpoint = 5;

  game->my_bullet.x = -1;
  game->my_bullet.y = -1;
  game->my_bullet.chara = '+';
  game->my_bullet.direct = -1;
  game->my_bullet.remain = -1;
  game->my_bullet.another_world = false;

  game->it_bullet.x = -1;
  game->it_bullet.y = -1;
  game->it_bullet.chara = '*';
  game->it_bullet.direct = -1;
  game->it_bullet.remain = -1;
  game->it_bullet.another_world = false;

  game->Last_Move_Client = 0;
  game->Last_Move_Server = 0;

  // 前回のプレイヤー情報を初期化(現在のプレイヤー情報と同じにする)
  memcpy(&game->preMy, &game->my, sizeof(Player));
  memcpy(&game->preIt, &game->it, sizeof(Player));

  //
  // 画面の初期化
  //
  initscr();               // curses ライブラリの初期化
  signal(SIGINT, die);     // Ctrl-C 時に端末を復旧する関数 die を登録
  signal(SIGTERM, die);    // kill 時に端末を復旧する関数 die を登録
  noecho();                // エコーバックの中止
  cbreak();                // キーボードバッファリングの中止

  game->Map1 = readmap("O-map.txt", game, &game->MAP1_MAX_X, &game->MAP1_MAX_Y);
  game->Map2 = readmap("S-map.txt", game, &game->MAP2_MAX_X, &game->MAP2_MAX_Y);
  game->Result_Win = readresult("result_win.txt", game, &game->RS_WIN_MAX_X, &game->RS_WIN_MAX_Y);
  game->Result_Lose = readresult("result_lose.txt", game, &game->RS_LOSE_MAX_X, &game->RS_LOSE_MAX_Y);

  // ゲーム画面の作成
  game->MainWin = newwin(MAINWIN_LINES , MAINWIN_COLUMS , MAINWIN_SY, MAINWIN_SX);
  game->SubWin  = newwin(SUBWIN_LINES  , SUBWIN_COLUMS  , SUBWIN_SY , SUBWIN_SX);
  game->StatusWin = newwin(STATUSWIN_LIENS, STATUSWIN_COLUMS  , STATUSWIN_SY , STATUSWIN_SX);


  fprintf(stderr, "g:%d",game->MAP1_MAX_X);
  fprintf(stderr, "g:%d",game->MAP1_MAX_Y);
 
  
  // 画面が小さい場合
  if (game->MainWin == NULL) {
    endwin();
    fprintf(stderr, "Error: terminal size is too small\n");
    exit(1);
  }

  if (game->SubWin == NULL) {
    endwin();
    fprintf(stderr, "Error: terminal size is too small...sub\n");
    exit(1);
  }

  if (game->SubWin == NULL) {
    endwin();
    fprintf(stderr, "Error: terminal size is too small...status\n");
    exit(1);
  }

  
  // 各制御キーはエスケープシーケンスで表現されている
  // それを論理キーコードに変換する
  keypad(game->MainWin, TRUE);
  keypad(game->SubWin,  TRUE);
  keypad(game->StatusWin, TRUE);

  start_color(); // カラーモードの有効化
  can_change_color();

  // カラーペアの定義   
  
  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_GREEN, COLOR_GREEN);
  init_pair(3, COLOR_BLUE, COLOR_BLUE);
  init_pair(4, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(5, COLOR_MAGENTA, COLOR_MAGENTA);
  init_pair(6, COLOR_CYAN, COLOR_CYAN);
  init_pair(7, COLOR_WHITE, COLOR_WHITE);
  init_pair(0, 48, 48);
  init_pair(8, COLOR_BLACK, COLOR_BLACK);
  
  init_pair(100, 254, 254);
  init_pair(110, 159, COLOR_BLACK);
  init_pair(111, 198, COLOR_BLACK);
  init_pair(120, 48, 48); // player
  init_pair(121, 87,87);

  init_pair(200, COLOR_WHITE, COLOR_BLACK);
  
  

  // 背景色の設定し、画面をクリア
  bkgdset(COLOR_PAIR(8));
  clear();


  return game;
}

/*
 * 鬼ごっこゲームの準備
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 *   s    - 相手との会話用ファイルデスクリプタ
 */
void setupTagGame(TagGame *game, int s)
{
  //
  // データ入力のための準備
  //
  game->s = s;                    // 相手との会話用ファイルデスクリプタを登録
  FD_ZERO(&(game->fdset));        // 入力を監視するファイルデスクリプタマスクを初期化
  FD_SET(0, &(game->fdset));      // 標準入力(キーボード)を監視する
  FD_SET(s, &(game->fdset));      // 相手との会話用デスクリプタを監視する
  game->fdsetWidth = s + 1;       // fdset のビット幅(=最大デスクリプタ番号＋１)
  game->watchTime.tv_sec  = 0;    // 監視時間を 100 msec にセット
  game->watchTime.tv_usec = 40 * 500;

  //
  // 画面の準備
  //


  // 画面の静的要素の描画
  wattron(game->MainWin, COLOR_PAIR(2));
  wattron(game->SubWin, COLOR_PAIR(2));
  wattron(game->StatusWin, COLOR_PAIR(2));
  
  box(game->MainWin, ACS_VLINE, ACS_HLINE);
  box(game->SubWin, ACS_VLINE, ACS_HLINE);
  box(game->StatusWin, ACS_VLINE, ACS_HLINE);

  wattroff(game->MainWin, COLOR_PAIR(2));
  wattroff(game->SubWin, COLOR_PAIR(2));
  wattroff(game->StatusWin, COLOR_PAIR(2));

  // 物理画面に描画
  wrefresh(game->MainWin);
  wrefresh(game->SubWin);
  wrefresh(game->StatusWin);
}

/*
 * サーバー側鬼ごっこゲームの開始
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 */
void playServerTagGame(TagGame *game)
{
  ServerInputData serverData;
  bool isWin;
  while (1) {
    
    // 表示する
    printGame(game);

    // ユーザのキー入力と相手から届いたキー入力データを読む
    getServerInputData(game, &serverData);

    // プレイヤーの状態を更新する
    updatePlayerStatus(game, &serverData);

    // 弾の状態を更新する
    updatemyBulletStatus(game, &serverData);
    updateitBulletStatus(game, &serverData);

    // ユーザもしくは相手から終了のメッセージが届いた場合,終了する
    if (serverData.quit)
      break;

    // ゲームの状態を相手に知らせる
    sendGameInfo(game);

    //ゲーム終了
    if(game->my.hitpoint <= 0){
      isWin = false;
      break;
    }

    if(game->it.hitpoint <= 0){
      isWin = true;
      break;
    }
  }

  // 相手も終了するようメッセージを送る
  write(game->s, "quit", 5);  

      // ウィンドウを廃棄
  delwin(game->MainWin);
  delwin(game->SubWin);
  delwin(game->StatusWin);


  showresult(isWin, game);

  destroyTagGame(game);
}


/*
 * クライアント側鬼ごっこゲームの開始
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 */
void playClientTagGame(TagGame *game)
{
  ClientInputData clientData;
  bool isWin;

  while (1) {

    // 表示する
    printGame(game);
    
    // ユーザのキー入力と, 相手から届いたゲームの状態を読む
    getClientInputData(game, &clientData);

    // ゲームの状態を更新する
    copyGameState(game, &clientData);

    // 自分の押しているキーを相手に送る
    sendMyPressedKey(game, &clientData);
    
    // ユーザもしくは相手から終了のメッセージが届いた場合,終了する
    if (clientData.quit)
      break;
    
    //ゲーム終了
    if(game->my.hitpoint <= 0){
      isWin = false;
      break;
    }

    if(game->it.hitpoint <= 0){
      isWin = true;
      break;
    }
  }

  // 相手も終了するようメッセージを送る
  write(game->s, "quit", 5);  

  delwin(game->MainWin);
  delwin(game->SubWin);
  delwin(game->StatusWin);

  showresult(isWin, game);
  
  destroyTagGame(game);

}

/*
 * 鬼ごっこゲームの後始末
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 */
void destroyTagGame(TagGame *game)
{ 
  // ファイルデスクリプタを閉じる
  close(game->s);
  // オブジェクトを解放する
  free(game);
  // 端末を元に戻して終了
  die();
}


//--------------------------------------------------------------------
//  外部に公開しない関数の定義
//--------------------------------------------------------------------

/*
 * サーバー側: データが届いているファイルデスクリプタからデータを読む
 * 引数 :
 *   game       - 鬼ごっこゲームオブジェクトへのポインタ
 *   serverData - 届いたデータを格納する ServerInputData 構造体へのポインタ(出力)
 */
static void getServerInputData(TagGame *game, ServerInputData *serverData)
{
  fd_set  arrived   = game->fdset;        // データが届いたファイルデスクリプタの集合
  TimeVal watchTime = game->watchTime;    // ファイルデスクリプタの監視時間
  char    msg[CLIENT_MSG_LEN];            // 相手から届いたメッセージ

  // すべてのメンバを０で初期化
  // データが届いていない場合, メンバの値は０
  bzero(serverData, sizeof(ServerInputData));

  //
  // データが届いているファイルデスクリプタを調べる
  //
  select(game->fdsetWidth, &arrived, NULL, NULL, &watchTime);
  
  //
  // 標準入力 (キーボード, ０番) にデータが届いている場合
  //
  if (FD_ISSET(0, &arrived)) {
      if(!game->my.another_world){
        serverData->myKey = wgetch(game->MainWin);    // メインウィンドウから入力を取得
      } else {
        serverData->myKey = wgetch(game->SubWin);    // サブウィンドウから入力を取得
      }
      
      // 終了するかどうかチェック
		  if (serverData->myKey == 'p')
			  serverData->quit = TRUE;
  }

  //
  // 相手との会話用ファイルデスクリプタにデータが届いている場合
  //
  if (FD_ISSET(game->s, &arrived)) {
    read(game->s, msg, CLIENT_MSG_LEN);   // メッセージを読み取る
    
    // 終了するかどうかチェック
    if (strcmp(msg, "quit") == 0)
      serverData->quit = TRUE;
    // 届いたメッセージから押下情報を抽出
    else 
      sscanf(msg, "%3d ", &serverData->itKey);
  }

  // すでにデータが届いていた場合, select() は直ちに終了する
  // そこで余った監視時間だけ休止する(Linux でのみ有効)
  usleep(watchTime.tv_usec); 
  // 休止中にたまったキー入力をクリア
  if (serverData->myKey != 0)  
    flushinp(); 
}

/*
 * クライアント側: データが届いているファイルデスクリプタからデータを読む
 * 引数 :
 *   game       - 鬼ごっこゲームオブジェクトへのポインタ
 *   clientData - 届いたデータを格納する ClientInputData 構造体へのポインタ(出力)
 */
static void getClientInputData(TagGame *game, ClientInputData *clientData)
{
  fd_set  arrived   = game->fdset;        // データが届いたファイルデスクリプタの集合
  TimeVal watchTime = game->watchTime;    // ファイルデスクリプタの監視時間
  char    msg[SERVER_MSG_LEN];            // 相手から届いたメッセージ

  // すべてのメンバを０で初期化
  // データが届いていない場合, メンバの値は０
  bzero(clientData, sizeof(ClientInputData));

  //
  // データが届いているファイルデスクリプタを調べる
  //
  select(game->fdsetWidth, &arrived, NULL, NULL, &watchTime);
  
  //
  // 標準入力 (キーボード, ０番) にデータが届いている場合
  //
  if (FD_ISSET(0, &arrived)) {
    if (FD_ISSET(0, &arrived)) {
      if(!game->my.another_world){
        clientData->myKey = wgetch(game->MainWin);    // メインウィンドウから入力を取得
      } else {
        clientData->myKey = wgetch(game->SubWin);    // サブウィンドウから入力を取得
      }  
      // 終了するかどうかチェック
		  if (clientData->myKey == 'p')
			  clientData->quit = TRUE;
  }
      
  }

  //
  // 相手との会話用ファイルデスクリプタにデータが届いている場合
  //
  if (FD_ISSET(game->s, &arrived)) {
    read(game->s, msg, SERVER_MSG_LEN);          // メッセージを読み取る
    lseek(game->s, 0, SEEK_END);

    // 終了するかどうかチェック
    if (strcmp(msg, "quit") == 0)
      clientData->quit = TRUE;
    // 届いたメッセージから座標を抽出
    else {
      // 自分と相手の座標情報を抽出
      int tempBoolit;
      int tempBoolmy;
      int tempBoolit_bullet;
      int tempBoolmy_bullet;
      sscanf(msg, "%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d", 
      &clientData->ithitpoint, &clientData->itX, &clientData->itY, &tempBoolit, &clientData->itbulletX, &clientData->itbulletY, &tempBoolit_bullet, &clientData->itbullet_remain,
      &clientData->myhitpoint, &clientData->myX, &clientData->myY, &tempBoolmy, &clientData->mybulletX, &clientData->mybulletY, &tempBoolmy_bullet, &clientData->mybullet_remain
      );

      clientData->itAnother = (tempBoolit != 0);
      clientData->myAnother = (tempBoolmy != 0);
      clientData->itbullet_Another = (tempBoolit_bullet != 0);  
      clientData->mybullet_Another = (tempBoolmy_bullet != 0);
    }
  }

  // すでにデータが届いていた場合, select() は直ちに終了する
  // そこで余った監視時間だけ休止する(Linux でのみ有効)
  usleep(watchTime.tv_usec); 
  // 休止中にたまったキー入力をクリア
  if (clientData->myKey != 0)  
    flushinp(); 
}

/*
 * プレイヤーの状態を更新する
 * 引数 :
 *   game       - 鬼ごっこゲームオブジェクトへのポインタ
 *   serverData - 鬼ごっこゲームに対する入力データ
 */
static void updatePlayerStatus(TagGame *game, ServerInputData *serverData)
{
  Player *my = &game->my;    // ショートカット
  Player *it = &game->it;    // ショートカット
  Bullet *mb = &game->my_bullet;
  Bullet *ib = &game->it_bullet;

  // 前回のプレイヤー情報を保存
  memcpy(&game->preMy, &game->my, sizeof(Player));
  memcpy(&game->preIt, &game->it, sizeof(Player));

  // Todo:テレポート処理
  if(!my->another_world && game->Map1[my->y][my->x] == 2){ // 世界１から２へ
    my->x = game->MAP1_WARP_TARGET_X;
    my->y = game->MAP1_WARP_TARGET_Y;
    my->another_world = !my->another_world;
    return ;
  } 
    
  if(my->another_world && game->Map2[my->y][my->x] == 2){ // 世界２から１へ
    my->x = game->MAP2_WARP_TARGET_X;
    my->y = game->MAP2_WARP_TARGET_Y;
    my->another_world = !my->another_world;
    return ;
  }

    // Todo:テレポート処理
  if(!it->another_world && game->Map1[it->y][it->x] == 2){ // 世界１から２へ
    it->x = game->MAP1_WARP_TARGET_X;
    it->y = game->MAP1_WARP_TARGET_Y;
    it->another_world = !it->another_world;
    return ;
  } 
    
  if(it->another_world && game->Map2[it->y][it->x] == 2){ // 世界２から１へ
    it->x = game->MAP2_WARP_TARGET_X;
    it->y = game->MAP2_WARP_TARGET_Y;
    it->another_world = !it->another_world;
    return ;
  }
  
  // キーに応じて処理。自分側。
  switch (serverData->myKey) {
  case KEY_UP: 
  // case MOVE_UP: 
    if (my->y > 1) my->y--;
    game->Last_Move_Server = 0;
    break;

  case KEY_DOWN: 
  // case MOVE_DOWN:
    if (my->y < MAINWIN_LINES - 2) my->y++;
    game->Last_Move_Server = 2;
    break;

  case KEY_LEFT: 
  // case MOVE_LEFT:
    if (my->x > 1) my->x--;
    game->Last_Move_Server = 3;
    break;

  case KEY_RIGHT: 
  // case MOVE_RIGHT:
    if (my->x < MAINWIN_COLUMS - 2) my->x++;
    game->Last_Move_Server = 1;
    break;
  
  }

  // キーに応じて処理。相手側
  switch (serverData->itKey) {
  case KEY_UP: 
  // case MOVE_UP: 
    if (it->y > 1) it->y--;
    game->Last_Move_Client = 0;
    break;

  case KEY_DOWN: 
  // case MOVE_DOWN:
    if (it->y < MAINWIN_LINES - 2) it->y++;
    game->Last_Move_Client = 2;
    break;

  case KEY_LEFT: 
  // case MOVE_LEFT:
    if (it->x > 1) it->x--;
    game->Last_Move_Client = 3;
    break;

  case KEY_RIGHT: 
  // case MOVE_RIGHT:
    if (it->x < MAINWIN_COLUMS - 2) it->x++;
    game->Last_Move_Client = 1;
    break;
  
  }

  if(!my->another_world && game->Map1[my->y][my->x] == 1){
    memcpy(&game->my, &game->preMy, sizeof(Player));
  }

  if(my->another_world && game->Map2[my->y][my->x] == 1){
    // 前回のプレイヤー情報を適用
    memcpy(&game->my, &game->preMy, sizeof(Player));
  }

  if(!it->another_world && game->Map1[it->y][it->x] == 1){
    memcpy(&game->it, &game->preIt, sizeof(Player));
  }

  if(it->another_world && game->Map2[it->y][it->x] == 1){
    // 前回のプレイヤー情報を適用
    memcpy(&game->it, &game->preIt, sizeof(Player));
  }

  // 銃弾の判定
  if(my->another_world == ib->another_world && my->y == ib->y && my->x == ib->x){
    my->hitpoint--;
  }

  if(it->another_world == mb->another_world && it->y == mb->y && it->x == mb->x){
    it->hitpoint--;
  }

}

/*
 * ゲームの状態を更新する
 * 引数 :
 *   game       - 鬼ごっこゲームオブジェクトへのポインタ
 *   clientData - 鬼ごっこゲームに対する入力データ
 */
static void copyGameState(TagGame *game, ClientInputData *clientData)
{
  Player *my = &game->my;    // ショートカット
  Player *it = &game->it;    // ショートカット
  Bullet *mbt = &game->my_bullet;
  Bullet *ibt = &game->it_bullet;

  
  // データが届いていなければ, 何もする必要はない
  if (clientData->myX == 0) 
    return; 
  

  // 前回のプレイヤー情報を保存
  memcpy(&game->preMy, &game->my, sizeof(Player));
  memcpy(&game->preIt, &game->it, sizeof(Player));
  
  // 位置を更新
  my->hitpoint = clientData->myhitpoint;
  my->x = clientData->myX;
  my->y = clientData->myY;
  my->another_world = clientData->myAnother;
  mbt->x = clientData->mybulletX;
  mbt->y = clientData->mybulletY;
  mbt->another_world = clientData->myAnother;
  mbt->remain = clientData->mybullet_remain;

  it->hitpoint = clientData->ithitpoint;
  it->x = clientData->itX;
  it->y = clientData->itY;
  it->another_world = clientData->itAnother;
  ibt->x = clientData->itbulletX;
  ibt->y = clientData->itbulletY;
  ibt->another_world = clientData->itAnother;
  ibt->remain = clientData->itbullet_remain;
}

/*
 * ゲーム画面を表示する
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 */
static void printGame(TagGame *game)
{
  WINDOW *mw    = game->MainWin;    // ショートカット
  WINDOW *sw    = game->SubWin;     // ショートカット
  WINDOW *pw    = game->StatusWin;  //
  Player *my    = &game->my;        // ショートカット
  Player *preMy = &game->preMy;     // ショートカット
  Player *it    = &game->it;        // ショートカット
  Player *preIt = &game->preIt;     // ショートカット

  static int pre_my_bullet_x = -1;
  static int pre_my_bullet_y = -1;
  static int pre_it_bullet_x = -1;
  static int pre_it_bullet_y = -1;

  // 自分の弾の描画を消す
  if(pre_my_bullet_x != -1 && pre_my_bullet_y != -1){
    if(!game->my_bullet.another_world){
      mvwaddch(mw, pre_my_bullet_y, pre_my_bullet_x, ' ');
    } else {
      mvwaddch(sw, pre_my_bullet_y, pre_my_bullet_x, ' ');
    }

    pre_my_bullet_x = -1;
    pre_my_bullet_y = -1;
  }

  // 相手の弾の描画を消す
    if(pre_it_bullet_x != -1 && pre_it_bullet_y != -1){
    if(!game->it_bullet.another_world){
      mvwaddch(mw, pre_it_bullet_y, pre_it_bullet_x, ' ');
    } else {
      mvwaddch(sw, pre_it_bullet_y, pre_it_bullet_x, ' ');
    }

    pre_it_bullet_x = -1;
    pre_it_bullet_y = -1;
  }

  
  // 自分の弾の描画
  if(game->my_bullet.remain > 0){
    if(!game->my_bullet.another_world){ // 通常世界の表示
      wattron(mw, COLOR_PAIR(1));
      mvwaddch(mw, game->my_bullet.y + LOGICWIN_SY, game->my_bullet.x + LOGICWIN_SX, ' ');
      wattroff(mw, COLOR_PAIR(1));
    } else { // 異世界の表示
      wattron(sw, COLOR_PAIR(1));
      mvwaddch(sw, game->my_bullet.y + LOGICWIN_SY, game->my_bullet.x + LOGICWIN_SX, ' ');
      wattroff(sw, COLOR_PAIR(1));
    }
    pre_my_bullet_x = game->my_bullet.x + LOGICWIN_SX;
    pre_my_bullet_y = game->my_bullet.y + LOGICWIN_SY;
  }

  // 相手側の弾の描画
  if(game->it_bullet.remain > 0){
    if(!game->it_bullet.another_world){ // 通常世界の表示
      wattron(mw, COLOR_PAIR(1));
      mvwaddch(mw, game->it_bullet.y + LOGICWIN_SY, game->it_bullet.x + LOGICWIN_SX, ' ');
      wattroff(mw, COLOR_PAIR(1));
    } else { // 異世界の表示
      wattron(sw, COLOR_PAIR(1));
      mvwaddch(sw, game->it_bullet.y + LOGICWIN_SY, game->it_bullet.x + LOGICWIN_SX, ' ');
      wattroff(sw, COLOR_PAIR(1));
    }
    pre_it_bullet_x = game->it_bullet.x + LOGICWIN_SX;
    pre_it_bullet_y = game->it_bullet.y + LOGICWIN_SY;
  }



  static int color_index = 0;

  // Mainマップの表示
  for (int i = 0; i < game->MAP1_MAX_Y; i++) {
      for (int j = 0; j < game->MAP1_MAX_X; j++) {
          if (game->Map1[i][j] == 1) {
              wattron(mw, COLOR_PAIR(OBJECT_COLOR));
              mvwaddch(mw, i + LOGICWIN_SY, j + LOGICWIN_SX, ' '); // 障害物の表示
              wattroff(mw, COLOR_PAIR(OBJECT_COLOR));
          } else if (game->Map1[i][j] == 2) {
              wattron(mw, COLOR_PAIR(color_index / 10)); // カラー属性ON (ペア1)
              mvwaddch(mw, i + LOGICWIN_SY, j + LOGICWIN_SX, ' '); // ワープの表示
              wattroff(mw, COLOR_PAIR(color_index / 10)); // カラー属性OFF
          } else if (game->Map1[i][j] == 3) {
              wattron(mw, COLOR_PAIR(4)); // カラー属性ON (ペア2)
              mvwaddch(mw, i + LOGICWIN_SY, j + LOGICWIN_SX, ' '); // ワープ先を表示
              wattroff(mw, COLOR_PAIR(4)); // カラー属性ON (ペア2)
          }
      }
  }

  // Subマップの表示
  for (int i = 0; i < game->MAP2_MAX_Y; i++) {
      for (int j = 0; j < game->MAP2_MAX_X; j++) {
          if (game->Map2[i][j] == 1) {
              wattron(sw, COLOR_PAIR(OBJECT_COLOR));
              mvwaddch(sw, i + LOGICWIN_SY, j + LOGICWIN_SX, ' '); // 障害物の表示
              wattroff(sw, COLOR_PAIR(OBJECT_COLOR));
          } else if (game->Map2[i][j] == 2) {
              wattron(sw, COLOR_PAIR(color_index / 10)); // カラー属性ON (ペア2)
              mvwaddch(sw, i + LOGICWIN_SY, j + LOGICWIN_SX, ' '); // ワープの表示
              wattroff(sw, COLOR_PAIR(color_index / 10)); // カラー属性OFF
          } else if (game->Map2[i][j] == 3) {
              wattron(sw, COLOR_PAIR(4)); // カラー属性ON (ペア2)
              mvwaddch(sw, i + LOGICWIN_SY, j + LOGICWIN_SX, ' '); // ワープ先を表示
              wattroff(sw, COLOR_PAIR(4)); // カラー属性ON (ペア2)
          }
      }
  }
  color_index++;
  color_index %= 70;

  // Todo: マジックナンバー
  for(int i = 0;i < 5;i++){
    mvwaddch(game->StatusWin, LOGICWIN_SY, 12+i*2, ' ');
  }
  for(int i = 0;i < 5;i++){
    mvwaddch(game->StatusWin, LOGICWIN_SY+2, 12+i*2, ' ');
  }

  // Statusマップの表示
  
  for(int i = 0;i < game->my.hitpoint;i++){
    wattron(game->StatusWin, COLOR_PAIR(1));
    mvwaddch(game->StatusWin, LOGICWIN_SY, 12+i*2, ' ');
    wattroff(game->StatusWin, COLOR_PAIR(1));
  }
  for(int i = 0;i < game->it.hitpoint;i++){
    wattron(game->StatusWin, COLOR_PAIR(1));
    mvwaddch(game->StatusWin, LOGICWIN_SY+2, 12+i*2, ' ');
    wattroff(game->StatusWin, COLOR_PAIR(1));
  }
  
  wattron(game->StatusWin, COLOR_PAIR(200));
  mvwaddstr(game->StatusWin, LOGICWIN_SY, LOGICWIN_SX, "You:");
  wattroff(game->StatusWin, COLOR_PAIR(200));

  wattron(game->StatusWin, COLOR_PAIR(120));
  mvwaddch(game->StatusWin, LOGICWIN_SY, LOGICWIN_SX + 6,' ');
  wattroff(game->StatusWin, COLOR_PAIR(120));

  wattron(game->StatusWin, COLOR_PAIR(200));
  mvwaddstr(game->StatusWin, LOGICWIN_SY+2, LOGICWIN_SX,"Enemy:");
  wattroff(game->StatusWin, COLOR_PAIR(200));

  wattron(game->StatusWin, COLOR_PAIR(121));
  mvwaddch(game->StatusWin, LOGICWIN_SY+2, LOGICWIN_SX + 6,' ');
  wattroff(game->StatusWin, COLOR_PAIR(121));

  // printdebug(game, LOGICWIN_SY, LOGICWIN_SX, COLORS);



  // 相手の描画 (もし自分と重なった場合, 自分を上に描画したいので相手が先)
  if(it->another_world == false){
    mvwaddch(mw, preIt->y + LOGICWIN_SY, preIt->x + LOGICWIN_SX, ' ');    // 消去
    wattron(mw, COLOR_PAIR(121));
    mvwaddch(mw, it->y + LOGICWIN_SY, it->x + LOGICWIN_SX, ' ');    // 表示
    wattroff(mw, COLOR_PAIR(121));
  } else { // 別世界の表示
    mvwaddch(sw, preIt->y + LOGICWIN_SY, preIt->x + LOGICWIN_SX, ' ');    // 消去
    wattron(sw, COLOR_PAIR(121));
    mvwaddch(sw, it->y + LOGICWIN_SY, it->x + LOGICWIN_SX, ' ');    // 表示  
    wattroff(sw, COLOR_PAIR(121));
  }

  // 自分の描画
  if(my->another_world == false){
    mvwaddch(mw, preMy->y + LOGICWIN_SY, preMy->x + LOGICWIN_SX, ' ');    // 消去
    wattron(mw, COLOR_PAIR(120));
    mvwaddch(mw, my->y + LOGICWIN_SY, my->x + LOGICWIN_SX , ' ');    // 表示
    wattroff(mw, COLOR_PAIR(120));
  } else { // 別世界の表示
    mvwaddch(sw, preMy->y + LOGICWIN_SY, preMy->x + LOGICWIN_SX, ' ');    // 消去
    wattron(sw, COLOR_PAIR(120));
    mvwaddch(sw, my->y + LOGICWIN_SY, my->x + LOGICWIN_SX, ' ');    // 表示
    wattroff(sw, COLOR_PAIR(120));
  }




  // 物理画面へ描画
  wrefresh(mw);
  wrefresh(sw);
  wrefresh(pw);
}

/*
 * ゲームの状態を相手に知らせる
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 */
static void sendGameInfo(TagGame *game)
{
  Player *my = &game->my;        // ショートカット
  Player *it = &game->it;        // ショートカット
  Bullet  *bmy = &game->my_bullet;
  Bullet  *bit = &game->it_bullet;
  char    msg[SERVER_MSG_LEN];   // 相手に送るメッセージ

  // 自分の情報が変化していなければ送る必要はない
  if (memcmp(&game->my, &game->preMy, sizeof(Player)) == 0 &&
      memcmp(&game->it, &game->preIt, sizeof(Player)) == 0 && 
      memcmp(&game->my_bullet, &game->pre_my_bullet, sizeof(Bullet)) == 0 &&
      memcmp(&game->it_bullet, &game->pre_it_bullet, sizeof(Bullet)) == 0)
    return;

  //
  // 位置座標をメッセージに変換
  //

  // プレイヤーの座標情報
  sprintf(msg, "%3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d", 
  my->hitpoint, my->x, my->y, my->another_world, bmy->x, bmy->y, bmy->another_world, bmy->remain,
  it->hitpoint, it->x, it->y, it->another_world, bit->x, bit->y, bit->another_world, bit->remain);

  // 送る
  write(game->s, msg, SERVER_MSG_LEN);    
}

/*
 * 自分の押しているキーを相手に送る
 * 引数 :
 *   game      - 鬼ごっこゲームオブジェクトへのポインタ
 *   clietData - クライアントに対する入力データ
 */
static void sendMyPressedKey(TagGame *game, ClientInputData *clietData)
{
  char    msg[CLIENT_MSG_LEN];   // 相手に送るメッセージ

  
  // 何も押されていなければ送る必要はない
  if (clietData->myKey == 0)
    return;
  

  //
  // 位置座標をメッセージに変換
  //

  // プレイヤーの座標情報
  sprintf(msg, "%d", clietData->myKey);

  // 送る
  write(game->s, msg, CLIENT_MSG_LEN);    
}

/*
 * 端末を復元し終了する
 */
static void die()
{
  endwin();
  exit(1);
}


static int** readmap(char *FileName, TagGame* game, int *MAX_X, int *MAX_Y)
{
  int N = 100;
  FILE *fp;
  char readline[N];
  int i, j = 0;
  int lines, columes;

  int** map = (int**)malloc(sizeof(int*) * N);
  for(int i = 0;i < N; i++){
    map[i] = malloc(sizeof(int) * N);
  }

  if((fp = fopen(FileName , "r")) == NULL){
    fprintf(stderr, "cannot open %s.\n", FileName );
    exit(1);
  }

  if(fscanf(fp, "%d, %d", &lines, &columes) != EOF){
    printf("line: %d, columns: %d\n", lines, columes);
  } else {
    fprintf(stderr, "format error: %s. \n", FileName );
  }

  i = 0;
  while (fgets(readline, N, fp) != NULL){
    // 1行目はスキップ
    if(i == 0){
      i++;
      continue;
    }
		
		// 2行目以降をマップに読み込む
		for(j = 0; j < columes; j++){
			
			if(readline[j] == '#'){
				map[i-1][j] = 1;
			} else if(readline[j] == 'W'){
        map[i-1][j] = 2;
      }	else if(readline[j] == 'T') {
        map[i-1][j] = 3;

        if(game->MAP2_WARP_TARGET_X == -1 && game->MAP2_WARP_TARGET_Y == -1){ // Todo:もし一個目のマップのTならば
          game->MAP2_WARP_TARGET_X = j;
          game->MAP2_WARP_TARGET_Y = i-1;
        } else {
          game->MAP1_WARP_TARGET_X = j;
          game->MAP1_WARP_TARGET_Y = i-1;         
        }
       
      } else {
				map[i-1][j] = 0;
			}
		}
		
		i++;
  }

  *MAX_X = columes;
  *MAX_Y = i-1;

  fclose(fp);
  return map;
}

static char** readresult(char *FileName, TagGame* game, int *MAX_X, int *MAX_Y)
{
  int N = 100;
  FILE *fp;
  char readline[N];
  int i, j = 0;
  int lines, columes;

  char** map = (char**)malloc(sizeof(int*) * N);
  for(int i = 0;i < N; i++){
    map[i] = malloc(sizeof(int) * N);
  }

  if((fp = fopen(FileName , "r")) == NULL){
    fprintf(stderr, "cannot open %s.\n", FileName );
    exit(1);
  }

  if(fscanf(fp, "%d, %d", &lines, &columes) != EOF){
    printf("line: %d, columns: %d\n", lines, columes);
  } else {
    fprintf(stderr, "format error: %s. \n", FileName );
  }

  i = 0;
  while (fgets(readline, N, fp) != NULL){

    // 1行目はスキップ
    if(i == 0){
      i++;
      continue;
    }
		
		for(j = 0; j < columes; j++){
      map[i-1][j] = readline[j];
		}
		
		i++;
  }

  *MAX_X = columes;
  *MAX_Y = i-1;

  fclose(fp);
  return map;
}


//--------------------------------------------------------------------
//  自分で作成した関数の定義
//--------------------------------------------------------------------

static void showresult(bool isWin, TagGame* game)
{
  WINDOW *RESULTWIN = newwin(RESUWIN_LINES , RESUWIN_COLUMS , RESUWIN_SY, RESUWIN_SX);
  wattron(RESULTWIN, COLOR_PAIR(2));
  box(RESULTWIN, ACS_VLINE, ACS_HLINE);
  wattroff(RESULTWIN, COLOR_PAIR(2));
  wrefresh(RESULTWIN);
  if(isWin){//Todo:勝敗の判定

    // 勝利時の処理
    wattron(RESULTWIN, COLOR_PAIR(111));
    for(int i = 0;i < game->RS_WIN_MAX_Y;i++){
      for(int j = 0;j < game->RS_WIN_MAX_X;j++){
        mvwaddch(RESULTWIN, i + LOGICWIN_SY, j + LOGICWIN_SX , game->Result_Win[i][j]); // 障害物の表示
      }
    }
    wattroff(RESULTWIN, COLOR_PAIR(111));

  } else {

     // 敗北時の処理
    wattron(RESULTWIN, COLOR_PAIR(110));
    for(int i = 0;i < game->RS_LOSE_MAX_Y;i++){
      for(int j = 0;j < game->RS_LOSE_MAX_X;j++){
          mvwaddch(RESULTWIN, i + LOGICWIN_SY, j + LOGICWIN_SX , game->Result_Lose[i][j]); // 障害物の表示
      }
    }
    wattroff(RESULTWIN, COLOR_PAIR(110));

  }
  wrefresh(RESULTWIN);
  sleep(10);
  delwin(RESULTWIN);
}

static void updatemyBulletStatus(TagGame *game, ServerInputData *serverData){
  
  if(game->my_bullet.remain <= 0){
    switch (serverData->myKey) {
      case ENTER: 
      case SPACE:
        game->my_bullet.x = game->my.x;
        game->my_bullet.y = game->my.y;
        game->my_bullet.another_world = game->my.another_world;
        game->my_bullet.remain = 20;
        game->my_bullet.direct = game->Last_Move_Server;
        break;
      case MOVE_UP:
        game->my_bullet.x = game->my.x;
        game->my_bullet.y = game->my.y;
        game->my_bullet.another_world = game->my.another_world;
        game->my_bullet.remain = 30;
        game->my_bullet.direct = 0;
        break;
      case MOVE_RIGHT:
        game->my_bullet.x = game->my.x;
        game->my_bullet.y = game->my.y;
        game->my_bullet.another_world = game->my.another_world;
        game->my_bullet.remain = 30;
        game->my_bullet.direct = 1;
        break;
      case MOVE_DOWN:
        game->my_bullet.x = game->my.x;
        game->my_bullet.y = game->my.y;
        game->my_bullet.another_world = game->my.another_world;
        game->my_bullet.remain = 30;
        game->my_bullet.direct = 2;
        break;
      case MOVE_LEFT:
        game->my_bullet.x = game->my.x;
        game->my_bullet.y = game->my.y;
        game->my_bullet.another_world = game->my.another_world;
        game->my_bullet.remain = 30;
        game->my_bullet.direct = 3;
        break;
    }
  }

  // 弾の移動
  if(game->my_bullet.remain > 0){
    game->my_bullet.remain--;
    switch(game->my_bullet.direct){
      case 0:
        game->my_bullet.y--;
        break;
      case 1:
        game->my_bullet.x++;
        break;
      case 2:
        game->my_bullet.y++;
        break;
      case 3:
        game->my_bullet.x--;
        break;
    }

    //　弾の消滅を管理
    if(!game->my_bullet.another_world){ // 通常世界の処理

      if(game->my_bullet.remain <= 0){
        return ;
      }
      
      if(game->my_bullet.y < 0 || game->my_bullet.y >= game->MAP1_MAX_Y){
        game->my_bullet.remain = -1;
        return ;
      }

      if(game->my_bullet.x < 0 || game->my_bullet.x >= game->MAP1_MAX_X){
        game->my_bullet.remain = -1;
        mvwaddch(game->MainWin, game->pre_my_bullet.y + LOGICWIN_SY, game->pre_my_bullet.x + LOGICWIN_SX, ' ');
        return ;
      }

      if(game->Map1[game->my_bullet.y][game->my_bullet.x] == 1){
        game->my_bullet.remain = -1;
        return ;
      }
    } else { // 異世界での処理

      if(game->my_bullet.remain <= 0){
        return ;
      }
      
      if(game->my_bullet.y < 0 || game->my_bullet.y >= game->MAP2_MAX_Y){
        game->my_bullet.remain = -1;
        return ;
      }

      if(game->my_bullet.x < 0 || game->my_bullet.x >= game->MAP2_MAX_X){
        game->my_bullet.remain = -1;
        return ;
      }

      if(game->Map2[game->my_bullet.y][game->my_bullet.x] == 1){
        game->my_bullet.remain = -1;
        return ;
      }
    }
    
}
}

static void updateitBulletStatus(TagGame *game, ServerInputData *serverData){

  if(game->it_bullet.remain <= 0){
    switch (serverData->itKey) {
      case ENTER: 
      case SPACE:
        game->it_bullet.x = game->it.x;
        game->it_bullet.y = game->it.y;
        game->it_bullet.another_world = game->it.another_world;
        game->it_bullet.remain = 30;
        game->it_bullet.direct = game->Last_Move_Client;
        break;
      case MOVE_UP:
        game->it_bullet.x = game->it.x;
        game->it_bullet.y = game->it.y;
        game->it_bullet.another_world = game->it.another_world;
        game->it_bullet.remain = 30;
        game->it_bullet.direct = 0;
        break;
      case MOVE_RIGHT:
        game->it_bullet.x = game->it.x;
        game->it_bullet.y = game->it.y;
        game->it_bullet.another_world = game->it.another_world;
        game->it_bullet.remain = 30;
        game->it_bullet.direct = 1;
        break;
      case MOVE_DOWN:
        game->it_bullet.x = game->it.x;
        game->it_bullet.y = game->it.y;
        game->it_bullet.another_world = game->it.another_world;
        game->it_bullet.remain = 30;
        game->it_bullet.direct = 2;
        break;
      case MOVE_LEFT:
        game->it_bullet.x = game->it.x;
        game->it_bullet.y = game->it.y;
        game->it_bullet.another_world = game->it.another_world;
        game->it_bullet.remain = 30;
        game->it_bullet.direct = 3;
        break;
    }
  }

  // 弾の移動
  if(game->it_bullet.remain > 0){
    game->it_bullet.remain--;
    switch(game->it_bullet.direct){
      case 0:
        game->it_bullet.y--;
        break;
      case 1:
        game->it_bullet.x++;
        break;
      case 2:
        game->it_bullet.y++;
        break;
      case 3:
        game->it_bullet.x--;
        break;
    }

    //　弾の消滅を管理
    if(!game->it_bullet.another_world){ // 通常世界の処理

      if(game->it_bullet.remain <= 0){
        return ;
      }
      
      if(game->it_bullet.y < 0 || game->it_bullet.y >= game->MAP1_MAX_Y){
        game->it_bullet.remain = -1;
        return ;
      }

      if(game->it_bullet.x < 0 || game->it_bullet.x >= game->MAP1_MAX_X){
        game->it_bullet.remain = -1;
        return ;
      }

      if(game->Map1[game->it_bullet.y][game->it_bullet.x] == 1){
        game->it_bullet.remain = -1;
        return ;
      }
    } else { // 異世界での処理

      if(game->it_bullet.remain <= 0){
        return ;
      }
      
      if(game->it_bullet.y < 0 || game->it_bullet.y >= game->MAP2_MAX_Y){
        game->it_bullet.remain = -1;
        return ;
      }

      if(game->it_bullet.x < 0 || game->it_bullet.x >= game->MAP2_MAX_X){
        game->it_bullet.remain = -1;
        return ;
      }

      if(game->Map2[game->it_bullet.y][game->it_bullet.x] == 1){
        game->it_bullet.remain = -1;
        return ;
      }
    }

}
}