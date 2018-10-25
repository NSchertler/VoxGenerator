#include "ReadXYZ.h"

void ReadXYZ(std::ifstream& file, double voxelSize, std::vector<Voxel>& outVoxels, Position& outMin, Position& outMax)
{
	//initialize the bounding box
	outMin = MAXPOS;
	outMax = MINPOS;

	//variables for reading
	double readCoordinate[3];
	int color;
	
	//read until there is nothing more to read
	while (file >> readCoordinate[0] >> readCoordinate[1] >> readCoordinate[2] >> color)
	{
		//create a voxel from the read data
		Voxel voxel;
		for (int i = 0; i < 3; ++i)
		{
			//quantize the voxel position with the given voxel size
			voxel.position[i] = (int)std::round(readCoordinate[i] / voxelSize);

			//update the bounding box
			if (voxel.position[i] < outMin[i])
				outMin[i] = voxel.position[i];
			if (voxel.position[i] > outMax[i])
				outMax[i] = voxel.position[i];
		}
		voxel.color = (uint8_t)color;
		outVoxels.push_back(voxel);
	}
}