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
		printf("オープンファイル名を指定してください\n");
		exit(EXIT_FAILURE);
	}

	/* バイナリ書き込み読み込みモードでファイルをオープン */
	if ((fp = fopen(argv[1], "wb+")) == NULL) {
		printf("ファイルオープンエラー\n");
		exit(EXIT_FAILURE);
	}

	/* ファイルにデータを書き込み */
	fwrite(outbf, sizeof(int), 10, fp);

	/* ファイルの先頭に移動 */
	fseek(fp, 0L, SEEK_SET);

	/* 書き込んだデータを読み込んでみる */
	fread(inbuf, sizeof(int), 10, fp);

	/* ファイルクローズ */
	fclose(fp);

	/* 読み込みデータの確認 */
	for (i = 0; i < 10; i++)
		printf("%3d", inbuf[i]);
	printf("\n");

	return 0;
}
