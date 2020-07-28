uniform sampler2D texture; 
uniform bool instance;

void main()
{
	float2 uv = gl_TexCoord[0].xy;
	float4 pixel = texture2D(texture, uv);

	if (instance)
		pixel.rgba *= gl_Color.rgba;

	gl_FragColor = float4(0,0,0,pixel.a);
}