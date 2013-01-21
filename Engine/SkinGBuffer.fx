Texture2D ColorTexture : register(t0);
SamplerState Linear : register(s0);

cbuffer SkinningMatrices  : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	matrix SkinMat[200];
};

struct VS_INPUT
{
	float4 position : POSITION;
	int4 bone : BONEIDS;
	float4 weights : BONEWEIGHTS;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 normal : NORMAL;
	float2 tex : TEXCOORD0;
};

VS_OUTPUT VSMAIN(VS_INPUT input)
{
	VS_OUTPUT output;
	input.position.w=1.0f;
	
	output.position=(mul(input.position, SkinMat[input.bone.x]))*input.weights.x;
	output.position+=(mul(input.position, SkinMat[input.bone.y]))*input.weights.y;
	output.position+=(mul(input.position, SkinMat[input.bone.z]))*input.weights.z;
	//--------------------------------------------------------------------------------//
	float last=input.weights.x+input.weights.z+input.weights.y;

	output.position+=(mul(input.position, SkinMat[input.bone.w]))*(1-last);

	output.position=mul(output.position, mul(View, Projection));
	output.tex=input.tex;
	output.normal=float4(1.0f, 0, 0, 0);
	return output;
}
struct PSOUT
{
	float4 color: SV_TARGET0;
	float4 normal: SV_TARGET1;
	//float  depth: SV_TARGET2;
};
////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
PSOUT PSMAIN(VS_OUTPUT input)
{
	PSOUT psout;
	
	psout.color = ColorTexture.Sample(Linear, input.tex);
	psout.normal.rgb=input.normal.rgb;
	psout.normal.a=0;
	//psout.depth=input.position.z;
	return psout;
}



