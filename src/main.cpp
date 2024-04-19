#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "iostream"
#include <vector>

using namespace std;
constexpr int WorldSize = 2;
#include "worldbuilder.hpp"


// lighting
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"


// voxel size constants
constexpr int ChunkDepth = 16;;
constexpr int ChunkSize = 16;

Mesh mesh[WorldSize*WorldSize];

// the voxel data for a single chunk. Tells us what is in each voxel
char  VoxelChunk[ChunkSize * ChunkSize * ChunkDepth] = { 0 };

// texture rectangles for various block colors //Works through percentages
Rectangle BlockColors[5] = {Rectangle{0,0,0.2f,1}, Rectangle{0.2f,0,0.4f,1}, Rectangle{0.4f,0,0.6f,1}, Rectangle{0.6f,0,0.8f,1}, Rectangle{0.8f,0,1,1}};

// get the index into the voxel array for a h,v,d coordinate
int GetIndex(int h, int v, int d)
{
	return (d * (ChunkSize * ChunkSize)) + (v * ChunkSize) + h;
}

// a simple class to help build up faces of a cube
// can be made to be pure C and take the global data in a structure or global data
#include "geobuilder.hpp"

#include "chunkbuilder.hpp"


bool BlockIsSolid(int h, int v, int d)
{
	if (h < 0 || h >= ChunkSize)
		return false;

	if (v < 0 || v >= ChunkSize)
		return false;

	if (d < 0 || d >= ChunkDepth)
		return false;

	return VoxelChunk[GetIndex(h, v, d)] >= 0;
}

//check all the adjacent blocks to see if they are open, if they are, we need a face for that side of the block.
size_t GetChunkFaceCount()
{
	size_t count = 0;
	for (int d = 0; d < ChunkDepth; d++)
	{
		for (int v = 0; v < ChunkSize; v++)
		{
			for (int h = 0; h < ChunkSize; h++)
			{
				if (!BlockIsSolid(h,v,d))
					continue;

				if (!BlockIsSolid(h + 1, v, d))
					count++;

				if (!BlockIsSolid(h - 1, v, d))
					count++;

				if (!BlockIsSolid(h, v + 1, d))
					count++;

				if (!BlockIsSolid(h, v - 1, d))
					count++;

				if (!BlockIsSolid(h, v, d + 1))
					count++;

				if (!BlockIsSolid(h, v, d - 1))
					count++;
			}
		}
	} 

	return count;
}

Mesh MeshChunk()
{
	Mesh mesh = { 0 };
	CubeGeometryBuilder builder(mesh);

	// figure out how many faces will be in this chunk and allocate a mesh that can store that many
	builder.Allocate(GetChunkFaceCount());

	size_t count = 0;
	for (int d = 0; d < ChunkDepth; d++)
	{
		for (int v = 0; v < ChunkSize; v++)
		{
			for (int h = 0; h < ChunkSize; h++)
			{
				if (!BlockIsSolid(h, v, d))
					continue;

				// build up the list of faces that this block needs
				bool faces[6] = { false, false, false, false, false, false };

				if (!BlockIsSolid(h - 1, v, d))
					faces[CubeGeometryBuilder::EastFace] = true;

				if (!BlockIsSolid(h + 1, v, d))
					faces[CubeGeometryBuilder::WestFace] = true;

				if (!BlockIsSolid(h, v - 1, d))
					faces[CubeGeometryBuilder::NorthFace] = true;

				if (!BlockIsSolid(h, v + 1, d))
					faces[CubeGeometryBuilder::SouthFace] = true;

				if (!BlockIsSolid(h, v, d + 1))
					faces[CubeGeometryBuilder::UpFace] = true;

				if (!BlockIsSolid(h, v, d - 1))
					faces[CubeGeometryBuilder::DownFace] = true;

				// build the faces that hit open air for this voxel block
				builder.AddCube(Vector3{ (float)h, (float)d, (float)v }, faces, (int)VoxelChunk[GetIndex(h, v, d)]);
			}
		}
	}

	UploadMesh(&mesh, false);

	return mesh;
}

int main()

{
	InitWindow(720, 480, "voxels!");
    //SetWindowState(FLAG_VSYNC_HINT);
	SetTargetFPS(144);
	Texture2D tileTexture = LoadTexture("./Arrow2.png");

	// setup a simple camera
	
	Camera3D camera = { 0 };

	camera.fovy = 45;
	camera.up.y = 1;
    camera.position.x = 32;
	camera.position.z = 32;
	camera.position.y = 16;

	// load basic lighting
	Shader shader = LoadShader("resources/shaders/base_lighting.vs", "resources/shaders/lighting.fs");
	
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	// Ambient light level (some basic lighting)
	int ambientLoc = GetShaderLocation(shader, "ambient");
	//float val[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    float val[4] = { -1.0f, -1.0f, -1.0f, -1.0f};
	SetShaderValue(shader, ambientLoc, val , SHADER_UNIFORM_VEC4);

	Light lights[MAX_LIGHTS] = { 0 };
	lights[0] = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), Vector3{ -2, -4, -3 }, WHITE, shader);
	lights[1] = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), Vector3{ 2, 2, 5 }, GRAY, shader);

	for (size_t i = 0; i < end(mesh)-begin(mesh); i++)
	{BuildChunk();
	//EmptyChunk();

	        // build a mesh for the chunk
    mesh[i] = MeshChunk();
	}
	
	// set the mesh to the correct material/shader
	Material mat = LoadMaterialDefault();
	mat.maps[0].color = WHITE;
	mat.maps[0].texture = tileTexture;
	mat.shader = shader;

	
	DisableCursor();

	while (!WindowShouldClose())
	{
		
		
		//camera.position = Vector3Transform(camera.position, MatrixRotateY(GetFrameTime()*DEG2RAD * 15));
		UpdateCamera(&camera, CAMERA_FREE);
		camera.up = {0,1,0};
		SetMousePosition(GetScreenWidth()/2,GetScreenHeight()/2);
        if(IsKeyPressed('N'))
        { for (size_t i = 0; i < end(mesh)-begin(mesh); i++)
			{
            BuildChunk();
            UnloadMesh(mesh[i]);
            mesh[i] = MeshChunk();
			}
        }
		if(IsKeyPressed('M'))
        { for (size_t i = 0; i < end(mesh)-begin(mesh); i++)
			{
            EmptyChunk();
            UnloadMesh(mesh[i]);
            mesh[i] = MeshChunk();
			}
        }
		if(IsKeyPressed('B'))
        {for (size_t i = 0; i < end(mesh)-begin(mesh); i++)
			{
            SolidChunk();
            UnloadMesh(mesh[i]);
            mesh[i] = MeshChunk();
        }}
		
		// update lights
		UpdateLightValues(shader, lights[0]);
		UpdateLightValues(shader, lights[1]);

		// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position, SHADER_UNIFORM_VEC3);

		Ray temp = {0};
		int chunkz = 0;
		int chunkx = 0;
		
		if(IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			for (size_t i = 0; i < end(mesh)-begin(mesh); i++)
			{
				Matrix Debugmatrix = {  1.0f, 0.0f, 0.0f, i*16, // x is 16
                      			0.0f, 1.0f, 0.0f, 0.0f, //y stays zero
                      			0.0f, 0.0f, 1.0f, 0.0f,
                      			0.0f, 0.0f, 0.0f, 1.0f };
		
				if(GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).hit && GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).distance <10)
				{
					cout << "outputting ray" << endl;
					//GetRayCollisionMesh(GetMouseRay({GetScreenHeight()/2, GetScreenWidth()/2}, camera), mesh, Debugmatrix).point


					//{GetScreenHeight()/2, GetScreenWidth()/2}
					int h = (GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).point.x); //x has problems...
					int d = Clamp((GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).point.y), 0, ChunkDepth-1);// this is fine since no vertical chunks;
					int v = (GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).point.z);
					cout << h << '/' << v << '/' << d << endl;
				
					int index = GetIndex(h, v, d);

					VoxelChunk[index] = -1;

				UnloadMesh(mesh[i]);
           		 mesh[i] = MeshChunk();
				}

			}
		}

		//make block NEED TO MAKE OFFSET WORK RN IT ONLY IGNORES BLOCKS THAT EXIST, NOT VERY GOOD BUT IT WORKS
		if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
		{
			for (size_t i = 0; i < end(mesh)-begin(mesh); i++)
			{
				Matrix Debugmatrix = {  1.0f, 0.0f, 0.0f, i*16, // x is 16
                      			0.0f, 1.0f, 0.0f, 0.0f, //y stays zero
                      			0.0f, 0.0f, 1.0f, 0.0f,
                      			0.0f, 0.0f, 0.0f, 1.0f };
		
				if(GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).hit && GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).distance <10)
				{
					cout << "outputting ray" << endl;
					//GetRayCollisionMesh(GetMouseRay({GetScreenHeight()/2, GetScreenWidth()/2}, camera), mesh, Debugmatrix).point


					//{GetScreenHeight()/2, GetScreenWidth()/2}
					int h = (GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).point.x); //x has problems...
					int d = Clamp((GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).point.y), 0, ChunkDepth-1);// this is fine since no vertical chunks;
					int v = (GetRayCollisionMesh(GetMouseRay(GetMousePosition(), camera), mesh[i], Debugmatrix).point.z);
					cout << h << '/' << v << '/' << d << endl;
				
					int index = GetIndex(h, v, d);

					if(VoxelChunk[index] == -1)
		 			{
		 				VoxelChunk[index] = 4;
		 				//subtract block from inv
		 			}

				UnloadMesh(mesh[i]);
           		 mesh[i] = MeshChunk();
				}

			}
		}
		

		BeginDrawing();
		ClearBackground(RAYWHITE);
		

		BeginMode3D(camera);


		// draw some simple origin and axis markers
		DrawGrid(16, 16);
		DrawSphere(Vector3Zero(), 0.125f, GRAY);
		DrawSphere(Vector3{ 1,0,0 }, 0.0125f, RED);
		DrawSphere(Vector3{ 0,1,0 }, 0.0125f, GREEN);
		DrawSphere(Vector3{ 0,0,1 }, 0.0125f, BLUE);

		//Draw World
		for (size_t i = 0; i < end(mesh)-begin(mesh); i++)
		{
			// int GetIndex(int h, int v)
			// {
			// 	return (d * (ChunkSize * ChunkSize)) + (v * ChunkSize) + h;
			// }
			DrawMesh(mesh[i], mat,  {1.0f, 0.0f, 0.0f, i*16, // x is 16
                      			0.0f, 1.0f, 0.0f, 0.0f, //y stays zero
                      			0.0f, 0.0f, 1.0f, 0*16,
                      			0.0f, 0.0f, 0.0f, 1.0f });
		}
	
		// int rendered = 0;
		// while (rendered < renderdistance)
		// {		
		// 	DrawMesh(mesh[0], mat, Debugmatrix);
		// 	rendered ++;
		// }
		
		// draw the chunk
		

		EndMode3D();

		DrawFPS(0, 0);

		DrawCircle(GetMousePosition().x,GetMousePosition().y, 2.5f, RED);
		EndDrawing();
	}

	for (size_t i = 0; i < end(mesh)-begin(mesh); i++)
	{
		UnloadMesh(mesh[i]);
	}
	
	
	UnloadTexture(tileTexture);
	CloseWindow();
	return 0;
}