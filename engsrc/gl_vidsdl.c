#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>


#include <dlfcn.h>

#include "SDL.h"
#include "quakedef.h"

#if defined(__linux__) || defined(__ANDROID__)
#define HAVE_OPENGLES
#endif

#ifdef HAVE_OPENGLES
#include "SDL_opengles.h"
#else
#include "SDL_opengl.h"
#endif
SDL_Window* window;
SDL_GLContext context = NULL;
SDL_DisplayMode mode;
SDL_Event event;
static int depth;
viddef_t    vid;

#define WARP_WIDTH              320*2
#define WARP_HEIGHT             240*2
unsigned short	d_8to16table[256];
unsigned		d_8to24table[256];
unsigned char	d_15to8table[65536];
cvar_t	vid_mode = {"vid_mode","0",false};
static qboolean        mouse_avail;
static qboolean        mouse_active;
static int mouse_oldbuttonstate = 0;
static float   mouse_x, mouse_y;
static int scr_width, scr_height;
/*-----------------------------------------------------------------------*/

//int		texture_mode = GL_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_LINEAR;
int		texture_mode = GL_LINEAR;
//int		texture_mode = GL_LINEAR_MIPMAP_NEAREST;
//int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;

int		texture_extension_number = 1;

float		gldepthmin, gldepthmax;

cvar_t	gl_ztrick = {"gl_ztrick","1"};

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;
static float vid_gamma = 1.0;
qboolean is8bit = false;
qboolean isPermedia = false;
qboolean gl_mtexable = false;

void (*qglColorTableEXT) (int, int, int, int, int, const void*);
void (*qgl3DfxSetPaletteEXT) (GLuint *);
/*
================
Sys_SendKeyEvents
================
*/

void Sys_SendKeyEvents(void)
{
    //SDL_Event event;
    int sym, state;
    int modstate;

    while (SDL_PollEvent(&event))
    {
        switch (event.type) {

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                sym = event.key.keysym.sym;
                state = event.key.state;
                modstate = SDL_GetModState();
                switch(sym)
                {
                   case SDLK_DELETE: sym = K_DEL; break;
                   case SDLK_BACKSPACE: sym = K_BACKSPACE; break;
                   case SDLK_F1: sym = K_F1; break;
                   case SDLK_F2: sym = K_F2; break;
                   case SDLK_F3: sym = K_F3; break;
                   case SDLK_F4: sym = K_F4; break;
                   case SDLK_F5: sym = K_F5; break;
                   case SDLK_F6: sym = K_F6; break;
                   case SDLK_F7: sym = K_F7; break;
                   case SDLK_F8: sym = K_F8; break;
                   case SDLK_F9: sym = K_F9; break;
                   case SDLK_F10: sym = K_F10; break;
                   case SDLK_F11: sym = K_F11; break;
                   case SDLK_F12: sym = K_F12; break;
                   case SDLK_ESCAPE: sym = K_ESCAPE; break;
                   case SDLK_PAUSE: sym = K_PAUSE; break;
                   case SDLK_UP: sym = K_UPARROW; break;
                   case SDLK_DOWN: sym = K_DOWNARROW; break;
                   case SDLK_RIGHT: sym = K_RIGHTARROW; break;
                   case SDLK_LEFT: sym = K_LEFTARROW; break;
                   case SDLK_INSERT: sym = K_INS; break;
                   case SDLK_HOME: sym = K_HOME; break;
                   case SDLK_END: sym = K_END; break;
                   case SDLK_PAGEUP: sym = K_PGUP; break;
                   case SDLK_PAGEDOWN: sym = K_PGDN; break;
                   case SDLK_RSHIFT:
                   case SDLK_LSHIFT: sym = K_SHIFT; break;
                   case SDLK_RCTRL:
                   case SDLK_LCTRL: sym = K_CTRL; break;
                   case SDLK_RALT:
                   case SDLK_LALT: sym = K_ALT; break;
                   case SDLK_KP_0: 
                       if(modstate & KMOD_NUM) sym = K_INS; 
                       else sym = SDLK_0;
                       break;
                   case SDLK_KP_1:
                       if(modstate & KMOD_NUM) sym = K_END;
                       else sym = SDLK_1;
                       break;
                   case SDLK_KP_2:
                       if(modstate & KMOD_NUM) sym = K_DOWNARROW;
                       else sym = SDLK_2;
                       break;
                   case SDLK_KP_3:
                       if(modstate & KMOD_NUM) sym = K_PGDN;
                       else sym = SDLK_3;
                       break;
                   case SDLK_KP_4:
                       if(modstate & KMOD_NUM) sym = K_LEFTARROW;
                       else sym = SDLK_4;
                       break;
                   case SDLK_KP_5: sym = SDLK_5; break;
                   case SDLK_KP_6:
                       if(modstate & KMOD_NUM) sym = K_RIGHTARROW;
                       else sym = SDLK_6;
                       break;
                   case SDLK_KP_7:
                       if(modstate & KMOD_NUM) sym = K_HOME;
                       else sym = SDLK_7;
                       break;
                   case SDLK_KP_8:
                       if(modstate & KMOD_NUM) sym = K_UPARROW;
                       else sym = SDLK_8;
                       break;
                   case SDLK_KP_9:
                       if(modstate & KMOD_NUM) sym = K_PGUP;
                       else sym = SDLK_9;
                       break;
                   case SDLK_KP_PERIOD:
                       if(modstate & KMOD_NUM) sym = K_DEL;
                       else sym = SDLK_PERIOD;
                       break;
                   case SDLK_KP_DIVIDE: sym = SDLK_SLASH; break;
                   case SDLK_KP_MULTIPLY: sym = SDLK_ASTERISK; break;
                   case SDLK_KP_MINUS: sym = SDLK_MINUS; break;
                   case SDLK_KP_PLUS: sym = SDLK_PLUS; break;
                   case SDLK_KP_ENTER: sym = SDLK_RETURN; break;
                   case SDLK_KP_EQUALS: sym = SDLK_EQUALS; break;
                }
                // If we're not directly handled and still above 255
                // just force it to 0
                if(sym > 255) sym = 0;
                Key_Event(sym, state);
                break;

            case SDL_MOUSEMOTION:
                if ( (event.motion.x != (vid.width/2)) ||
                     (event.motion.y != (vid.height/2)) ) {
                    mouse_x = event.motion.xrel*10;
                    mouse_y = event.motion.yrel*10;
                    if ( (event.motion.x < ((vid.width/2)-(vid.width/4))) ||
                         (event.motion.x > ((vid.width/2)+(vid.width/4))) ||
                         (event.motion.y < ((vid.height/2)-(vid.height/4))) ||
                         (event.motion.y > ((vid.height/2)+(vid.height/4))) ) {
                        //SDL_WarpMouse(vid.width/2, vid.height/2);
			SDL_WarpMouseInWindow(window, vid.width/2, vid.height/2);
                    }
                }
                break;

            case SDL_QUIT:
                CL_Disconnect ();
                Host_ShutdownServer(false);        
                Sys_Quit ();
                break;
            default:
                break;
        }
    }
}
void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}




void    VID_Shutdown (void)
{
    SDL_Quit();
}
void signal_handler(int sig)
{
	printf("Received signal %d, exiting...\n", sig);
	Sys_Quit();
	exit(0);
}

void InitSig(void)
{
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTRAP, signal_handler);
	signal(SIGIOT, signal_handler);
	signal(SIGBUS, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGTERM, signal_handler);
}


void VID_ShiftPalette(unsigned char *p)
{
//	VID_SetPalette(p);
}

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned r,g,b;
	unsigned v;
	int     r1,g1,b1;
	int		j,k,l,m;
	unsigned short i;
	unsigned	*table;
	FILE *f;
	char s[255];
	int dist, bestdist;

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] &= 0xffffff;	// 255 is transparent

	for (i=0; i < (1<<15); i++) {
		/* Maps
		000000000000000
		000000000011111 = Red  = 0x1F
		000001111100000 = Blue = 0x03E0
		111110000000000 = Grn  = 0x7C00
		*/
		r = ((i & 0x1F) << 3)+4;
		g = ((i & 0x03E0) >> 2)+4;
		b = ((i & 0x7C00) >> 7)+4;
		pal = (unsigned char *)d_8to24table;
		for (v=0,k=0,bestdist=10000*10000; v<256; v++,pal+=4) {
			r1 = (int)r - (int)pal[0];
			g1 = (int)g - (int)pal[1];
			b1 = (int)b - (int)pal[2];
			dist = (r1*r1)+(g1*g1)+(b1*b1);
			if (dist < bestdist) {
				k=v;
				bestdist = dist;
			}
		}
		d_15to8table[i]=k;
	}
}

void CheckMultiTextureExtensions(void) 
{
  
	void *prjobj;

	if (strstr(gl_extensions, "GL_SGIS_multitexture ") && !COM_CheckParm("-nomtex")) {
		Con_Printf("Found GL_SGIS_multitexture...\n");

		if ((prjobj = dlopen(NULL, RTLD_LAZY)) == NULL) {
			Con_Printf("Unable to open symbol list for main program.\n");
			return;
		}

		qglMTexCoord2fSGIS = (void *) dlsym(prjobj, "glMTexCoord2fSGIS");
		qglSelectTextureSGIS = (void *) dlsym(prjobj, "glSelectTextureSGIS");

		if (qglMTexCoord2fSGIS && qglSelectTextureSGIS) {
			Con_Printf("Multitexture extensions found.\n");
			gl_mtexable = true;
		} else
			Con_Printf("Symbol not found, disabled.\n");

		dlclose(prjobj);
	}
	
}

/*
===============
GL_Init
===============
*/
void GL_Init (void)
{
	gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
	//todo fix after sdlvideo port
	// ("GL_EXTENSIONS: %s\n", gl_extensions);

//	Con_Printf ("%s %s\n", gl_renderer, gl_version);

	CheckMultiTextureExtensions ();

	//glClearColor (1,0,0,0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//glCullFace(GL_FRONT_AND_BACK);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	//glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glShadeModel (GL_FLAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
/*
=================
GL_BeginRendering

=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	extern cvar_t gl_clear;
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	*x = *y = 0;
	*width = scr_width;
	*height = scr_height;

//    if (!wglMakeCurrent( maindc, baseRC ))
//		Sys_Error ("wglMakeCurrent failed");

//	glViewport (*x, *y, *width, *height);
}
void GL_EndRendering (void)
{
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glFlush();
	SDL_GL_SwapWindow( window );
}

qboolean VID_Is8bit(void)
{
	return is8bit;
}

void VID_Init8bitPalette(void) 
{
	// Check for 8bit Extensions and initialize them.
	int i;
	void *prjobj;

	if ((prjobj = dlopen(NULL, RTLD_LAZY)) == NULL) {
		Con_Printf("Unable to open symbol list for main program.\n");
		return;
	}

	if (strstr(gl_extensions, "3DFX_set_global_palette") &&
		(qgl3DfxSetPaletteEXT = dlsym(prjobj, "gl3DfxSetPaletteEXT")) != NULL) {
		GLubyte table[256][4];
		char *oldpal;

		Con_SafePrintf("8-bit GL extensions enabled.\n");
		//glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
		oldpal = (char *) d_8to24table; //d_8to24table3dfx;
		for (i=0;i<256;i++) {
			table[i][2] = *oldpal++;
			table[i][1] = *oldpal++;
			table[i][0] = *oldpal++;
			table[i][3] = 255;
			oldpal++;
		}
		qgl3DfxSetPaletteEXT((GLuint *)table);
		is8bit = true;

	} else if (strstr(gl_extensions, "GL_EXT_shared_texture_palette") &&
		(qglColorTableEXT = dlsym(prjobj, "glColorTableEXT")) != NULL) {
		char thePalette[256*3];
		char *oldPalette, *newPalette;

		Con_SafePrintf("8-bit GL extensions enabled.\n");
		//glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
		oldPalette = (char *) d_8to24table; //d_8to24table3dfx;
		newPalette = thePalette;
		for (i=0;i<256;i++) {
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			oldPalette++;
		}
		qglColorTableEXT(0, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, (void *) thePalette);//GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, (void *) thePalette);
		is8bit = true;
	}
	
	dlclose(prjobj);
}


static void Check_Gamma (unsigned char *pal)
{
	float	f, inf;
	unsigned char	palette[768];
	int		i;

	if ((i = COM_CheckParm("-gamma")) == 0) {
		if ((gl_renderer && strstr(gl_renderer, "Voodoo")) ||
			(gl_vendor && strstr(gl_vendor, "3Dfx")))
			vid_gamma = 1;
		else
			vid_gamma = 0.7; // default to 0.7 on non-3dfx hardware
	} else
		vid_gamma = Q_atof(com_argv[i+1]);

	for (i=0 ; i<768 ; i++)
	{
		f = pow ( (pal[i]+1)/256.0 , vid_gamma );
		inf = f*255 + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		palette[i] = inf;
	}

	memcpy (pal, palette, sizeof(palette));
}
int GetCurrentDisplayMode()
{
      int i;

      // Declare display mode structure to be filled in.
      //SDL_DisplayMode current;

      //SDL_Init(SDL_INIT_VIDEO);

      // Get current display mode of all displays.
      for(i = 0; i < SDL_GetNumVideoDisplays(); ++i){

	      int should_be_zero = SDL_GetCurrentDisplayMode(i, &mode);

	      if(should_be_zero != 0){
		// In case of error...
		SDL_Log("Could not get display mode for video display #%d: %s", i, SDL_GetError());
		SDL_Quit();
	      }
	      else
		  // On success, print the current display mode.
		  //SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz.", i, mode.w, mode.h, mode.refresh_rate);
		  //SDL_GetCurrentDisplayMode(0, &mode);
		  SDL_Log("Display #%d: \n",i);
		  SDL_Log("Current display mode is: \n");
		  SDL_Log("Screen Width: %dx\n",mode.w);
		  SDL_Log("Screen Height: %dpx\n",mode.h);
		  SDL_Log("Screen Refresh rate: %dhz\n",mode.refresh_rate);
		  SDL_Log("Screen bpp: %d\n", SDL_BITSPERPIXEL(mode.format));
		  SDL_Log("\n");
		  SDL_Log("Vendor     : %s\n", glGetString(GL_VENDOR));
		  SDL_Log("Renderer   : %s\n", glGetString(GL_RENDERER));
		  SDL_Log("Version    : %s\n", glGetString(GL_VERSION));
		  SDL_Log("Extensions : %s\n", glGetString(GL_EXTENSIONS));
		  SDL_Log("\n");
	}
	return 0;
}
void VID_Init(unsigned char *palette)
{
	int i;
	char	gldir[MAX_OSPATH];
	int width = WARP_WIDTH; 
	int height = WARP_HEIGHT;
	Uint32 flags;
	qboolean fullscreen = true;

	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&gl_ztrick);
	
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));



	/* Enable standard application logging */
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);    
	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		SDL_Quit();
	}
	else
	{
		GetCurrentDisplayMode();
	// interpret command-line params
	// set vid parameters
		width = mode.w;
		height = mode.h;
		depth = SDL_BITSPERPIXEL(mode.format);
		if ((i = COM_CheckParm("-window")) != 0)
			fullscreen = false;

		if ((i = COM_CheckParm("-width")) != 0)
			width = atoi(com_argv[i+1]);

		if ((i = COM_CheckParm("-height")) != 0)
			height = atoi(com_argv[i+1]);

		if ((i = COM_CheckParm("-conwidth")) != 0)
			vid.conwidth = Q_atoi(com_argv[i+1]);
		else
			vid.conwidth = width;

		vid.conwidth &= 0xfff8; // make it a multiple of eight

		if (vid.conwidth < width)
			vid.conwidth = width;

		// pick a conheight that matches with correct aspect
		vid.conheight = vid.conwidth*3 / 4;

		if ((i = COM_CheckParm("-conheight")) != 0)
			vid.conheight = Q_atoi(com_argv[i+1]);
		if (vid.conheight < height)
			vid.conheight = height;
		vid.width = width;
		vid.height = height;
		
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,            5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,          5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,           5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,          5);
	    
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,          depth);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,         32);

		SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,      5);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,    5);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,     5);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,    5);
	    
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  0);//intel doesnt like this
	    
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);//intel doesnt like this
		
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 1);
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );

		//Create window
		flags = (SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
		if ( fullscreen == true ) flags |= SDL_WINDOW_FULLSCREEN;
		
		window = SDL_CreateWindow( "QSdlGles", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags );
		if( window == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			SDL_Quit();
			//return 1;
		}	    
		else
		{
			//Create context
			context = SDL_GL_CreateContext( window );
			if( context == NULL )
			{
				printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
				SDL_Quit();
				//return 1;
			}
			else
			{
				SDL_Log("\n");
				SDL_Log("Vendor     : %s\n", glGetString(GL_VENDOR));
				SDL_Log("Renderer   : %s\n", glGetString(GL_RENDERER));
				SDL_Log("Version    : %s\n", glGetString(GL_VERSION));
				SDL_Log("Extensions : %s\n", glGetString(GL_EXTENSIONS));
				SDL_Log("\n");	
			}
		}
	}

	//angelo mouse enable disable for debuging , because if it locks you wont be able to get to the debugger
	if ((i = COM_CheckParm("-nomouse")) != 0)
	  mouse_avail = false;
	else
	  mouse_avail = true;
	
	SDL_ShowCursor(SDL_DISABLE);

	SDL_GL_MakeCurrent(window, context);

	scr_width = width;
	scr_height = height;

	if (vid.conheight > height)
		vid.conheight = height;
	if (vid.conwidth > width)
		vid.conwidth = width;
	vid.width = vid.conwidth;
	vid.height = vid.conheight;

	vid.aspect = ((float)vid.height / (float)vid.width);// * (320.0 / 240.0);
	vid.numpages = 2;
	//reenable after debugging, or maybe make a switch?
	//InitSig(); // trap evil signals

	GL_Init();

	sprintf (gldir, "%s/glquake", com_gamedir);
	Sys_mkdir (gldir);

	VID_SetPalette(palette);


	Con_SafePrintf ("Video mode %dx%d initialized.\n", width, height);

	vid.recalc_refdef = 1;				// force a surface cache flush
}






void IN_Init (void)
{
    if ( COM_CheckParm ("-nomouse") )
        return;
    mouse_x = mouse_y = 0.0;
    mouse_avail = 1;
}

void IN_Shutdown (void)
{
    mouse_avail = 0;
}

void IN_Commands (void)
{
    int i;
    int mouse_buttonstate;
   
    if (!mouse_avail) return;
   
    i = SDL_GetMouseState(NULL, NULL);
    /* Quake swaps the second and third buttons */
    mouse_buttonstate = (i & ~0x06) | ((i & 0x02)<<1) | ((i & 0x04)>>1);
    for (i=0 ; i<3 ; i++) {
        if ( (mouse_buttonstate & (1<<i)) && !(mouse_oldbuttonstate & (1<<i)) )
            Key_Event (K_MOUSE1 + i, true);

        if ( !(mouse_buttonstate & (1<<i)) && (mouse_oldbuttonstate & (1<<i)) )
            Key_Event (K_MOUSE1 + i, false);
    }
    mouse_oldbuttonstate = mouse_buttonstate;
}

void IN_Move (usercmd_t *cmd)
{
    if (!mouse_avail)
        return;
   
    mouse_x *= sensitivity.value;
    mouse_y *= sensitivity.value;
   
    if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
        cmd->sidemove += m_side.value * mouse_x;
    else
        cl.viewangles[YAW] -= m_yaw.value * mouse_x;
    if (in_mlook.state & 1)
        V_StopPitchDrift ();
   
    if ( (in_mlook.state & 1) && !(in_strafe.state & 1)) {
        cl.viewangles[PITCH] += m_pitch.value * mouse_y;
        if (cl.viewangles[PITCH] > 80)
            cl.viewangles[PITCH] = 80;
        if (cl.viewangles[PITCH] < -70)
            cl.viewangles[PITCH] = -70;
    } else {
        if ((in_strafe.state & 1) && noclip_anglehack)
            cmd->upmove -= m_forward.value * mouse_y;
        else
            cmd->forwardmove -= m_forward.value * mouse_y;
    }
    mouse_x = mouse_y = 0.0;
}


//#endif//HAVE_OPENGLES