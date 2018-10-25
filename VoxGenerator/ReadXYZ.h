#pragma once

#include <fstream>
#include <vector>

#include "common.h"

//Reads the voxels from a XYZ file. The file must contain four entries per line and no header
//  file - the file to read, must be opened and pointing at the beginning
//  voxelSize - the edge length of a single voxel
//  outVoxels - output parameter, contains a list of all read voxels
//  outMin - output parameter, contains the lower corner of the voxels' bounding box
//  outmax - output parameter, contains the upper corner of the voxel's bounding box
void ReadXYZ(std::ifstream& file, double voxelSize, std::vector<Voxel>& outVoxels, Position& outMin, Position& outMax);