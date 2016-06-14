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
static u32 UserApp_u32Timeout;                      /* Timeout counter used across states */
static u8 au8TestMessage[1]={00};
static u8 au8DataContent[] = "xx";
static u8 UserApp_CursorPosition;
u8 counter=0;
u8 au8Message[] = "The   accent  is :";
u8 au8Message1[] = "send  up  down  ";
u8 counter1=0;
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
  UserApp_CursorPosition = LINE1_START_ADDR+19;
  LCDMessage(LINE1_START_ADDR, au8Message);
  LCDCommand(LINE1_START_ADDR+19);  
  LCDMessage(LINE2_START_ADDR, au8Message1); 
  LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR | LCD_DISPLAY_BLINK);

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
    counter++;
  if(counter==50)   
  {
    
    counter=0;
    au8DataContent[0]=au8TestMessage[0]/10+0x30;
    au8DataContent[1]=(au8TestMessage[0]%10)+0x30;
    LCDMessage(LINE1_START_ADDR+18, au8DataContent);
   
  if( WasButtonPressed(BUTTON1) )
  {
    ButtonAcknowledge(BUTTON1);
    if(UserApp_CursorPosition==LINE1_START_ADDR+18)
    {
        au8TestMessage[0]=au8TestMessage[0]+10;
    }
    else
      au8TestMessage[0]=au8TestMessage[0]+1;
  }
  if( WasButtonPressed(BUTTON2) )
  {
    ButtonAcknowledge(BUTTON2);
   if(UserApp_CursorPosition==LINE1_START_ADDR+18)
    {
        au8TestMessage[0]=au8TestMessage[0]-10;
    }
    else
      au8TestMessage[0]=au8TestMessage[0]-1;
  } 
  if(AntRadioStatus()==ANT_OPEN )///////////panduanant
  {
    LedOff(RED);
    LedOn(BLUE);
   }
  
  else
  {
    LedOff(BLUE);
    LedOn(RED);
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
      AntQueueBroadcastMessage(au8TestMessage);
      au8TestMessage[0]=00;
    }
   }
 }
 } 
  if(WasButtonPressed(BUTTON3))
  {
    ButtonAcknowledge(BUTTON3);
    
    /* Handle the two special cases or just the regular case */
    if(UserApp_CursorPosition == LINE1_START_ADDR+19)
    {
      UserApp_CursorPosition = LINE1_START_ADDR+18;
      
    }
    else
    {
      UserApp_CursorPosition++;
    }
    /* New position is set, so update */
  } /* end AntReadData() */  
  
  /* end BUTTON3 */  
} /* end UserAppSM_Idle() */ 
   
 
 

     
    
 
    
  
 

    
  

     

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
