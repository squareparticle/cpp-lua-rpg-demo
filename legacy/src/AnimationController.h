int AnimationPointer = 0;
int cammeraMoveX=0;
int cammeraMoveY=0;
int heroMoveDirection=Jcompas.CENTER;
int AnimationDelay = 0;

bool processAnimation (char * command);

void DoNextFrame(){

    sprintf(strMsgDebug,"%d",scripts[0].key);
	if(AnimationDelay !=0)
		AnimationDelay --;
	else
	   while (processAnimation (mapAnimationBuffer[AnimationPointer++]));
    
	MAP_CAMERAX += cammeraMoveX;
	MAP_CAMERAY += cammeraMoveY;
	
	if (heroMoveDirection != Jcompas.CENTER)
  	   InputModePlayingOverRide(heroMoveDirection);
}

int testForError = 0;
bool processAnimation (char * command){
   char * pstrParse;
   pstrParse = NULL;
   pstrParse = strtok(command, "=");

   bool hasNextCommand = true;

   if (testForError++ == 101){
       hasNextCommand = false;
	   testForError = 0;
	   AnimationPointer = 0;
	   CURRENT_GAMESTATE = STATE_PLAYING;
	   addMessagetoConsole(">> Error in Animation Script");
   }
   
   if(command == NULL){
       hasNextCommand = false;
	   testForError = 0;
	   AnimationPointer = 0;
	   CURRENT_GAMESTATE = STATE_PLAYING;
   }
   else{
      if (pstrParse){
         if(strcmp(command,"CameraMoveX")==0){
            pstrParse = strtok(NULL, "=");

            if (pstrParse)
               cammeraMoveX = atoi(pstrParse);
		 }

         if(strcmp(command,"CameraMoveY")==0){
            pstrParse = strtok(NULL, "=");

            if (pstrParse)
               cammeraMoveY = atoi(pstrParse);
		 }

         if(strcmp(command,"HeroDirection")==0){
            pstrParse = strtok(NULL, "=");

            if (pstrParse){
               if(strcmp(pstrParse,"NORTH")==0)
                  heroMoveDirection = Jcompas.NORTH;
               if(strcmp(pstrParse,"SOUTH")==0)
                  heroMoveDirection = Jcompas.SOUTH;
               if(strcmp(pstrParse,"EAST")==0)
                  heroMoveDirection = Jcompas.EAST;
               if(strcmp(pstrParse,"WEST")==0)
                  heroMoveDirection = Jcompas.WEST;
               if(strcmp(pstrParse,"STOP")==0)
                  heroMoveDirection = Jcompas.CENTER;
			}
		 }

		 if(strcmp(command,"Message")==0){
           sprintf(strMsgDebug,"message");
		   changeState(STATE_MESSAGE,STATE_ANIMATION);
           f_needScreenShot = true;
		 }

         if(strcmp(command,"Delay")==0){
            pstrParse = strtok(NULL, "=");
            hasNextCommand = false;
    	    testForError = 0;

            if (pstrParse)
               AnimationDelay = atoi(pstrParse);
		 }
	  }
   }
   return hasNextCommand;
}
