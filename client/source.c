#include <windows.h>
#include <stdio.h>
#include <intrin.h>
#include <glew.h>
#include <GL/gl.h>

WSADATA data;
SOCKET sock;
SOCKADDR_IN addr;

unsigned int mode = 1;

typedef struct{
	float x;
	float y;
}VEC2;

typedef struct{
	int x;
	int y;
}IVEC2;

typedef struct{
	float p1x[16];
	float p1y[16];
	float p2x[16];
	float p2y[16];
	unsigned char c;
}BULLET;

typedef struct{
	unsigned char id;
	unsigned char Cid;
	unsigned char c;
	float r[8];
	float x[8];
	float y[8];
	float vx[8];
	float vy[8];
}PLAYER;

typedef struct{
	unsigned char c;
	unsigned char m[256];
}OPENGLMES;

OPENGLMES glmes;

PLAYER players;

BULLET bullet;

IVEC2 reso;

unsigned char serverdata[1024];

unsigned int vramP;

unsigned char *map;

unsigned int shaderProgram;
unsigned int mapText;

unsigned int vertexShader;
unsigned int fragmentShader;

unsigned int VBO;

char *VERTsource;
char *FRAGsource;

HWND window;
MSG Msg;
HDC wndcontext;

HANDLE renderThread,networkThread;

float *vram;

inline float fabsft(float x){
	if(x > 0.0){
		return x;
	}
	else{
		return -x;
	}
}

inline float fmaxf(float x,float y){
	if(x < y){
		return y;
	}
	else{
		return x;
	}
}

PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1,
PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,PFD_TYPE_RGBA,
24,0, 0, 0, 0, 0, 0,0,0,0,
0,0,0,0,32,0,0,PFD_MAIN_PLANE,
0,0,0,0	};

IVEC2 getCurPos(){
	IVEC2 r;
	POINT cp;
	GetCursorPos(&cp);
	ScreenToClient(window,&cp);
	cp.y = 1080-cp.y;
	r.x = cp.x;
	r.y = cp.y;
	return r;
}

float sqrtfl(float square){
    float root=square/3.0f;
    int i;
    if (square <= 0) return 0.0f;
    for (i=0; i<8; i++)
        root = (root + square / root) / 2.0f;
    return root;
}

float distance(VEC2 p1,VEC2 p2){
	float r1 = p1.x - p2.x;
	float r2 = p1.y - p2.y;
	return sqrtfl(r1*r1+r2*r2);
}

void drawSquare(float x,float y,float xs,float ys,float id){
	vram[vramP]    = x;
	vram[vramP+1]  = y;
	vram[vramP+2]  = 0.0f;
	vram[vramP+3]  = 0.0f;
	vram[vramP+4]  = id;
	vram[vramP+5]  = x+xs;
	vram[vramP+6]  = y;
	vram[vramP+7]  = 1.0f;
	vram[vramP+8]  = 0.0f;
	vram[vramP+9]  = id;
	vram[vramP+10] = x;
	vram[vramP+11] = y+ys;
	vram[vramP+12] = 0.0f;
	vram[vramP+13] = 1.0f;
	vram[vramP+14] = id;
	vram[vramP+15] = x+xs;
	vram[vramP+16] = y+ys;
	vram[vramP+17] = 1.0f;
	vram[vramP+18] = 1.0f;
	vram[vramP+19] = id;
	vram[vramP+20] = x+xs;
	vram[vramP+21] = y;
	vram[vramP+22] = 1.0f;
	vram[vramP+23] = 0.0f;
	vram[vramP+24] = id;
	vram[vramP+25] = x;
	vram[vramP+26] = y+ys;
	vram[vramP+27] = 0.0f;
	vram[vramP+28] = 1.0f;
	vram[vramP+29] = id;
	vramP+=30;
}

void drawLine(float x1,float y1,float x2,float y2,float id){
	float ofx = x1-x2;
	float ofy = y1-y2;
	float mof = fmaxf(fabsf(ofx),fabsf(ofy));
	ofx /= mof;
	ofy /= mof;
	float rofx = (cosf(1.62f) * ofx - sinf(1.62f) * ofy) * 0.005f;
	float rofy = (sinf(1.62f) * ofx + cosf(1.62f) * ofy) * 0.005f;
	vram[vramP]    = x1-rofx;
	vram[vramP+1]  = y1-rofy;
	vram[vramP+2]  = 0.0f;
	vram[vramP+3]  = 0.0f;
	vram[vramP+4]  = id;
	vram[vramP+5]  = x1+rofx;
	vram[vramP+6]  = y1+rofy;
	vram[vramP+7]  = 1.0f;
	vram[vramP+8]  = 0.0f;
	vram[vramP+9]  = id;
	vram[vramP+10] = x2-rofx;
	vram[vramP+11] = y2-rofy;
	vram[vramP+12] = 0.0f;
	vram[vramP+13] = 1.0f;
	vram[vramP+14] = id;
	vram[vramP+15] = x2+rofx;
	vram[vramP+16] = y2+rofy;
	vram[vramP+17] = 1.0f;
	vram[vramP+18] = 1.0f;
	vram[vramP+19] = id;
	vram[vramP+20] = x1+rofx;
	vram[vramP+21] = y1+rofy;
	vram[vramP+22] = 1.0f;
	vram[vramP+23] = 0.0f;
	vram[vramP+24] = id;
	vram[vramP+25] = x2-rofx;
	vram[vramP+26] = y2-rofy;
	vram[vramP+27] = 0.0f;
	vram[vramP+28] = 1.0f;
	vram[vramP+29] = id;
	vramP+=30;	
}

unsigned int proc(HWND hwnd,unsigned int msg,WPARAM wparam,LPARAM lparam){
	switch(msg){
	case WM_CLOSE:
	case WM_QUIT:
		closesocket(sock);
		ExitProcess(0);
	}
	return DefWindowProc(hwnd,msg,wparam,lparam);
}

WNDCLASS wndclass = {0,proc,0,0,0,0,0,0,"window","window"};

void input(){
	for(;;){
		Sleep(15);
	}
}

void network(){
	WORD ver = MAKEWORD(2, 2);
	WSAStartup(ver, &data);
	sock = socket(AF_INET,SOCK_STREAM,0);
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(7778);
	addr.sin_addr.S_un.S_addr = inet_addr("77.172.240.97");
	while(connect(sock,(SOCKADDR*)&addr,sizeof(addr))){}
	CreateThread(0,0,input,0,0,0);
	recv(sock,&players.id,1,0);
	recv(sock,map,65536*4,0);
	glmes.m[glmes.c] = 1;
	glmes.c++;
	for(;;){
		serverdata[0] = 0;
		if(GetKeyState(0x57)&0x80){
			serverdata[0] |= 0x01;
		}
		if(GetKeyState(0x53)&0x80){	
			serverdata[0] |= 0x02;
		}
		if(GetKeyState(0x44)&0x80){	
			serverdata[0] |= 0x04;
		}
		if(GetKeyState(0x41)&0x80){	
			serverdata[0] |= 0x08;
		}
		if(GetKeyState(VK_LBUTTON)&0x80){
			serverdata[0] |= 0x10;
		}
		IVEC2 c = getCurPos();
		c.x -= reso.x>>1;
		c.y -= reso.y>>1;
		short ang = (atan2f(c.x,c.y)+3.141592f)*10000.0f;
		serverdata[1] = ang;
		serverdata[2] = ang>>8;
		send(sock,serverdata,3,0);
		recv(sock,serverdata,1024,0);
		unsigned int pP = 0;
		players.c = serverdata[0];
		for(int i = 0;i < players.c;i++){
			players.x[i]   = (float)serverdata[i*9+1]/reso.x;
			players.x[i]  += (float)(serverdata[i*9+2]<<8)/reso.x;
			players.y[i]   = (float)serverdata[i*9+3]/reso.y;
			players.y[i]  += (float)(serverdata[i*9+4]<<8)/reso.y;
			players.vx[i]  = (float)(char)serverdata[i*9+5]/reso.x;
			players.vy[i]  = (float)(char)serverdata[i*9+6]/reso.y;
			players.r[i]  = serverdata[i*9+7];
			players.r[i]  += serverdata[i*9+8]<<8;
			if(serverdata[i*9+9] == players.id){
				players.Cid = i;
			}
		}
		pP += players.c*9+1;
		bullet.c = serverdata[pP];
		for(int i = 0;i < bullet.c;i++){
			bullet.p1x[i] =  (float)serverdata[i*8+pP+1]/reso.x;
			bullet.p1x[i] += (float)(serverdata[i*8+pP+2]<<8)/reso.x;
			bullet.p1y[i] =  (float)serverdata[i*8+pP+3]/reso.y;
			bullet.p1y[i] += (float)(serverdata[i*8+pP+4]<<8)/reso.y;
			bullet.p2x[i] =  (float)serverdata[i*8+pP+5]/reso.x;
			bullet.p2x[i] += (float)(serverdata[i*8+pP+6]<<8)/reso.x;
			bullet.p2y[i] =  (float)serverdata[i*8+pP+7]/reso.y;
			bullet.p2y[i] += (float)(serverdata[i*8+pP+8]<<8)/reso.y;
		}
	}
}

void prediction(){	
	for(;;){
		for(int i = 0;i < players.c;i++){
			players.x[i] += players.vx[i];
			players.y[i] += players.vy[i];
		}	
		Sleep(15);
	}
}

void render(){
	HANDLE h = CreateFile("shaders/vertex.vs",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	int fsize = GetFileSize(h,0);
	VERTsource = HeapAlloc(GetProcessHeap(),8,fsize+1);
	ReadFile(h,VERTsource,fsize+1,0,0);
	CloseHandle(h);

	h = CreateFile("shaders/fragment.fs",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	fsize = GetFileSize(h,0);
	FRAGsource = HeapAlloc(GetProcessHeap(),8,fsize+1);
	ReadFile(h,FRAGsource,fsize+1,0,0);
	CloseHandle(h);

	SetPixelFormat(wndcontext, ChoosePixelFormat(wndcontext, &pfd), &pfd);
	wglMakeCurrent(wndcontext, wglCreateContext(wndcontext));

	glewInit();

	((int(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(1);

	shaderProgram = glCreateProgram();
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertexShader,1,(const char**)&VERTsource,0);
	glShaderSource(fragmentShader,1,(const char**)&FRAGsource,0);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	char *bericht = HeapAlloc(GetProcessHeap(),8,1000);
	glGetShaderInfoLog(fragmentShader,1000,0,bericht);
	printf("%s\n",bericht);
	glGetShaderInfoLog(vertexShader,1000,0,bericht);
	printf("%s\n",bericht);
	HeapFree(GetProcessHeap(),8,bericht);
	glAttachShader(shaderProgram,vertexShader);
	glAttachShader(shaderProgram,fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	glCreateBuffers(1,&VBO);	
	glBindBuffer(GL_ARRAY_BUFFER,VBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,2,GL_FLOAT,0,5 * sizeof(float),(void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,0,5 * sizeof(float),(void*)(sizeof(float)*2));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,1,GL_FLOAT,0,5 * sizeof(float),(void*)(sizeof(float)*4));

	for(;;){
		while(glmes.c){
			glmes.c--;
			switch(glmes.m[glmes.c]){
			case 1:
				glGenTextures(1,&mapText);
				glBindTexture(GL_TEXTURE_2D,mapText);
				glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,map);
				glGenerateMipmap(GL_TEXTURE_2D);
				break;
			}
		}
		drawSquare(-1.0,-1.0,2.0,2.0,0);
		for(int i = 0;i < players.c;i++){
			VEC2 dpos = {players.y[i]-players.y[players.Cid],players.x[i]-players.x[players.Cid]};
			drawSquare(dpos.x-0.0235f,dpos.y-0.04f,0.0473f,0.08f,1.0f);
			VEC2 rpos;
			rpos.x = dpos.x + sinf(players.r[i]*0.0001f+3.14f)*0.06f;
			rpos.y = dpos.y + cosf(players.r[i]*0.0001f+3.14f)*0.1f;
			vram[vramP]    = dpos.x + sinf(players.r[i]*0.0001f+1.62f)*0.006f;
			vram[vramP+1]  = dpos.y + cosf(players.r[i]*0.0001f+1.62f)*0.01f;
			vram[vramP+2]  = 0.0f;
			vram[vramP+3]  = 0.0f;
			vram[vramP+4]  = 2;
			vram[vramP+5]  = dpos.x - sinf(players.r[i]*0.0001f+1.62f)*0.006f;
			vram[vramP+6]  = dpos.y - cosf(players.r[i]*0.0001f+1.62f)*0.01f;
			vram[vramP+7]  = 1.0f;
			vram[vramP+8]  = 0.0f;
			vram[vramP+9]  = 2;
			vram[vramP+10] = rpos.x + sinf(players.r[i]*0.0001f+1.62f)*0.006f;
			vram[vramP+11] = rpos.y + cosf(players.r[i]*0.0001f+1.62f)*0.01f;
			vram[vramP+12] = 0.0f;
			vram[vramP+13] = 1.0f;
			vram[vramP+14] = 2;
			vram[vramP+15] = rpos.x - sinf(players.r[i]*0.0001f+1.62f)*0.006f;
			vram[vramP+16] = rpos.y - cosf(players.r[i]*0.0001f+1.62f)*0.01f;
			vram[vramP+17] = 1.0f;
			vram[vramP+18] = 1.0f;
			vram[vramP+19] = 2;
			vram[vramP+20] = dpos.x - sinf(players.r[i]*0.0001f+1.62f)*0.006f;
			vram[vramP+21] = dpos.y - cosf(players.r[i]*0.0001f+1.62f)*0.01f;
			vram[vramP+22] = 1.0f;
			vram[vramP+23] = 0.0f;
			vram[vramP+24] = 2;
			vram[vramP+25] = rpos.x + sinf(players.r[i]*0.0001f+1.62f)*0.006f;
			vram[vramP+26] = rpos.y + cosf(players.r[i]*0.0001f+1.62f)*0.01f;
			vram[vramP+27] = 0.0f;
			vram[vramP+28] = 1.0f;
			vram[vramP+29] = 2;
			vramP+=30;
		}	
		for(int i = 0;i < bullet.c;i++){
			drawLine(bullet.p1y[i]-players.y[players.Cid],bullet.p1x[i]-players.x[players.Cid],bullet.p2y[i]-players.y[players.Cid],bullet.p2x[i]-players.x[players.Cid],3.0f);
		}
		glUniform2f(glGetUniformLocation(shaderProgram,"cam"),players.y[players.Cid]*0.5f,players.x[players.Cid]*0.5f);
		glBufferData(GL_ARRAY_BUFFER,vramP * sizeof(float),vram,GL_STATIC_DRAW);
		glClear(GL_COLOR_BUFFER_BIT); 
		glDrawArrays(GL_TRIANGLES,0,vramP);
		SwapBuffers(wndcontext);
		vramP = 0;
	}
}

void main(){
	map  = HeapAlloc(GetProcessHeap(),8,65536*4);
	vram = HeapAlloc(GetProcessHeap(),8,4000);
	timeBeginPeriod(1);
	wndclass.hInstance = GetModuleHandle(0);
	RegisterClass(&wndclass);
	reso.x = GetSystemMetrics(SM_CXSCREEN);
	reso.y = GetSystemMetrics(SM_CYSCREEN);
	window = CreateWindowEx(0,"window","game",0x90080000,0,0,reso.x,reso.y,0,0,wndclass.hInstance,0);
	wndcontext = GetDC(window);

	networkThread = CreateThread(0,0,network,0,0,0);
	renderThread  = CreateThread(0,0,render,0,0,0);
	CreateThread(0,0,prediction,0,0,0);
	
	for(;;){
		while(PeekMessage(&Msg,window,0,0,0)){
			GetMessage(&Msg,window,0,0);
			TranslateMessage(&Msg);
			DispatchMessageW(&Msg);
		}
		Sleep(1);
	}
}
