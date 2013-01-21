#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_
#define NUM_SKIN_MATRICES 150
struct SimpleVertex
{
    XMFLOAT3 Pos;  // Position
	XMFLOAT2 Texcoord;
	XMFLOAT3 Normal;
};

struct SkinnedVertex
{
	XMFLOAT3	Pos;
	XMDEC4		BoneId;
	XMFLOAT4	BoneWeight;
	XMFLOAT3	Normal;
	XMFLOAT2	Texcoord;
};

#endif