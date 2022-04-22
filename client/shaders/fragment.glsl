#version 460 core

out vec4 FragColor;
in vec2 Tcrd;
in float id;
uniform vec2 cam;
uniform sampler2D map;

void main(){
	switch(int(id)){
	case 0:{
		vec2 crd = vec2(0.5-Tcrd.x-cam.x,0.5-Tcrd.y-cam.y);
		crd.x *= 1.69;
		crd = - crd;
		if(crd.x<0.0||crd.y<0.0){
			FragColor.r = 1.0;
			return;
		}
		else{
			switch(int(texelFetch(map,ivec2(crd.x*32.0,crd.y*32.0),0).x*256.0)){
			case 0:
				FragColor = vec4(0.3,0.3,0.3,1.0);
				return;
			case 1:	
				FragColor = vec4(0.6,0.6,0.6,1.0);
				return;
			}
		}
		return;
	}
	case 1:{
		float d = distance(vec2(0.5),Tcrd);
		if(d<0.5){
			FragColor = vec4(0.0,1.0,0.0,1.0) * (1.0f-d);
			return;
		}	
		discard;
		}
	case 2:
		float d = distance(vec2(0.5),Tcrd);
		FragColor = vec4(0.0,1.0,0.0,1.0) * (1.0f-d);
		return;
	case 3:
		FragColor = vec4(0.7,0.0,0.0,1.0);
		return;
	}
} 
