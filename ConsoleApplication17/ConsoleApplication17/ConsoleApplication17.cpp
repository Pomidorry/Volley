#define _USE_MATH_DEFINES
#include <windows.h>
#include <gl/gl.h>
#include<mutex>
#include<cmath>
#include<string>
#include"stb_easy_font.h"
#pragma comment(lib, "opengl32.lib")
using namespace std;
float k;
float g=0.002;
float gP = 0.004;
float netH = -0.2;
int count1 = 0;
int count2 = 0;
BOOL isCross(float x1, float y1, float r, float x2, float y2) {
	return pow(x1 - x2, 2) + pow(y1 - y2, 2) < r * r;
}

void DrawCircle(int cnt) {
	float x, y;
	float da = M_PI*2.0/cnt;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0, 0);
	for (int i = 0; i <= cnt; i++) {
		x = sin(da * i);
		y = cos(da * i);
		glVertex2f(x, y);
	}
	glEnd();
}
void print_string(float x, float y, char *text, float r, float g, float b)
{
	static char buffer[99999];
	int num_quads;

	num_quads = stb_easy_font_print(x, y, text, NULL, buffer, sizeof(buffer));

	glColor3f(r, g, b);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 16, buffer);
	glDrawArrays(GL_QUADS, 0, num_quads * 4);
	glDisableClientState(GL_VERTEX_ARRAY);
}
struct TBall {
	float x, y, dx, dy, r;
};
TBall ball;
TBall player[2];
TBall sun;
void TBall_Init(TBall* obj, float x1, float y1, float dx1, float dy1, float r1) {
	obj->x = x1;
	obj->y = y1;
	obj->dx = dx1;
	obj->dy = dy1;
	obj->r = r1;
}
void G_Init() {
	TBall_Init(&ball, 0.25, 0.5, 0, 0, 0.2);
	TBall_Init(player+0, 1, -0.5, 0, 0, 0.2);
	TBall_Init(player+1, -1,-0.5 , 0, 0, 0.2);
}
void TBall_Show(TBall obj) {
	glPushMatrix();
	glTranslatef(obj.x, obj.y, 0);
	glScalef(obj.r, obj.r, 1);
	DrawCircle(30);
	glPopMatrix();
}
void Mirror(TBall *obj, float x, float y, float speed){
	float objVec = atan2(obj->dx, obj->dy);
	float crossVec = atan2(obj->x - x, obj->y - y);
	float resVec =speed==0 ? M_PI - objVec + crossVec * 2: crossVec;
	speed = speed == 0? sqrt(pow(obj->dx, 2) + pow(obj->dy, 2)):speed;
	obj->dx = sin(resVec) * speed;
	obj->dy = cos(resVec) * speed;
}
void Reflect(float* da, float* a, BOOL cond, float wall) {
	if (!cond) return;
	*da *= -0.85;
	*a = wall;
}
void TBall_Move(TBall* obj) {
	obj->x += obj->dx;
	obj->y += obj->dy;
	Reflect(&obj->dy, &obj->y, (obj->y < obj->r - 1), obj->r - 1);
	Reflect(&obj->dy, &obj->y, (obj->y >1- obj->r), 1-obj->r);
	obj->dy -= g;
	Reflect(&obj->dx, &obj->x, (obj->x < obj->r - k), obj->r - k);
	Reflect(&obj->dx, &obj->x, (obj->x >k- obj->r), k-obj->r);
	if (obj->y < netH) {
		if (obj->x > 0) {
			Reflect(&obj->dx, &obj->x, (obj->x < +obj->r), +obj->r);
		}
		else {
			Reflect(&obj->dx, &obj->x, (obj->x > -obj->r), -obj->r);
		}
	}
	else {
		if (isCross(obj->x, obj->y, obj->r, 0, netH)) {
			Mirror(obj, 0, netH, 0);
		}
	}
}
void Counter(TBall*obj1, float w1, float w2, float W1, float W2) {
	BOOL c1 = false;
	BOOL c2 = false;

	if (obj1->x - obj1->r > W1&& obj1->x + obj1->r < W2&& obj1->y-obj1->r==-1&count1<=5) {
		c1 = true;
		c2 = false;
		++count1;
		TBall_Init(&ball, -0.25, 0.5, 0, 0, 0.2);
	}
	else if (obj1->x - obj1->r > w1 && obj1->x + obj1->r < w2&&obj1->y - obj1->r == -1&&count2<=5) {
		c2 = true;
		c1 = false;
		++count2;
		TBall_Init(&ball, 0.25, 0.5, 0, 0, 0.2);
	}
	else {
		c2 = false;
		c1 = false;

	}
	

}
void PlayerMove(TBall* obj, char left, char right, char jump, float wl1, float wl2) {
	static float speed = 0.05;
	if (GetKeyState(left) < 0) obj->x -= speed;
	else if (GetKeyState(right) < 0) obj->x += speed;

	std::once_flag flag1, flag2;
	if (obj->x - obj->r < wl1)obj->x = wl1 + obj->r;
	if (obj->x + obj->r > wl2)obj->x = wl2 - obj->r;
	if ((GetKeyState(jump) < 0) && (obj->y < -0.99 + obj->r)) 
		obj->dy = speed * 1.4;
	obj->y += obj->dy;
	obj->dy -= gP;
	
	if (obj->y - obj->r < -1) {
		obj->y = -1 + obj->r;
		obj->dy = 0;
	}
	if (isCross(obj->x, obj->y, obj->r + ball.r, ball.x, ball.y)) {
		Mirror(&ball, obj->x, obj->y, 0.07);
		ball.dy += 0.01;
	}
}
void Quad(float x, float y, float dx, float dy) {
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y);
	glVertex2f(x+dx, y);
	glVertex2f(x+dx, y+dy);
	glVertex2f(x, y+dy);
	glEnd();
}
void House(float x, float y, float size) {
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);
	glColor3f(1, 0, 0);
	Quad(-0.5, -0.5, 1, 0.5);
	glColor3f(1, 1, 0);
	Quad(0.2, 0, 0.2, 0.5);
	Quad(-0.1, -0.3, 0.2, 0.2);
	glBegin(GL_TRIANGLES);
	glVertex2f(-0.6, 0);
	glVertex2f(0.6, 0);
	glVertex2f(0, 0.5);
	glEnd();
	glPopMatrix();
}
void Game_Show() {
	Counter(&ball, -k, 0, 0, k);
	glColor3f(0.83, 0.81, 0.67);
	Quad(-k, -1, k * 2, 1);
	glColor3f(0.21, 0.67, 0.88);
	Quad(-k, 0, k * 2, 1);
	glColor3f(0.67, 0.85, 1);
	Quad(-k, 0.2, k * 2, 1);
	glColor3f(0, 0, 1);
	glColor3f(1, 1, 1);
	Quad(0.5, 0.65, 0.5, 0.25);
	Quad(-0.75, 0.65, 0.5, 0.25);
	Quad(-0.5, 0.25, 0.5, 0.25);
	Quad(-1.5, 0.25, 0.5, 0.25);
	House(-0.5, -0.5, 0.4);
	House(0.5, -0.5, 0.4);
	glColor3f(1, 1, 0.5);
	TBall_Show(ball);
	glColor3f(1, 0, 0);
	TBall_Show(player[0]);
	glColor3f(0, 1, 0);
	TBall_Show(player[1]);
	glColor3f(0, 0, 0);
	glLineWidth(5);
	glBegin(GL_LINES);
	glVertex2f(0, netH);
	glVertex2f(0, -1);
	glEnd();
	glPushMatrix();
	glScalef(0.015, -0.015, 1);
	char a1[] = " ";
	char c1 = '0' + count1;
	a1[0] =c1;
	print_string(-100,-50, a1, 1, 1, 1);
	char a2[] = " ";
	char c2 = '0' + count2;
	a2[0] = c2;
	print_string(100, -50, a2, 1, 1, 1);
	glPopMatrix();
	static float alpha = 0;
	alpha -= 2;
	glPushMatrix();
	glTranslatef(0, -0.5, 0);
	glRotatef(alpha, 0, 0, 1);
	glTranslatef(1.7, 0, 0);
	glColor3f(1, 1, 0);
	Quad(0.1, 0.1, 0.2, 0.2);
	glPopMatrix();
}
void GameOver() {
	if (count1 == 5) {
		glPushMatrix();
		glScalef(0.015, -0.015, 1);
		char a1[] = "Congratulation! Player1 is a winner";
		print_string(-100, -50, a1, 1, 0, 1);
		glPopMatrix();
		g = 0;
	}
	if (count2 == 5) {
		glPushMatrix();
		glScalef(0.015, -0.015, 1);
		char a1[] = "Congratulation! Player2 is a winner";
		print_string(-100, -50, a1, 1, 0, 1);
		glPopMatrix();
		g = 0;
	}
}
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	BOOL bQuit = FALSE;
	float theta = 0.0f;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"GLSample";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


	if (!RegisterClassEx(&wcex))
		return 0;

	hwnd = CreateWindowEx(0, L"GLSample", L"Easy volley",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1000,
		600,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);

	EnableOpenGL(hwnd, &hDC, &hRC);
	k = (float)1000 / 600;
	glScalef(1/k, 1, 1);
	G_Init();
	while (!bQuit)
	{
	
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
		
			if (msg.message == WM_QUIT)
			{
				bQuit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
		
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			PlayerMove(player + 1, 'A', 'D', 'W', -k, 0);
			PlayerMove(player + 0, VK_LEFT, VK_RIGHT, VK_UP, 0, k);
			TBall_Move(&ball);
			Game_Show();
			if (count1 >= 5 || count2 >= 5) {
				GameOver();
			}
			SwapBuffers(hDC);
			theta += 1.0f;
			Sleep(1);
		}
	}

	
	DisableOpenGL(hwnd, hDC, hRC);

	DestroyWindow(hwnd);

	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;



	case WM_DESTROY:
		return 0;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
	}
	break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	int iFormat;

	*hDC = GetDC(hwnd);

	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	iFormat = ChoosePixelFormat(*hDC, &pfd);

	SetPixelFormat(*hDC, iFormat, &pfd);

	*hRC = wglCreateContext(*hDC);

	wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, hDC);
}


