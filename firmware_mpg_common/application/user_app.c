/**********************************************************************************************************************
File: user_app.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app as a template:
 1. Copy both user_app.c and user_app.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserAppInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserAppRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserAppFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern AntSetupDataType G_stAntSetupData;                         /* From ant.c */

extern u32 G_u32AntApiCurrentDataTimeStamp;                       /* From ant_api.c */
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;    /* From ant_api.c */
extern u8 G_au8AntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES];  /* From ant_api.c */

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp_StateMachine;            /* The state machine function pointer */
static u32 UserApp_u32Timeout;                      /* Timeout u8counter used across states */
static u8 au8TestMessage[1]={0};
static u8 au8TestMessage1[1]={0};
static u8 au8DataContent[1] = {0};
static u16 u16frequency[] = {0,523,586,658,697,783,879,987,262,293,329,349,392,440,494,1045,1171,1316,1393,1563,1755,1971};
static u8 u8letter[]={'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','\0'}; 
static u8 u8letter4[]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','\0'}; 
static u8 u8letter1[]="A";
static u8 u8letter2[]={50};            /*save fixed digital ,it is signal to send*/
static u8 u8letter3[]={200};            /*save fixed digital ,it is signal to send*/
static u8 u8au8TestMessage2[1]={30};      /*save fixed digital ,it is signal to send*/
static u8 u8u8countername=0;              
static u8 u8u8countername1=0;
static u8 u8au8Message[] = "The  accent is :";
static u8 u8au8Message1[] = "send  up  low  high";
static u8 u8au8Message2[] = "please input name:";
static u8 u8au8Message3[] = "Back  up  small Send";
static u8 u8au8Message4[] = "H";
static u8 u8au8Message5[] = "L";
static u8 u8counter1buzzeroff=0;
static u8 u8counter=0;
static u8 u8counter2=1;                     /*to judge digital*/
static u8 u8counter3timer=0;
static u8 u8counter4=1;                     /*to judge digital*/
static u8 u8matchconserver=0;                        /*to conserver*/
static u8 u8matchconserver1=0;
/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserAppInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserAppInitialize(void)
{
  
  /* Configure ANT for this application */
  G_stAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  G_stAntSetupData.AntSerialLo         = ANT_SERIAL_LO_USERAPP;
  G_stAntSetupData.AntSerialHi         = ANT_SERIAL_HI_USERAPP;
  G_stAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  G_stAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  G_stAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  G_stAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  G_stAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  G_stAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, u8au8Message);
  LCDMessage(LINE2_START_ADDR, u8au8Message1); 
  PWMAudioOn(BUZZER1);
  /* If good initialization, set state to Idle */
  if( AntChannelConfig(ANT_MASTER) )
  {
    AntOpenChannel();
    UserApp_StateMachine = UserAppSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp_StateMachine = UserAppSM_FailedInit;
  }
} /* end UserAppInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserAppRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserAppRunActiveState(void)
{
  UserApp_StateMachine();

} /* end UserAppRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserAppSM_Idle(void)
{
   u8counter++;
   u8counter1buzzeroff++;
   /*control buzzer*/
   if( u8counter1buzzeroff==200)
   {
     u8counter1buzzeroff=0;
     PWMAudioSetFrequency(BUZZER1, 0); 
   }
   /* main looper*/
   if(u8counter==20)
   {
     u8counter=0;
     au8TestMessage1[0]=au8TestMessage[0];
     if(au8TestMessage[0]>7&&au8TestMessage[0]<15)
     {
        au8TestMessage1[0]=au8TestMessage1[0]-7;
     }
     if(au8TestMessage[0]>14&&au8TestMessage[0]<22)
     {
       au8TestMessage1[0]=au8TestMessage1[0]-14;
     }
     au8DataContent[0]=au8TestMessage1[0]+0x30;
     LCDMessage(LINE1_START_ADDR+19, au8DataContent);
     /* increase the accent*/
    if( WasButtonPressed(BUTTON1) )
    {
      ButtonAcknowledge(BUTTON1);
      au8TestMessage[0]=au8TestMessage[0]+1;
      if(au8TestMessage[0]==8)
      {
        au8TestMessage[0]=0;
      }
    }
   /*it will let the accent become low*/
  if( WasButtonPressed(BUTTON2) )
  {
    ButtonAcknowledge(BUTTON2);
    LCDMessage(LINE1_START_ADDR+17, u8au8Message5);
    au8TestMessage[0]=au8TestMessage[0]+7;
  } 
  /*it will let the accent become high*/
  if( WasButtonPressed(BUTTON3) )
  {
    ButtonAcknowledge(BUTTON3);
    LCDMessage(LINE1_START_ADDR+17, u8au8Message4);
    au8TestMessage[0]=au8TestMessage[0]+14;
  } 
  /* Check all the buttons and update au8TestMessage according to the button state */ 
  if( AntReadData() )
  {
    /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      
      /* We got some data */
    }
    else if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      
     if( WasButtonPressed(BUTTON0) )
    {
      ButtonAcknowledge(BUTTON0);
      LCDClearChars(LINE1_START_ADDR + 17, 1);
      if(u8matchconserver==au8TestMessage[0])
        {
              u8counter2++;
        }
      else
      {
        u8counter2=1;
      }
       u8matchconserver=au8TestMessage[0];
    switch(au8TestMessage[0])
      {
        case 0: /* white */
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOn(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
       case 1: /* purple */
          LedOn(LCD_RED);
          LedOff(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOn(PURPLE);
          LedOff(WHITE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
        case 2: /* blue */
          LedOff(LCD_RED);
          LedOff(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOn(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
        case 3: /* cyan */
          LedOff(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOn(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1,u16frequency[au8TestMessage[0]]);
          break;
        case 4: /* green */
          LedOff(LCD_RED);
          LedOn(LCD_GREEN);
          LedOff(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOn(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
        case 5: /* yellow */
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOff(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOn(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
        case 6: /* red */
          LedOn(LCD_RED);
          LedOff(LCD_GREEN);
          LedOff(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOn(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
       case 7: /*WHITE*/
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOn(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
       case 8: /* purple */
          LedOn(LCD_RED);
          LedOff(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOn(PURPLE);
          LedOff(WHITE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
      case 9: /* blue */
          LedOff(LCD_RED);
          LedOff(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOn(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
     case 10: /* cyan */
          LedOff(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOn(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1,u16frequency[au8TestMessage[0]]);
          break;
     case 11: /* green */
          LedOff(LCD_RED);
          LedOn(LCD_GREEN);
          LedOff(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOn(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
     case 12: /* yellow */
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOff(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOn(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
      case 13: /* red */
          LedOn(LCD_RED);
          LedOff(LCD_GREEN);
          LedOff(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOn(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
      case 14: /*WHITE*/
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOn(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
      case 15: /* purple */
          LedOn(LCD_RED);
          LedOff(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOn(PURPLE);
          LedOff(WHITE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
      case 16: /* blue */
          LedOff(LCD_RED);
          LedOff(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOn(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
      case 17: /* cyan */
          LedOff(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOn(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1,u16frequency[au8TestMessage[0]]);
          break;
      case 18: /* green */
          LedOff(LCD_RED);
          LedOn(LCD_GREEN);
          LedOff(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOn(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
     case 19: /* yellow */
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOff(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOn(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
     case 20: /* red */
          LedOn(LCD_RED);
          LedOff(LCD_GREEN);
          LedOff(LCD_BLUE);
          LedOff(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOn(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
     case 21: /*WHITE*/
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          LedOn(WHITE);
          LedOff(PURPLE);
          LedOff(BLUE);
          LedOff(CYAN);
          LedOff(GREEN);
          LedOff(YELLOW);
          LedOff(ORANGE);
          PWMAudioSetFrequency(BUZZER1, u16frequency[au8TestMessage[0]]);
          break;
      } /* end switch */
      if(u8counter2%2==0)
      {
        AntQueueBroadcastMessage(u8au8TestMessage2);
      }
      else
      {
      AntQueueBroadcastMessage(au8TestMessage);
      }
      au8TestMessage[0]=0;
    }
   }
 }
 /*change the place of number*/
/*go to input letter*/
 if( IsButtonHeld(BUTTON3, 2000) )
  {
    LedOn(LCD_RED);
    LedOn(LCD_GREEN);
    LedOn(LCD_BLUE);
    LedOff(WHITE);
    LedOff(PURPLE);
    LedOff(BLUE);
    LedOff(CYAN);
    LedOff(GREEN);
    LedOff(YELLOW);
    LedOff(ORANGE);
    LedOff(RED);
    LCDCommand(LCD_CLEAR_CMD);
    au8TestMessage[0]=au8TestMessage[0]-14;
    LCDMessage(LINE1_START_ADDR, u8au8Message2);
    LCDMessage(LINE2_START_ADDR, u8au8Message3);
    LCDMessage(LINE1_START_ADDR+19, u8letter1);
    u8counter3timer=0;
    AntQueueBroadcastMessage(u8letter2);
    UserApp_StateMachine = UserAppSM_namemoudle;
    u8counter=0;
   }
} /* end UserAppSM_Idle() */ 
}


/*-------------------------------------------------------------------------------------------------------------------*/
/* change letter and send lettter */
static void UserAppSM_namemoudle(void)
{
  u8counter3timer++;
  u8counter1buzzeroff++;
   if( u8counter1buzzeroff==200)
   {
     u8counter1buzzeroff=0;
     PWMAudioSetFrequency(BUZZER1, 0); 
   }
  if(u8counter3timer==20)
  {
    u8counter3timer=0;
   LCDMessage(LINE1_START_ADDR+19, u8letter1);
    if(WasButtonPressed(BUTTON1))
    { 
      ButtonAcknowledge(BUTTON1);
      u8u8countername++;
       u8letter1[0]=u8letter[u8u8countername-1];
       
       u8u8countername1=u8u8countername-1;
       if(u8u8countername==26)
       {
         u8u8countername=0;
       }
     }   
    if(WasButtonPressed(BUTTON2))
    { 
      ButtonAcknowledge(BUTTON2);
      u8u8countername1++;
      u8letter1[0]=u8letter4[u8u8countername1-1];
      
      if(u8u8countername1==26)
       {
         u8u8countername1=0;
       } 
      u8u8countername=u8u8countername1-1;
    }
   if( AntReadData() )
  {
    /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      
      /* We got some data */
    }
    else if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      
     if( WasButtonPressed(BUTTON3) )
    {
      ButtonAcknowledge(BUTTON3);
      PWMAudioSetFrequency(BUZZER1, u16frequency[7]);
     if(u8matchconserver1== u8letter1[0])
        {
           u8counter4++;
        }
      else
      {
        u8counter4=1;
      }
       u8matchconserver1=u8letter1[0];
      if(u8counter4%2==0)
      {
        AntQueueBroadcastMessage(u8au8TestMessage2);
      }
      else
      {
      AntQueueBroadcastMessage(u8letter1);
      }
     }
   }
}
/*Back to input number*/
 if( WasButtonPressed(BUTTON0))
  {
   ButtonAcknowledge(BUTTON0);
    u8counter=0;
    u8counter1buzzeroff=0;
    AntQueueBroadcastMessage(u8letter3);
     LCDCommand(LCD_CLEAR_CMD);
     LCDMessage(LINE1_START_ADDR, u8au8Message);
    LCDMessage(LINE2_START_ADDR, u8au8Message1); 
    UserApp_StateMachine = UserAppSM_Idle;
  }
 }
}      
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_Error(void)          
{
  
} /* end UserAppSM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserAppSM_FailedInit(void)          
{
    
} /* end UserAppSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
