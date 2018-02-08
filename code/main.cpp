#include <time.h>
#include <stdio.h>
#include <windows.h>

#define WIN32_KEY_DOWN 0x8000
#define IS_KEY_DOWN(key) ((GetAsyncKeyState(key) & WIN32_KEY_DOWN) == WIN32_KEY_DOWN)

void main()
{
	int frame_count = 0;
	float time_since_last = 0.0f;
	
	clock_t frame_start = clock();
	while(!IS_KEY_DOWN(VK_ESCAPE))
	{

		//frame stuff

		frame_count++;
		clock_t frame_end = clock();
		float diff_seconds = ((float)(frame_end - frame_start) / 1000000.0f ) * 1000;  
		time_since_last += diff_seconds;
		if(time_since_last >= 1.0f)
		{
			printf("fps: %d\n", frame_count);
			time_since_last = 0.0f;
			frame_count = 0;
		}
		frame_start = clock();
	}
}