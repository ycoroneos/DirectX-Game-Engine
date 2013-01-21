#include <assimp.hpp>     
#include <aiScene.h>     
#include <aiPostProcess.h> 
#include <fstream>
#define AI_LMW_MAX_WEIGHTS 4
#define AI_CONFIG_PP_LBW_MAX_WEIGHTS 4
WCHAR* aiToWCHAR(aiString source)
{
	WCHAR* file= new WCHAR[sizeof(source.data)];
	 MultiByteToWideChar
  (
    CP_ACP,         
    0,         
	source.data, 
    -1,       
    file,  
    256        
  );
	return file;
}

std::string aiToString(aiString source)
{
	std::string out;
	for (int i=0; source.data[i]!=NULL; ++i)
	{
		out.push_back(source.data[i]);
	}
	return out;
}

XMFLOAT3 aiVec3ToXMFLOAT3(aiVector3D in)
{
	return XMFLOAT3(in.x, in.y, in.z);
}

XMFLOAT2 aiVec3ToXMFLOAT2(aiVector3D in)
{
	return XMFLOAT2(in.x, in.y);
}

BoneFrame* MakeASkeleton(aiNode* root_node, aiMesh* mesh)
{
	/*
	aiBoneMap hiarch;
	for (int i=0; i<mesh->mNumBones; ++i)
	{
		hiarch.AddBone(mesh->mBones[i]);
	}
	*/

	AllocateHierarchy list;
	for (int i=0; i<mesh->mNumBones; ++i)
	{
		list.addBone(mesh->mBones[i]);
	}
	//BoneFrame* bones=list.make(root_node);
	//list.print_bones(bones);

	return list.make(root_node);

	
	/*
	std::vector<aiMatrix4x4> boneMatrix(mesh->mNumBones);
	for (size_t a=0; a<mesh->mNumBones; ++a)
	{
		const aiBone* bone=mesh->mBones[a];
		aiNode* node=root_node->FindNode(bone->mName);
		boneMatrix[a]=bone->mOffsetMatrix;
		const aiNode* tempnode=node;
		while(tempnode)
		{
			boneMatrix[a]=tempnode->mTransformation*boneMatrix[a];
			tempnode=tempnode->mParent;
		}
	}
	return boneMatrix;
	*/
}

class ObjectLibrary
{
private:
public:
	UINT numberObjects;
	//ModelObject* objects;
	vector<ModelObject*> objects;
	vector<SkinnedModelObject*> skinnedObjects;
	bool bones;
	ObjectLibrary()
	{
		//objects=NULL;
		numberObjects=0;
		bones=false;
	}
	~ObjectLibrary()
	{
		//delete[] objects;
		objects.clear();
		skinnedObjects.clear();
	}

	void addObject(ModelObject* in)
	{
		objects.push_back(in);
	}

	int LoadOBJ(const char* file, ID3D11Device* device, ID3D11DeviceContext* dcon)
	{

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, aiProcess_MakeLeftHanded | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_FlipUVs | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes);
		
		aiMesh** rawmeshes = scene->mMeshes;
		aiNode** nodes = scene->mRootNode->mChildren;
		aiMaterial** material = scene->mMaterials;
		aiString Texture;
		numberObjects=scene->mNumMeshes;
		//objects=new ModelObject[scene->mNumMeshes];

		SimpleVertex* verts;
		SkinnedVertex* sverts;
		WORD* indices;
		
		BoneFrame* root;
		
		int n=0;
		do 
		{
			if (nodes[n]->mNumMeshes==0)
			{++n; continue;}

		for (int i=0; i<nodes[n]->mNumMeshes ; ++i)
		{
			//Skeleton mr_bones;
			if (rawmeshes[nodes[n]->mMeshes[i]]->HasBones()&&rawmeshes[nodes[n]->mMeshes[i]]->mNumBones>1)
			{
				root=NULL;
				root=MakeASkeleton(scene->mRootNode, rawmeshes[nodes[n]->mMeshes[i]]);
				bones=true;
			}
			if (bones==false)
			{
				sverts=NULL;
				verts= new SimpleVertex[rawmeshes[nodes[n]->mMeshes[i]]->mNumVertices];
				for (int j=0; j<rawmeshes[nodes[n]->mMeshes[i]]->mNumVertices; ++j)
				{
					verts[j].Pos=aiVec3ToXMFLOAT3(rawmeshes[nodes[n]->mMeshes[i]]->mVertices[j]);
					verts[j].Normal=aiVec3ToXMFLOAT3(rawmeshes[nodes[n]->mMeshes[i]]->mNormals[j]);
					verts[j].Texcoord=aiVec3ToXMFLOAT2(rawmeshes[nodes[n]->mMeshes[i]]->mTextureCoords[0][j]);
				}
			}
			else if (bones==true)
			{
				verts=NULL;
				sverts=new SkinnedVertex[rawmeshes[nodes[n]->mMeshes[i]]->mNumVertices];
				/*
				std::ofstream outfile;
				outfile.open("bones.txt");
				for (int y=0; y<rawmeshes[nodes[n]->mMeshes[i]]->mNumBones; ++y)
				{
				for (int h=0; h<rawmeshes[nodes[n]->mMeshes[i]]->mBones[y]->mNumWeights; ++h)
				{
					outfile << rawmeshes[nodes[n]->mMeshes[i]]->mBones[y]->mWeights[h].mVertexId << "\n";
				}
				}
				outfile.close();
				*/
				for (int j=0; j<rawmeshes[nodes[n]->mMeshes[i]]->mNumVertices; ++j)
				{
					sverts[j].Pos=aiVec3ToXMFLOAT3(rawmeshes[nodes[n]->mMeshes[i]]->mVertices[j]);
					sverts[j].Normal=aiVec3ToXMFLOAT3(rawmeshes[nodes[n]->mMeshes[i]]->mNormals[j]);
					sverts[j].Texcoord=aiVec3ToXMFLOAT2(rawmeshes[nodes[n]->mMeshes[i]]->mTextureCoords[0][j]);
					vector<bone_indexer> pBones=FindBone(rawmeshes[nodes[n]->mMeshes[i]]->mVertices[j], rawmeshes[nodes[n]->mMeshes[i]]->mBones, rawmeshes[nodes[n]->mMeshes[i]]->mVertices, rawmeshes[nodes[n]->mMeshes[i]]->mNumBones);
					float weights[4];
					int id[4];
					memset(id, 0, 4*sizeof(int));
					memset(weights, 0, 4*sizeof(float));
					for (int k=0; k<pBones.size(); k++)
					{
						index_recursion(root, pBones[k].bone->mName.data, &id[k], false);
						weights[k]=pBones[k].bone->mWeights[pBones[k].index].mWeight;
						if (k>4)
							return 0;
					}
					sverts[j].BoneId=XMDEC4(id[0], id[1], id[2], id[3]);
					sverts[j].BoneWeight=XMFLOAT4(weights[0],weights[1],weights[2],weights[3]);
					ZeroMemory(weights, 4);
					ZeroMemory(id, 4);
				}
				/*
				for (int j=0; j<rawmeshes[nodes[n]->mMeshes[i]]->mNumBones; ++j)
				{
					aiBone* pBone=rawmeshes[nodes[n]->mMeshes[i]]->mBones[j];
					int weights[4];
					int id[4];
					for (int b=0; b<pBone->mNumWeights; ++b)
					{
						sverts[j].BoneId=FindFrame(pBone->mName.data, root);
						sverts[j].Pos=aiVec3ToXMFLOAT3(rawmeshes[nodes[n]->mMeshes[i]]->mVertices[pBone->mWeights[b].mVertexId]);
						sverts[j].Normal=aiVec3ToXMFLOAT3(rawmeshes[nodes[n]->mMeshes[i]]->mNormals[pBone->mWeights[b].mVertexId]);
						sverts[j].Texcoord=aiVec3ToXMFLOAT2(rawmeshes[nodes[n]->mMeshes[i]]->mTextureCoords[0][pBone->mWeights[b].mVertexId]);
						weights[b]=pBone->mWeights[b].mWeight;
					}
					
				}
				*/
			}

			indices=new WORD[rawmeshes[nodes[n]->mMeshes[i]]->mNumFaces*3];
			
			for (int j=0; j<rawmeshes[nodes[n]->mMeshes[i]]->mNumFaces; ++j)
			{
				indices[j*3+0] = rawmeshes[nodes[n]->mMeshes[i]]->mFaces[j].mIndices[0];
				indices[j*3+1] = rawmeshes[nodes[n]->mMeshes[i]]->mFaces[j].mIndices[1];
				indices[j*3+2] = rawmeshes[nodes[n]->mMeshes[i]]->mFaces[j].mIndices[2];
			}

			

			if (device!=NULL)
			{
				if (bones!=true)
				{
					ModelObject* temp=new ModelObject();
				temp->init(device, dcon, verts, indices, rawmeshes[nodes[n]->mMeshes[i]]->mNumFaces*3, rawmeshes[nodes[n]->mMeshes[i]]->mNumVertices);  //make mesh
				temp->name=aiToString(nodes[n]->mName);
				material[rawmeshes[nodes[n]->mMeshes[i]]->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &Texture, NULL, NULL, NULL, NULL, NULL);
				if (Texture.length>0)
					temp->texture=aiToWCHAR(Texture);
				//objects[n].texture=L"bfbc2.jpg";
				temp->LoadTexture(device);
				objects.push_back(temp);
				}
				if (bones==true)
				{
					SkinnedModelObject* tskin = new SkinnedModelObject(root);//, rawmeshes[nodes[n]->mMeshes[i]]->mNumBones);
					tskin->init(device, dcon, sverts, indices, rawmeshes[nodes[n]->mMeshes[i]]->mNumFaces*3, rawmeshes[nodes[n]->mMeshes[i]]->mNumVertices);  //make mesh
				tskin->name=aiToString(nodes[n]->mName);
				material[rawmeshes[nodes[n]->mMeshes[i]]->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &Texture, NULL, NULL, NULL, NULL, NULL);
				if (Texture.length>0)
					tskin->texture=aiToWCHAR(Texture);
				tskin->LoadTexture(device);
				skinnedObjects.push_back(tskin);
				bones=false;
				}
			}
			if (sverts!=NULL)
				delete[] sverts;
			if (verts!=NULL)
				delete[] verts;
			if (indices!=NULL)
				delete[] indices;
		}
		++n;
		} while (n<scene->mRootNode->mNumChildren);
 
		return 1;
	}
};