#include <fstream>
#include <string>
using namespace std;

void Normalize(XMFLOAT3* in)
{
	float length=abs((in->x*in->x)+(in->y*in->y)+(in->z*in->z));
	in->x/=length;
	in->y/=length;
	in->z/=length;
}

ModelObject* LoadMesh(const char* filename, ObjectLibrary* lib, ID3D11Device* device, ID3D11DeviceContext* dcon)
{
	const char* directory="C:\\Users\\Rula\\Documents\\Visual Studio 2010\\Projects\\Engine\\Engine\\meshes";

	ifstream infile;
	std::string file=std::string(directory)+"\\"+std::string(filename)+".mesh";
	infile.open(file);
	int num_verts;
	int num_faces;
	char texture[256];
	bool bones;
	std::string bone;
	SimpleVertex* verts=NULL;
	SkinnedVertex* sverts=NULL;
	WORD* indices;
	void* pVert=NULL;
	WORD* pIndex=NULL;

	XMFLOAT3 pos, normal;
	XMFLOAT2 tex;

	int bone_id[4];
	float weight[4];

	int face[3];

	int i=0;

	while (infile.good())
	{
		char line[500];
		infile.getline(line, 500);
		sscanf(line, "verts %d", &num_verts);
		sscanf(line, "faces %d", &num_faces);
		sscanf(line, "color texture %s", texture);
		if (sscanf(line, "bones %s", bone.c_str())>0)
		{
			if (strncmp(bone.c_str(), "true", 4)==0)
			{
				bones=true;
				sverts=new SkinnedVertex[num_verts];
				pVert=&sverts[0];
			}
			else if (strncmp(bone.c_str(), "false", 4)==0)
			{
				bones=false;
				verts=new SimpleVertex[num_verts];
				pVert=&verts[0];
			}
			indices=new WORD[num_faces*3];
			pIndex=&indices[0];
		}
		if (sscanf(line, "vnt [%f,%f,%f],[%f,%f,%f],[%f,%f,%*f]", &pos.x, &pos.y, &pos.z, &normal.x, &normal.y, &normal.z, &tex.x, &tex.y)>0)
		{
			SimpleVertex* temp=(SimpleVertex*)pVert;
			(*temp).Pos=pos;
			Normalize(&normal);
			(*temp).Normal=normal;
			(*temp).Texcoord=tex;
			pVert=(SimpleVertex*)pVert+1;
		}
		if (sscanf(line, "vntbw [%f,%f,%f],[%f,%f,%f],[%f,%f,%*f],#(%d),#(%f)", &pos.x, &pos.y, &pos.z, &normal.x, &normal.y, &normal.z, &tex.x, &tex.y, &bone_id[0], &weight[0])>0)
		{
			SkinnedVertex* temp=(SkinnedVertex*)pVert;
			(*temp).Pos=pos;
			Normalize(&normal);
			(*temp).Normal=normal;
			(*temp).Texcoord=tex;
			(*temp).BoneId=XMDEC4(bone_id[0],0,0,0);
			(*temp).BoneWeight=XMFLOAT4(weight[0],0,0,0);
			pVert=(SkinnedVertex*)pVert+1;
		}
		if (sscanf(line, "vntbw [%f,%f,%f],[%f,%f,%f],[%f,%f,%*f],#(%d, %d),#(%f, %f)", &pos.x, &pos.y, &pos.z, &normal.x, &normal.y, &normal.z, &tex.x, &tex.y, &bone_id[0], &bone_id[1], &weight[0], &weight[1])>0)
		{
			SkinnedVertex* temp=(SkinnedVertex*)pVert;
			(*temp).Pos=pos;
			Normalize(&normal);
			(*temp).Normal=normal;
			(*temp).Texcoord=tex;
			(*temp).BoneId=XMDEC4(bone_id[0],bone_id[1],0,0);
			(*temp).BoneWeight=XMFLOAT4(weight[0],weight[1],0,0);
			pVert=(SkinnedVertex*)pVert+1;
		}
		if (sscanf(line, "vntbw [%f,%f,%f],[%f,%f,%f],[%f,%f,%*f],#(%d, %d, %d),#(%f, %f, %f)", &pos.x, &pos.y, &pos.z, &normal.x, &normal.y, &normal.z, &tex.x, &tex.y, &bone_id[0], &bone_id[1], &bone_id[2], &weight[0], &weight[1], &weight[2])>0)
		{
			SkinnedVertex* temp=(SkinnedVertex*)pVert;
			(*temp).Pos=pos;
			Normalize(&normal);
			(*temp).Normal=normal;
			(*temp).Texcoord=tex;
			(*temp).BoneId=XMDEC4(bone_id[0],bone_id[1],bone_id[2],0);
			(*temp).BoneWeight=XMFLOAT4(weight[0],weight[1],weight[2],0);
			pVert=(SkinnedVertex*)pVert+1;
		}
		if (sscanf(line, "vntbw [%f,%f,%f],[%f,%f,%f],[%f,%f,%*f],#(%d, %d, %d, %d),#(%f, %f, %f, %f)", &pos.x, &pos.y, &pos.z, &normal.x, &normal.y, &normal.z, &tex.x, &tex.y, &bone_id[0], &bone_id[1], &bone_id[2], &bone_id[3], &weight[0], &weight[1], &weight[2], &weight[3])>0)
		{
			SkinnedVertex* temp=(SkinnedVertex*)pVert;
			(*temp).Pos=pos;
			Normalize(&normal);
			(*temp).Normal=normal;
			(*temp).Texcoord=tex;
			(*temp).BoneId=XMDEC4(bone_id[0],bone_id[1],bone_id[2],bone_id[3]);
			(*temp).BoneWeight=XMFLOAT4(weight[0],weight[1],weight[2],weight[3]);
			pVert=(SkinnedVertex*)pVert+1;
		}
		if (sscanf(line, "%s, %d{ (matrix3 [%f,%f,%f] [%f,%f,%f] [%f,%f,%f] [%f,%f,%f]) undefined")>0)
		{
		}

		if (sscanf(line, "[%d,%d,%d]", &face[0], &face[1], &face[2])>0)
		{
			for (int i=0; i<3; ++i)
			{
				(*pIndex)=face[i];
				++pIndex;
			}
		}
	}
	if (bones==false)
	{
		ModelObject* temp=new ModelObject();
		temp->init(device, dcon, verts, indices, num_faces*3, num_verts);
		temp->name=std::string(filename);
		temp->texture=(WCHAR*)texture;
		temp->texture=NULL;
		temp->LoadTexture(device);
		lib->addObject(temp);
		return temp;
		//delete temp;
	}
	lib->numberObjects+=1;
	pVert=NULL;
	delete pVert;
	pIndex=NULL;
	delete pIndex;
	delete[] indices;
	if (verts!=NULL)
		delete[] verts;
	if (sverts!=NULL)
		delete[] sverts;
}