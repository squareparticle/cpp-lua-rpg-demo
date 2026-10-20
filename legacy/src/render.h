/*
Name: render.h
Created: Jan 6 2002

Description:
	This header renders the images to the screen.  The images rendered depends on the game state.

*/
void Paint();
void DrawWindow(UCHAR * buffer);
void setupCameraView();
void DebugController();
void RenderModePlaying();
void RenderModeMessage();
void RenderModeConsole();
void ScreenCapture();
void RenderModeOverLay();

DDSURFACEDESC2        ddsd;

/*
   This method is called 20 times per second.  It will handle witch set of images to render to the
   screen.
*/
void Paint(){
	switch(CURRENT_GAMESTATE){
  	    case STATE_PLAYING:
		   RenderModePlaying();
		break;
  	    case STATE_MESSAGE:
		   RenderModeMessage();
		break;
  	    case STATE_CONSOLE:
		   RenderModeConsole();
		break;
        case STATE_OVERLAYMENU:
		   RenderModeOverLay();
		break;
		case STATE_TRANSITION:
			transitionController();
		break;
		case STATE_ANIMATION:
			RenderModePlaying();
		break;
	}
}

void DebugController(){
	switch(DEBUGING_MODE){
  	    case DEBUGING_MODE_NONE:
           displayDebugNone();
		break;
  	    case DEBUGING_MODE_HEROINFO:
           displayDebugHero();
		break;
  	    case DEBUGING_MODE_MAPINFO:
           displayDebugMap();
		break;
	}
}

int atemp=0;
void RenderModeOverLay(){
  RECT clipper;
  UCHAR *back_buffer = NULL;
  UINT loc  = 0;
  int ans =0;
  short int BLUEIMG_1,BLUEIMG_2;
  short int GREENIMG_1,GREENIMG_2;
  short int REDIMG_1,REDIMG_2;

  if (f_needScreenShot){
     lpDDSBack->Lock(NULL,&ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
     back_buffer = (UCHAR *)ddsd.lpSurface;

     for (int y = 110;y < 404;y++){
        for (int x = 110;x < 485;x++){
           loc = (x * 2) + (y * ddsd.lPitch);
           
		   BLUEIMG_1 = back_buffer[loc] & 0xf8 >> 3;
           GREENIMG_1 = ((back_buffer[loc] & 0xE0) >> 5) | ((back_buffer[loc+1] & 0x07) << 3);
           REDIMG_1 = (back_buffer[loc+1] & 0xF8) >> 3;

           BLUEIMG_2 = 31;
           GREENIMG_2 = 0;
           REDIMG_2 = 0;

           BLUEIMG_1 = (short)((BLUEIMG_1 + BLUEIMG_2) / 2);
           GREENIMG_1 = (short)((GREENIMG_1 + GREENIMG_2) / 2);
           REDIMG_1 = (short)((REDIMG_1 + REDIMG_2) / 2);

           back_buffer[loc] = (BLUEIMG_1) | ((GREENIMG_1 & 0x07) << 5);
           back_buffer[loc + 1] = ((GREENIMG_1 & 0x38)>>3)|(REDIMG_1 << 3);
		}
	 }

     lpDDSBack->Unlock(NULL);

     lpDDSPrimary->Flip(0,DDFLIP_WAIT);
	 ScreenCapture();
     f_needScreenShot = false;
  }
 
  clipper.top = 0;
  clipper.bottom = SCREEN_HEIGHT;
  clipper.left = 0;
  clipper.right = SCREEN_WIDTH;
  lpDDSBack->BltFast(0,0,lpDDSScreenShot,&clipper,DDBLTFAST_WAIT);

  clipper.top = 0;
  clipper.bottom = 320;
  clipper.left = 0;
  clipper.right = 400;
  lpDDSBack->BltFast(100,100,lpDDSMenuOverLay,&clipper,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);
  
  clipper.top = 9+(40*atemp);
  clipper.bottom = 9+(40*atemp)+40;
  clipper.left = 0;
  clipper.right = 120;

  if(++atemp==30)
	  atemp=0;
  
  lpDDSBack->BltFast(120,130+(Sword_loc*43),surf_Weather,&clipper,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);

  lpDDSPrimary->Flip(0,DDFLIP_WAIT);

}

int LayerHeight = 800;
void RenderModePlaying(){
  RECT                    clipper;
  int yVal;	
  setupCameraView();
  
  lpDDSBack->BltFast(0,0,layers[0].surf,&layers[0].clipper,DDBLTFAST_WAIT);

  if(layers[0].clipper.bottom == LayerHeight){
	clipper.left    = layers[0].clipper.left;
	yVal = layers[0].clipper.bottom - layers[0].clipper.top;

	if(yVal < 0)
	    clipper.top     = -1 * (yVal);
	else
	    clipper.top = 0;

	clipper.right   = layers[0].clipper.right;
	clipper.bottom  = SCREEN_HEIGHT - (layers[0].clipper.bottom - layers[0].clipper.top);

   //  itoa(clipper.bottom,strMsgDebug,10);
   //  sprintf(strMsgDebug,"h = %d, %d - %d = %d, y = %d",SCREEN_HEIGHT,layers[0].clipper.bottom,layers[0].clipper.top,layers[0].clipper.bottom - layers[0].clipper.top,clipper.bottom);
   //  sprintf(strMsgDebug,"%d - %d = %d",MAP_CAMERAY,layers[0].clipper.bottom,MAP_CAMERAY - layers[0].clipper.bottom);
   //  sprintf(strMsgDebug,"%d - %d = %d",layers[0].clipper.bottom,layers[0].clipper.top,layers[0].clipper.bottom - layers[0].clipper.top);
   //  sprintf(strMsgDebug,"%d, %d",clipper.top,yVal);
	
	if (yVal < 0)
		yVal = 0;

	lpDDSBack->BltFast(0,yVal,layers[0].surf,&clipper,DDBLTFAST_WAIT);
  }

  lpDDSBack->BltFast(0,0,layers[1].surf,&layers[1].clipper,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);  
  lpDDSBack->BltFast(heroStruct.mapClipper.x, heroStruct.mapClipper.y, heroStruct.surf, &heroStruct.clipper,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);
  lpDDSBack->BltFast(0,0,layers[2].surf,&layers[2].clipper,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);

  if (DEBUGING_GAMESTATE == STATE_DEBUGING_ON)
      DebugController();

  memset(&ddsd,0,sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  
  WeatherContoller();
  lpDDSPrimary->Flip(0,DDFLIP_WAIT);
}

void RenderModeMessage(){
	switch(MESSAGEWINDOW_GAMESTATE){
  	    case STATE_MESSAGEWINDOW_OPENING:
			RenderMessageWindowOpeningClosing(STATE_MESSAGEWINDOW_OPENING);
		break;
  	    case STATE_MESSAGEWINDOW_ON:
            RenderMessageWindow();
		break;
  	    case STATE_MESSAGEWINDOW_CLOSING:
            RenderMessageWindowOpeningClosing(STATE_MESSAGEWINDOW_CLOSING);
		break;
	}
}

// Display the cool console background
void RenderModeConsole(){
  UCHAR *back_buffer = NULL;
  int yCounter =0;
  UINT loc_1 =0;
  UINT loc_2 =0;
  UINT width =0;
  RECT clipper;

  if (f_needScreenShot){
	  ScreenCapture();
      clipper.top = 0;
      clipper.bottom = SCREEN_HEIGHT;
      clipper.left = 0;
      clipper.right = SCREEN_WIDTH;
      lpDDSBack->BltFast(0,0,lpDDSScreenShot,&clipper,DDBLTFAST_WAIT);
  }
  else{
     f_needScreenShot = false;
  }

  lpDDSBack->Lock(NULL,&ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
  back_buffer = (UCHAR *)ddsd.lpSurface;

  for (int y=0; y < 220; y++){
         width = Img_Console.bitmapinfoheader.biWidth * 2;  
	     loc_1 = y * width;
         loc_2 = y*ddsd.lPitch;
         memcpy(&back_buffer[loc_2],&Img_Console.buffer[loc_1],width);
  }

  lpDDSBack->Unlock(NULL);
  
  displayConsolePrompt(); 
  
  lpDDSPrimary->Flip(0,DDFLIP_WAIT);
}

void setupCameraView(){
    layers[0].offset.y =  layers[0].offset.y + layers[0].speed;

	if (layers[0].offset.y > LayerHeight)
       layers[0].offset.y = 0; 

	layers[0].clipper.left    = MAP_CAMERAX;
	layers[0].clipper.top     = MAP_CAMERAY + layers[0].offset.y;
	layers[0].clipper.right   = SCREEN_WIDTH + MAP_CAMERAX;
	
    layers[0].clipper.bottom  = SCREEN_HEIGHT + MAP_CAMERAY + layers[0].offset.y;

	if(layers[0].clipper.bottom > LayerHeight)
      layers[0].clipper.bottom = LayerHeight;

	layers[1].clipper.left    = MAP_CAMERAX;
	layers[1].clipper.top     = MAP_CAMERAY;
	layers[1].clipper.right   = SCREEN_WIDTH + MAP_CAMERAX;
	layers[1].clipper.bottom  = SCREEN_HEIGHT + MAP_CAMERAY;
	layers[2].clipper.left    = MAP_CAMERAX;
	layers[2].clipper.top     = MAP_CAMERAY;
	layers[2].clipper.right   = SCREEN_WIDTH + MAP_CAMERAX;
	layers[2].clipper.bottom  = SCREEN_HEIGHT + MAP_CAMERAY;

    heroStruct.clipper.left   = 0;
	heroStruct.clipper.top    = 0;
    heroStruct.clipper.right  = heroStruct.width;
	heroStruct.clipper.bottom = heroStruct.height;

	if((heroStruct.map.x - MAP_CAMERAX) <= 0)
		heroStruct.clipper.left = MAP_CAMERAX - heroStruct.map.x;

	if((heroStruct.map.x + heroStruct.width) > (SCREEN_WIDTH + MAP_CAMERAX))
		heroStruct.clipper.right = (SCREEN_WIDTH + MAP_CAMERAX) - heroStruct.map.x;

	if((heroStruct.map.y - MAP_CAMERAY) <= 0)
		heroStruct.clipper.top = MAP_CAMERAY - heroStruct.map.y;

	if((heroStruct.map.y + heroStruct.height) > (SCREEN_HEIGHT + MAP_CAMERAY))
		heroStruct.clipper.bottom = (SCREEN_HEIGHT + MAP_CAMERAY) - heroStruct.map.y;

	heroStruct.clipper.left   = heroStruct.clipper.left + heroStruct.frame.x;
	heroStruct.clipper.right  = heroStruct.clipper.right + heroStruct.frame.x;
    heroStruct.clipper.top    = heroStruct.clipper.top + heroStruct.frame.y;
	heroStruct.clipper.bottom = heroStruct.clipper.bottom + heroStruct.frame.y;

	heroStruct.mapClipper.x = heroStruct.map.x - MAP_CAMERAX;
	heroStruct.mapClipper.y = heroStruct.map.y - MAP_CAMERAY;

    if(heroStruct.mapClipper.x < 0)
		heroStruct.mapClipper.x = 0;
    
	if(heroStruct.mapClipper.y < 0)
		heroStruct.mapClipper.y = 0;
}

/*
   This  helper method is used to create a transparent window to the screen for displaying spoken
   dialog from the characters.
*/
/*
void DrawWindow(UCHAR * buffer){
//   UINT col = 137;
   int xStart = (int)(SCREEN_WIDTH/4);
   int yStart = (int)(SCREEN_HEIGHT/4);
   int yEnd = yStart + 200;
   int xEnd = xStart + 320;
   int yCounter =0;
   WORD col = _RGB15BIT(0,0,255);

   for (int y=yStart; y < yEnd; y++){
	  yCounter ++;
      for (int x=xStart; x < xEnd * 2; x=x +2){
         buffer[x+(y*ddsd.lPitch)] &= LO_BYTE(col);
         buffer[x+(y*ddsd.lPitch)+1] &= HI_BYTE(col);
	  }
   }   


}
*/
// Display the cool console background
void DrawWindow(UCHAR * buffer){
}

void ScreenCapture(){
  
  RECT clipper;

  clipper.top = 0;
  clipper.bottom = SCREEN_HEIGHT;
  clipper.left = 0;
  clipper.right = SCREEN_WIDTH;

  lpDDSScreenShot->BltFast(0,0,lpDDSPrimary,&clipper,DDBLTFAST_WAIT);
}
/*
	  if(yCounter > 25){
		 yCounter  =0;
	     //col = col + 2;
	  }
      for (int x=xStart; x < xEnd * 2; x=x +4){
		 if(y%2==0){
     		if(x%2==0){
               buffer[x+(y*ddsd.lPitch)] = LO_BYTE(col);
               buffer[x+(y*ddsd.lPitch)+1] = HI_BYTE(col);
			}
		 }
		 else{
     		if(x%2!=0){
               buffer[x+(y*ddsd.lPitch)] = LO_BYTE(col);
               buffer[x+(y*ddsd.lPitch)+1] = HI_BYTE(col);
			}
		 }
	  }
*/