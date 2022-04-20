#include <windows.h>
#include <stdio.h>
#include <intrin.h>

WSADATA data;
SOCKET sock;
SOCKADDR_IN addr;

unsigned int mode = 1;

typedef struct{
	float x;
	float y;

	float vx;
	float vy;

	float deltax;
	float deltay;

	float sidex;
	float sidey;

	int stepx;
	int stepy;

	int side;

	int ix;
	int iy;
}RAY;

typedef struct{
	float x;
	float y;
}VEC2;

typedef struct{
	int x;
	int y;
}IVEC2;

typedef struct{
	float x;
	float y;
	float vx;
	float vy;
}PROJECTILE;

typedef struct{
	unsigned int c;
	PROJECTILE p[256];
}BULLET;

typedef struct{
	unsigned char id;
	unsigned char Cid;
	unsigned char c;
	char rx[8];
	char ry[8];
	unsigned short x[8];
	unsigned short y[8];
	char vx[8];
	char vy[8];
}PLAYER;

PLAYER players;

BULLET bullet;

unsigned char serverdata[1024];

BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER),700,700,1,24,BI_RGB};
char *vram;
char *map;

HWND window;
MSG Msg;
HDC wndcontext;

HANDLE renderThread,networkThread;

inline unsigned int c2vram(unsigned int x,unsigned int y,unsigned int c){
	return x*700*3+y*3+c;
}

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

RAY rayCreate(float x,float y,float rx,float ry){
	RAY ray;
	ray.x  		= x;
	ray.y  		= y;
	ray.vx		= rx;
	ray.vy 		= ry;
	
	ray.deltax  = fabsft(1.0/ray.vx);
	ray.deltay  = fabsft(1.0/ray.vy);

	if(ray.vx < 0.0){
		ray.stepx = -1;
		ray.sidex = (ray.x-(int)ray.x) * ray.deltax;
	}
	else{
		ray.stepx = 1;
		ray.sidex = ((int)ray.x + 1.0 - ray.x) * ray.deltax;
	}
	if(ray.vy < 0.0){
		ray.stepy = -1;
		ray.sidey = (ray.y-(int)ray.y) * ray.deltay;
	}
	else{
		ray.stepy = 1;
		ray.sidey = ((int)ray.y + 1.0 - ray.y) * ray.deltay;
	}
	ray.ix = ray.x;
	ray.iy = ray.y;
	return ray;
}

void rayItterate(RAY *ray){
    if(ray->sidex < ray->sidey){
		ray->ix += ray->stepx;
		ray->sidex += ray->deltax;
		ray->side = 0;
    }
    else{
		ray->iy += ray->stepy;
		ray->sidey += ray->deltay;
		ray->side = 1;
    }
}

IVEC2 getCurPos(){
	IVEC2 r;
	POINT cp;
	GetCursorPos(&cp);
	ScreenToClient(window,&cp);
	cp.y = 702-cp.y;
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

void drawSphere(int x,int y,float r){
	for(int i = x-(int)r;i < x+(int)r;i++){
		for(int i2 = y-(int)r;i2 < y+(int)r;i2++){
			if(i < 0 || i > 700 || i2 < 0 || i2 > 700){
				continue;
			}
			if(distance((VEC2){x,y},(VEC2){i,i2})<r){
				vram[c2vram(i,i2,0)] = 255;
			}
		}
	}
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
	recv(sock,serverdata,1,0);
	players.id = serverdata[0];
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
		IVEC2 c = getCurPos();
		c.x -= 350;
		c.y -= 350;
		VEC2 nc;
		float mc = fmaxf(fabsft(c.x),fabsft(c.y));
		nc.x = (float)c.x / mc;
		nc.y = (float)c.y / mc;
		nc.x *= 127.0f;
		nc.y *= 127.0f;
		serverdata[1] = nc.x;
		serverdata[2] = nc.y;
		send(sock,serverdata,3,0);
		recv(sock,serverdata,1024,0);
		players.c = serverdata[0];
		for(int i = 0;i < players.c;i++){
			players.x[i]   = serverdata[i*9+1];
			players.x[i]  += serverdata[i*9+2]<<8;
			players.y[i]   = serverdata[i*9+3];
			players.y[i]  += serverdata[i*9+4]<<8;
			players.vx[i]  = serverdata[i*9+5];
			players.vy[i]  = serverdata[i*9+6];
			players.ry[i]  = serverdata[i*9+7];
			players.rx[i]  = serverdata[i*9+8];
			if(serverdata[i*9+9] == players.id){
				players.Cid = i;
			}
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
	for(;;){
		for(int i = 0;i < players.c;i++){
			IVEC2 rpos = {players.x[i]-players.x[players.Cid]+350,players.y[i]-players.y[players.Cid]+350};
			drawSphere(rpos.x,rpos.y,20);
			RAY ray = rayCreate(rpos.x,rpos.y,players.rx[i],players.ry[i]);
			while(distance((VEC2){ray.ix,ray.iy},(VEC2){rpos.x,rpos.y})<40.0){
				rayItterate(&ray);
				vram[c2vram(ray.ix,ray.iy,0)] = 255;
				vram[c2vram(ray.ix+1,ray.iy,0)] = 255;
				vram[c2vram(ray.ix-1,ray.iy,0)] = 255;
				vram[c2vram(ray.ix,ray.iy+1,0)] = 255;
				vram[c2vram(ray.ix,ray.iy-1,0)] = 255;
			}
		}
		for(int i = 0;i < 700;i++){
			for(int i2 = 0;i2 < 700;i2++){
				if(350-players.x[players.Cid]-i>0){
					vram[c2vram(i,i2,1)] = 255;
				}
				else if(350-players.y[players.Cid]-i2>0){
					vram[c2vram(i,i2,1)] = 255;
				}
				else if(map[(350+i+players.x[players.Cid]>>5)*32+(350+i2+players.y[players.Cid]>>5)] == 1){
					vram[c2vram(i,i2,2)] = 255;
				}
			}
		}
		StretchDIBits(wndcontext,0,0,700,700,0,0,700,700,vram,&bmi,DIB_RGB_COLORS,SRCCOPY);
		memset(vram,0,700*700*3);
	}
}

void main(){
	vram = HeapAlloc(GetProcessHeap(),8,700*700*3);
	map  = HeapAlloc(GetProcessHeap(),8,32*32);
	for(int i = 0;i < 32*32;i++){
		map[i] = 1;
	}
	timeBeginPeriod(1);
	wndclass.hInstance = GetModuleHandle(0);
	RegisterClass(&wndclass);
	window = CreateWindowEx(0,"window","game",0x10080000,0,0,716,739,0,0,wndclass.hInstance,0);
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
