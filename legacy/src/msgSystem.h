/*
Name: msgSystem.h
Created: March 10 2002

Description:
	This header was created to write debuging messages to the screen.
*/

void displayDebugNone();
void displayDebugHero();
void displayDebugMap();
void AppOutputText(LPDIRECTDRAWSURFACE4 pddsRenderSurface, DWORD x, DWORD y, CHAR* str );

void displayDebugNone(){
   sprintf(strMsg, "Debug mode on!");
   AppOutputText(lpDDSBack, 0, 0, strMsg);

   sprintf(strMsg, "Message Buffer = %s",strMsgDebug);
   AppOutputText(lpDDSBack, 0, 20, strMsg);
}

void displayDebugHero(){
   sprintf(strMsg, "Hero Health = 100");
   AppOutputText(lpDDSBack, 0, 0, strMsg);
   sprintf(strMsg, "Hero Strength = 10");
   AppOutputText(lpDDSBack, 0, 15, strMsg);
   sprintf(strMsg, "Hero Defence = 12");
   AppOutputText(lpDDSBack, 0, 30, strMsg);
   sprintf(strMsg, "Hero Speed = 4");
   AppOutputText(lpDDSBack, 0, 45, strMsg);
   sprintf(strMsg, "Hero Xloc = %d",heroStruct.map.x);
   AppOutputText(lpDDSBack, 0, 60, strMsg);
   sprintf(strMsg, "Hero Yloc = %d",heroStruct.map.y);
   AppOutputText(lpDDSBack, 0, 75, strMsg);
   sprintf(strMsg, "Frame = %d",heroStruct.currentFrame);
   AppOutputText(lpDDSBack, 0, 90, strMsg);
  
   if(heroStruct.direction == NORTH){
      sprintf(strMsg, "Direction = %s","North");
      AppOutputText(lpDDSBack, 0, 105, strMsg);
   }
   if(heroStruct.direction == SOUTH){
      sprintf(strMsg, "Direction = %s","South");
      AppOutputText(lpDDSBack, 0, 105, strMsg);
   }
   if(heroStruct.direction == EAST){
      sprintf(strMsg, "Direction = %s","East");
      AppOutputText(lpDDSBack, 0, 105, strMsg);
   }
   if(heroStruct.direction == WEST){
      sprintf(strMsg, "Direction = %s","West");
      AppOutputText(lpDDSBack, 0, 105, strMsg);
   }
}

void displayDebugMap(){
   sprintf(strMsg, "Script = %s",MAP_SCRIPTNAME);
   AppOutputText(lpDDSBack, 0, 0, strMsg);
   sprintf(strMsg, "MAP_CAMERAX = %d",MAP_CAMERAX);
   AppOutputText(lpDDSBack, 0, 15, strMsg);
   sprintf(strMsg, "MAP_CAMERAY  = %d",MAP_CAMERAY);
   AppOutputText(lpDDSBack, 0, 30, strMsg);
}

void AppOutputText(LPDIRECTDRAWSURFACE4 pddsRenderSurface, DWORD x, DWORD y, CHAR* str ){

    HDC hDC;
    if(SUCCEEDED(pddsRenderSurface->GetDC(&hDC))){
        SetTextColor( hDC, RGB(255,255,0) );
        SetBkMode( hDC, TRANSPARENT );
        ExtTextOut( hDC, x, y, 0, NULL, str, strlen(str), NULL );
    
        pddsRenderSurface->ReleaseDC(hDC);
    }
}