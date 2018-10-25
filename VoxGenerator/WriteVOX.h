#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "common.h"

class Chunk;

//Data type for dictionaries defined in VOX files
class Dictionary
{
public:
	void AddEntry(const std::string& key, const std::string& value);

private:
	std::vector<std::pair<std::string, std::string>> entries;

	friend class Chunk;
};

//Base data type for all chunks in a VOX file
class Chunk
{
public:
	//Create a new chunk.
	//  id - the chunk id (name, e.g. "MAIN")
	Chunk(const char id[4]);

	//Add a child chunk to this chunk
	void AddChunk(Chunk* c);

	//Write this chunk and all children to a file
	void WriteToFile(std::ofstream& f);

	virtual ~Chunk();

protected:
	//The chunk id, e.g. "MAIN"
	char id[4];

	//Generic chunk content, may be empty
	std::vector<char> content;

	//child chunks, may be empty
	std::vector<Chunk*> children;

	//Returns the total size of this chunk in the file, including header information
	int32_t GetSize()
	{
		int32_t size = 12; //header size
		size += (int32_t)content.size();
		for (auto child : children)
			size += child->GetSize();
		return size;
	};

	//Adds the binary representation of a value to the content of this chunk
	template <typename T>
	void AppendToContent(T& value)
	{
		auto size = sizeof(T);
		auto offset = content.size();
		content.resize(content.size() + size);
		memcpy(&content[offset], &value, size);
	}

	template <>
	void AppendToContent(std::string& value)
	{
		uint32_t size = (uint32_t)value.size();
		AppendToContent(size);
		auto offset = content.size();
		content.resize(content.size() + value.size());
		memcpy(&content[offset], value.data(), value.size());
	}

	template <>
	void AppendToContent(Dictionary& value)
	{
		uint32_t size = value.entries.size();
		AppendToContent(size);
		for (auto& entry : value.entries)
		{
			AppendToContent(entry.first);
			AppendToContent(entry.second);
		}
	}
};

//There is one main chunk in a VOX file
class MainChunk : public Chunk
{
public:
	MainChunk();
};

//The pack chunk is optional. If present, it contains the number of models in a file.
class PackChunk : public Chunk
{
public:
	PackChunk(int32_t numberOfModels);
};

//The size chunk of a model describes its extents
class SizeChunk : public Chunk
{
public:
	SizeChunk(int32_t sizeX, int32_t sizeY, int32_t sizeZ);
};

//The voxel chunk contains voxel data for a model.
class VoxelChunk : public Chunk
{
public:
	VoxelChunk();

	//Adds a new voxel to the chunk.
	void AddVoxel(uint8_t x, uint8_t y, uint8_t z, uint8_t colorId);

private:
	int32_t voxelCount;
};

//A transform node can be used to rotate or translate a model (rotation not implemented)
class TransformNode : public Chunk
{
public:
	//Create a new transform node.
	//  nodeId - the id of this node
	//  childNodeId - the id of the child node
	//  tx - translation along the x-axis
	//  ty - translation along the y-axis
	//  tz - translation along the z-axis
	TransformNode(int32_t nodeId, int32_t childNodeId, int32_t tx, int32_t ty, int32_t tz);
};

//The shape node references a model in the scene graph
class ShapeNode : public Chunk
{
public:
	//Create a new shape node.
	//  nodeId - the id of this node
	//  modelId - the id of the model as defined by the order in which they have been added to the file
	ShapeNode(int32_t nodeId, int32_t modelId);
};

class GroupNode : public Chunk
{
public:
	GroupNode(int32_t nodeId);

	void AddChildNode(int32_t nodeId);

private:
	int32_t children;
};

//Generates a VOX file with the content described by a given main chunk.
//Make sure that the file is opened in binary mode.
void WriteVOX(MainChunk& main, std::ofstream& file);