uniform sampler2D texture;
uniform sampler2D lut;
uniform int lutIndex;

void main()
{
	float2 uv = gl_TexCoord[0].xy;
	float4 pixel = texture2D(texture, uv);

	// Force palette
    int palette = lutIndex;

	pixel.rgb = sampleAs3DTexture(lut, pixel.rgb, 16, palette, 16).rgb;
	pixel.rgba *= gl_Color.rgba;

	gl_FragColor = pixel;
}