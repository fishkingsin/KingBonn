uniform sampler2D tex;

void main (void) {
	vec4 texel = texture2D(tex, gl_TexCoord[0].st);
    if(texel.a < 0.5)
    discard;
	gl_FragColor = texel * gl_Color;
}
