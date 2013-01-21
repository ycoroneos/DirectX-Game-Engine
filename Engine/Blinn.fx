////////////////////////////////////////////////////////////////////////////////
// Filename: light.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////

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
	float4 lightDirection;
	float4 ambientColor;
	float4 diffuseColor;
	float4 specularColor;
	float2 specularPower;
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
	float4 textureColor;
	float4 color;
	float3 lightDir;
	float lightIntensity;
	float3 reflection;
    float4 specular;
	

	// Sample the texture pixel at this location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	
	// Set the default output color to the ambient light value for all pixels.
    color = ambientColor;

	// Invert the light direction for calculations.
	lightDir = -lightDirection.xyz;

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, lightDir));

	if(lightIntensity > 0.0f)
    {
        // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
        color += (diffuseColor * lightIntensity);
        
        // Saturate the ambient and diffuse color.
        color = saturate(color);

		// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
        reflection = normalize(2 * lightIntensity * input.normal - lightDir); 
		// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower.x);
    }

	// Multiply the texture pixel and the input color to get the final result.
	color = color * textureColor;

	// Add the specular component last to the output color.
    color = saturate(color + specular);
	
	return color;
}
