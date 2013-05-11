varying float intensity;
void main(){
//    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
//    gl_Position = ftransform();
    intensity = dot(ftransform().xyz,gl_Normal);
    gl_FrontColor = gl_Color;
	gl_Position = ftransform();
}