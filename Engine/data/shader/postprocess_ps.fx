uniform sampler2D texture;
uniform vec3 color;
uniform float saturation;
uniform float blur;
uniform float pixelize;
uniform float2 screenSize;
uniform sampler2D lut;
uniform sampler2D dUdV;
uniform bool final;

void main()
{
	float4 pixel = float4(0,0,0,0);

	float2 uv = gl_TexCoord[0].xy;

	if (pixelize > 0.0)
	{
		float scale = lerp(1, 16.0, pixelize);
		uv = round(uv * screenSize / scale) / (screenSize / scale);
	}
	
	if (blur > 0.0)
	{
		pixel = texture_blur(texture, uv, screenSize.xy, 5, blur);
	}
	else
	{	
		pixel = texture2D(texture, uv);
	}

	pixel.rgb = color.rgb * pixel.rgb;
	//pixel.rgb = sampleAs3DTexture(lut, pixel.rgb, 16, 0, 2);
	pixel.rgb = color_saturation(pixel.rgb, saturation);

	if (final)
	{
		gl_FragColor.rgb = pixel.rgb;
		gl_FragColor.a = 1.0;
	}
	else
	{
		#if 0
		if (uv.x > 0.5)
			gl_FragColor.rgb = pixel.a;
		else
			gl_FragColor.rgb = pixel.rgb;
		#else
		gl_FragColor.rgb = lerp(pixel.rgb*0.75, pixel.rgb, pixel.a);
		#endif

		gl_FragColor.a = 1.0;
	}
}