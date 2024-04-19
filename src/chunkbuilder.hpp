
void EmptyChunk()
{
	for (int d = 0; d < ChunkDepth; d++)
	{
		char block = -1;

		for (int v = 0; v < ChunkSize; v++)
		{
			for (int h = 0; h < ChunkSize; h++)
			{
				int index = GetIndex(h, v, d);

				VoxelChunk[index] = block;
			}
		}
	}

}

void SolidChunk()
{
	for (int d = 0; d < ChunkSize; d++)
	{
		char block = 0;

		for (int v = 0; v < ChunkSize; v++)
		{
			for (int h = 0; h < ChunkSize; h++)
			{
				int index = GetIndex(h, v, d);

				VoxelChunk[index] = block;
			}
		}
	}

}



// build a simple random voxel chunk
void BuildChunk(int x = 0, int y = 0)
{
	//world gen code
	//also render code

	// fill the chunk with layers of blocks
	for (int d = 0; d < ChunkDepth; d++)
	{
		char block = 0;
		if (d > 6)
		{
			block = 1;
			if (d > 8)
			{
				block = 2;
				if (d > 10)
					block = -1;
			}
		}

		for (int v = 0; v < ChunkSize; v++)
		{
			for (int h = 0; h < ChunkSize; h++)
			{
				int index = GetIndex(h, v, d);

				VoxelChunk[index] = block;
			}
		}
	}

	// Remove some voxels 
	for (int i = 0; i < 500; i++)
	{
		int h = GetRandomValue(0, ChunkSize-1);
		int v = GetRandomValue(0, ChunkSize-1);
		int d = GetRandomValue(0, 10);

		int index = GetIndex(h, v, d);

		VoxelChunk[index] = -1;
	}

	// Add some gold
	for (int i = 0; i < 100; i++)
	{
		int h = GetRandomValue(0, ChunkSize - 1);
		int v = GetRandomValue(0, ChunkSize - 1);
		int d = GetRandomValue(0, 10);

		int index = GetIndex(h, v, d);

		VoxelChunk[index] = 3;
	}
}