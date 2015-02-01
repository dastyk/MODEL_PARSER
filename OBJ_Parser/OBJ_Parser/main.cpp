////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////


//////////////
// INCLUDES //
//////////////
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <string>
using namespace DirectX;
using namespace std;

//////////////
// TYPEDEFS //
//////////////
typedef struct
{
	float x, y, z;
}VertexType;

typedef struct
{
	int vIndex1, vIndex2, vIndex3, vIndex4;
	int tIndex1, tIndex2, tIndex3, tIndex4;
	int nIndex1, nIndex2, nIndex3, nIndex4;
	int Count;
	unsigned int ID;
}FaceType;

struct Header
{
	unsigned long VertexCount;
	unsigned long IndexCount;
	unsigned long bfOffBits;	
	unsigned int ObjectCount;
};

struct vertexData
{
	XMFLOAT3 pos;
	XMFLOAT2 tex;
	XMFLOAT3 Normal;
	unsigned int ID;
};

struct Body
{
	vertexData* vertices;
	unsigned long* Indices;
	unsigned int* textureSize;
	char** textureName;
};


/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
void GetModelFilename(char*);
bool ReadFileCounts(char*, int&, int&, int&, int&, int&);
bool LoadDataStructures(char*, int, int, int, int,int);
bool PrintDataInFile(char*);

void fToXM(XMFLOAT3* xm, VertexType v)
{
	xm->x = v.x;
	xm->y = v.y;
	xm->z = v.z;
}
void fToXM(XMFLOAT2* xm, VertexType v)
{
	xm->x = v.x;
	xm->y = v.y;
}

void insertData(vertexData* data, VertexType p, VertexType t, VertexType n, unsigned int id)
{
	fToXM(&data->pos, p);
	fToXM(&data->tex, t);
	fToXM(&data->Normal, n);
	data->ID = id;
}

std::string removeExtension(const std::string& filename) {
	size_t lastdot = filename.find_last_of(".");
	if (lastdot == std::string::npos) return filename;
	return filename.substr(0, lastdot);
}


//////////////////
// MAIN PROGRAM //
//////////////////
int main()
{
	bool result;
	char filename[256];
	int vertexCount, textureCount, normalCount, faceCount, objectCount;
	char garbage;

	// Read in the name of the model file.
	GetModelFilename(filename);

	PrintDataInFile(filename);

	cin >> garbage;

	return 0;


	// Read in the number of vertices, tex coords, normals, and faces so that the data structures can be initialized with the exact sizes needed.
	result = ReadFileCounts(filename, vertexCount, textureCount, normalCount, faceCount, objectCount);
	if (!result)
	{
		return -1;
	}

	// Display the counts to the screen for information purposes.
	cout << endl;
	cout << "Parts: " << objectCount << endl;
	cout << "Vertices: " << vertexCount << endl;
	cout << "UVs:      " << textureCount << endl;
	cout << "Normals:  " << normalCount << endl;
	cout << "Faces:    " << faceCount << endl;

	// Now read the data from the file into the data structures and then output it in our model format.
	result = LoadDataStructures(filename, vertexCount, textureCount, normalCount, faceCount, objectCount);
	if (!result)
	{
		return -2;
	}

	// Notify the user the model has been converted.
	cout << "\nFile has been converted." << endl;

	return 0;
}



void GetModelFilename(char* filename)
{
	bool done;
	ifstream fin;


	// Loop until we have a file name.
	done = false;
	while (!done)
	{
		// Ask the user for the filename.
		cout << "Enter model filename: ";

		// Read in the filename.
		cin >> filename;



		// Attempt to open the file.
		fin.open(filename);

		if (fin.good())
		{
			// If the file exists and there are no problems then exit since we have the file name.
			done = true;
		}
		else
		{
			// If the file does not exist or there was an issue opening it then notify the user and repeat the process.
			fin.clear();
			cout << endl;
			cout << "File " << filename << " could not be opened." << endl << endl;
		}
	}

	return;
}


bool ReadFileCounts(char* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount, int& objectCount)
{
	ifstream fin;
	char input;
	int byteWidth = 0;

	// Initialize the counts.
	vertexCount = 0;
	textureCount = 0;
	normalCount = 0;
	faceCount = 0;
	objectCount = 0;

	// Open the file.
	fin.open(filename);

	// Check if it was successful in opening the file.
	if (fin.fail() == true)
	{
		return false;
	}

	// Read from the file and continue to read until the end of the file is reached.
	fin.get(input);
	while (!fin.eof())
	{
		// If the line starts with 'v' then count either the vertex, the texture coordinates, or the normal vector.
		if (input == 'v')
		{
			fin.get(input);
			if (input == ' ') { vertexCount++; }
			if (input == 't') { textureCount++; }
			if (input == 'n') { normalCount++; }
		}

		// If the line starts with 'f' then increment the face count.
		if (input == 'f')
		{
			fin.get(input);
			if (input == ' ') { faceCount++; }
		}

		if (input == 'o' || input == 'g')
		{
			fin.get(input);
			if (input == ' ')
			{
				objectCount++;
			}
		}

		// Otherwise read in the remainder of the line.
		while (input != '\n')
		{
			fin.get(input);
		}

		// Start reading the beginning of the next line.
		fin.get(input);
	}

	// Close the file.
	fin.close();

	return true;
}


bool LoadDataStructures(char* filename, int vertexCount, int textureCount, int normalCount, int faceCount,int objectCount)
{
	VertexType *vertices, *texcoords, *normals;
	FaceType *faces;
	ifstream fin;
	int vertexIndex, texcoordIndex, normalIndex, faceIndex, vIndex, tIndex, nIndex, objectIndex;
	char input, input2;
	ofstream fout;
	ofstream bout;
	bool dr = false;


	// Initialize the four data structures.
	vertices = new VertexType[vertexCount];
	if (!vertices)
	{
		return false;
	}

	texcoords = new VertexType[textureCount];
	if (!texcoords)
	{
		return false;
	}

	normals = new VertexType[normalCount];
	if (!normals)
	{
		return false;
	}

	faces = new FaceType[faceCount];
	if (!faces)
	{
		return false;
	}

	string* textureArray = new string[objectCount];
	if (!textureArray)
	{
		return false;
	}

	// Initialize the indexes.
	vertexIndex = 0;
	texcoordIndex = 0;
	normalIndex = 0;
	faceIndex = 0;
	objectIndex = -1;

	// Open the file.
	fin.open(filename);

	// Check if it was successful in opening the file.
	if (fin.fail() == true)
	{
		return false;
	}
	int vCount = 0;
	// Read in the vertices, texture coordinates, and normals into the data structures.
	// Also convert to left hand coordinate
	fin.get(input);
	while (!fin.eof())
	{
		if (input == 'v')
		{
			fin.get(input);

			// Read in the vertices.
			if (input == ' ')
			{
				fin >> vertices[vertexIndex].x >> vertices[vertexIndex].y >> vertices[vertexIndex].z;

				//Invert the Z vertex to change to left hand system.
				vertices[vertexIndex].z = vertices[vertexIndex].z * -1.0f;
				vertexIndex++;
			}

			// Read in the texture uv coordinates.
			if (input == 't')
			{
				fin >> texcoords[texcoordIndex].x >> texcoords[texcoordIndex].y;

				// Invert the V texture coordinates to left hand system.
				texcoords[texcoordIndex].y = 1.0f - texcoords[texcoordIndex].y;
				texcoordIndex++;
			}

			// Read in the normals.
			if (input == 'n')
			{
				fin >> normals[normalIndex].x >> normals[normalIndex].y >> normals[normalIndex].z;

				// Invert the Z normal to change to left hand system.
				normals[normalIndex].z = normals[normalIndex].z * -1.0f;
				normalIndex++;
			}
		}

		// Read in the faces.
		if (input == 'f')
		{
			fin.get(input);
			if (input == ' ')
			{
				// Read the face data in backwards to convert it to a left hand system from right hand system.

				fin >> faces[faceIndex].vIndex4 >> input2 >> faces[faceIndex].tIndex4 >> input2 >> faces[faceIndex].nIndex4
					>> faces[faceIndex].vIndex3 >> input2 >> faces[faceIndex].tIndex3 >> input2 >> faces[faceIndex].nIndex3
					>> faces[faceIndex].vIndex2 >> input2 >> faces[faceIndex].tIndex2 >> input2 >> faces[faceIndex].nIndex2;
				
				fin.get(input);
				if (input == ' ')
				{
					fin >> faces[faceIndex].vIndex1 >> input2 >> faces[faceIndex].tIndex1 >> input2 >> faces[faceIndex].nIndex1;
					faces[faceIndex].Count = 4;
					vCount += 6;
					dr = false;

				}
				else
				{
					faces[faceIndex].Count = 3;
					vCount += 3;
					dr = false;
				}
				faces[faceIndex].ID = objectIndex ;

				faceIndex++;
			}


			
		}

		if (input == 'o' || input == 'g')
		{
			fin.get(input);
			if (input == ' ')
			{
				objectIndex++;
			}
		}

		if (input == 'u')
		{
			fin.get(input);
			if (input == 's')
			{
				fin.get(input);
				if (input == 'e')
				{
					fin.get(input);
					if (input == 'm')
					{
						fin.get(input);
						if (input == 't')
						{
							fin.get(input);
							if (input == 'l')
							{
								fin.get(input);
								if (input == ' ')
								{
									getline(fin, textureArray[objectIndex]);
									int test = 0;
								}
							}
						}
					}
				}
			}
		}


		while (input != '\n')
		{
			fin.get(input);
		}

		// Start reading the beginning of the next line.
		fin.get(input);

	}

	// Close the file.
	fin.close();

	string name = string(filename);

	bout.open(removeExtension(name) + ".smf", ios_base::binary);

	Header head;
	head.VertexCount = vCount;
	head.IndexCount = vCount;
	head.bfOffBits = sizeof(Header);
	head.ObjectCount = objectCount;

	bout.write((char*)&head, sizeof(Header));

	vertexData* data = new vertexData[head.VertexCount];
	unsigned long* Indices = new unsigned long[head.IndexCount];

	for (unsigned long i = 0; i < head.IndexCount; i++)
	{
		Indices[i] = i;
	}

	unsigned int* textureSize = new unsigned int[head.ObjectCount];
	for (unsigned int i = 0; i < head.ObjectCount; i++)
	{
		textureSize[i] = textureArray[i].size();
	}

	int index = 0;
	for (int i = 0; i < faceCount; i++)
	{
		if (faces[i].Count == 4)
		{
			vIndex = faces[i].vIndex1 - 1;
			tIndex = faces[i].tIndex1 - 1;
			nIndex = faces[i].nIndex1 - 1;

			insertData(&data[index], vertices[vIndex], texcoords[tIndex], normals[nIndex], faces[i].ID);

			index++;

			vIndex = faces[i].vIndex2 - 1;
			tIndex = faces[i].tIndex2 - 1;
			nIndex = faces[i].nIndex2 - 1;

			insertData(&data[index], vertices[vIndex], texcoords[tIndex], normals[nIndex], faces[i].ID);

			index++;

			vIndex = faces[i].vIndex3 - 1;
			tIndex = faces[i].tIndex3 - 1;
			nIndex = faces[i].nIndex3 - 1;

			insertData(&data[index], vertices[vIndex], texcoords[tIndex], normals[nIndex], faces[i].ID);

			index++;

			insertData(&data[index], vertices[vIndex], texcoords[tIndex], normals[nIndex], faces[i].ID);

			index++;

			vIndex = faces[i].vIndex4 - 1;
			tIndex = faces[i].tIndex4 - 1;
			nIndex = faces[i].nIndex4 - 1;

			insertData(&data[index], vertices[vIndex], texcoords[tIndex], normals[nIndex], faces[i].ID);

			index++;

			vIndex = faces[i].vIndex1 - 1;
			tIndex = faces[i].tIndex1 - 1;
			nIndex = faces[i].nIndex1 - 1;

			insertData(&data[index], vertices[vIndex], texcoords[tIndex], normals[nIndex], faces[i].ID);

			index++;

		}
		else
		{
			vIndex = faces[i].vIndex2 - 1;
			tIndex = faces[i].tIndex2 - 1;
			nIndex = faces[i].nIndex2 - 1;

			insertData(&data[index], vertices[vIndex], texcoords[tIndex], normals[nIndex], faces[i].ID);

			index++;

			vIndex = faces[i].vIndex3 - 1;
			tIndex = faces[i].tIndex3 - 1;
			nIndex = faces[i].nIndex3 - 1;

			insertData(&data[index], vertices[vIndex], texcoords[tIndex], normals[nIndex], faces[i].ID);

			index++;

			vIndex = faces[i].vIndex4 - 1;
			tIndex = faces[i].tIndex4 - 1;
			nIndex = faces[i].nIndex4 - 1;

			insertData(&data[index], vertices[vIndex], texcoords[tIndex], normals[nIndex], faces[i].ID);

			index++;
		}

	}

	bout.write((char*)data, sizeof(vertexData)*head.VertexCount);
	bout.write((char*)Indices, sizeof(unsigned long)*head.IndexCount);
	bout.write((char*)textureSize, sizeof(unsigned int)*head.ObjectCount);
	for (unsigned int i = 0; i < head.ObjectCount; i++)
	{
		bout.write((char*)textureArray[i].c_str(), textureSize[i]);
	}

	bout.close();

	// Release the four data structures.
	if (vertices)
	{
		delete[] vertices;
		vertices = 0;
	}
	if (texcoords)
	{
		delete[] texcoords;
		texcoords = 0;
	}
	if (normals)
	{
		delete[] normals;
		normals = 0;
	}
	if (faces)
	{
		delete[] faces;
		faces = 0;
	}

	return true;
}


bool PrintDataInFile(char* filename)
{
	FILE* filePtr;
	Header head;
	int count;
	int error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return -3;
	}

	count = fread(&head, sizeof(Header), 1, filePtr);

	cout << "VertexCount: " << head.VertexCount << endl;
	cout << "IndexCount: " << head.IndexCount << endl;
	cout << "ObjectCount: " << head.ObjectCount << endl;
	cout << "bfOffBits: " << head.bfOffBits << endl << endl;

	vertexData* data = new vertexData[head.VertexCount];
	unsigned long* indices = new unsigned long[head.IndexCount];
	unsigned int* textureSize = new unsigned int[head.ObjectCount];
	char** textureArray = new char*[head.ObjectCount];

	fseek(filePtr, head.bfOffBits, SEEK_SET);

	fread(data, sizeof(vertexData), head.VertexCount, filePtr);
	fread(indices, sizeof(unsigned long), head.IndexCount, filePtr);
	fread(textureSize, sizeof(unsigned int), head.ObjectCount, filePtr);


	for (unsigned int i = 0; i < head.ObjectCount; i++)
	{
		textureArray[i] = new char[textureSize[i]];
	}
	for (unsigned int i = 0; i < head.ObjectCount; i++)
	{
		fread(textureArray[i], textureSize[i], 1, filePtr);
	}

	for (unsigned int i = 0; i < head.ObjectCount; i++)
	{
		cout << "textureSize" << i << ": " << textureSize[i] << endl;
		cout << "TextureName" << i << ": " << textureArray[i] << endl;
	}

	cout << endl;

	for (int i = 0; i < head.VertexCount; i++)
	{
		cout << "Point" << i << ": " << data[i].pos.x << ", " << data[i].pos.y << ", " << data[i].pos.z << " | " << data[i].tex.x << ", " << data[i].tex.y << " | " << data[i].Normal.x << ", " << data[i].Normal.y << ", " << data[i].Normal.z << " | " << data[i].ID << endl;
	}
	cout << "Index: " << endl << endl;

	for (int i = 0; i < head.IndexCount; i++)
	{
		cout << indices[i] << ", ";
	}

	return true;
}