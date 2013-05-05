//water shader 
uniform sampler2D tex;

void main (void) {
	vec4 texel = texture2D(tex, gl_TexCoord[0].st);

	gl_FragColor = texel * gl_Color;
}
