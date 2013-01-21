Texture2D shaderTexture : register( t0 );
SamplerState SampleType : register( s0 );

cbuffer MatrixBuffer : register( b0 )
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer : register( b1 )
{
	float4 cameraPosition;
};

cbuffer ObjectBuffer : register( b3 )
{
	float4 diffuseAroughness;
	float4 specular;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
   	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};
///////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
	float4 worldPosition;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
	// Calculate the normal vector against the world matrix only.
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	
	// Normalize the normal vector.
	output.normal = normalize(output.normal);

	return output;
}

////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////

struct PSOUT
{
	float4 color: SV_TARGET0;
	float4 normal: SV_TARGET1;
	//float  depth: SV_TARGET2;
};
////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
PSOUT LightPixelShader(PixelInputType input)
{
	PSOUT psout;
	
	psout.color = shaderTexture.Sample(SampleType, input.tex);
	psout.normal.rgb=input.normal.rgb;
	psout.normal.a=0;
	//psout.depth=input.position.z;
	return psout;
}