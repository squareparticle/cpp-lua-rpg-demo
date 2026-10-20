
void RenderMessageWindowOpeningClosing(int directon){
  UCHAR *back_buffer = NULL;
  UINT loc_1 =0;
  UINT loc_2 =0;
  RECT clipper;
  UINT width = Img_MessageWindow.bitmapinfoheader.biWidth * 2;
  int midHeight = Img_MessageWindow.bitmapinfoheader.biHeight / 2;
  int c_height;
  int height = Img_MessageWindow.bitmapinfoheader.biHeight;
 
  if(directon == STATE_MESSAGEWINDOW_OPENING)
     c_height = MESSAGEWINDOW_HIEGHT += MESSAGEWINDOWPOPUP_SPEED;
  else
     c_height = MESSAGEWINDOW_HIEGHT -= MESSAGEWINDOWPOPUP_SPEED;

  if (f_needScreenShot){
     ScreenCapture();
     f_needScreenShot = false;
  }

  clipper.top = 0;
  clipper.bottom = SCREEN_HEIGHT;
  clipper.left = 0;
  clipper.right = SCREEN_WIDTH;
  lpDDSBack->BltFast(0,0,lpDDSScreenShot,&clipper,DDBLTFAST_WAIT);

  lpDDSBack->Lock(NULL,&ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
  back_buffer = (UCHAR *)ddsd.lpSurface;
 
  for (int y=0; y < c_height; y++){
	     loc_1 = (c_height-y) * width;
         loc_2 = 250 + ((150+midHeight-y)*ddsd.lPitch);
         memcpy(&back_buffer[loc_2],&Img_MessageWindow.buffer[loc_1],width);
	     loc_1 = (height - c_height+y) * width;
         loc_2 = 250 + ((150+midHeight+y)*ddsd.lPitch);
         memcpy(&back_buffer[loc_2],&Img_MessageWindow.buffer[loc_1],width);
  }

  lpDDSBack->Unlock(NULL);
  
  lpDDSPrimary->Flip(0,DDFLIP_WAIT);

  if((directon == STATE_MESSAGEWINDOW_OPENING) && (MESSAGEWINDOW_HIEGHT > (100-MESSAGEWINDOWPOPUP_SPEED))){
     MESSAGEWINDOW_GAMESTATE = STATE_MESSAGEWINDOW_ON;
  }

  if((directon == STATE_MESSAGEWINDOW_CLOSING) && (MESSAGEWINDOW_HIEGHT < MESSAGEWINDOWPOPUP_SPEED)){
       MESSAGEWINDOW_GAMESTATE = STATE_MESSAGEWINDOW_OPENING;
	   MESSAGEWINDOW_HIEGHT == 0;
	   changeState(STATE_PLAYING);
  }
}

void RenderMessageWindow(){
  UCHAR *back_buffer = NULL;
  UINT loc_1 =0;
  UINT loc_2 =0;
  UINT width =0;
  width = Img_MessageWindow.bitmapinfoheader.biWidth * 2;  

  lpDDSBack->Lock(NULL,&ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
  back_buffer = (UCHAR *)ddsd.lpSurface;
 
  for (int y=0; y < Img_MessageWindow.bitmapinfoheader.biHeight; y++){
	     loc_1 = y * width;
         loc_2 = 250 + ((y+150)*ddsd.lPitch);
         memcpy(&back_buffer[loc_2],&Img_MessageWindow.buffer[loc_1],width);
  }

  lpDDSBack->Unlock(NULL);
  
  lpDDSPrimary->Flip(0,DDFLIP_WAIT);
}

void RenderMessageWindowClosing(){
  UCHAR *back_buffer = NULL;
  UINT loc_1 =0;
  UINT loc_2 =0;
  UINT width =0;
  width = Img_MessageWindow.bitmapinfoheader.biWidth * 2;  

  lpDDSBack->Lock(NULL,&ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
  back_buffer = (UCHAR *)ddsd.lpSurface;
 
  for (int y=0; y < Img_MessageWindow.bitmapinfoheader.biHeight; y++){
	     loc_1 = y * width;
         loc_2 = 250 + ((y+150)*ddsd.lPitch);
         memcpy(&back_buffer[loc_2],&Img_MessageWindow.buffer[loc_1],width);
  }

  lpDDSBack->Unlock(NULL);
  
  lpDDSPrimary->Flip(0,DDFLIP_WAIT);
}

void InputModeMessage(){
   int delaytime = 1;
   if (KEY_DOWN(VK_SPACE)){
      MESSAGEWINDOW_GAMESTATE = STATE_MESSAGEWINDOW_CLOSING;
   }
   if (KEY_DOWN(VK_RIGHT)){
	    if(current_window == 8)
			current_window = 0;

     	Load_Bitmap_File(&Img_MessageWindow, ptr_window[++current_window]);
	    setBusyState(delaytime);
   }
   if (KEY_DOWN(VK_LEFT)){
	    if(current_window == 0)
			current_window = 8;

     	Load_Bitmap_File(&Img_MessageWindow, ptr_window[--current_window]);
        setBusyState(delaytime);
   }

}

void InputModeMessage_JoyPad(){

   // get joystick data
   lpdijoy2->Poll(); // this is needed for joysticks only
   lpdijoy2->GetDeviceState(sizeof(DIJOYSTATE), (LPVOID)&joy_state);

   int delaytime = 1;
   if (joy_state.rgbButtons[0] > 100){
      MESSAGEWINDOW_GAMESTATE = STATE_MESSAGEWINDOW_CLOSING;
   }
   if (joy_state.lX > 10){
	    if(current_window == 8)
			current_window = 0;

     	Load_Bitmap_File(&Img_MessageWindow, ptr_window[++current_window]);
	    setBusyState(delaytime);
   }
   if (joy_state.lX < -10){
	    if(current_window == 0)
			current_window = 8;

        free (Img_MessageWindow.buffer);
     	Load_Bitmap_File(&Img_MessageWindow, ptr_window[--current_window]);
        setBusyState(delaytime);
   }

}