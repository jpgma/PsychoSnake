#include <stdio.h>
#include <windows.h>
#define comprimento 32
#define altura 24

int main ()
{

	char bitmap [altura][comprimento];
	
	for(int i=0;i<altura;i++)
	{
		for(int j=0;j<comprimento;j++)
		{	
			bitmap[i][j] = -80;
		}

	}
	
	for(int i=0;i<altura; i++)
	{
		for(int j=0; j<comprimento; j++)
		{
			printf("%c", bitmap[i][j]);
		}
		printf("\n");
	}



	return 0;
}