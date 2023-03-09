// header files
#include<windows.h>
#include<stdlib.h>	// for exit()
#include<stdio.h>	// for file I/O functions
#include<strsafe.h> // for StringCbPrintf()
#include"../include/app.h"
#include"../include/LS/ls.h"

// OpenGL headers
#include<GL/gl.h>
#include<GL/glu.h>	// graphics library utility

// OpenGL libraries
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define MAX_ITERATIONS_FRACTAL_TREE 6
#define MAX_SEQUENCE_LENGTH 65536

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpLog = NULL;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;

// animation
BOOL gbRotate = FALSE;
float gAngleZ = 0.0f;
float gDelta = 25.0f;
int gIterations = 0;

// enumerations
enum Alphabet {

	/* variables */
	DRAW_FORWARD = 'F',
	DRAW_LEAF = 'L',

	/* constants */
	PUSH = '[',
	POP = ']',
	TURN_LEFT_DELTA = '+',
	TURN_RIGHT_DELTA = '-',
	PITCH_UP_DELTA = '^',
	PITCH_DOWN_DELTA = '&',
	ROLL_LEFT_DELTA = '\\',
	ROLL_RIGHT_DELTA = '/',
	TURN_AROUND = '|',
	SHRINK_SEGMENT_DIA = '!',
};

// rendering globals
LSystem lSystem;
char drawingInstructions[MAX_SEQUENCE_LENGTH];
size_t sequenceLength = 0;
GLUquadric *quadric = NULL;

// entry-point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function prototypes
	int initialize(void);	// declaring according to the order of use
	void display(void);
	void update(void);
	void uninitialize(void);

	// variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;
	int iRetVal = 0;
	int cxScreen, cyScreen;

	// code
	if (fopen_s(&gpLog, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("fopen_s: failed to open log file"), TEXT("File I/O Error"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gpLog, "fopen_s: log file opened successfully\n");
	}

	// initialization of the wndclass structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// registering the wndclass
	RegisterClassEx(&wndclass);

	// getting the screen size
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);

	// create the window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Kaivalya Deshpande : 3D Tree"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(cxScreen - WIN_WIDTH) / 2,
		(cyScreen - WIN_HEIGHT) / 2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghwnd = hwnd;

	// initialize
	iRetVal = initialize();
	if (iRetVal == -1)
	{
		fprintf(gpLog, "ChoosePixelFormat(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -2)
	{
		fprintf(gpLog, "SetPixelFormat(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -3)
	{
		fprintf(gpLog, "wglCreateContext(): failed\n");
		uninitialize();
	}
	else if (iRetVal == -4)
	{
		fprintf(gpLog, "wglMakeCurrent(): failed\n");
		uninitialize();
	}
	else
	{
		fprintf(gpLog, "created OpenGL context successfully and made it the current context\n");
	}

	// show the window
	ShowWindow(hwnd, iCmdShow);

	// foregrounding and focussing the window
	SetForegroundWindow(hwnd);	// using ghwnd is obviously fine, but by common sense ghwnd is for global use while we have hwnd locally available in WndProc and here
	SetFocus(hwnd);

	// game loop
	while (bDone != TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow)
			{
				// render the scene
				display();

				// update the scene
				update();
			}
		}
	}

	// uninitialize
	uninitialize();

	return (int)msg.wParam;
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function prototypes
	void ToggleFullScreen(void);
	void resize(int, int);

	// variable declarations
	Sequence *seq = NULL;
	size_t size = 0U;

	// code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		fprintf(gpLog, "window in focus\n");
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		fprintf(gpLog, "window out of focus\n");
		break;
	case WM_ERASEBKGND:
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:
			fprintf(gpLog, "destroying after receiving esc\n");
			DestroyWindow(hwnd);
			break;
		default:
			break;
		}
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;

		case 'l':
			gIterations = gIterations + 1;
			if (gIterations > MAX_ITERATIONS_FRACTAL_TREE)
				gIterations = MAX_ITERATIONS_FRACTAL_TREE;

			/**
			 * regenerate the sequence at 1 more iteration than before
			 * (read initialize() for more detail about this code segment)
			 */
			lsGenSequence(&lSystem, gIterations, &seq);
			{
				size = lsSequenceSize() + 1;

				if (size > MAX_SEQUENCE_LENGTH)
				{
					fprintf(gpLog, "sequence length exceeded: %zd\n", size);
					size = MAX_SEQUENCE_LENGTH;
				}

				lsSequenceString(seq, size, drawingInstructions);
				sequenceLength = size;
			}
			lsDestroySequence(&seq);
			break;

		case 'L':
			gIterations = gIterations - 1;
			if (gIterations < 0)
				gIterations = 0;

			/**
			 * regenerate the sequence at 1 less iteration than before
			 * (read initialize() for more detail about this code segment)
			 */
			lsGenSequence(&lSystem, gIterations, &seq);
			{
				size = lsSequenceSize() + 1;

				if (size < 2)
					size = 2;

				lsSequenceString(seq, size, drawingInstructions);
				sequenceLength = size;
			}
			lsDestroySequence(&seq);
			break;

		case 'd':
			gDelta = gDelta + 0.5f;
			if (gDelta >= 360.0f)
				gDelta = 360.0f - gDelta;
			break;

		case 'D':
			gDelta = gDelta - 0.5f;
			if (gDelta < 0)
				gDelta = -gDelta;
			break;

		case 'R':
		case 'r':
			if (!gbRotate)
				gbRotate = TRUE;
			else
				gbRotate = FALSE;
			break;

		default:
			break;
		}
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CLOSE:	// disciplined code: sent as a signal that a window or an application should terminate
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void)
{
	// variable declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

	// code
	wp.length = sizeof(WINDOWPLACEMENT);

	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
			gbFullScreen = TRUE;
			fprintf(gpLog, "fullscreen mode on\n");
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
		fprintf(gpLog, "fullscreen mode off\n");
	}
}

int initialize(void)
{
	// function prototypes
	void resize(int, int);
	void productionRules3DTree(SequenceNode **);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;
	RECT rc;

	GLfloat lightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat lightDiffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat lightSpecular[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat lightPosition[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	GLfloat materialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat materialShininess = 128.0f;

	Sequence *seq = NULL;
	size_t size = 0;

	// code
	// initialize PIXELFORMATDESCRIPTOR
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;	// R
	pfd.cGreenBits = 8;	// G
	pfd.cBlueBits = 8;	// B
	pfd.cAlphaBits = 8;	// A
	pfd.cDepthBits = 32;	// 24 is another option

	// get DC
	ghdc = GetDC(ghwnd);

	// choose pixel format
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
		return -1;

	// set chosen pixel format
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
		return -2;

	// create OpenGL rendering context
	ghrc = wglCreateContext(ghdc);	// calling my first bridging API
	if (ghrc == NULL)
		return -3;

	// make the rendering context the current context
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)	// the second bridging API
		return -4;

	// here starts the OpenGL code
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// changes concerning depth
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// lighting and material
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

	glEnable(GL_LIGHT0);

	// initialize quadric
	quadric = gluNewQuadric();

	// set the rasterizer's polygon mode to fill
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// generate an LSystem; set its production rules function and axiom
	lsGenLSystem(&lSystem, 1, productionRules3DTree);
	lSystem.w[0] = DRAW_FORWARD;  // set the 1 (and only) axiom (in greek=omega, in english=w)

	/**
	 * generate the 1st set of drawing instructions at `iterations'=0
	 * (subsequent ones are generated on keystrokes)
	 * 
	 * This call WILL allocate memory for `seq', and it is the caller's
	 * responsibility to free/release that memory by calling
	 * lsDestroySequence() on `seq' after use
	 */
	lsGenSequence(&lSystem, gIterations, &seq);
	{
		size = lsSequenceSize() + 1;

		if (size > MAX_SEQUENCE_LENGTH)
		{
			fprintf(gpLog, "sequence length exceeded: %zd\n", size);
			size = MAX_SEQUENCE_LENGTH;
		}

		/**
		 * get a semantic copy of `seq' (i.e. the string generated
		 * using production rules `iterations' number of times on the
		 * axiom specified at the time the LSystem was created)
		 * 
		 * For most purposes, a semantic copy is what one
		 * would want to use in their application
		 */
		lsSequenceString(seq, size, drawingInstructions);
		sequenceLength = size;
	}
	lsDestroySequence(&seq);  // since `drawingInstructions' now holds a useful copy of `seq', free `seq'
	seq = NULL;

	// warm-up resize call
	GetClientRect(ghwnd, &rc);
	resize(rc.right - rc.left, rc.bottom - rc.top);

	return 0;
}

void resize(int width, int height)
{
	// code
	if (height == 0)
		height = 1;	// to prevent a divide by zero when calculating the width/height ratio

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION); // show a projection
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	// local variables
	float branchBaseRadius = 0.08f;
	float branchTopRadius = 0.03f;
	float branchHeight = 0.5f;
	static TCHAR newTitle[255];

	// code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		0.0f, 0.0f, 7.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	);

	glRotatef(gAngleZ, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, -1.0f, 0.0f);
	for (int i = 0; i < sequenceLength; i++)
	{
		switch (drawingInstructions[i])
		{
		case DRAW_FORWARD:
			glEnable(GL_LIGHTING);
			{
				glPushMatrix();
				{
					glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
					gluCylinder(quadric, branchBaseRadius, branchTopRadius, branchHeight, 10, 10);
				}
				glPopMatrix();
			}
			glDisable(GL_LIGHTING);

			glTranslatef(0.0f, branchHeight, 0.0f);
			break;

		case DRAW_LEAF:
			glPushMatrix();
			{
				glColor3f(0.0f, 1.0f, 0.0f);
				glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
				gluSphere(quadric, 0.05f, 5, 5);
			}
			glPopMatrix();
			break;
		
		case PUSH:
			glPushMatrix();
			break;

		case POP:
			glPopMatrix();
			break;

		case TURN_LEFT_DELTA:
			glRotatef(gDelta, 0.0f, 1.0f, 0.0f);
			break;

		case TURN_RIGHT_DELTA:
			glRotatef(-gDelta, 0.0f, 1.0f, 0.0f);
			break;

		case PITCH_UP_DELTA:
			glRotatef(gDelta, 1.0f, 0.0f, 0.0f);
			break;

		case PITCH_DOWN_DELTA:
			glRotatef(-gDelta, 1.0f, 0.0f, 0.0f);
			break;

		case ROLL_LEFT_DELTA:
			glRotatef(gDelta, 0.0f, 0.0f, 1.0f);
			break;

		case ROLL_RIGHT_DELTA:
			glRotatef(-gDelta, 0.0f, 0.0f, 1.0f);
			break;

		case SHRINK_SEGMENT_DIA:
			branchBaseRadius = branchTopRadius;
			branchTopRadius = branchTopRadius - 0.00001f;
			break;

		default:
			break;
		}
	}

	// update window title
	StringCbPrintf(newTitle,
		sizeof(newTitle),
		TEXT("Kaivalya Deshpande: 3D Tree | iterations = %d | delta = %.2f | rotation = %s"),
		gIterations,
		gDelta,
		gbRotate ? TEXT("ON") : TEXT("OFF")
	);
	SetWindowText(ghwnd, newTitle);

	SwapBuffers(ghdc);
}

void update(void)
{
	// code
	if (gbRotate)
	{
		gAngleZ = gAngleZ + 0.5f;
		if (gAngleZ >= 360.0f)
			gAngleZ = gAngleZ - 360.0f;
	}
}

void uninitialize(void)
{
	// function prototypes
	void ToggleFullScreen(void);

	// code
	if (gbFullScreen)
	{
		ToggleFullScreen();
	}

	if (lSystem.active)
	{
		lsDestroyLSystem(&lSystem);
	}

	if (quadric)
	{
		gluDeleteQuadric(quadric);
		quadric = NULL;
	}

	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);	// if unitialize() was not called from WM_DESTROY
		ghwnd = NULL;
	}

	if (gpLog)
	{
		fprintf(gpLog, "fclose: closing log file\n");
		fclose(gpLog);
		gpLog = NULL;
	}
}

void productionRules3DTree(SequenceNode **ppNode)
{
	// code
	switch (lsSequenceNodeSymbol(*ppNode))
	{
	case DRAW_FORWARD:

		lsReplaceSymbol(ppNode, SHRINK_SEGMENT_DIA);
		lsAddSymbol(ppNode, DRAW_FORWARD);

		lsAddSymbol(ppNode, PUSH);
		{	
			lsAddSymbol(ppNode, TURN_LEFT_DELTA);
			lsAddSymbol(ppNode, PITCH_DOWN_DELTA);
			lsAddSymbol(ppNode, ROLL_LEFT_DELTA);
			// lsAddSymbol(ppNode, PITCH_DOWN_DELTA);
			lsAddSymbol(ppNode, TURN_LEFT_DELTA);

			lsAddSymbol(ppNode, DRAW_FORWARD);
			lsAddSymbol(ppNode, DRAW_LEAF);
		}
		lsAddSymbol(ppNode, POP);

		lsAddSymbol(ppNode, PUSH);
		{
			lsAddSymbol(ppNode, TURN_LEFT_DELTA);
			lsAddSymbol(ppNode, PITCH_DOWN_DELTA);
			lsAddSymbol(ppNode, ROLL_RIGHT_DELTA);
			lsAddSymbol(ppNode, PITCH_UP_DELTA);
			// lsAddSymbol(ppNode, TURN_LEFT_DELTA);

			lsAddSymbol(ppNode, DRAW_FORWARD);
			lsAddSymbol(ppNode, DRAW_LEAF);
		}
		lsAddSymbol(ppNode, POP);

		lsAddSymbol(ppNode, PUSH);
		{
			lsAddSymbol(ppNode, PITCH_UP_DELTA);

			lsAddSymbol(ppNode, DRAW_FORWARD);
			lsAddSymbol(ppNode, DRAW_LEAF);
		}
		lsAddSymbol(ppNode, POP);
		break;

	default:
		break;
	}
}
