#include <vector>
#include <assimp.hpp>     
#include <aiScene.h>
#include <aiPostProcess.h> 
//#include <Importer.hpp> // C++ importer interface
//#include <scene.h> // Output data structure
//#include <postprocess.h> // Post processing flags
#include <iostream>
#include <string>
using namespace std;

XMMATRIX ConvertAiMat(aiMatrix4x4 mat) 
{
	XMMATRIX out(mat[0]);
	mat.Transpose();
	/*
	out._11=mat.a1; out._12=mat.a2; out._13=mat.a3; out._14=mat.a4;
	out._21=mat.b1; out._22=mat.b2; out._23=mat.b3; out._24=mat.b4;
	out._31=mat.c1; out._32=mat.c2; out._33=mat.c3; out._34=mat.c4;
	out._41=mat.d1; out._42=mat.d2; out._43=mat.d3; out._44=mat.d4;
	/*
	out._11=mat.a1; out._12=mat.b1; out._13=mat.c1; out._14=mat.d1;
	out._21=mat.a2; out._22=mat.b2; out._23=mat.c2; out._24=mat.d2;
	out._31=mat.a3; out._32=mat.b3; out._33=mat.c3; out._34=mat.d3;
	out._41=mat.a4; out._42=mat.b4; out._43=mat.c4; out._44=mat.d4;
	return out;
	*/
	return out;
}

struct BoneFrame
{
private:
public:
	char* name;
	XMMATRIX Combined;
	XMMATRIX Local;
	XMMATRIX Offset;
	void* userData;
	BoneFrame* parent;
	BoneFrame* first_child;
	BoneFrame* sibling;
	int num_children;
};

void CopyBones(BoneFrame* dest, BoneFrame* source, BoneFrame* parent=NULL)
{
	dest->num_children=source->num_children;
	dest->name=new char[strlen(source->name)];
	strncpy(dest->name, source->name, strlen(source->name));
	dest->Combined=source->Combined;
	dest->Local=source->Local;
	dest->Offset=source->Offset;
	dest->parent=parent;
	if (source->sibling!=NULL)
	{
		dest->sibling=new BoneFrame();
		CopyBones(dest->sibling, source->sibling, parent);
	}
	else
		dest->sibling=NULL;
	if (source->first_child!=NULL)
	{
		dest->first_child=new BoneFrame();
		CopyBones(dest->first_child, source->first_child, dest);
	}
	else
		dest->first_child=NULL;
	return;
}

void CalculateWorldMatrices(BoneFrame** bone, XMMATRIX parentMatrix)
{
	if ((*bone)==NULL)
		return;
	(*bone)->Combined=(*bone)->Local*parentMatrix;
	CalculateWorldMatrices(&(*bone)->sibling, parentMatrix);
	CalculateWorldMatrices(&(*bone)->first_child, (*bone)->Combined);
}
void GetMats(BoneFrame* bone, XMMATRIX* out, int* num)
{
	if (bone==NULL)
		return;
	//out[(*num)]=bone->Combined;
	//(*num)++;
	(*out)=bone->Offset*bone->Combined;
	++out;
	GetMats(bone->sibling, &(*out), &(*num));
	GetMats(bone->first_child, &(*out), &(*num));
	return;
}

void index_recursion(BoneFrame* bone, const char* name, int* num, bool done)
{
	if (done==true || bone==NULL)
		return;
	
	if (strncmp(bone->name, name, strlen(name))==0)
	{
		done=true;
		return;
	}

	(*num)++;

	index_recursion(bone->sibling, name, &(*num), done);
	index_recursion(bone->first_child, name, &(*num), done);
}

int FindFrame(const char* name, BoneFrame* frames)
{
	int i=0;
	return 1;
}

class bone_indexer
{
private:
public:
	aiBone* bone;
	int index;
	bone_indexer(aiBone* in, int num)
	{
		index=num;
		bone=in;
	}
};

vector<bone_indexer> FindBone(aiVector3D pos, aiBone** bones, aiVector3D* vertices, int num_bones)
{
	vector<bone_indexer> out;
	for (int i=0; i<num_bones; ++i)
	{
		aiBone* pBone=bones[i];
		for (int i=0; i<pBone->mNumWeights; ++i)
		{
			if (vertices[pBone->mWeights[i].mVertexId]==pos&&pBone->mWeights[i].mWeight!=0)
			{
				out.push_back(bone_indexer(pBone, i));
				break;
			}
		}
	}
	return out;
}

class AllocateHierarchy
{
private:
	BoneFrame* out;
	vector<aiBone*> aibones;
public:
	void print_bones(BoneFrame* bone)
	{
		std::cout << bone->name << "\n";
		if (bone->sibling!=NULL)
			print_bones(bone->sibling);
		if (bone->first_child!=NULL)
			print_bones(bone->first_child);
	}
	void addBone(aiBone* in)
	{
		aibones.push_back(in);
	}

	aiNode* findNode(aiNode* root)
	{
		return root->FindNode(aibones[0]->mName);
	}

	bool isBone(aiNode* node)
	{
		for (int i=0; i<aibones.size(); ++i)
		{
			if (node->mName==aibones[i]->mName)
				return true;
		}
		return false;
	}

	aiBone* FindBone(aiNode* node)
	{
		for (int i=0; i<aibones.size(); ++i)
		{
			if (node->mName==aibones[i]->mName)
				return aibones[i];
		}
		return NULL;
	}

	void recursiveBone(BoneFrame* bone, aiNode* cur_node, BoneFrame* parent, int num_siblings)
	{
		if (isBone(cur_node)==true)
		{
			if (bone==NULL)
			{
				(BoneFrame*) bone=new BoneFrame();
				//memset(bone, 0, sizeof(BoneFrame));
			}
			//memcpy(bone->name, cur_node->mName.data, cur_node->mName.length);
			bone->name=new char[cur_node->mName.length];
			strncpy(bone->name, cur_node->mName.data, cur_node->mName.length);
			bone->Local=ConvertAiMat(cur_node->mTransformation);
			bone->Offset=ConvertAiMat(FindBone(cur_node)->mOffsetMatrix);
			bone->num_children=cur_node->mNumChildren;
			bone->parent=parent;
			if (num_siblings>0)
			{
				bone->sibling=new BoneFrame();
				recursiveBone(bone->sibling, cur_node->mParent->mChildren[cur_node->mParent->mNumChildren-num_siblings], bone, --num_siblings);
			}
			else 
			{
				bone->sibling=NULL;
			}
			if (cur_node->mNumChildren>0)
			{
				bone->first_child=new BoneFrame();
				recursiveBone(bone->first_child, cur_node->mChildren[0], bone, bone->num_children-1);
			}
			else
				bone->first_child=NULL;/*
			for (int i=0; i<bone->num_children; ++i)
			{
				recursiveBone((BoneFrame*) ++bone->children, cur_node->mChildren[i], bone);
			}
			*/
		}
		else
		{
			bone=NULL;
			return;
		}
	}

	BoneFrame* make(aiNode* pRoot)
	{
		aiNode* pNode=findNode(pRoot);
		while (isBone(pNode->mParent)==true)
		{
			pNode=pNode->mParent;
		}
		out=new BoneFrame();
		recursiveBone(out, pNode, NULL, 0);
		//out=new BoneFrame();
		//out->num_children=5;
		return out;
	}
};

class Bone
{
private:
	aiString name;
	XMMATRIX LocalTransform;
	XMMATRIX WorldTransform;
	XMMATRIX AssimpTransform;
	int num_children;
	int parent;
	int num_weights;
	int* children;
public:
	int* vert_index;
	float* weight;
	
	Bone(aiString input, int n_child, int n_weight, XMMATRIX LocalTrans, XMMATRIX AssimpTrans)
	{
		name=input;
		num_children=n_child;
		num_weights=n_weight;
		if (num_children!=0)
			children = new int[num_children];
		else
			children = NULL;
		vert_index= new int[num_weights];
		weight= new float[num_weights];
		parent=999;
		LocalTransform=LocalTrans;
		AssimpTransform=LocalTransform;
	}
	void SetParent(int p_in)
	{ 
		parent=p_in;
	}
	void SetChildren(int num, int index)
	{
		children[num]=index;
	}
	void SetWorldTransform(XMMATRIX in)
	{
		WorldTransform=in;
	}
	aiString GetName()
	{
		return name;
	}
	float GetWeight(int index)
	{
		return weight[index];
	}
	int GetVertId(int index)
	{
		return vert_index[index];
	}
	int GetNumWeights()
	{
		return num_weights;
	}
	int GetNumChildren()
	{
		return num_children;
	}
	int GetChild(int num)
	{
		return children[num];
	}
	int GetParent()
	{
		return parent;
	}
	XMMATRIX GetLocalTransform()
	{
		return LocalTransform;
	}
	XMMATRIX GetWorldTransform()
	{
		return WorldTransform;
	}
	XMMATRIX GetAssimpTransform()
	{
		return AssimpTransform;
	}
};

class Skeleton
{
private:
	vector<Bone*> bones;
	int num_bones;
public:
	Skeleton()
	{
	}
	Skeleton(vector<Bone*> input, int num) : num_bones(num)
	{
		//memcpy(&bones, &input, num);
		bones=input;
	}
	~Skeleton()
	{
		bones.clear();
	}
	XMFLOAT4 FindWeights(int index)
	{
		int iter=0;
		int out[4];

		for (int i=0; i<num_bones; ++i)
		{
				for (int j=0; j<bones[i]->GetNumWeights(); ++j)
				{
					if (index==bones[i]->GetVertId(j))
					{
						out[iter]=bones[i]->GetWeight(j);
						++iter;
					}
				}
			
		}
		return XMFLOAT4(out[0], out[1], out[2], out[3]);
	}
	XMDEC4 FindBones(int index)
	{
		int iter=0;
		int out[4];
		for (int i=0; i<num_bones; ++i)
		{
				for (int j=0; j<bones[i]->GetNumWeights(); ++j)
				{
					if (index==bones[i]->GetVertId(j))
					{
						out[iter]=i;
						++iter;
					}
				}
			
		}
		return XMDEC4(out[0], out[1], out[2], out[3]);
	}
	void recursive_update(int current, XMMATRIX worldTrans)
	{
		//XMMATRIX inverse=XMMatrixInverse(&XMMatrixDeterminant(bones[current]->GetLocalTransform()) ,bones[current]->GetLocalTransform());
		XMMATRIX final=bones[current]->GetAssimpTransform()*worldTrans;
		XMMATRIX inverse=bones[current]->GetLocalTransform();
		bones[current]->SetWorldTransform(inverse*final);
		if (bones[current]->GetNumChildren()>0)
		{
			for (int i=0; i<bones[current]->GetNumChildren(); ++i)
			{
				recursive_update(bones[current]->GetChild(i), final);
				//recursive_update(bones[current]->GetChild(i), XMMatrixIdentity());
			}
		}
	}
	void ReCalc(XMMATRIX in)
	{
		int i=0;
		while (bones[i]->GetParent()!=999)
		{
			++i;
		}
		recursive_update(i, in);
	}
	void makematrices(XMMATRIX out[])
	{
		//XMMATRIX out[NUM_SKIN_MATRICES];
		for (int i=0; i<num_bones; ++i)
		{
			out[i]=bones[i]->GetWorldTransform();
		}
		return;
	}
	int GetNumBones()
	{
		return num_bones;
	}
};

class aiBoneMap
{
private:
	vector<aiBone*> inbones;
	vector<aiNode*> nodes_used;
	//Bone* bones;
	vector<Bone*> bones;
public:
	~aiBoneMap()
	{
		//delete []bones;
	}
	void AddBone(aiBone* bone)
	{
		inbones.push_back(bone);
	}

	Skeleton CreateHierarchy(aiNode* root_node)
	{
		int num_bones=inbones.size();
		int root_index;
		for (vector<aiBone*>::iterator iter=inbones.begin(); iter!=inbones.end(); ++iter)
		{
			aiBone* temp=*iter;
			nodes_used.push_back(root_node->FindNode(temp->mName));
		}
		//bones = new Bone[nodes_used.size()];
		setBones();
		//bones[1];
		return Skeleton(bones, nodes_used.size());
	}

	aiNode* FindNode(aiString name)
	{
		for (int i=0; i<nodes_used.size(); ++i)
		{
			if (nodes_used[i]->mName==name)
				return nodes_used[i];
		}
		return NULL;
	}

	int FindBone(aiString name)
	{
		for (int i=0; i<nodes_used.size(); ++i)
		{
			if (strncmp(bones[i]->GetName().data, name.data, name.length)==0)
				return i;
		}
		return 999;
	}

	int FindInBone(aiString name)
	{
		for (int i=0; i<inbones.size(); i++)
		{
			if (strncmp(inbones[i]->mName.data, name.data, name.length)==0)
				return i;
		}
		return 999;
	}

	void setBones()
	{
		int num_bones=nodes_used.size();
		for (int i=0; i<num_bones; ++i)
		{
			//strncpy(bones[i].name.c_str(), inbones[i]->mName.data, inbones[i]->mName.length);
			///aiNode* node=FindNode(inbones[i]->mName);
			aiNode* node=nodes_used[i];
			int bone_num=FindInBone(node->mName);
			Bone* tBone=new Bone(inbones[bone_num]->mName, node->mNumChildren, inbones[bone_num]->mNumWeights, ConvertAiMat(inbones[bone_num]->mOffsetMatrix), ConvertAiMat(node->mTransformation));
			//bones[i].name=inbones[i]->mName;
			//bones[i].LocalTransform=ConvertAiMat(inbones[i]->mOffsetMatrix);         ////CHECK MATRIX CONVERT FUNCTION!!!!!!!!!
			//bones[i].num_children=node->mNumChildren;
			//bones[i].children=new int[bones[i].num_children];
			//bones[i].num_weights=inbones[i]->mNumWeights;
			//bones[i].vert_index=new int[bones[i].num_weights];
			//bones[i].weight=new float[bones[i].num_weights];
			for (int j=0; j<tBone->GetNumWeights(); ++j)
			{
				tBone->vert_index[j]=inbones[bone_num]->mWeights[j].mVertexId;
				tBone->weight[j]=inbones[bone_num]->mWeights[j].mWeight;
			}
			bones.push_back(tBone);
		}
		for (int i=0; i<num_bones; ++i)
		{
			aiString name;
			Bone* tBone=bones[i];
			//name.Set(bones[i].name);
			name=tBone->GetName();
			aiNode*node = FindNode(name);
			int parent_num=FindBone(node->mParent->mName);
			tBone->SetParent(parent_num);
			if (!tBone->GetNumChildren()==NULL)
			{
				for (int j=0; j<tBone->GetNumChildren(); ++j)
				{
					tBone->SetChildren(j, FindBone(node->mChildren[j]->mName));
				}
			}
		}
	}

	/*
	bool AssertName(aiNode* node)
	{
		for (int i=0; i<bones.size(); ++i)
		{
			if (bones[i]->mName==node->mName)
				return true;
		}
		return false;
	}
	*/

	
	/*
	void recursive_make(aiNode* cur_node, Bone* cur_bone, Bone* parent, XMMATRIX cur_worldTransform)
	{
		if (AssertName(cur_node)!=true)
		{
			cur_bone=NULL;
			return;
		}
		ystrncpy(cur_bone, cur_node);
		cur_bone->Parent= new Bone*(parent);
		cur_bone->LocalTransform=ConvertAiMat(cur_node->mTransformation.Transpose());
		cur_worldTransform*=cur_bone->LocalTransform;
		cur_bone->WorldTransform=cur_worldTransform;
		if (cur_node->mNumChildren!=0)
		{
			cur_bone->Child=new Bone*[cur_node->mNumChildren];
			cur_bone->num_children=cur_node->mNumChildren;
			for (int i=0; i<cur_node->mNumChildren; ++i)
			{
				cur_bone->Child[i]=new Bone();
				recursive_make(cur_node->mChildren[i], cur_bone->Child[i], cur_bone, cur_worldTransform);
			}
		}
		return;
	}
	*/
};