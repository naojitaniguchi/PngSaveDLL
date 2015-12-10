// FileIOCheck.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	int outbf[10] = { 1,2,3,4,5,6,7,8,9,10 };
	int inbuf[10];
	int i;

	if (argc != 2) {
		printf("�I�[�v���t�@�C�������w�肵�Ă�������\n");
		exit(EXIT_FAILURE);
	}

	/* �o�C�i���������ݓǂݍ��݃��[�h�Ńt�@�C�����I�[�v�� */
	if ((fp = fopen(argv[1], "wb+")) == NULL) {
		printf("�t�@�C���I�[�v���G���[\n");
		exit(EXIT_FAILURE);
	}

	/* �t�@�C���Ƀf�[�^���������� */
	fwrite(outbf, sizeof(int), 10, fp);

	/* �t�@�C���̐擪�Ɉړ� */
	fseek(fp, 0L, SEEK_SET);

	/* �������񂾃f�[�^��ǂݍ���ł݂� */
	fread(inbuf, sizeof(int), 10, fp);

	/* �t�@�C���N���[�Y */
	fclose(fp);

	/* �ǂݍ��݃f�[�^�̊m�F */
	for (i = 0; i < 10; i++)
		printf("%3d", inbuf[i]);
	printf("\n");

	return 0;
}
