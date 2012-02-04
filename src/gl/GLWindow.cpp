/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "GLWindow.h"
#include "GLDefines.h"
#include "Debug.h"

GLWindow::GLWindow(const char * title, int width, int height, int bits, bool fullscreen, WNDPROC wndproc)
{
	GLuint PixelFormat;			            // Holds The Results After Searching For A Match
	WNDCLASS wc;						            // Windows Class Structure
	DWORD dwExStyle;				            // Window Extended Style
	DWORD dwStyle;				              // Window Style
	RECT WindowRect;				            // Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long) 0;			    // Set Left Value To 0
	WindowRect.right =(long) width;		  // Set Right Value To Requested Width
	WindowRect.top = (long) 0;				  // Set Top Value To 0
	WindowRect.bottom = (long) height;  // Set Bottom Value To Requested Height
  bool result = true;
  mInitialized = false;

  if(result) {
	  mhInstance = GetModuleHandle(NULL);				
	  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	
	  wc.lpfnWndProc = (WNDPROC) wndproc;
	  wc.cbClsExtra = 0;									
	  wc.cbWndExtra = 0;									
    wc.hInstance = mhInstance;					
	  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);		
	  wc.hCursor = LoadCursor(NULL, IDC_ARROW);	
	  wc.hbrBackground = NULL;							
	  wc.lpszMenuName = NULL;							
	  wc.lpszClassName = "OpenGL";
  }
  
  if(result) {
	  if(!RegisterClass(&wc)) {
		  result = false;
	  }
  }

  if(result) {
	  if(fullscreen) {
		  DEVMODE dmScreenSettings;								
		  memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		  dmScreenSettings.dmSize=sizeof(dmScreenSettings);
		  dmScreenSettings.dmPelsWidth	= width;
		  dmScreenSettings.dmPelsHeight	= height;
		  dmScreenSettings.dmBitsPerPel	= bits;
		  dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		  ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN);
		  dwExStyle = WS_EX_APPWINDOW;
		  dwStyle = WS_POPUP;					
		  ShowCursor(FALSE);	
      gScreen = GLvector2f(0.0f, 0.0f);
	  }	else {
		  dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		  dwStyle = WS_OVERLAPPEDWINDOW;					
	  }

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);	
  }

  if(result) {
    dwStyle |=	WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    mhWnd = CreateWindowEx(dwExStyle, "OpenGL", title, dwStyle, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top,	NULL,	NULL, mhInstance, NULL);

    if(!mhWnd) {
		  free();
		  result = false;		
	  }
  }
	
  if(result) {
    mhDC = GetDC(mhWnd);
	  if(!mhDC)	{
		  free();
		  result = false;	
	  }
  }

  static PIXELFORMATDESCRIPTOR pfd =	
	{
		sizeof(PIXELFORMATDESCRIPTOR),		
		1,										
		PFD_DRAW_TO_WINDOW |	
		PFD_SUPPORT_OPENGL |	
		PFD_DOUBLEBUFFER,			
		PFD_TYPE_RGBA,				
		bits,									
		0, 0, 0, 0, 0, 0,			
		0,										
		0,										
		0,										
		0, 0, 0, 0,						
		16,										
		0,										
		0,										
		PFD_MAIN_PLANE,				
		0,										
		0, 0, 0								
	};

  if(result) {
    PixelFormat = ChoosePixelFormat(mhDC, &pfd);
	  if(!PixelFormat) {
		  free();
		  result = false;	
	  }
  }

  if(result) {
	  if(!SetPixelFormat(mhDC, PixelFormat, &pfd))	{
		  free();
		  result = false;	
	  }
  }

  if(result) {
    mhRC = wglCreateContext(mhDC);
	  if(!mhRC) {
		  free();
		  result = false;	
	  }
  }

  if(result) {
	  if(!wglMakeCurrent(mhDC, mhRC))	{
		  free();
		  result = false;							
	  }
  }

  if(result) {
    ShowCursor(FALSE);
	  ShowWindow(mhWnd, SW_SHOW);				
	  SetForegroundWindow(mhWnd);			
	  SetFocus(mhWnd);									
  }

  if(result) {
	  if(!initOpenGL()) {
		  free();
		  result = false;	
	  }
  }

  mInitialized = result;
}

GLWindow::~GLWindow()
{
  glDeleteProgram(mGaussianShader);
  free();
}

bool GLWindow::initOpenGL()							
{
  bool result = true;

  if(glewInit()) {
    result = false;
  }

// shader
#if 0
  GLuint fragmentShader;
  static const char * fragmentShaderSource =
    "#version 120\n"
    "uniform sampler2D tex;"
    "uniform vec2 u_direction;"
    ""
    "const vec2 gaussFilter[7] = const vec2[7]("
    "  -3.0,	0.015625,"
    "  -2.0,	0.09375 ,"
    "  -1.0,	0.234375,"
    "   0.0,	0.3125  ,"
    "   1.0,	0.234375,"
    "   2.0,	0.09375 ,"
    "   3.0,	0.015625"
    ");"
    ""
    "void main()"
    "{"
    "  vec4 color;"
    "  color = vec4(0.0, 0.0, 0.0, 0.0);"
    "  for(int i = 0; i < 7; i++) {"
    "    color += texture2D( tex, vec2( gl_TexCoord[0].x + gaussFilter[i].x * u_direction.x,"
    "                                   gl_TexCoord[0].y + gaussFilter[i].x * u_direction.y ) ) * gaussFilter[i].y;"
    "  }"
    ""
    "  gl_FragColor = color;"
    "}"
    "";

  if(result) {
    GLint length = strlen(fragmentShaderSource);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const GLcharARB **) &fragmentShaderSource, &length);
    glCompileShader(fragmentShader);

    GLint compiled;
    glGetObjectParameterivARB(fragmentShader, GL_COMPILE_STATUS, &compiled);
    if(!compiled) {
      GLchar* log = new GLchar[2048];
      glGetShaderInfoLog(fragmentShader, 2048, NULL, log);
      Debug::Log("Could not compile:\n %s", log);
      delete[] log;

      glDeleteShader(fragmentShader);
      result = false;
    }
  }

  if(result) {
    mGaussianShader = glCreateProgram();
    glAttachShader(mGaussianShader, fragmentShader);
    glLinkProgram(mGaussianShader);    

    GLint linked;
    glGetProgramiv(mGaussianShader, GL_LINK_STATUS, &linked);
    if(!linked) {
      Debug::Log("Could not link, saving log.");
      GLchar* log = new GLchar[65636];
      glGetProgramInfoLog(mGaussianShader, 65636, NULL, log);
      Debug::LogToFile("linkerlog.txt", "Could not link:\n %s", log);
      result = false;
      delete[] log;
    } else {
      mGaussDirLoc = glGetUniformLocation(mGaussianShader, "u_direction");
      GL_ASSERT();
    }
    glDeleteShader(fragmentShader);
  }
#endif

  if(result) {
    glMatrixMode(GL_PROJECTION);		
    glLoadIdentity();									
    glViewport(0, 0, GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT);
    glOrtho(0, GL_SCREEN_IWIDTH, 0, GL_SCREEN_IHEIGHT, 0, 128);

    glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
    glLoadIdentity();									// Reset The Modelview Matrix

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);							
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	
    glClearDepth(1.0f);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
  }

  return result;										
}

void GLWindow::free()
{
  if(mhRC) {									
		if(!wglMakeCurrent(NULL, NULL)) {
		}
		if (!wglDeleteContext(mhRC))	{
		}
    mhRC = 0;	
	}

	if(mhDC && !ReleaseDC(mhWnd, mhDC)) {
		mhDC = 0;
	}

	if(mhWnd && !DestroyWindow(mhWnd)) {
		mhWnd = 0;
	}

	if(!UnregisterClass("OpenGL", mhInstance)) {
		mhInstance = 0;
	}
}