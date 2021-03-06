#include <fstream>
#include <iostream>

#include "HashGrid.h"
#include "ReadXYZ.h"
#include "WriteVOX.h"

void PrintHelp()
{
	std::cout << "Usage: VoxGenerator -i inputXYZFile -o outputVOXFile -s voxelEdgeLength (optional)" << std::endl;
}

int main(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		PrintHelp();
		return 2;
	}

	//read console parameters
	std::string inputFile, outputFile;
	double voxelSize = 1;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-i") == 0)
		{
			inputFile = argv[i + 1];
			++i;
			continue;
		}
		if (strcmp(argv[i], "-o") == 0)
		{
			outputFile = argv[i + 1];
			++i;
			continue;
		}
		if (strcmp(argv[i], "-s") == 0)
		{
			voxelSize = std::atof(argv[i + 1]);
			++i;
			continue;
		}
	}

	if (inputFile.empty())
	{
		std::cout << "You need to provide an input file." << std::endl;
		PrintHelp();
		return 1;
	}

	if (outputFile.empty())
	{
		std::cout << "You need to provide an output file." << std::endl;
		PrintHelp();
		return 1;
	}	

	const int MAX_MODEL_SIZE = 126;

	std::vector<Voxel> voxels;
	Position min, max;

	//Read the input file
	std::cout << "Reading from \"" << inputFile << "\" with a voxel size of " << voxelSize << "..." << std::endl;
	std::ifstream input(inputFile);
	ReadXYZ(input, 0.1, voxels, min, max);
	input.close();

	std::cout << "Read " << voxels.size() << " voxels from XYZ file." << std::endl;	

	//A single model can not be larger than MAX_MODEL_SIZE, split the voxels into
	//multiple models as necessary. The models are stored in a hash grid and addressed
	//by their 3D index.
	HashGridType hashGrid;
	for (auto& v : voxels)
	{
		//After we find the model that the voxel belongs to, we must transform the
		//voxel position into the model's local coordinate system.
		Voxel localVoxel = v;
		std::array<int, 3> modelCoordinate; //the model's index along the three dimensions (starting at (0, 0, 0))
		for (int i = 0; i < 3; ++i)
		{
			//move the lower corner of the voxel's bounding box to the origin
			localVoxel.position[i] -= min[i];

			//calculate model index
			modelCoordinate[i] = localVoxel.position[i] / MAX_MODEL_SIZE;

			//transform voxel coordinate to model local coordinates
			localVoxel.position[i] -= modelCoordinate[i] * MAX_MODEL_SIZE;
		}
		//add to voxel to the correct model
		hashGrid[modelCoordinate].voxels.push_back(localVoxel);
	}

	std::cout << "Voxels are distributed over " << hashGrid.size() << " models." << std::endl;

	//Prepare the chunks that we need to write to file
	MainChunk main;

	//The pack chunk contains the number of models
	main.AddChunk(new PackChunk(hashGrid.size()));

	//add all the model chunks
	for (auto& model : hashGrid)
	{
		//find size of model		
		for (auto& voxel : model.second.voxels)
		{
			for (int i = 0; i < 3; ++i)
			{
				if (voxel.position[i] < model.second.min[i])
					model.second.min[i] = voxel.position[i];
				if (voxel.position[i] > model.second.max[i])
					model.second.max[i] = voxel.position[i];
			}
		}

		//the size chunk contains the size of the model
		main.AddChunk(new SizeChunk(
			model.second.max[0] - model.second.min[0] + 1, 
			model.second.max[1] - model.second.min[1] + 1, 
			model.second.max[2] - model.second.min[2] + 1));

		//the voxel chunk contains the actual data
		auto voxelChunk = new VoxelChunk();
		for (auto& voxel : model.second.voxels)
			//the model will only start at the lower corner of its bounding box (it will not contain all the
			//space of size MAX_MODEL_SIZE), therfore subtract the coordinate of the lower corner
			voxelChunk->AddVoxel(
				voxel.position[0] - model.second.min[0], 
				voxel.position[1] - model.second.min[1], 
				voxel.position[2] - model.second.min[2], 
				voxel.color);

		main.AddChunk(voxelChunk);
	}

	//set up the scene graph (includes the global positions of the models)
	//root node
	main.AddChunk(new TransformNode(0, 1, 0, 0, 0));

	//we have a single group that contains all other nodes
	auto grp = new GroupNode(1);
	main.AddChunk(grp);

	int nextModel = 0;
	//create a transform node and a shape node for each model
	for (auto& model : hashGrid)
	{
		auto transformNodeId = 2 + nextModel * 2;
		//add the transform node to the global group
		grp->AddChildNode(transformNodeId);

		//add the transform node chunk with the correct transform for the model (translation defines the center of the model)
		main.AddChunk(new TransformNode(
			transformNodeId,
			transformNodeId + 1, //child is the next node, i.e. the shape node
			model.first[0] * MAX_MODEL_SIZE + model.second.min[0] + (model.second.max[0] - model.second.min[0] + 1) / 2,
			model.first[1] * MAX_MODEL_SIZE + model.second.min[1] + (model.second.max[1] - model.second.min[1] + 1) / 2,
			model.first[2] * MAX_MODEL_SIZE + model.second.min[2] + (model.second.max[2] - model.second.min[2] + 1) / 2));
		main.AddChunk(new ShapeNode(transformNodeId + 1, nextModel));
		++nextModel;
	}	

	//finally, write everything to file
	std::cout << "Writing to \"" << outputFile << "\"..." << std::endl;
	std::ofstream output(outputFile, std::ios::binary);
	WriteVOX(main, output);
	output.close();

	std::cout << "Conversion finished." << std::endl;

    return 0;
}

