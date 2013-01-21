#include <fstream>
#include <stdio.h>
#include <stdlib.h>


class HeightMap : public ModelObject
{
private:
	//int num_cols;
	//int num_rows;

	//int cell_width;
	//int cell_height;

	int num_verts;
	int numIndices;

	SimpleVertex* vertices;
	int* indices;

	float SCALE;
public:

	HeightMap() : ModelObject()
	{
		vertices=NULL;
		indices=NULL;
	}

	~HeightMap()
	{
		delete vertices;
		delete indices;
	}

	void LoadTexture(ID3D11Device* device, WCHAR* name)
	{
		D3DX11CreateShaderResourceViewFromFile(device, name, NULL, NULL, &Tex, NULL);
	}

	void init(ID3D11Device* device, ID3D11DeviceContext* dcon, SimpleVertex* vertices, int* indices, int numIndex, int numVertex)
	{
		numIndexes=numIndex;
		D3D11_BUFFER_DESC bd;
		ZeroMemory( &bd, sizeof(bd) );
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( SimpleVertex ) * numVertex;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory( &InitData, sizeof(InitData) );
		InitData.pSysMem = vertices;
		device->CreateBuffer( &bd, &InitData, &VertexBuffer );

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( int ) * numIndexes;        // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indices;
		device->CreateBuffer( &bd, &InitData, &IndexBuffer );
	}

	float* GeneratePositionTexturedWithHeight(int verticesAlongWidth, int verticesAlongLength, unsigned short* pHeight)
	{
    vertices = new SimpleVertex[verticesAlongLength * verticesAlongWidth];
	float* Samples= new float[verticesAlongLength*verticesAlongWidth+2+1];
	Samples[0]=verticesAlongLength; Samples[1]=verticesAlongWidth;

	float MAX=0;
	//float SCALE=100;
	for (int i=0; i<verticesAlongLength*verticesAlongWidth; ++i)
	{
		if (MAX<(float)pHeight[i])
			MAX=(float)pHeight[i];

		
	}

	Samples[3]=MAX;

    for ( int z = 0; z < verticesAlongLength; z++ )
    {
        for ( int x = 0; x < verticesAlongWidth; x++ )
        {
            float halfWidth = ((float)verticesAlongWidth - 1.0f) / 2.0f;
            float halfLength = ((float)verticesAlongLength - 1.0f) / 2.0f;
			vertices[z * verticesAlongLength + x].Pos=XMFLOAT3((float)x - halfWidth, ((float)(pHeight[z * verticesAlongLength + x])/MAX)*SCALE, (float)z - halfLength);
			//Samples[z * verticesAlongLength + x+3]=((float)(pHeight[z * verticesAlongLength + x]));
			vertices[z * verticesAlongLength + x].Texcoord=XMFLOAT2((float)z / (verticesAlongWidth - 1), (float)x / (verticesAlongLength - 1));
			vertices[z * verticesAlongLength + x].Normal=XMFLOAT3(0, 1, 0);   //needs fixed
        }
    }
	/*
	PxHeightFieldDesc desc;
	desc.setToDefault();
	desc.nbColumns=(int)verticesAlongWidth;
	desc.nbRows=(int)verticesAlongLength;
	desc.samples.data=Samples;
	desc.samples.stride=sizeof(PxHeightFieldSample);
	PxSDK->MakeHeightField(desc, 1, 1, 1, this);
	*/
	return Samples;
	}

	int GenerateIndices(int verticesAlongWidth, int verticesAlongLength )
{
    numIndices = (verticesAlongWidth * 2) * (verticesAlongLength - 1) + (verticesAlongLength - 2);

    indices = new int[numIndices];

    int index = 0;
    for ( int z = 0; z < verticesAlongLength - 1; z++ )
    {
        // Even rows move left to right, odd rows move right to left.
        if ( z % 2 == 0 )
        {
            // Even row
            int x;
            for ( x = 0; x < verticesAlongWidth; x++ )
            {
                indices[index++] = x + (z * verticesAlongWidth);
                indices[index++] = x + (z * verticesAlongWidth) + verticesAlongWidth;
            }
            // Insert degenerate vertex if this isn't the last row
            if ( z != verticesAlongLength - 2)
            {
                indices[index++] = --x + (z * verticesAlongWidth);
            }
        } 
        else
        {
            // Odd row
            int x;
            for ( x = verticesAlongWidth - 1; x >= 0; x-- )
            {
                indices[index++] = x + (z * verticesAlongWidth);
                indices[index++] = x + (z * verticesAlongWidth) + verticesAlongWidth;
            }
            // Insert degenerate vertex if this isn't the last row
            if ( z != verticesAlongLength - 2)
            {
                indices[index++] = ++x + (z * verticesAlongWidth);
            }
        }
    } 
    return numIndices;
}

	int GetNumVerts()
	{
		return num_verts;
	}

	int GetNumIndices()
	{
		return numIndices;
	}

	SimpleVertex* GetVertices()
	{
		return vertices;
	}

	int* GetIndices()
	{
		return indices;
	}

	void create(ID3D11Device* device, ID3D11DeviceContext* dcon, const char *rawFile, WCHAR *tex_name, float scale)
	{
		SCALE=scale;
		FILE* file;
		file=fopen(rawFile, "rb");
   // std::ifstream heightStream;
    //heightStream.open( rawFile, std::ios::binary );

    // Get number of vertices
    //heightStream.seekg( 0, std::ios::end );
    //int num_verts = heightStream.tellg()/2;
    //heightStream.seekg( 0, std::ios::beg );
		fseek (file , 0 , SEEK_END);
		num_verts = ftell (file)/sizeof(short);
		rewind (file);

    // Allocate memory and read the data
   // UCHAR* m_pHeight = new UCHAR[num_verts];
	unsigned short* m_pHeight = new unsigned short[num_verts];
	fread(m_pHeight, sizeof(short), num_verts, file);
   // heightStream.read( (char *)m_pHeight, num_verts );
    //heightStream.close();
	fclose(file);

    // Generate vertices
    UINT width = (int)sqrt( (float)num_verts );
    float* samps=GeneratePositionTexturedWithHeight(width, width, m_pHeight);

    // Generate indices
    int num_index = GenerateIndices(width, width );
  
		init(device, dcon, vertices, indices, num_index, num_verts);
		LoadTexture(device, tex_name);

		delete m_pHeight;
	}

	void Draw(ID3D11DeviceContext* dcon)
	{
		dcon->PSSetShaderResources(0, 1, &Tex);
		UINT stride = sizeof( SimpleVertex );
		UINT offset = 0;
		dcon->IASetVertexBuffers( 0, 1, &VertexBuffer, &stride, &offset );
		dcon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		dcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		dcon->DrawIndexed(numIndexes-2, 0, 0);
	}
};

