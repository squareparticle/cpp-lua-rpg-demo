/*
Project Dragon Quest
Jan - Aug 2002

Designed and programmed 
by Jamie Whitten

	This is a game that takes the user back into the past and allows them to fight magic creatures 
and save the planet.  Play the game to find more out about the story and game play.

    The game is mostly script based.  The init.scr script is the first script that the program needs
to open in order to load the first map.  Characters, maps, animations all have their own scripts.


Basic inner workings of the Engine:

    To put the program in console debugging mode, just run the program and pass -console as a 
command line parameter.  Then hit F12 to enter commands.

	The engine is state based.  Depending on the current state is what will be drawn on the screen
and how the input devices will react.

	When in the playing state the renderer will paint the screen in layers.  The first is the 
background.  The second could be a second level background for parrallaxing purposes.  Next is the 
character layer then the obtraction layer.  The collision detection is on done on a seperate layer
that doesn't get rendered.  This layer is a mask to allow the hero to walk in some areas, but not 
others.

Specs:

	* Res 640 x 480
	* 20 fps
	* 16 bit color depth = 65535 colors
	* wav sounds
	* midi and Direct Music
	* bmp images
	* Direct Draw, Direct Sound, Direct Music, Direct Input

Directory Structure:
   >Root
      >Graphics
	     >Map ...
	     >Charaters
	  >Sounds
	  >Scripts
	     >maps
	     >characters
	     >animations
	  >Music
*/

#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include <windowsx.h>
#define INITGUID
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <dsound.h>
#include <fcntl.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <ddraw.h>
#include <dsound.h>
#include <dinput.h>
#include <objbase.h>
#include <winsock.h>
#include "wave.h"

#include "toolkit.h"          // Macros, Constants and type definitions
#include "presets.h"          // Project globals 
#include "Networking.h"     // Project globals 
#include "statemanager.h"     // Project globals 
#include "sound.h"            // Music control
#include "surface.h"          // loading bitmaps into surfaces
#include "bitmap.h"           // loading raw Bitmap
#include "scriptio.h"         // Script I/O

#include "inits.h"            // Direct X Initalizations
#include "input.h"            // keyboard, game port and mouse control
#include "msgSystem.h"        // display text to the screen
#include "console.h"          // Console I/O
#include "render.h"           // painting to the screen
#include "messagewindow.h"    // painting message window to screen
#include "weatherManger.h"   // painting differnent weather patterns to the screen
#include "ScreenTransition.h" // Fading from one screen to the next
#include "AnimationController.h" // Fading from one screen to the next

DWORD WINAPI readPort(LPVOID);
BOOL Game_Init(void *parms=NULL);
int Game_Main(void *parms=NULL);
bool checkforReady();
void Cleanup();
bool AllocateGlobalMemory();
void FreeGlobalMemory();

static HINSTANCE g_hInst = NULL;
static char szClass[] = "XmplHr6Class";
static char szCaption[] = "Dragon Quest";


UINT nTimerDelay = 50;        // timer set for 0.050 seconds
const UINT idTimer1 = 1;

/* 
   Message loop that reneders the screen, reads Input devices, and closes the program window.
   There is a timer that sends a message to the renderer and reads from the input devices 20 times
   per second.
*/

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
   PAINTSTRUCT	ps;
   HDC			hdc;

   switch(msg){
	    case WM_CREATE:{
           SetTimer(hwnd, idTimer1, nTimerDelay, NULL);
  		   return(0);
		}

        case WM_PAINT:{
          hdc = BeginPaint(hwnd,&ps);
          EndPaint(hwnd,&ps);
          return(0);
		}

        case WM_TIMER:{
	  	    Paint();
			if (checkforReady()) 
       	       InputController();
		}break;

   	    case WM_DESTROY:{
		   PostQuitMessage(0);
		   return(0);
		}

        case WM_COMMAND:{
		} break;

	    default:break;
    }

    return (DefWindowProc(hwnd, msg, wparam, lparam));
}

bool checkforReady(){
	//if (PROCESSING_TTL == UNLIMITED)
	//   return false;

	if ((PROCESSING_GAMESTATE == STATE_BUSY) && (PROCESSING_TTL > 0)){
	   PROCESSING_TTL --;
	   return false;
	}
	else{
		PROCESSING_TTL = PROCESSING_TTL_MAX;
        PROCESSING_GAMESTATE = STATE_READY;
		return true;
	}
}

/* 
	Start the program, create a Direct X window, init the Direct X modules, Start the windows
	message loop witch controls screen rendering and input devices.
*/
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int ncmdshow){

   WNDCLASS winclass;
   HWND	 hwnd;
   MSG		 msg;

   g_hInst = hinstance;

   winclass.style		    = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
   winclass.lpfnWndProc	    = WindowProc;
   winclass.cbClsExtra		= 0;
   winclass.cbWndExtra		= 0;
   winclass.hInstance		= hinstance;
   winclass.hIcon		    = LoadIcon(NULL, IDI_APPLICATION);
   winclass.hCursor		    = LoadCursor(NULL, IDC_ARROW);
   winclass.hbrBackground	= GetStockObject(LTGRAY_BRUSH);
   winclass.lpszMenuName	= NULL;
   winclass.lpszClassName	= WINDOW_CLASS_NAME;
   winclass.hCursor         = LoadCursor(NULL, IDC_ARROW);

   if (!RegisterClass(&winclass))
	   return(0);

   if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, "WinX Game Console", WS_POPUP | WS_VISIBLE, 0,0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hinstance, NULL)))
      return(0);

   ShowCursor(FALSE);

   main_window_handle = hwnd;
   main_instance      = hinstance;
   
   Game_Init(lpcmdline);

   while(1){
	   if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
         if (msg.message == WM_QUIT)
            break;

		   TranslateMessage(&msg);
		   DispatchMessage(&msg);
		}

      Game_Main();
	}

   closesocket(theSocket);
   WSACleanup();
   Cleanup();
   return(msg.wParam);
}

void FreeGlobalMemory(){
   for(int i_ConsoleLine=0;i_ConsoleLine < ConsoleLine_MAX; i_ConsoleLine ++)
  	  free (rg_ConsoleLine[i_ConsoleLine]);

   free (msg_Console);
}

bool AllocateGlobalMemory(){

   // ************************* Allocating memory for Console Window **************************
   for(int i_ConsoleLine=0;i_ConsoleLine < ConsoleLine_MAX; i_ConsoleLine ++){
	   if ((rg_ConsoleLine[i_ConsoleLine] = (char *) malloc(InputStringMax)) == NULL)
		   return false;
	   else
           rg_ConsoleLine[i_ConsoleLine][0] = NULL;
   }

   if ((msg_Console = (char *) malloc(InputStringMax)) == NULL)
	   return false;
   else
       msg_Console[0] = NULL;
    
   rg_ConsoleLine[1] = "Dragon Quest Ver 1.0.0";
   rg_ConsoleLine[2] = "Programmed By Jamie Whitten";
   rg_ConsoleLine[3] = "Copy Right 2002";
   rg_ConsoleLine[ConsoleLine_MAX-2] = "Console Window";
   rg_ConsoleLine[ConsoleLine_MAX-1] = "Type in the command and hit Enter";
   
   return true;						                
}

// initalize Direct X moduals and load the init script
BOOL Game_Init(void *parms){
	
	InitGlobals();

    if (!AllocateGlobalMemory())
		return false;

	if (!InitDirectDraw())
		return false;

	if (!InitDirectInput())
		return false;

    Init_Weather();
/*	if (!InitDirectSound())
		return false;
	if (!InitDirectMusic())
		return false;

    Load_VOC("DAVE.VOC",&work_voc_2);
    if (Load_VOC("DAVE.VOC",&work_voc_1)){
       work_voc_1.lpdsbuffer->Play(0,0,DSBPLAY_LOOPING);
    }
*/    
    //Change_Tempo(1);
	strMsgDebug = "Empty";
	char * strCommand = (char *)(parms);
    if(strcmp(strCommand,"-console")==0)
		DEBUGING_GAMESTATE = STATE_DEBUGING_ON;

	loadmapScript(MAP_SCRIPTNAME);
    Load_Bitmap_File(&Img_Console, "graphics\\console.bmp");
	Load_Bitmap_File(&Img_MessageWindow, ptr_window[current_window]);

	if (!setupLayers())
    	return false;

	if (!setupHero())
    	return false;
    
	return true;
}

/* 
   This is used to override any states key commands.  State key commands only get called 20 times
   a second and this over-ride is in the event loop of the program.  Really used to press escape to
   quit at any time in the program.
*/
int Game_Main(void *parms){
   OverRideKeyBoardController();
   return(1);
}

/*  
   Working with C++ means I need to clean up anything that I have created :(
*/
void Cleanup(){
	//waiter = WaitForMultipleObjects(1,hThreads,TRUE,INFINITE);

	// release loaded image surfaces
    SafeRelease(layers[0].surf);
    SafeRelease(layers[1].surf);
    SafeRelease(layers[2].surf);
    SafeRelease(heroStruct.surf);

	//FreeGlobalMemory();
	// release DirectDraw interfaces

	SafeRelease(lpDDSPrimary);
	SafeRelease(lpDD);

    // release DirectSound interface

    if (work_voc_1.lpdsbuffer)
      work_voc_1.lpdsbuffer->Release();

    SafeRelease(lpDS);

	// release keyboard
    lpdikey->Unacquire();
    lpdikey->Release();

    // release joystick
    lpdijoy2->Unacquire();
    lpdijoy2->Release();
    lpdi->Release();

	//free (ScreenCapture);

    // display error if one thrown
	if (ErrStr) {
		MessageBox(NULL, ErrStr, szCaption, MB_OK);
		ErrStr=NULL;
	}
	
}
