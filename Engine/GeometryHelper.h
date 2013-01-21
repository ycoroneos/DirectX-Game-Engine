#define CACHE_SIZE 240
#define D3DX_PI 3.14159265
struct LightVertex
{
    XMFLOAT3 pos;  // Position
	XMFLOAT3 norm;
};

static inline void sincosf( float angle, float* psin, float* pcos )
{
    *psin = sinf( angle );
    *pcos = cosf( angle );
}

static void MakeSphere( LightVertex* pVertices, WORD* pwIndices, float fRadius, UINT uSlices, UINT uStacks )
{
    UINT i, j;


    // Sin/Cos caches
    float sinI[CACHE_SIZE], cosI[CACHE_SIZE];
    float sinJ[CACHE_SIZE], cosJ[CACHE_SIZE];

    for( i = 0; i < uSlices; i++ )
        sincosf( 2.0f * D3DX_PI * i / uSlices, sinI + i, cosI + i );

    for( j = 0; j < uStacks; j++ )
        sincosf( D3DX_PI * j / uStacks, sinJ + j, cosJ + j );



    // Generate vertices
    LightVertex* pVertex = pVertices;

    // +Z pole
    pVertex->pos = XMFLOAT3( 0.0f, 0.0f, fRadius );
    pVertex->norm = XMFLOAT3( 0.0f, 0.0f, 1.0f );
    pVertex++;

    // Stacks
    for( j = 1; j < uStacks; j++ )
    {
        for( i = 0; i < uSlices; i++ )
        {
            XMFLOAT3 norm( sinI[i]* sinJ[j], cosI[i]* sinJ[j], cosJ[j] );
			XMVECTOR vec=XMLoadFloat3(&norm) * fRadius;
            pVertex->pos = XMFLOAT3(XMVectorGetX(vec), XMVectorGetY(vec), XMVectorGetZ(vec));
            pVertex->norm = norm;

            pVertex++;
        }
    }

    // Z- pole
    pVertex->pos = XMFLOAT3( 0.0f, 0.0f, -fRadius );
    pVertex->norm = XMFLOAT3( 0.0f, 0.0f, -1.0f );
    pVertex++;



    // Generate indices
    WORD* pwFace = pwIndices;
    UINT uRowA, uRowB;

    // Z+ pole
    uRowA = 0;
    uRowB = 1;

    for( i = 0; i < uSlices - 1; i++ )
    {
        pwFace[0] = ( WORD )( uRowA );
        pwFace[1] = ( WORD )( uRowB + i + 1 );
        pwFace[2] = ( WORD )( uRowB + i );
        pwFace += 3;
    }

    pwFace[0] = ( WORD )( uRowA );
    pwFace[1] = ( WORD )( uRowB );
    pwFace[2] = ( WORD )( uRowB + i );
    pwFace += 3;

    // Interior stacks
    for( j = 1; j < uStacks - 1; j++ )
    {
        uRowA = 1 + ( j - 1 ) * uSlices;
        uRowB = uRowA + uSlices;

        for( i = 0; i < uSlices - 1; i++ )
        {
            pwFace[0] = ( WORD )( uRowA + i );
            pwFace[1] = ( WORD )( uRowA + i + 1 );
            pwFace[2] = ( WORD )( uRowB + i );
            pwFace += 3;

            pwFace[0] = ( WORD )( uRowA + i + 1 );
            pwFace[1] = ( WORD )( uRowB + i + 1 );
            pwFace[2] = ( WORD )( uRowB + i );
            pwFace += 3;
        }

        pwFace[0] = ( WORD )( uRowA + i );
        pwFace[1] = ( WORD )( uRowA );
        pwFace[2] = ( WORD )( uRowB + i );
        pwFace += 3;

        pwFace[0] = ( WORD )( uRowA );
        pwFace[1] = ( WORD )( uRowB );
        pwFace[2] = ( WORD )( uRowB + i );
        pwFace += 3;
    }

    // Z- pole
    uRowA = 1 + ( uStacks - 2 ) * uSlices;
    uRowB = uRowA + uSlices;

    for( i = 0; i < uSlices - 1; i++ )
    {
        pwFace[0] = ( WORD )( uRowA + i );
        pwFace[1] = ( WORD )( uRowA + i + 1 );
        pwFace[2] = ( WORD )( uRowB );
        pwFace += 3;
    }

    pwFace[0] = ( WORD )( uRowA + i );
    pwFace[1] = ( WORD )( uRowA );
    pwFace[2] = ( WORD )( uRowB );
    pwFace += 3;
}


HRESULT DXUTCreateSphere(ID3D11Device* pDevice, float fRadius, UINT uSlices, UINT uStacks, WORD* pwIndices, LightVertex* pVertices)
{
    HRESULT hr = S_OK;

    pwIndices = NULL;
    pVertices = NULL;

    // Validate parameters
    if( !pDevice )
        return D3DERR_INVALIDCALL;
    if( fRadius < 0.0f )
        return D3DERR_INVALIDCALL;
    if( uSlices < 2 )
        return D3DERR_INVALIDCALL;
    if( uStacks < 2 )
        return D3DERR_INVALIDCALL;

    // Create the mesh
    UINT cFaces = 2 * ( uStacks - 1 ) * uSlices;
    UINT cVertices = ( uStacks - 1 ) * uSlices + 2;

    // Create enough memory for the vertices and indices
    pVertices = new LightVertex[ cVertices ];
    if( !pVertices )
        return E_OUTOFMEMORY;
    pwIndices = new WORD[ cFaces * 3 ];
    if( !pwIndices )
        return E_OUTOFMEMORY;

    // Create a sphere
    MakeSphere( pVertices, pwIndices, fRadius, uSlices, uStacks );

    // Free up the memory
	delete [] pVertices;
	delete [] pwIndices;

    return hr;
}