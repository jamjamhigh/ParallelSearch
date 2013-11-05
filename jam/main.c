#include <stdio.h>
#include <omp.h>
#include <windows.h>
#include <process.h>

int main()
{
	//testSearch();

	/*
	int NUM_THREADS = omp_get_num_procs();
	
	printf("���̃v���Z�b�T����%d�ł�\n", NUM_THREADS);
	Sleep(5000);
	return 0;
	*/

	/*
#pragma omp parallel
	printf("Hello!\n");
	Sleep(5000);
	return 0;
	*/
}


// �\����
typedef struct{
	int *A; //�����z��
	int num; //������
	int key; //�T������
	int threadID; //�X���b�h�̐�
} sParam;
//���̍\���̖̂��O�́usParam�v

BOOL Done = FALSE; //�T�[�`�I����ʒm���邽�߂̃O���[�o���t���O


// ���\�[�g�f�[�^���T�[�`����֐�
// A:�����z��@N:�������@key:�T�[�`���镶���@position:�ڎ�
void LinearSearch(int *A, int N, int key, int *position)
{
	int i;
	*position = -1; // ������Ȃ��ꍇ��-1
	
	#pragma omp parallel for // for���̕��s���������s��
	for(i = 0; i < N; i++){
		if(A[i] == key){
			*position = i;
		}
	}
}


//���j�A�T�[�`�֐��i�ォ�牺�܂ŏ��ɂ���������֐��j
/* 
A:�����z��@s:���ׂ�X�^�[�g�ʒu�@e:���׏I���ʒu�@key:�T�[�`���镶���@position:�ڎ�
DWORD�Ƃ͂��Ƃ���unsigned long�̌^�̂��Ƃł���
int�̑���Ɏg���Ă��邪�c�c�Ȃ��H
*/
void LinearPSearch(int *A, int s, int e, int key, DWORD *position)
{	
	int i;
	
	for(i = s; i < e; i++){
		if(Done) return; //�����T�[�`���I�����Ă���Ȃ�ΏI��
		if(A[i] == key){ //���������ꍇ
			*position = i; //���̃|�C���^�̖ڎ��������������ԍ��ɂ���
			Done = TRUE; //���������t���O��^�ɂ���
			break; //�J��Ԃ��I��
		}
	}
	return;
}


//�w���p�֐�
/*
����LinerPSearch���g���Ă���B
�p�����[�^�[�i�\���̂ɂ���ϐ��Ȃǁj��W�J����������j�A�T�[�`�֐��֓n���Č��ʂ𗘗p�\�ɂ���B
����ɏo�Ă���LPVOID�Ƃ́A������^�̃f�[�^�ւ̃|�C���^�ł���B
*/
unsigned __stdcall pSearch(LPVOID pArg)
{
	// �󂯎�����\���̂��|�C���^���Ƃɓ��Ă͂߂Ă���
	sParam *inArg = (sParam *)pArg;
	int *A = inArg->A;
	int N = inArg->num;
	int key = inArg->key;
	int tNum = inArg->threadID;
	
	int start, end;
	DWORD pos = -1;

	// �v���Z�b�T�����`�A���Ă͂߂�B�@�����Ŏg���Ă���֐���OpenMP�̂��́B
	int NUM_THREADS = omp_get_num_procs();
	
	start = ((int)N/NUM_THREADS) * tNum; //�i���������v���Z�b�T���j�~�X���b�h��
	end = ((int)N/NUM_THREADS) * (tNum+1);
	if(tNum == (NUM_THREADS - 1)) end = N;
	
	LinearPSearch(A, start, end, key, &pos);
	//delete inArg; //���������
	free(inArg); //�������J���idelete�̑���j
	ExitThread(pos); //�X���b�h���I������
}



void testSearch(int *S, int NumKeys, int sKey, int i, int *position)
{
	int NUM_THREADS = omp_get_num_procs();
	HANDLE tH[1024];

	for(i = 0; i < NUM_THREADS; i++)
	{
		sParam *pArg = new sParam;
		pArg->A = S;
		pArg->num = NumKeys;
		pArg->key = sKey;
		pArg->threadID = i;
		//�X���b�h�����B
		//�R�Ԗڂ̓X���b�h�֐��̃A�h���X���A�S�Ԗڂ��X���b�h�֐��ɓn������
		//�T�Ԗڂ�0�ł������s�Ƃ���
		tH[i] = (HANDLE) _beginthreadex(NULL, 0, pSearch, (LPVOID)pArg, 0, NULL);
	}

	//�A���[�g�\�ȑҋ@��Ԃɓ���
	WaitForMultipleObjects(NUM_THREADS, tH, TRUE, INFINITE);

	for(i = 0; i < NUM_THREADS; i++){
		// LPDWORD��DWORD�^�ւ̃|�C���^�ł���
		GetExitCodeThread(tH[i], (LPDWORD)position);
		if(*position != -1){
			printf("key = %d found at index %d\n", sKey, *position);
			break;
		}
	}
	if(*position == -1) printf("key = %d NOT found.\n", sKey);
}
