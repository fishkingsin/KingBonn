uniform samplerCube EnvMap;

varying vec3  texcoord;


void main (void)
{
 //   float aoLight		= 1.0 - length( vVertex.xyz ) * ( 0.0015 + ( 0.015 ) );
    vec3 envColor = vec3 (textureCube(EnvMap, texcoord));
	// //vec3 envColor = vec3 (textureCube(EnvMap, gl_TexCoord[0]));
	float gray = dot(envColor.rgb, vec3(0.299, 0.587, 0.114));


	// float ceiling		= 0.0;
	// if( vNormal.y < -0.5 ) ceiling = 1.0;
	
	// float yPer = clamp( vVertex.y/roomDims.y, 0.0, 1.0 );
	// float ceilingGlow	= pow( yPer, 2.0 ) * 0.25;
	// ceilingGlow			+= pow( yPer, 200.0 );
	// ceilingGlow			+= pow( max( yPer - 0.7, 0.0 ), 3.0 ) * 4.0;
	
	// vec3 litRoomColor	= vec3( aoLight + ( ceiling + ceilingGlow * timePer ) * lightPower );

    gl_FragColor = vec4 (gray*0.5,gray*0.5,gray*0.5, 1.0);
}
