#include "WriteVOX.h"

// ----- Dictionary -----

void Dictionary::AddEntry(const std::string& key, const std::string& value)
{
	entries.push_back(std::make_pair(key, value));
}

// ----- Chunk -----

Chunk::Chunk(const char id[4])
{
	memcpy(this->id, id, 4);
}

void Chunk::AddChunk(Chunk* c)
{
	children.push_back(c);
}

//Write this chunk and all children to a file
void Chunk::WriteToFile(std::ofstream& f)
{
	//Write the header of the chunk
	f.write(id, 4);

	int32_t contentSize = (int32_t)content.size();
	f.write(reinterpret_cast<char*>(&contentSize), 4);

	//Accumulate the size of all child chunks
	int32_t childrenSize = 0;
	for (auto child : children)
		childrenSize += child->GetSize();
	f.write(reinterpret_cast<char*>(&childrenSize), 4);

	//write chunk content
	f.write(content.data(), content.size());

	//write children
	for (auto child : children)
		child->WriteToFile(f);
}

Chunk::~Chunk()
{
	for (auto child : children)
		delete child;
}

// ----- Chunks -----

MainChunk::MainChunk()
	: Chunk("MAIN")
{ }

PackChunk::PackChunk(int32_t numberOfModels)
	: Chunk("PACK")
{
	AppendToContent(numberOfModels);
}

SizeChunk::SizeChunk(int32_t sizeX, int32_t sizeY, int32_t sizeZ)
	: Chunk("SIZE")
{
	AppendToContent(sizeX);
	AppendToContent(sizeY);
	AppendToContent(sizeZ);
}

VoxelChunk::VoxelChunk()
	: Chunk("XYZI"), voxelCount(0)
{
	AppendToContent(voxelCount);
}

//Adds a new voxel to the chunk.
void VoxelChunk::AddVoxel(uint8_t x, uint8_t y, uint8_t z, uint8_t colorId)
{
	//append voxel data to end of content
	AppendToContent(x);
	AppendToContent(y);
	AppendToContent(z);
	AppendToContent(colorId);

	//update voxel count		
	++voxelCount;
	memcpy(content.data(), &voxelCount, 4);
}

TransformNode::TransformNode(int32_t nodeId, int32_t childNodeId, int32_t tx, int32_t ty, int32_t tz)
	: Chunk("nTRN")
{
	AppendToContent(nodeId);
	Dictionary attributes;
	AppendToContent(attributes);
	AppendToContent(childNodeId);
	int32_t reservedId = -1;
	AppendToContent(reservedId);
	int32_t layerId = 0;
	AppendToContent(layerId);
	int32_t numOfFrames = 1;
	AppendToContent(numOfFrames);
	for (int frame = 0; frame < numOfFrames; ++frame)
	{
		Dictionary frameTransform;
		frameTransform.AddEntry("_t", std::to_string(tx) + " " + std::to_string(ty) + " " + std::to_string(tz));
		AppendToContent(frameTransform);
	}
}

ShapeNode::ShapeNode(int32_t nodeId, int32_t modelId)
	: Chunk("nSHP")
{
	AppendToContent(nodeId);
	Dictionary nodeAttributes;
	AppendToContent(nodeAttributes);
	int32_t numOfModels = 1;
	AppendToContent(numOfModels);
	AppendToContent(modelId);
	Dictionary modelAttributes;
	AppendToContent(modelAttributes);
}

GroupNode::GroupNode(int32_t nodeId)
	: Chunk("nGRP"), children(0)
{
	AppendToContent(nodeId);
	Dictionary nodeAttributes;
	AppendToContent(nodeAttributes);
	AppendToContent(children);
}

void GroupNode::AddChildNode(int32_t nodeId)
{
	++children;
	AppendToContent(nodeId);
	memcpy(content.data() + 8, reinterpret_cast<char*>(&children), 4);
}

// ----- WriteVOX -----

void WriteVOX(MainChunk& main, std::ofstream& file)
{
	file << "VOX ";

	int32_t version = 150;
	file.write(reinterpret_cast<char*>(&version), 4);

	main.WriteToFile(file);
}