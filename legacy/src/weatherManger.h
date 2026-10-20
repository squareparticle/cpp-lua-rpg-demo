int bufferMAP_CAMERAX = 0;
int bufferMAP_CAMERAY = 0;
int temp = 0;

void WeatherContoller(){
   switch(CURRENT_WEATHERSTATE){
  	    case NO_WEATHER:
		break;
  	    case WEATHER_RAIN:
           render_RainSprites();
		break;
  	    case WEATHER_SNOW:
           render_SnowSprites();
		break;
  	    case WEATHER_PARTICLE:
           render_Particles();
		break;
  	    case WEATHER_FERRY:
           render_FerrySprites();
		break;
   }
}

void init_RainSprites(){
   for (int i_rainDrop = 0;i_rainDrop < RAIN_MAX;i_rainDrop++){
       rainParticle[i_rainDrop].WindSpeed= RandomRainWindInit();
       rainParticle[i_rainDrop].Velocity= RandomRainFallInit();
	   rainParticle[i_rainDrop].surfaceX = rand() % 4;
   }
}
void init_SnowSprites(){
   for (int i_rainDrop = 0;i_rainDrop < RAIN_MAX;i_rainDrop++){
       rainParticle[i_rainDrop].WindSpeed = RandomSnowWindInit();
       rainParticle[i_rainDrop].Velocity = RandomSnowFallInit();
	   rainParticle[i_rainDrop].surfaceX = 4 + rand() % 4;
   }
}
void init_Particles(){
}

void init_Ferries(){
   for (int i_rainDrop = 0;i_rainDrop < RAIN_MAX;i_rainDrop++){
       rainParticle[i_rainDrop].WindSpeed = RandomFerryWindInit();
       rainParticle[i_rainDrop].Velocity = RandomFerryFallInit();
	   rainParticle[i_rainDrop].surfaceX = 8 + rand() % 4;
   }
}

void render_Particles(){
  UCHAR *back_buffer = NULL;
  UINT loc  = 0;
  int ans =0;
  short int BLUEIMG_1,BLUEIMG_2;
  short int GREENIMG_1,GREENIMG_2;
  short int REDIMG_1,REDIMG_2;

  lpDDSBack->Lock(NULL,&ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
  back_buffer = (UCHAR *)ddsd.lpSurface;

  for (int i_rainDrop = 0;i_rainDrop < RAIN_MAX;i_rainDrop++){
	 ans = MAP_CAMERAX - bufferMAP_CAMERAX;

     rainParticle[i_rainDrop].WindSpeed += newspeed;
	 rainParticle[i_rainDrop].x -= rainParticle[i_rainDrop].WindSpeed + (ans);
     rainParticle[i_rainDrop].y += rainParticle[i_rainDrop].Velocity + (bufferMAP_CAMERAY - MAP_CAMERAY);

     if((rainParticle[i_rainDrop].x < 0) || (rainParticle[i_rainDrop].y > SCREEN_HEIGHT-1) || (rainParticle[i_rainDrop].x > SCREEN_WIDTH-1) || (rainParticle[i_rainDrop].y < 1)){
        temp ++;
		if ((temp % 2) == 0){
		    rainParticle[i_rainDrop].x = rand() % SCREEN_WIDTH;
            rainParticle[i_rainDrop].y = 0;
            rainParticle[i_rainDrop].WindSpeed = 5 + rand() % 5;
            rainParticle[i_rainDrop].Velocity =  5 + rand() % 5;
		}
		else{
            rainParticle[i_rainDrop].x = SCREEN_WIDTH-1;
		    rainParticle[i_rainDrop].y = rand() % SCREEN_HEIGHT;
            rainParticle[i_rainDrop].WindSpeed =  5 + rand() % 5;
            rainParticle[i_rainDrop].Velocity = 5 + rand() % 5;
		}
	 }

     loc = (rainParticle[i_rainDrop].x * 2) + (rainParticle[i_rainDrop].y * ddsd.lPitch);
     BLUEIMG_1 = back_buffer[loc] & 0xf8 >> 3;
     GREENIMG_1 = ((back_buffer[loc] & 0xE0) >> 5) | ((back_buffer[loc+1] & 0x07) << 3);
     REDIMG_1 = (back_buffer[loc+1] & 0xF8) >> 3;

     BLUEIMG_2 = 31;
     GREENIMG_2 = 63;
     REDIMG_2 = 31;

     BLUEIMG_1 = (short)((BLUEIMG_1 + BLUEIMG_2) / 2);
     GREENIMG_1 = (short)((GREENIMG_1 + GREENIMG_2) / 2);
     REDIMG_1 = (short)((REDIMG_1 + REDIMG_2) / 2);

     back_buffer[loc] = (BLUEIMG_1) | ((GREENIMG_1 & 0x07) << 5);
     back_buffer[loc + 1] = ((GREENIMG_1 & 0x38)>>3)|(REDIMG_1 << 3);
     //back_buffer[loc] = back_buffer[loc] & 0x20;
     //back_buffer[loc+1] = back_buffer[loc+1] & 0x20;
  }
  bufferMAP_CAMERAX = MAP_CAMERAX;
  bufferMAP_CAMERAY = MAP_CAMERAY;
  
  //loc = rainParticle[0].x + (rainParticle[0].y * ddsd.lPitch);
  //back_buffer[loc] = 0;
  //sprintf(strMsgDebug,"x = %d, y = %d, ans = %d",rainParticle[0].x, rainParticle[0].y, ans);

  lpDDSBack->Unlock(NULL);
}

void render_RainSprites(){
  RECT clipper;
  for (int i_rainDrop = 0;i_rainDrop < RAIN_MAX;i_rainDrop++){
     //rainParticle[i_rainDrop].WindSpeed += newspeed;
	 rainParticle[i_rainDrop].x -= rainParticle[i_rainDrop].WindSpeed + (MAP_CAMERAX - bufferMAP_CAMERAX);
     rainParticle[i_rainDrop].y += rainParticle[i_rainDrop].Velocity + (bufferMAP_CAMERAY - MAP_CAMERAY);

     if((rainParticle[i_rainDrop].x < 0) || (rainParticle[i_rainDrop].y > SCREEN_HEIGHT-1) || (rainParticle[i_rainDrop].x > SCREEN_WIDTH-1) || (rainParticle[i_rainDrop].y < 1)){
        temp ++;
		if ((temp % 2) == 0){
		    rainParticle[i_rainDrop].x = rand() % SCREEN_WIDTH;
            rainParticle[i_rainDrop].y = 0;
            rainParticle[i_rainDrop].WindSpeed = RandomRainWindInit();
            rainParticle[i_rainDrop].Velocity =  RandomRainFallInit();
   		    rainParticle[i_rainDrop].surfaceX = rand() % 4;
		}
		else{
            rainParticle[i_rainDrop].x = SCREEN_WIDTH-1;
		    rainParticle[i_rainDrop].y = rand() % SCREEN_HEIGHT;
            rainParticle[i_rainDrop].WindSpeed = RandomRainWindInit();
            rainParticle[i_rainDrop].Velocity =  RandomRainFallInit();
		    rainParticle[i_rainDrop].surfaceX = rand() % 4;
		}
	 }
     
     clipper.top = 0;
     clipper.bottom = 8;
     clipper.left = rainParticle[i_rainDrop].surfaceX * 8;
     clipper.right = clipper.left + 8;

	 lpDDSBack->BltFast(rainParticle[i_rainDrop].x,rainParticle[i_rainDrop].y,surf_Weather,&clipper,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);  
  }

  bufferMAP_CAMERAX = MAP_CAMERAX;
  bufferMAP_CAMERAY = MAP_CAMERAY;
}

void render_SnowSprites(){
  RECT clipper;
  for (int i_rainDrop = 0;i_rainDrop < RAIN_MAX;i_rainDrop++){
     //rainParticle[i_rainDrop].WindSpeed += newspeed;
	 rainParticle[i_rainDrop].x -= rainParticle[i_rainDrop].WindSpeed + (MAP_CAMERAX - bufferMAP_CAMERAX);
     rainParticle[i_rainDrop].y += rainParticle[i_rainDrop].Velocity + (bufferMAP_CAMERAY - MAP_CAMERAY);

     if((rainParticle[i_rainDrop].x < 0) || (rainParticle[i_rainDrop].y > SCREEN_HEIGHT-1) || (rainParticle[i_rainDrop].x > SCREEN_WIDTH-1) || (rainParticle[i_rainDrop].y < 1)){
        temp ++;
		if ((temp % 2) == 0){
		    rainParticle[i_rainDrop].x = rand() % SCREEN_WIDTH;
            rainParticle[i_rainDrop].y = 0;
            rainParticle[i_rainDrop].WindSpeed = RandomSnowWindInit();
            rainParticle[i_rainDrop].Velocity =  RandomSnowFallInit();
   		    rainParticle[i_rainDrop].surfaceX = 4 + rand() % 4;
		}
		else{
            rainParticle[i_rainDrop].x = SCREEN_WIDTH-1;
		    rainParticle[i_rainDrop].y = rand() % SCREEN_HEIGHT;
            rainParticle[i_rainDrop].WindSpeed = RandomSnowWindInit();
            rainParticle[i_rainDrop].Velocity =  RandomSnowFallInit();
		    rainParticle[i_rainDrop].surfaceX = 4 + rand() % 4;
		}
	 }
     
     clipper.top = 0;
     clipper.bottom = 8;
     clipper.left = rainParticle[i_rainDrop].surfaceX * 8;
     clipper.right = clipper.left + 8;

	 lpDDSBack->BltFast(rainParticle[i_rainDrop].x,rainParticle[i_rainDrop].y,surf_Weather,&clipper,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);  
  }

  bufferMAP_CAMERAX = MAP_CAMERAX;
  bufferMAP_CAMERAY = MAP_CAMERAY;
}

void render_FerrySprites(){
  RECT clipper;
  for (int i_rainDrop = 0;i_rainDrop < RAIN_MAX;i_rainDrop++){
	 rainParticle[i_rainDrop].x -= rainParticle[i_rainDrop].WindSpeed + (MAP_CAMERAX - bufferMAP_CAMERAX);
     rainParticle[i_rainDrop].y += rainParticle[i_rainDrop].Velocity + (bufferMAP_CAMERAY - MAP_CAMERAY);
     rainParticle[i_rainDrop].surfaceX ++;

	 if (rainParticle[i_rainDrop].surfaceX >= 5 || rainParticle[i_rainDrop].surfaceX < 0)
		 rainParticle[i_rainDrop].surfaceX =  rand() % 5;

     if((rainParticle[i_rainDrop].x < 0) || (rainParticle[i_rainDrop].y > SCREEN_HEIGHT-1) || (rainParticle[i_rainDrop].x > SCREEN_WIDTH-1) || (rainParticle[i_rainDrop].y < 1)){
        temp ++;
		if ((temp % 2) == 0){
		    rainParticle[i_rainDrop].x = rand() % SCREEN_WIDTH;
            rainParticle[i_rainDrop].y = 0;
            rainParticle[i_rainDrop].WindSpeed = RandomFerryWindInit();
            rainParticle[i_rainDrop].Velocity =  RandomFerryFallInit();
   		    rainParticle[i_rainDrop].surfaceX = rand() % 5;
		}
		else{
            rainParticle[i_rainDrop].x = SCREEN_WIDTH-1;
		    rainParticle[i_rainDrop].y = rand() % SCREEN_HEIGHT;
            rainParticle[i_rainDrop].WindSpeed = RandomFerryWindInit();
            rainParticle[i_rainDrop].Velocity =  RandomFerryFallInit();
		    rainParticle[i_rainDrop].surfaceX = rand() % 5;
		}
	 }
     
     if(rainParticle[i_rainDrop].surfaceY <= 10){
        clipper.top = 0;
        clipper.bottom = 9;
	    clipper.left = 64 + 13 + (3 * 9);
        clipper.right = clipper.left + 9;
	 }
     else if(rainParticle[i_rainDrop].surfaceY == 11){
        clipper.top = 0;
        clipper.bottom = 9;
	    clipper.left = 64 + 13 + (FERRY_ANIM[rainParticle[i_rainDrop].surfaceX] * 9);
        clipper.right = clipper.left + 9;
	 }
	 else{
        clipper.top = 0;
        clipper.bottom = 9;
	    clipper.left = 64;
        clipper.right = clipper.left + 13;
	 }

	 lpDDSBack->BltFast(rainParticle[i_rainDrop].x,rainParticle[i_rainDrop].y,surf_Weather,&clipper,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);  
  }

  bufferMAP_CAMERAX = MAP_CAMERAX;
  bufferMAP_CAMERAY = MAP_CAMERAY;
}
