Texture2D DepthTexture : register( t1 );
//Texture2D ColorTexture : register( t2 );
Texture2D NormalTexture : register( t3 );

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

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
   float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
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
	float4 light: SV_TARGET0;
};
////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
PSOUT LightPixelShader(PixelInputType input)
{
	PSOUT psout;
	float depth=DepthTexture.Sample(SampleType, input.position.xy).x;
	//if (depth>input.position.z)
	//{
		//discard;
		//psout.light=float4(0.0f, 0.0f, 1.0f, 1.0f);
		//return psout;
	//}
	float3 normal=NormalTexture.Sample(SampleType, input.position.xy).xyz;
	psout.light=float4(1.0f, 0.0f, 0.0f, 1.0f);
	return psout;
}