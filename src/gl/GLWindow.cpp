/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "GLWindow.h"
#include "GLResources.h"
#include "Debug.h"

#ifndef _WIN32
static int attrListSgl[] =                                             
{                                                                      
    GLX_RGBA, GLX_RED_SIZE, 4,                                         
    GLX_GREEN_SIZE, 4,                                                 
    GLX_BLUE_SIZE, 4,                                                  
    GLX_DEPTH_SIZE, 16,                                                
    None                                                               
};                                                                     
 
/* attributes for a double buffered visual in RGBA format with at least
 * 4 bits per color and a 16 bit depth buffer */                       
static int attrListDbl[] =                                             
{                                                                      
    GLX_RGBA, GLX_DOUBLEBUFFER,                                        
    GLX_RED_SIZE, 4,                                                   
    GLX_GREEN_SIZE, 4,                                                 
    GLX_BLUE_SIZE, 4,                                                  
    GLX_DEPTH_SIZE, 16,                                                
    None                                                               
};   
#endif

GLWindow::GLWindow(const char * title, int width, int height, int bits, bool fullscreen, WNDPROC wndproc)
{
#ifdef _WIN32
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
	  DEVMODE dmScreenSettings;								
	  memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
	  dmScreenSettings.dmSize=sizeof(dmScreenSettings);
	  dmScreenSettings.dmPelsWidth	= width;
	  dmScreenSettings.dmPelsHeight	= height;
	  dmScreenSettings.dmBitsPerPel	= bits;
	  dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

    if(fullscreen) {
		  ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
		  dwExStyle = WS_EX_APPWINDOW;
		  dwStyle = WS_POPUP;
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
#else            
  XVisualInfo *vi;
  Colormap cmap;  
  int i, dpyWidth, dpyHeight;
  int glxMajor, glxMinor, vmMajor, vmMinor;
  XF86VidModeModeInfo **modes;             
  int modeNum, bestMode;                   
  Atom wmDelete;                           
  Window winDummy;                         
  unsigned int borderDummy;                

  mFullscreen = fullscreen;

  /* set best mode to current */
  bestMode = 0;                 
  /* get a connection */        
  mDisplay = XOpenDisplay(0);    
  mScreen = DefaultScreen(mDisplay);
  XF86VidModeQueryVersion(mDisplay, &vmMajor, &vmMinor);
  XF86VidModeGetAllModeLines(mDisplay, mScreen, &modeNum, &modes);       
  /* save desktop-resolution before switching modes */                 
  mDeskMode = *modes[0];
  /* look for mode with requested resolution */                        
  for (i = 0; i < modeNum; i++)                                        
  {                                                                    
      if ((modes[i]->hdisplay == width) && (modes[i]->vdisplay == height))
          bestMode = i;                                                   
  }                                                                       
  /* get an appropriate visual */                                         
  vi = glXChooseVisual(mDisplay, mScreen, attrListDbl);                     
  if (vi == NULL)                                                         
  {                                                                       
      vi = glXChooseVisual(mDisplay, mScreen, attrListSgl);                 
      mDoubleBuffered = False;                                             
  }                                                                                   
  else                                                                                
  {                                                                                   
      mDoubleBuffered = True;                                                                                        
  }                                                                                   
  glXQueryVersion(mDisplay, &glxMajor, &glxMinor);                                     
  Debug::Log("GLX-Version %d.%d\n", glxMajor, glxMinor); 
  /* create a GLX context */                                                          
  mCtx = glXCreateContext(mDisplay, vi, 0, GL_TRUE);                                
  /* create a color map */                                                            
  cmap = XCreateColormap(mDisplay, RootWindow(mDisplay, vi->screen),                    
      vi->visual, AllocNone);                                                         
  mWinAttr.colormap = cmap;                                                            
  mWinAttr.border_pixel = 0;                                                           

  if(fullscreen)
  {              
      /* switch to fullscreen */
      XF86VidModeSwitchToMode(mDisplay, mScreen, modes[bestMode]);
      XF86VidModeSetViewPort(mDisplay, mScreen, 0, 0);            
      dpyWidth = modes[bestMode]->hdisplay;                     
      dpyHeight = modes[bestMode]->vdisplay;                          
      XFree(modes);                                             

      /* set window attributes */
      mWinAttr.override_redirect = True;
      mWinAttr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
          StructureNotifyMask;                                            
      mWindow = XCreateWindow(mDisplay, RootWindow(mDisplay, vi->screen),    
          0, 0, dpyWidth, dpyHeight, 0, vi->depth, InputOutput, vi->visual,
          CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,   
          &mWinAttr);                                                       
      XWarpPointer(mDisplay, None, mWindow, 0, 0, 0, 0, 0, 0);               
              XMapRaised(mDisplay, mWindow);                                 
      XGrabKeyboard(mDisplay, mWindow, True, GrabModeAsync,                  
          GrabModeAsync, CurrentTime);                                     
      XGrabPointer(mDisplay, mWindow, True, ButtonPressMask,                 
          GrabModeAsync, GrabModeAsync, mWindow, None, CurrentTime);        
  }                                                                        
  else                                                                     
  {                                                                        
      /* create a window in window mode*/                                  
      mWinAttr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | 
          StructureNotifyMask;                                             
      mWindow = XCreateWindow(mDisplay, RootWindow(mDisplay, vi->screen),        
          0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,      
          CWBorderPixel | CWColormap | CWEventMask, &mWinAttr);             
      /* only set window title and handle wm_delete_events if in windowed mode */
      wmDelete = XInternAtom(mDisplay, "WM_DELETE_WINDOW", True);                 
      XSetWMProtocols(mDisplay, mWindow, &wmDelete, 1);                            
      XSetStandardProperties(mDisplay, mWindow, title,                             
          title, None, NULL, 0, NULL);                                           
      XMapRaised(mDisplay, mWindow);                                               
  }                                                                              
  /* connect the glx-context to the window */                                    
  glXMakeCurrent(mDisplay, mWindow, mCtx);                                                                                
  initOpenGL();

  mInitialized = true;    
#endif
}

GLWindow::~GLWindow()
{
  free();
}

bool GLWindow::initOpenGL()							
{
  bool result = true;

#ifdef _WIN32
  if(glewInit()) {
    result = false;
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
#ifdef _WIN32
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
#else
  if(mCtx)    
  {
      if(!glXMakeCurrent(mDisplay, None, NULL))
      {                                        
          printf("Could not release drawing context.\n");
      }                                                  
      /* destroy the context */                          
      glXDestroyContext(mDisplay, mCtx);               
      mCtx = NULL;                                    
  }                                                      
  /* switch back to original desktop resolution if we were in fullscreen */
  if( mFullscreen )                                                         
  {                                                                        
      XF86VidModeSwitchToMode(mDisplay, mScreen, &mDeskMode);              
      XF86VidModeSetViewPort(mDisplay, mScreen, 0, 0);                       
  }                                                                        
  XCloseDisplay(mDisplay);                                                  
#endif
}