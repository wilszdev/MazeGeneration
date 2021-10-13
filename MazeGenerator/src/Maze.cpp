#include "Maze.h"

#include "vendor/stb_image_write.h"
#include <cstdio>

Maze::Maze()
{
}

Maze::Maze(size_t dimension) : m_X(dimension), m_Y(dimension)
{
}

Maze::Maze(size_t dimensionX, size_t dimensionY) : m_X(dimensionX), m_Y(dimensionY)
{
}

Maze::~Maze()
{
	if (m_Nodes)
	{
		delete[] m_Nodes;
		m_Nodes = nullptr;
	}
}

void Maze::PrintAsAsciiMap()
{
	size_t mapDimensionX = 2 * m_X + 1;
	size_t mapDimensionY = 2 * m_Y + 1;
	char** mapLines = new char* [mapDimensionY];
	for (size_t i = 0; i < mapDimensionY; ++i)
	{
		mapLines[i] = new char[mapDimensionX + 1];
		memset(mapLines[i], '#', sizeof(char) * mapDimensionX);
		mapLines[i][mapDimensionX] = 0;
	}

	for (size_t y = 0; y < mapDimensionY; ++y)
	{
		for (size_t x = 0; x < mapDimensionX; ++x)
		{
			NodeType* node = GetNode(x, y);
			size_t mapX = 2 * x + 1;
			size_t mapY = 2 * y + 1;
			mapLines[mapY][mapX] = ' ';

			if (*node & MAZE_NODE_CONNECTION_NORTH)
			{
				mapLines[mapY - 1][mapX] = ' ';
			}
			if (*node & MAZE_NODE_CONNECTION_SOUTH)
			{
				mapLines[mapY + 1][mapX] = ' ';
			}
			if (*node & MAZE_NODE_CONNECTION_EAST)
			{
				mapLines[mapY][mapX + 1] = ' ';
			}
			if (*node & MAZE_NODE_CONNECTION_WEST)
			{
				mapLines[mapY][mapX - 1] = ' ';
			}
		}
	}

	// print map and free memory
	for (int i = 0; i < mapDimensionY; ++i)
	{
		printf("%s\n", mapLines[i]);
		delete[] mapLines[i];
	}
	delete[] mapLines;
}

void Maze::GetImage(MAZE_IMAGE_RESULT* result, int scale,
	uint32_t backColour, uint32_t foreColour)
{
	size_t widthPx = (2 * m_X + 1) * scale;
	size_t heightPx = (2 * m_Y + 1) * scale;

	result->numChannels = 4;
	result->widthPx = widthPx;
	result->heightPx = heightPx;
	result->strideBytes = result->widthPx * result->numChannels;

	size_t imgSizePx = result->widthPx * result->heightPx;
	result->memory = new uint8_t[imgSizePx * result->numChannels];

#define width  widthPx
#define height heightPx

	uint32_t* img32 = reinterpret_cast<uint32_t*>(result->memory);
	// clear to backColour
	for (size_t i = 0; i < imgSizePx; ++i)
		img32[i] = backColour;

	// macro for colouring our scaled maze cells
#define COLOUR_CELL_AT_OFFSET(topLeftCornerOffset, colour) { \
	for (int m = 0; m < scale; ++m) \
		for (int n = 0; n < scale; ++n) \
			img32[topLeftCornerOffset + m + n * width] = colour; }

	// draw all of the nodes and their connections
	for (size_t y = 0; y < m_Y; ++y)
	{
		for (size_t x = 0; x < m_X; ++x)
		{
			NodeType* node = GetNode(x, y);
			size_t nodeXPx = 2 * x + 1;
			size_t nodeYPx = 2 * y + 1;

			COLOUR_CELL_AT_OFFSET((nodeYPx * width + nodeXPx) * scale, foreColour);

			if (*node & MAZE_NODE_CONNECTION_NORTH)
			{
				// colour block if we haven't already
				size_t targetCellTopLeftIndex = ((nodeYPx - 1) * width + nodeXPx) * scale;
				if (img32[targetCellTopLeftIndex] == backColour)
					COLOUR_CELL_AT_OFFSET(targetCellTopLeftIndex, foreColour);
			}
			if (*node & MAZE_NODE_CONNECTION_SOUTH)
			{
				size_t targetCellTopLeftIndex = ((nodeYPx + 1) * width + nodeXPx) * scale;
				if (img32[targetCellTopLeftIndex] == backColour)
					COLOUR_CELL_AT_OFFSET(targetCellTopLeftIndex, foreColour);
			}
			if (*node & MAZE_NODE_CONNECTION_EAST)
			{
				size_t targetCellTopLeftIndex = (nodeYPx * width + nodeXPx + 1) * scale;
				if (img32[targetCellTopLeftIndex] == backColour)
					COLOUR_CELL_AT_OFFSET(targetCellTopLeftIndex, foreColour);
			}
			if (*node & MAZE_NODE_CONNECTION_WEST)
			{
				size_t targetCellTopLeftIndex = (nodeYPx * width + nodeXPx - 1) * scale;
				if (img32[targetCellTopLeftIndex] == backColour)
					COLOUR_CELL_AT_OFFSET(targetCellTopLeftIndex, foreColour);
			}
		}
	}
}

void Maze::FreeImage(MAZE_IMAGE_RESULT* image)
{
	delete[] image->memory;
	image->memory = nullptr;
}

bool Maze::WriteImageToFile(MAZE_IMAGE_RESULT* image, const char* path)
{
	return stbi_write_png(path, (int)image->widthPx, (int)image->heightPx,
		(int)image->numChannels, image->memory, (int)image->strideBytes);
}

void Maze::Generate()
{
	struct coord
	{
		size_t x, y;
	};

	m_Nodes = new NodeType[m_X * m_Y];
	memset(m_Nodes, 0, m_X * m_Y * sizeof(NodeType));

	// for randomly selecting directions
	std::random_device rd;
	std::mt19937 engine(rd());
	std::uniform_int_distribution<> distr(0, 3);

	// set up our stack
	int tos = -1;
	coord* stack = new coord[m_X * m_Y];

	// start at (0, 0)
	stack[++tos] = coord{ 0, 0 };

	while (tos > -1) // loop will end when the stack is empty
	{
		// ensure we don't overflow the stack
		_ASSERT(tos < m_X* m_Y);

		// Peek at node on top of stack
		coord nodeLocation = stack[tos];
		NodeType* node = GetNode(nodeLocation.x, nodeLocation.y);
		_ASSERT(node);

		// Mark as visited
		*node |= MAZE_NODE_VISITED;

		// check surrounding nodes to see if we have any valid targets
		bool nodeHasValidTargets =
			TargetNodeIsValid(GetNode(nodeLocation.x + 1, nodeLocation.y)) ||
			TargetNodeIsValid(GetNode(nodeLocation.x - 1, nodeLocation.y)) ||
			TargetNodeIsValid(GetNode(nodeLocation.x, nodeLocation.y - 1)) ||
			TargetNodeIsValid(GetNode(nodeLocation.x, nodeLocation.y + 1));

		// if we have no valid targets, pop this node off of the stack
		if (!nodeHasValidTargets)
		{
			--tos;
			continue;
		}

		// if we do have valid targets, pick a (valid) direction
		uint8_t directionMask = 0;
		NodeType* target = nullptr;
		while (!TargetNodeIsValid(target))
		{
			// find the target node given a randomly selected direction
			directionMask = MAZE_NODE_CONNECTION_DIRECTIONS[distr(engine)];
			target = GetTargetNode(directionMask, nodeLocation.x, nodeLocation.y);
		}

		// make the connection
		*node |= directionMask;

		coord targetLocation = nodeLocation;
		switch (directionMask)
		{
		case MAZE_NODE_CONNECTION_NORTH:
			*target |= MAZE_NODE_CONNECTION_SOUTH;
			targetLocation.y -= 1;
			break;
		case MAZE_NODE_CONNECTION_SOUTH:
			*target |= MAZE_NODE_CONNECTION_NORTH;
			targetLocation.y += 1;
			break;
		case MAZE_NODE_CONNECTION_EAST:
			*target |= MAZE_NODE_CONNECTION_WEST;
			targetLocation.x += 1;
			break;
		case MAZE_NODE_CONNECTION_WEST:
			*target |= MAZE_NODE_CONNECTION_EAST;
			targetLocation.x -= 1;
			break;
		}

		// push target node onto stack
		stack[++tos] = targetLocation;
	}

	delete[] stack;
}

inline bool Maze::TargetNodeIsValid(NodeType* target)
{
	// target node is only valid if it has not been visited yet
	return target && !(*target & MAZE_NODE_VISITED);
}

inline Maze::NodeType* Maze::GetNode(size_t x, size_t y)
{
	NodeType* result = nullptr;
	if (x >= 0 && x < m_X && y >= 0 && y < m_Y)
	{
		result = &m_Nodes[y * m_X + x];
	}
	return result;
}

inline Maze::NodeType* Maze::GetTargetNode(uint8_t directionMask, size_t originX, size_t originY)
{
	NodeType* result = nullptr;
	switch (directionMask)
	{
	case MAZE_NODE_CONNECTION_NORTH:
		result = GetNode(originX, originY - 1);
		break;
	case MAZE_NODE_CONNECTION_SOUTH:
		result = GetNode(originX, originY + 1);
		break;
	case MAZE_NODE_CONNECTION_EAST:
		result = GetNode(originX + 1, originY);
		break;
	case MAZE_NODE_CONNECTION_WEST:
		result = GetNode(originX - 1, originY);
		break;
	}
	return result;
}
