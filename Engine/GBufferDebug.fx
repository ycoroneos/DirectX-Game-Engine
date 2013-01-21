SamplerState SampleType : register( s0 );

cbuffer MatrixBuffer : register( b0 )
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
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
    
	// Store the color for the pixel shader.
	output.color = input.color;
    
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
	//float4 normal: SV_TARGET1;
	//float  depth: SV_TARGET2;
};
////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
PSOUT LightPixelShader(PixelInputType input)
{
	PSOUT psout;
	
	psout.color = input.color;
	//psout.normal.rgb=input.normal.rgb;
	//psout.normal.a=0;
	//psout.depth=input.position.z;
	return psout;
}