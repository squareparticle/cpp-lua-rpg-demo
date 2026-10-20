/*
Name: init.h
Created: Jan 5 2002

Description:
	Initailize all the direct X moduals used in the game.

*/
BOOL setupLayers();
BOOL setupHero();
BOOL Init_DirectDraw();
BOOL InitDirectSound();
BOOL InitDirectMusic();
BOOL InitDirectInput();
BOOL CALLBACK DI_Enum_Joysticks(LPCDIDEVICEINSTANCE lpddi, LPVOID guid_ptr);
void Init_Weather();

void Init_Weather(){
	for (int i_rainDrop = 0;i_rainDrop < RAIN_MAX;i_rainDrop++){
       rainParticle[i_rainDrop].x = rand() % SCREEN_WIDTH;
       rainParticle[i_rainDrop].y = rand() % SCREEN_HEIGHT;
       rainParticle[i_rainDrop].WindSpeed=2;
       rainParticle[i_rainDrop].Velocity=6;
	   rainParticle[i_rainDrop].surfaceX = rand() % 4;
	   rainParticle[i_rainDrop].surfaceY = rand() % 13;
	}
}

BOOL InitDirectInput(){
   // first create the direct input object
   if (DirectInputCreate(main_instance,DIRECTINPUT_VERSION,&lpdi,NULL)!=DI_OK)
      return false;

   // create a keyboard device  //////////////////////////////////
   if (lpdi->CreateDevice(GUID_SysKeyboard, &lpdikey, NULL)!=DI_OK)
      return false;

   // set cooperation level
   if (lpdikey->SetCooperativeLevel(main_window_handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)!=DI_OK)
      return false;

   // set data format
   if (lpdikey->SetDataFormat(&c_dfDIKeyboard)!=DI_OK)
      return false;

   // acquire the keyboard
   if (lpdikey->Acquire()!=DI_OK)
      return false;

//************************ get joystick **********************************/

   lpdi->EnumDevices(DIDEVTYPE_JOYSTICK, DI_Enum_Joysticks, &joystickGUID, DIEDFL_ATTACHEDONLY); 

   if (lpdi->CreateDevice(joystickGUID, &lpdijoy, NULL)!=DI_OK)
      f_hasGamePort = false;

   if(f_hasGamePort){

      lpdijoy->QueryInterface(IID_IDirectInputDevice2, (void **) &lpdijoy2); 
      lpdijoy->Release();

      if (lpdijoy2->SetCooperativeLevel(main_window_handle,DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)!=DI_OK)
         return false;

      if (lpdijoy2->SetDataFormat(&c_dfDIJoystick)!=DI_OK)
         return false;

      // set the range of the joystick
      DIPROPRANGE joy_axis_range;

      // first x axis
      joy_axis_range.lMin = -24;
      joy_axis_range.lMax = 24;

      joy_axis_range.diph.dwSize       = sizeof(DIPROPRANGE); 
      joy_axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
      joy_axis_range.diph.dwObj        = DIJOFS_X;
      joy_axis_range.diph.dwHow        = DIPH_BYOFFSET;

      lpdijoy2->SetProperty(DIPROP_RANGE,&joy_axis_range.diph);

      // now y-axis
      joy_axis_range.lMin = -24;
      joy_axis_range.lMax = 24;

      joy_axis_range.diph.dwSize       = sizeof(DIPROPRANGE); 
      joy_axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
      joy_axis_range.diph.dwObj        = DIJOFS_Y;
      joy_axis_range.diph.dwHow        = DIPH_BYOFFSET;

      lpdijoy2->SetProperty(DIPROP_RANGE,&joy_axis_range.diph);

      // acquire the joystick
      if (lpdijoy2->Acquire()!=DI_OK)
         return false;
   }

   return true; 
}

BOOL InitDirectMusic(){
   // Create the DM objects
   CoInitialize(NULL);

   dmrval = CoCreateInstance(CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC_SERVER, IID_IDirectMusicPerformance, (LPVOID*)&lpIDMPerformance);

   if (FAILED(dmrval)){
      return FALSE;
   }

   dmrval = CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, IID_IDirectMusicLoader, (LPVOID*)&lpIDMLoader);
   if (FAILED(dmrval)){
      return FALSE;
   }

   dmrval = CoCreateInstance(CLSID_DirectMusicComposer, NULL, CLSCTX_INPROC, IID_IDirectMusicComposer, (LPVOID*)&lpIDMComposer);
   if (FAILED(dmrval)){
      return FALSE;
   }

   // Initialize the Performance object
   dmrval = lpIDMPerformance->Init(NULL, NULL, NULL);

   // Turn on automatic downloading of instruments
   BOOL fAutoDownload = TRUE;
   lpIDMPerformance->SetGlobalParam(GUID_PerfAutoDownload, &fAutoDownload, sizeof(BOOL));

   // Tell the Performance to create the default port
   lpIDMPerformance->AddPort(NULL);

   // Load the DM musical data
   if (!load_music())
	     return FALSE;

   // Since we want to play the music in the background, start playing the music
   lpIDMPerformance->PlaySegment(lpIDMSegment, 0, 0, NULL);

   // Disable tempo track in segment so that it does not reset tempo
   lpIDMSegment->SetParam( GUID_DisableTempo, 0xFFFF,0,0, NULL );

   return TRUE;
}

BOOL InitDirectSound(){
   if (DirectSoundCreate(NULL, &lpds, NULL)!= DS_OK)
      return FALSE;

   if (lpds->SetCooperativeLevel(main_window_handle,DSSCL_NORMAL)!=DS_OK)
      return FALSE;

   return TRUE;
}

BOOL InitDirectDraw(){
    HRESULT                     ddrval;
    LPDIRECTDRAW                pDD;
	// clear image pointers

	ZeroMemory(lpSlides,sizeof(lpSlides));

    // Create the main DirectDraw object

    ddrval = DirectDrawCreate(NULL, &pDD, NULL);
    if (ddrval != DD_OK) {
		ErrStr=Err_DirectDrawCreate;
		return FALSE;
	}

    // Fetch DirectDraw4 interface

    ddrval = pDD->QueryInterface(IID_IDirectDraw4, (LPVOID *) & lpDD);
    if (ddrval != DD_OK) {
        ErrStr=Err_Query;
		return FALSE;
	}

    // Set our cooperative level

    ddrval = lpDD->SetCooperativeLevel(main_window_handle, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
    if (ddrval != DD_OK) {
		ErrStr=Err_Coop;
		return FALSE;
	}

	// Set the display mode
	ddrval = lpDD->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, 0, 0);
	if (ddrval !=DD_OK) {
		ErrStr=Err_DispMode;
		return FALSE;
	}

    // Create the primary surface with 1 back buffer

    DDSURFACEDESC2 ddsd;
	DDSCAPS2 ddscaps;
	ZeroMemory(&ddsd,sizeof(ddsd));
    ddsd.dwSize = sizeof( ddsd );
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                          DDSCAPS_FLIP | 
                          DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;
    ddrval = lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL );
	if (ddrval!=DD_OK) {
		ErrStr=Err_CreateSurf;
		return FALSE;
	}

	// Fetch back buffer interface

	ddscaps.dwCaps=DDSCAPS_BACKBUFFER;
	ddrval=lpDDSPrimary->GetAttachedSurface(&ddscaps,&lpDDSBack);
	if (ddrval!=DD_OK) {
		ErrStr=Err_CreateSurf;
		return FALSE;
	}

	return true;
}

BOOL setupLayers(){
	DDPIXELFORMAT ddpf;
	ddpf.dwSize=sizeof(ddpf);
	lpDDSPrimary->GetPixelFormat(&ddpf);

	KeyColor = ddpf.dwGBitMask;

    lpDDSScreenShot = CreateBlankSuface();

	// set color keys
	DDCOLORKEY key;
	key.dwColorSpaceLowValue = KeyColor;
	key.dwColorSpaceHighValue = KeyColor;
	layers[0].surf=bitmap_surface(layers[0].name);
	
	if (!layers[0].surf)
		return FALSE;
	layers[0].clipper.left       = 0;
	layers[0].clipper.top        = 0;
	layers[0].clipper.right      = SCREEN_WIDTH;
	layers[0].clipper.bottom     = SCREEN_HEIGHT;

    if(layers[0].speed == NULL)
	   layers[0].speed              = 6;

	layers[1].surf=bitmap_surface(layers[1].name);
	if (!layers[1].surf)
		return FALSE;
	layers[1].surf->SetColorKey(DDCKEY_SRCBLT, &key);
	layers[1].clipper.left       = 0;
	layers[1].clipper.top        = 0;
	layers[1].clipper.right      = SCREEN_WIDTH;
	layers[1].clipper.bottom     = SCREEN_HEIGHT;
	layers[1].speed              = 6;

    layers[2].surf=bitmap_surface(layers[2].name);
	if (!layers[2].surf)
		return FALSE;
	layers[2].surf->SetColorKey(DDCKEY_SRCBLT, &key);
    layers[2].clipper.left       = 0;
	layers[2].clipper.top        = 0;
	layers[2].clipper.right      = SCREEN_WIDTH;
	layers[2].clipper.bottom     = SCREEN_HEIGHT;
    layers[2].speed              = 6;

    Load_Bitmap_File(&Img_BackMask, MAP_LAYER_MASK_FILE);

    surf_Weather = bitmap_surface("weather.bmp");
	surf_Weather->SetColorKey(DDCKEY_SRCBLT, &key);
    lpDDSMenuOverLay = bitmap_surface("text.bmp");
    lpDDSMenuOverLay->SetColorKey(DDCKEY_SRCBLT, &key);
    
	return true;
}

BOOL setupHero(){
    
	heroStruct.surf=bitmap_surface(heroStruct.filename);
	if (!heroStruct.surf)
		return FALSE;

    heroStruct.height           = 116;
	heroStruct.width            = 84;
    
	if(heroStruct.map.x == NULL)
       heroStruct.map.x            = 300;
    if(heroStruct.map.y == NULL)
       heroStruct.map.y            = 100;
	
	heroStruct.frameSequence[0] = 0;
	heroStruct.frameSequence[1] = 1;
	heroStruct.frameSequence[2] = 2;
	heroStruct.frameSequence[3] = 3;
	heroStruct.frameSequence[4] = 4;
	heroStruct.frameSequence[5] = 3;
	heroStruct.frameSequence[6] = 2;
	heroStruct.frameSequence[7] = 1;
	heroStruct.currentFrame     = 0;
	heroStruct.clipper.left     = 0;
	heroStruct.clipper.top      = 0;
	heroStruct.clipper.right    = heroStruct.width;
	heroStruct.clipper.bottom   = heroStruct.height;
    heroStruct.speed            = 6;
	heroStruct.direction        = Jcompas.WEST;
	heroStruct.mapClipper.x     = heroStruct.map.x - MAP_CAMERAX;
	heroStruct.mapClipper.y     = heroStruct.map.y - MAP_CAMERAY;
    heroStruct.frame.x          = 0;
    heroStruct.frame.y          = 0;


    if(heroStruct.mapClipper.x < 0)
		heroStruct.mapClipper.x = 0;
    
	if(heroStruct.mapClipper.y < 0)
		heroStruct.mapClipper.y = 0;

	DDPIXELFORMAT ddpf;
	ddpf.dwSize=sizeof(ddpf);
	lpDDSPrimary->GetPixelFormat(&ddpf);

	KeyColor = ddpf.dwGBitMask;

	// set color keys
	DDCOLORKEY key;
	key.dwColorSpaceLowValue = KeyColor;
	key.dwColorSpaceHighValue = KeyColor;

	heroStruct.surf->SetColorKey(DDCKEY_SRCBLT, &key);

	return true;
}

BOOL CALLBACK DI_Enum_Joysticks(LPCDIDEVICEINSTANCE lpddi, LPVOID guid_ptr) {

   *(GUID*)guid_ptr = lpddi->guidInstance; 
   strcpy(joyname, (char *)lpddi->tszProductName);
   return(DIENUM_STOP);

}
