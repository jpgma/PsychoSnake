#include <stdio.h>
#include <windows.h>
#define comprimento 20
#define altura 10

static char LOADED_BITMAP[] = 
{
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'
};

int main ()
{

	char bitmap [altura*comprimento];
	int alturaP, comprimentoP;
	for(int i=1;i<=altura;i++)
	{
		for(int j=1;j<=comprimento;j++)
		{	
			bitmap[i*j] = LOADED_BITMAP[i*j];
		}

	}
	/*printf("Posicao: \n");
	printf("Altura 1-10: ");
	scanf("%d", &alturaP);
	printf("Comprimento 1-20: ");
	scanf("%d", &comprimentoP);
	bitmap[alturaP*comprimentoP] = 'x';

*/
	int z=1;
	while(z<=3)
	{
		for(int i=1;i<=altura; i++)
		{
			for(int j=1; j<=comprimento; j++)
			{
				if(z==1)
				{

					bitmap[1*1] = 'x';	
				}
				else if(z==2)
				{
					bitmap[1*2] = 'x';
				}
				else if(z==3)
				{
					bitmap[1*3] = 'x';
				}
				printf("%c", bitmap[i*j]);
			}
			printf("\n");
		}
		
		z++;
		printf("\n");
	}



	return 0;
}