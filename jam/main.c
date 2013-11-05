#include <stdio.h>
#include <omp.h>
#include <windows.h>
#include <shlobj.h>
#include <stdlib.h>
#include <math.h>

int main()
{
	int NUM_THREADS = omp_get_num_procs();
	
	printf("私のプロセッサ数は%dです\n", NUM_THREADS);
	Sleep(5000);
	return 0;

	/*
#pragma omp parallel
	printf("Hello!\n");
	Sleep(5000);
	return 0;
	*/
}


// 構造体
typedef struct{
	int *A; //文字配列
	int num; //文字数
	int key; //探す文字
	int threadID; //スレッドの数
} sParam;
//この構造体の名前は「sParam」

BOOL Done = FALSE; //サーチ終了を通知するためのグローバルフラグ


// 未ソートデータをサーチする関数
// A:文字配列　N:文字数　key:サーチする文字　position:目次
void LinearSearch(int *A, int N, int key, int *position)
{
	int i;
	*position = -1; // 見つからない場合は-1
	
	#pragma omp parallel for // for文の並行化処理を行う
	for(i = 0; i < N; i++){
		if(A[i] == key){
			*position = i;
		}
	}
}


//リニアサーチ関数（上から下まで順にを検索する関数）
/* 
A:文字配列　s:調べるスタート位置　e:調べ終わる位置　key:サーチする文字　position:目次
DWORDとはもともとunsigned longの型のことである
intの代わりに使われているが……なぜ？
*/
void LinearPSearch(int *A, int s, int e, int key, DWORD *position)
{	
	int i;
	
	for(i = s; i < e; i++){
		if(Done) return; //もしサーチが終了しているならば終了
		if(A[i] == key){ //見つかった場合
			*position = i; //そのポインタの目次を見つけた文字番号にする
			Done = TRUE; //見つかったフラグを真にする
			break; //繰り返し終了
		}
	}
	return;
}


//ヘルパ関数
/*
中でLinerPSearchを使っている。
パラメーター（構造体にある変数など）を展開しそれをリニアサーチ関数へ渡して結果を利用可能にする。
直後に出てくるLPVOIDとは、あらゆる型のデータへのポインタである。
*/
unsigned __stdcall pSearch(LPVOID pArg)
{
	// 受け取った構造体をポインタごとに当てはめている
	sParam *inArg = (sParam *)pArg;
	int *A = inArg->A;
	int N = inArg->num;
	int key = inArg->key;
	int tNum = inArg->threadID;
	
	int start, end;
	DWORD pos = -1;

	// プロセッサ数を定義、当てはめる。　ここで使っている関数はOpenMPのもの。
	int NUM_THREADS = omp_get_num_procs();
	
	start = ((int)N/NUM_THREADS) * tNum; //（文字数÷プロセッサ数）×スレッド数
	end = ((int)N/NUM_THREADS) * (tNum+1);
	if(tNum == (NUM_THREADS - 1)) end = N;
	
	LinearPSearch(A, start, end, key, &pos);
	delete inArg; //メモリ解放
	ExitThread(pos); //スレッドを終了する
}


/*
	// act5
for(i = 0; i < NUM_THREADS; i++)
{
	sParam *pArg = new sParam;
	pArg->A = S;
	pArg->num = NumKeys;
	pArg->key = skey;
	pArg->threadID = i;
	tH[i] = (HANDLE) _beginthreadex(NULL, 0, pSearch, (LPVOID)pArg, 0, NULL);
}

WaitForMultipleObjects(NUM_THREADS, tH, TRUE, INFINITE);

for(i = 0; i < NUM_THREAD; i++){
	GetExitCodeThread(tH[i], (LPWORD)position);
	if(*position != -1){
		printf("key = %d found at index %d\n", sKey, *position);
		break;
	}
}
if(*position == -1) printf("key = %d NOT found.\n", sKey);

}
*/
