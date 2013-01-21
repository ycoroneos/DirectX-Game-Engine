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

cbuffer LightBuffer : register( b2 )
{
	float4 lightDirIntensity[9];
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
	float3 viewDirection : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
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

	// Calculate the position of the vertex in the world.
    worldPosition = mul(input.position, worldMatrix);

    // Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
	
    // Normalize the viewing direction vector.
    output.viewDirection = normalize(output.viewDirection);

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Filename: light.ps
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////



////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
	float4 color;
	
	color = shaderTexture.Sample(SampleType, input.tex);
	
	float3 sum={0, 0, 0};
	
	
	sum+=dot(input.normal, lightDirIntensity[0].xyz)*lightDirIntensity[0].a;
	sum+=dot(input.normal, lightDirIntensity[1].xyz)*lightDirIntensity[1].a;
	sum+=dot(input.normal, lightDirIntensity[2].xyz)*lightDirIntensity[2].a;
	sum+=dot(input.normal, lightDirIntensity[3].xyz)*lightDirIntensity[3].a;
	sum+=dot(input.normal, lightDirIntensity[4].xyz)*lightDirIntensity[4].a;
	sum+=dot(input.normal, lightDirIntensity[5].xyz)*lightDirIntensity[5].a;
	sum+=dot(input.normal, lightDirIntensity[6].xyz)*lightDirIntensity[6].a;
	sum+=dot(input.normal, lightDirIntensity[7].xyz)*lightDirIntensity[7].a;
	sum+=dot(input.normal, lightDirIntensity[8].xyz)*lightDirIntensity[8].a;
	
	float4 diffusediscrete={diffuseAroughness.xyz*sum, 1};
	
	float3 H;
	float e=pow(2, 12*diffuseAroughness.a);
	sum[0]=0; sum[1]=0; sum[2]=0;
	
	H=normalize(lightDirIntensity[0].xyz + input.viewDirection);
	sum+=((1+e)/(8*pow(dot(lightDirIntensity[0].xyz, H), 3)))*pow(dot(input.normal, H), e)*dot(input.normal, lightDirIntensity[0].xyz)*lightDirIntensity[0].a;
	
	H=normalize(lightDirIntensity[1].xyz + input.viewDirection);
	sum+=((1+e)/(8*pow(dot(lightDirIntensity[1].xyz, H), 3)))*pow(dot(input.normal, H), e)*dot(input.normal, lightDirIntensity[1].xyz)*lightDirIntensity[1].a;

	H=normalize(lightDirIntensity[2].xyz + input.viewDirection);
	sum+=((1+e)/(8*pow(dot(lightDirIntensity[2].xyz, H), 3)))*pow(dot(input.normal, H), e)*dot(input.normal, lightDirIntensity[2].xyz)*lightDirIntensity[2].a;

	H=normalize(lightDirIntensity[3].xyz + input.viewDirection);
	sum+=((1+e)/(8*pow(dot(lightDirIntensity[3].xyz, H), 3)))*pow(dot(input.normal, H), e)*dot(input.normal, lightDirIntensity[3].xyz)*lightDirIntensity[3].a;

	H=normalize(lightDirIntensity[4].xyz + input.viewDirection);
	sum+=((1+e)/(8*pow(dot(lightDirIntensity[4].xyz, H), 3)))*pow(dot(input.normal, H), e)*dot(input.normal, lightDirIntensity[4].xyz)*lightDirIntensity[4].a;

	H=normalize(lightDirIntensity[5].xyz + input.viewDirection);
	sum+=((1+e)/(8*pow(dot(lightDirIntensity[5].xyz, H), 3)))*pow(dot(input.normal, H), e)*dot(input.normal, lightDirIntensity[5].xyz)*lightDirIntensity[5].a;

	H=normalize(lightDirIntensity[6].xyz + input.viewDirection);
	sum+=((1+e)/(8*pow(dot(lightDirIntensity[6].xyz, H), 3)))*pow(dot(input.normal, H), e)*dot(input.normal, lightDirIntensity[6].xyz)*lightDirIntensity[6].a;

	H=normalize(lightDirIntensity[7].xyz + input.viewDirection);
	sum+=((1+e)/(8*pow(dot(lightDirIntensity[7].xyz, H), 3)))*pow(dot(input.normal, H), e)*dot(input.normal, lightDirIntensity[7].xyz)*lightDirIntensity[7].a;

	H=normalize(lightDirIntensity[8].xyz + input.viewDirection);
	sum+=((1+e)/(8*pow(dot(lightDirIntensity[8].xyz, H), 3)))*pow(dot(input.normal, H), e)*dot(input.normal, lightDirIntensity[8].xyz)*lightDirIntensity[8].a;

	float4 speculardiscrete={specular.xyz*sum, 1};
	color+=speculardiscrete+diffusediscrete;
	return color;
}