uniform sampler2D colormap;
uniform sampler2D bumpmap;

varying vec2  TexCoord;

const float _NoiseFreq = 4.0;
const float _NoiseAmp = -0.5;
const vec3 _NoiseAnim = vec3(0, -1, 0);
uniform float iGlobalTime;
varying float displace ;
// color gradient
// this should be in a 1D texture really
vec4 gradient(float x)
{
	const vec4 c0 = vec4(4, 4, 4, 1);	// hot white
	const vec4 c1 = vec4(1, 1, 0, 1);	// yellow
	const vec4 c2 = vec4(1, 0, 0, 1);	// red
	const vec4 c3 = vec4(0.4, 0.4, 0.4, 4);	// grey
	
	float t = fract(x*3.0);
	vec4 c;
	if (x < 0.3333) {
		c =  mix(c0, c1, t);
	} else if (x < 0.6666) {
		c = mix(c1, c2, t);
	} else {
		c = mix(c2, c3, t);
	}
	//return vec4(x);
	//return vec4(t);
	return c;
}
// matrix to rotate the noise octaves
mat3 m = mat3( 0.00,  0.80,  0.60,
-0.80,  0.36, -0.48,
-0.60, -0.48,  0.64 );
float hash( float n )
{
    return fract(sin(n)*43758.5453);
}
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    
    f = f*f*(3.0-2.0*f);
    
    float n = p.x + p.y*57.0 + 113.0*p.z;
    
    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
    mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}
float fbm( vec3 p )
{
    float f;
    f = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p );
    p = m*p*2.02; f += 0.03125*abs(noise( p ));
    return f/0.9375;
}

void main(void) {
    vec4 oriColor = texture2D(colormap, TexCoord);
    float displace = fbm(oriColor.rgb * _NoiseFreq + _NoiseAnim*iGlobalTime);
//
    vec4 c = gradient(displace);

    gl_FragColor = oriColor*c;
}