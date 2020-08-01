uniform sampler2D texture;
uniform sampler2D lut;

void main()
{
	float2 uv = gl_TexCoord[0].xy;
	float4 pixel = texture2D(texture, uv);

	int palette = int(gl_Color.a * 255.5);
	pixel.rgb = sampleAs3DTexture(lut, pixel.rgb, 16, palette, 16).rgb;

	gl_FragColor.rgb = pixel.rgb;
	gl_FragColor.a = 1.0;
}