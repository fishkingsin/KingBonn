//vertex shader buono!!! questo veramente funziona! 13 10 2011

varying vec3  texcoord;

// varying vec4 vVertex;
// varying vec3 vNormal;
void main(void) 
{

	// vVertex			= mMatrix * vec4( gl_Vertex );
	// vNormal			= gl_Normal;//normalize( vec3( mMatrix * vec4( gl_Normal, 0.0 ) ) );
	
	

 	vec4	texcoord0 = gl_ModelViewMatrix * gl_Vertex;
	//texcoord = texcoord0.xyz;
	texcoord = normalize(gl_Vertex.xyz);

    gl_Position    = ftransform();
   
}
