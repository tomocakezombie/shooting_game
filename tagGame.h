/********************************************************************
                       鬼ごっこゲームモジュール
                            ヘッダファイル
 ********************************************************************/ 
#include <curses.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

//--------------------------------------------------------------------
//   鬼ごっこゲームモジュールにおける型の定義
//--------------------------------------------------------------------
typedef struct timeval TimeVal;  // 簡便のために構造体に別名を定義

typedef struct {
  char    chara;
  int     x;
  int     y;
  int     remain;
  int     direct;
  bool    another_world;
} Bullet;

/*
 * プレーヤーデータ構造体の定義
 */
typedef struct {
  char    chara;                 // 自分を表すキャラクタ
  int     x;                     // 自分の X 座標
  int     y;                     // 自分の Y 座標
  bool another_world;
  int  hitpoint;
} Player;


/*
 * 鬼ごっこゲーム構造体の定義
 */
typedef struct {
  // ゲームのプレイヤーに関する変数
  Player  my;                    // 自分のデータ
  Player  preMy;                 // 前回の自分のデータ
  Player  it;                    // 相手のデータ
  Player  preIt;                 // 前回の相手のデータ

  // ゲームの弾に関する変数
  Bullet my_bullet;              // 自分の弾のデータ
  Bullet pre_my_bullet;          // 前回の自分の弾のデータ
  Bullet it_bullet;              // 相手の弾のデータ
  Bullet pre_it_bullet;          // 前回の相手の弾のデータ

  // ゲームのマップに関する変数
  int **Map1;                    // マップ1情報を保持する配列 1:障害物 0:空白 2:WARP入口 3:WARP出口
  int **Map2;                    // マップ2情報を保持する配列 1:障害物 0:空白 2:WARP入口 3:WARP出口
  int MAP1_MAX_X;                // マップ1の配列で最大横幅を示す
  int MAP1_MAX_Y;                // マップ1の配列で最大縦幅を示す
  int MAP2_MAX_X;                // マップ2の配列で最大横幅を示す
  int MAP2_MAX_Y;                // マップ2の配列で最大縦幅を示す
  int MAP1_WARP_TARGET_X;        // マップ1のワープ先のX座標を示す
  int MAP1_WARP_TARGET_Y;        // マップ1のワープ先のY座標を示す
  int MAP2_WARP_TARGET_X;        // マップ2のワープ先のX座標を示す
  int MAP2_WARP_TARGET_Y;        // マップ2のワープ先のY座標を示す

  // ゲームの結果表示に関する変数
  char **Result_Win;              // 勝利した時のリザルト画面情報を保持する配列
  char **Result_Lose;             // 敗北した時のリザルト画面情報を保持する配列
  int RS_WIN_MAX_X;              // 勝利リザルトの配列で最大横幅を示す
  int RS_WIN_MAX_Y;              // 勝利リザルトの配列で最大縦幅を示す
  int RS_LOSE_MAX_X;             // 敗北リザルトの配列で最大横幅を示す
  int RS_LOSE_MAX_Y;             // 敗北リザルトの配列で最大縦幅を示す

  // 画面関連のデータ
  WINDOW *MainWin;               // メインウィンドウ
  WINDOW *SubWin;                // サブウィンドウ
  WINDOW *StatusWin;             // ステータスを表すウィンドウ
  
  // 入力関連のデータ
  int     s;                     // 相手との会話用ファイルデスクリプタ
  fd_set  fdset;                 // 入力を監視するファイルデスクリプタの集合
  int     fdsetWidth;            // fdset のビット幅(=最大デスクリプタ番号＋１)
  TimeVal watchTime;             // 監視時間
  int Last_Move_Client;          // クライアント側が最後に入力した移動方向を保持する
  int Last_Move_Server;          // クライアント側が最後に入力した移動方向を保持する
} TagGame;


//--------------------------------------------------------------------
//   鬼ごっこゲームモジュールが外部に公開する関数のプロトタイプ宣言
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
                     char itChara, int itSX, int itSY);

/*
 * 鬼ごっこゲームの準備
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 *   s    - 相手との会話用ファイルデスクリプタ
 */
void setupTagGame(TagGame *game, int s);

/*
 * サーバー側鬼ごっこゲームの開始
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 */
void playServerTagGame(TagGame *game);

/*
 * クライアント側鬼ごっこゲームの開始
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 */
void playClientTagGame(TagGame *game);

/*
 * 鬼ごっこゲームの後始末
 * 引数 :
 *   game - 鬼ごっこゲームオブジェクトへのポインタ
 */
void destroyTagGame(TagGame *game);

