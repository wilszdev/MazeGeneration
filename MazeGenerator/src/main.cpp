#include "Maze.h"
#include "Timer.h"

#include <cstdio>

#define PNG_MAKE_RGBA(r, g, b, a) ((a << 24) | (b << 16) | (g << 8) | r)
#define PNG_MAKE_RGB(r, g, b) PNG_MAKE_RGBA(r, g, b, 255)

int main(int argc, char** argv)
{
	// mazes.exe out.png width height
	if (argc != 4)
	{
		printf("usage: %s <output path> <width in nodes> <height in nodes> \n", argv[0]);
		return 1;
	}

#define outputPath argv[1]

	size_t mazeDimX = atoi(argv[2]);
	size_t mazeDimY = atoi(argv[3]);

	Maze maze{ mazeDimX, mazeDimY };
	Timer timer; float time;

	printf("generating maze of %zd by %zd nodes... ", mazeDimX, mazeDimY);
	timer.Mark();
	maze.Generate();
	time = timer.Peek();
	printf("took %f seconds\n", time);

	printf("generating png image... ");
	timer.Mark();
	MAZE_IMAGE_RESULT image;
	maze.GetImage(&image, 2,
		PNG_MAKE_RGB(0x00, 0x00, 0x00),
		PNG_MAKE_RGB(0xFF, 0xFF, 0xFF));
	time = timer.Peek();
	printf("took %f seconds\n", time);

	printf("writing image to \"%s\"... ", outputPath);
	timer.Mark();
	bool success = maze.WriteImageToFile(&image, outputPath);
	time = timer.Peek();

	if (success)
	{
		printf("took %f seconds\n", time);
	}
	else
	{
		puts("failed");
	}

	maze.FreeImage(&image);
	return 0;
}