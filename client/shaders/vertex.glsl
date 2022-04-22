#version 460 core

layout (location = 0) in vec2 crd;
layout (location = 1) in vec2 tcrd;
layout (location = 2) in float ID;

out vec2 Tcrd;
out float id;

void main(){
	gl_Position = vec4(vec2(crd),1.0,1.0);
	Tcrd = tcrd;
	id = ID;
} 
