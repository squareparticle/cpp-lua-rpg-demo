/*  
Name: presets.h
Created: Jan 5 2002

Description:
  All the program's major globals go here 
*/

#ifndef _preset_h
#define _preset_h

#define WINDOW_CLASS_NAME "WINXCLASS"

LPDIRECTDRAW4 lpDD=NULL;
LPDIRECTDRAWSURFACE4 lpDDSPrimary=NULL;
LPDIRECTDRAWSURFACE4 lpDDSBack=NULL;
LPDIRECTDRAWSURFACE4 lpDDSScreenShot=NULL;
LPDIRECTDRAWSURFACE4 lpSlides[9];
LPDIRECTDRAWSURFACE4 lpDDSMenuOverLay;

LPDIRECTDRAWPALETTE  lpDDpal      = NULL;
PALETTEENTRY         palette[256];         // color palette

char strMsg[80];
char * strMsgDebug;
bool f_hasConscel = false;
unsigned int ptr_ConsolePrompt=0;

const int ConsoleLine_MAX = 13;
char * rg_ConsoleLine[ConsoleLine_MAX];

// **********************************  WEATHER ******************************
const int RAIN_MAX = 300;
PARTICLE rainParticle[RAIN_MAX];
LPDIRECTDRAWSURFACE4	surf_Weather;
int FERRY_ANIM[]={0,1,2,1,0};
int SWORD_ANIM[]={0,1,2,3,4,5,4,3,2,1,0};
int Sword_loc = 0;

#define RandomSnowWindInit() (2 + rand() % 5)
#define RandomSnowFallInit() (2 + rand() % 5)
#define RandomFerryWindInit() (1 + rand() % 3)
#define RandomFerryFallInit() (1 + rand() % 3)
#define RandomRainWindInit() (5 + rand() % 5)
#define RandomRainFallInit() (5 + rand() % 5)
// **************************************************************************

#define PRINTERROR(s)	\
		fprintf(stderr,"\n%: %d\n", s, WSAGetLastError())


DWORD KeyColor;

#define NUMSOUNDS 9
#define NVB_SIZE      6 // size of new voice block in bytes

LPDIRECTSOUND       lpDS;
LPDIRECTSOUNDBUFFER lpDSBSounds[NUMSOUNDS];
LONG                lSirenPan;
LONG                lSirenPanInc;
HRESULT             dmrval;

typedef struct VOC_FILE_TYP{
        int sample_rate;
        int length;
        LPDIRECTSOUNDBUFFER lpdsbuffer;
} VOC_FILE, *VOC_FILE_PTR;

//------ DirectSound Objects ------//
LPDIRECTSOUND		lpds;
DSBUFFERDESC		dsbd;
DSCAPS				dscaps;
HRESULT				dsresult;
DSBCAPS           dsbcaps;

LPDIRECTSOUNDBUFFER lpdsbprimary;
LPDIRECTSOUNDBUFFER lpdsbsecondary;

WAVEFORMATEX pcmwf;
VOC_FILE work_voc_1;
VOC_FILE work_voc_2;

//------ DirectMusic Objects ------//
IDirectMusicPerformance     *lpIDMPerformance;
IDirectMusicLoader          *lpIDMLoader;
IDirectMusicComposer        *lpIDMComposer;
IDirectMusicSegment         *lpIDMSegment;
IDirectMusicSegment         *lpIDMTemplate;
IDirectMusicStyle           *lpIDMStyle;
IDirectMusicChordMap        *lpIDMChordMap;

//------ Directinput globals -------//
LPDIRECTINPUT        lpdi      = NULL;    // dinput object
LPDIRECTINPUTDEVICE  lpdikey   = NULL;    // dinput keyboard
LPDIRECTINPUTDEVICE  lpdimouse = NULL;    // dinput mouse
LPDIRECTINPUTDEVICE  lpdijoy   = NULL;    // dinput joystick 
LPDIRECTINPUTDEVICE2 lpdijoy2  = NULL;    // dinput joystick
GUID                 joystickGUID;        // guid for main joystick
char                 joyname[80];         // name of joystick

// these contain the target records for all di input packets
UCHAR keyboard_state[256]; // contains keyboard state table
DIMOUSESTATE mouse_state;  // contains state of mouse
DIJOYSTATE joy_state;      // contains state of joystick
int f_hasGamePort = true;

//------ Define Stand-Still Tempo ------//
#define DEFAULT_TEMPO    60

const char *ErrStr=NULL;	

const int InputStringMax = 50;

const char Err_Reg_Class[]			= "Error Registering Window Class";
const char Err_Create_Win[]			= "Error Creating Window";
const char Err_DirectDrawCreate[]	= "DirectDrawCreate FAILED";
const char Err_Query[]				= "QueryInterface FAILED";
const char Err_Coop[]				= "SetCooperativeLevel FAILED";
const char Err_CreateSurf[]			= "CreateSurface FAILED";
const char Err_LoadBMP[]			= "Error Loading Image";
const char Err_DispMode[]			= "Error Setting Display Mode";
const char Err_LoadImage[]			= "Error Loading Image";
const char Err_DirectSoundCreate[]  = "DirectSoundCreate FAILED";
const char Err_CreateBuff[]         = "CreateBuffer FAILED";
const char Err_LoadWAV[]			= "Error Loading Sound";


#define SafeRelease(x) if (x) { x->Release(); x=NULL;}

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP    16

#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

HWND main_window_handle = NULL;
HINSTANCE main_instance = NULL;
/*
int CW_X = 50;  // couch window X Position
int CW_Y = 150;  // couch window Y Position
int CW_W = 252;  // couch window Width
int CW_H = 200;  // couch window Height
*/
// Hero variables from script
char * HERO_TILES = "NONE";
char * msg_Console = " ";
RECT rct_HEROBuffer;
int HERO_WALKFRAMEMAX=0;
int HERO_WIDTH=84;
int HERO_HEIGHT=116;

const int MESSAGEWINDOWPOPUP_SPEED = 15;

/*
int HframeX=0;
int HframeY=0;
int HERO_LOCX=130;
int HERO_LOCY=100;
int HERO_SPEEDX=6;
int HERO_SPEEDY=6;
int HERO_FRAME_ARRAY[]={0,1,2,3,4,3,2,1};
int HERO_CURRENT_FRAME=0;
*/
typedef struct JPoint {
   int x;
   int y;
} Jpoint;

LPDIRECTDRAWSURFACE4	Herosurf;
struct HERO {
	LPDIRECTDRAWSURFACE4  	surf;
	char *                  filename;
    RECT                    clipper;
	int                     height;
	int                     width;
	Jpoint                  frame;
	Jpoint                  map;
	Jpoint                  mapClipper;
	int                     frameSequence[8];
	int 					speed;
	int                     currentFrame;
	int                     direction;
} heroStruct;


struct JCompas{
	 int NORTH;
	 int SOUTH;
	 int EAST;
	 int WEST;	
     int CENTER;
}Jcompas;


// Map variables from script
char * MAP_LAYER1_FILE="NONE";
char * MAP_LAYER2_FILE="NONE";
char * MAP_LAYER3_FILE="NONE";
char * MAP_LAYER_MASK_FILE="NONE";
char * MAP_SCRIPTNAME="scripts\\maps\\INIT.MAP";
int MAP_LAYER_SPEEDX=6;
int MAP_LAYER_SPEEDY=6;
int MAP_WIDTH=800;
int MAP_HEIGHT=800;
int MAP_CHARACTERNUM=0;
int MAP_LAYERNUM=0;
int MAP_MONSTERNUM=0;
int MAP_RAN_MONSTER=0;
int MAP_CAMERAY=0;
int MAP_CAMERAX=0;

#define	NUM_LAYERS			4
#define NUM_SCRIPTS         256

struct Scripts {
	char *                  scriptName;
	int 					key;
    Scripts () {Clear ();}
	void  Clear () {key = -1;}

} scripts[NUM_SCRIPTS];
int colorkey =0;

struct LAYER {
	char *                  name;
	LPDIRECTDRAWSURFACE4	surf;
    RECT                    clipper;
	SPOINT                  offset;
	int 					speed;
} layers[NUM_LAYERS];

void msgErrorWindow(char * ERRmsg){
   ShowCursor(TRUE);
   MessageBox(main_window_handle,ERRmsg,"Exception dialog",MB_OK);
   ShowCursor(FALSE);
}

const int WEST = 0;
const int EAST = 1;
const int SOUTH = 2;
const int NORTH = 3;

void InitGlobals(){
   Jcompas.CENTER = -1;
   Jcompas.WEST  = 0;
   Jcompas.EAST  = 1;
   Jcompas.SOUTH = 2;
   Jcompas.NORTH = 3;
}

int current_window=0;
char * ptr_window[] = {"graphics\\window1.bmp",
                       "graphics\\window.bmp",
                       "graphics\\window2.bmp",
                       "graphics\\window3.bmp",
                       "graphics\\window4.bmp",
                       "graphics\\window5.bmp",
                       "graphics\\window6.bmp",
                       "graphics\\window7.bmp",
};

const int ANIMATIONSCRIPT_MAX = 100;
char * mapAnimationBuffer [ANIMATIONSCRIPT_MAX];

// Method delcarations that need to be placed at the highest level header file :(
void scrollConsole();
void addMessagetoConsole(char * msg);
void InputModeMessage();
void InputModeMessage_JoyPad();
void InputModePlayingOverRide(int Direction);
void RenderMessageWindowOpeningClosing(int directon);
void RenderMessageWindow();
void render_Particles();
void render_RainSprites();
void render_SnowSprites();
void render_FerrySprites();
void init_RainSprites();
void init_SnowSprites();
void init_Particles();
void init_Ferries();
void transitionController();
bool getAnimationScript(char * scriptName);
void setupClientSocket(char * ip);
void DatagramClient(char *szServer, short nPort, char * msgToSend);
SOCKET	theSocket;
SOCKADDR_IN saServer;
DWORD iID[2];		// array of thread id's
HANDLE hThreads[2]; // array of thread handles
DWORD waiter;		// flag which is set when all therads finish
void StartServer();
DWORD readPortS(LPVOID n);
void setupServerSocket(short nPort);


void DoNextFrame();

void WeatherContoller();

int newspeed = 0;

unsigned char * pScreenCapture = (unsigned char *) malloc((SCREEN_WIDTH*2) * SCREEN_HEIGHT);
int f_needScreenShot = false;
#endif