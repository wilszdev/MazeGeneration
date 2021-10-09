#pragma once

#include <stdint.h>
#include <sstream>
#include <string>
#include <random>

// masks for the node data
#define MAZE_NODE_CONNECTION_NORTH 0x01
#define MAZE_NODE_CONNECTION_SOUTH 0x02
#define MAZE_NODE_CONNECTION_EAST  0x04
#define MAZE_NODE_CONNECTION_WEST  0x08
#define MAZE_NODE_VISITED		   0x10

// put connection directions into an array so it's easier to select one
constexpr uint8_t MAZE_NODE_CONNECTION_DIRECTIONS[4] =
	{ MAZE_NODE_CONNECTION_NORTH, MAZE_NODE_CONNECTION_SOUTH, MAZE_NODE_CONNECTION_EAST, MAZE_NODE_CONNECTION_WEST };

struct MAZE_IMAGE_RESULT
{
	size_t heightPx;
	size_t  widthPx;
	size_t strideBytes;
	size_t numChannels;
	uint8_t* memory;
};

class Maze
{
public:
	using NodeType = uint8_t;
public:
	Maze();
	Maze(size_t dimension);
	Maze(size_t dimensionX, size_t dimensionY);
	~Maze();
	void PrintAsAsciiMap();
	void GetImage(MAZE_IMAGE_RESULT* result, int scale = 4,
		uint32_t backColour = 0xFF000000,  // white
		uint32_t foreColour = 0xFFFFFFFF); // black
	void FreeImage(MAZE_IMAGE_RESULT* image);
	bool WriteImageToFile(MAZE_IMAGE_RESULT* image, const char* path);
	void Generate();
private:
	inline NodeType* GetNode(size_t x, size_t y);
	inline NodeType* GetTargetNode(uint8_t directionMask, size_t originX, size_t originY);
	inline bool TargetNodeIsValid(NodeType* target);
private:
	size_t m_X = 8;
	size_t m_Y = 8;
	NodeType* m_Nodes = nullptr;
};