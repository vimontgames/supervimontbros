void main()
{
	float4 pos = gl_Vertex;

	gl_Position = gl_ModelViewProjectionMatrix * pos;

	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
}