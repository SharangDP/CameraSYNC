/*********************************************************************
   Name of the Program : CAM_SYNC_3


   Function:             Generate a hardware trigger for an IDS 2D Camera,
                         by synchronizing with an IFM 3D camera with the
                         help of CAN Bus Messages.
 **********************************************************************/
#include <SPI.h>
#include "mcp_can_2.h"
#define NOE 10

const int SPI_CS_PIN = 10;
const int TRIG_PIN = 7;
MCP_CAN CAN(SPI_CS_PIN);

/*********************GLOBAL Variables*************************/

unsigned char Buf1[7];
unsigned long CAN_ID1;
unsigned long SYNC[10];
unsigned long GLOB[10];
unsigned long T_ard[10];
unsigned char len1 = 0;
unsigned long Lag_time = 0;
unsigned long cyc_time = 0;
unsigned long next_trigger_time;
unsigned long count = 0;
unsigned long n = 1;
unsigned long repeat_frequence = 0;
unsigned char RESET = 1;
unsigned long calib_delay = 10000;



/**************Initialize CAN communication at 250KBPS. Initialize CAN Mask and Filter and Trigger Pin.***********/

void setup()
{

  Serial.begin(115200);

  while (CAN_OK != CAN.begin(CAN_250KBPS))              //baudrate = 250k
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println(" Init CAN BUS Shield again");
    delay(100);
  }
  Serial.println("CAN BUS Shield init ok!");

  CAN.init_Mask(0, 1, 0x3FFFFFFF);
  CAN.init_Mask(1, 1, 0x3FFFFFFF);

  CAN.init_Filt(0, 1, 83820783);
  CAN.init_Filt(1, 1, 83821039);
  pinMode(TRIG_PIN, OUTPUT);
  timer_init();
  timer_set();
  sei();
}

/***************************Initialize the Timer 1*******************/
void timer_init()
{
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
}
void timer_set()
{
  OCR1A = 0x7A11;
  TCCR1B |= (1 << WGM12);
  TIMSK1 |= (1 << OCIE1A);
}

void timer_start()
{
  TCCR1B |= (1 << CS12) | (1 << CS10) ;
}

void timer_reset()
{
  TCNT1 = 0;
}

/***************************Stores data in CAN Buffer*******************/

void read_can()
{
  if (CAN.readMsgBufID(&CAN_ID1, &len1, Buf1) == CAN_OK)
  {
    // read CAN
  }
}

/***************************Returns the SYNC_TIME from CAN Buffer*******************/

unsigned long read_sync_msg()
{
  unsigned long retval;
  if (CAN_ID1 == 83820783)
  {

    retval = (pow(256, 3) * Buf1[3]) + (65536 * Buf1[2]) + (256 * Buf1[1]) + (Buf1[0]);
  }
  return retval;
}

/***************************Returns the GLOB_TIME from CAN Buffer*******************/

unsigned long read_glob_msg()
{
  unsigned long retval;
  if (CAN_ID1 == 83821039)

  {
    retval = (pow(256, 3) * Buf1[3]) + (pow(256, 2) * Buf1[2]) + (pow(256, 1) * Buf1[1]) + (pow(256, 0) * Buf1[0]);
  }

  return retval;
}

/*********************Initialize the Time Array ********************/

void get_time()
{
  int i = 0;
  int j = 0;
  unsigned long t, temp1, temp2;
  Serial.print("\nInitializing");

  while (i < NOE || j < NOE)
  {

    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
      t = micros();
      Serial.print(". ");
      read_can();
      temp1 = read_sync_msg();
      temp2 = read_glob_msg();
      if (CAN_ID1 == 83820783 && i < NOE)
      {

        SYNC[i] = temp1;
        T_ard[i] = t;
        i++;
        /*Serial.print(" #");
          Serial.print(temp1);
          Serial.print("#");
          Serial.print(t);*/
      }

      if (CAN_ID1 == 83821039 && j < NOE && temp2 != 0)

      {
        GLOB[j] = temp2;
        j++;
        /*Serial.print(" !");
          Serial.print(temp2);*/
      }
    }
  }

}

/*********************** Returns the minimum value of lag time ****************/

unsigned long get_lag_min()
{
  unsigned long retval = 0;
  unsigned long temp;
  for (int i = 1; i < NOE; i++)
  {
    temp = (SYNC[i] - T_ard[i - 1]);

    if (i == 1 || temp < retval)
    {
      retval = temp;
    }
  }

  return retval;
}

/*********************** Returns the minimum value of cyc time ****************/

unsigned long get_cyc_min()
{
  unsigned long retval = 0;
  unsigned long temp;
  for (int i = 1; i < NOE; i++)
  {
    temp = (GLOB[i] - GLOB[i - 1]);
    if (i == 1 || temp < retval)
    {
      retval = temp;
    }
  }
  return retval;

}

/***************************Concatenate New SYNC_TIME Values ***********************/

void concat_sync(unsigned long syncval)
{
  for (int i = 0; i < NOE - 1; i++)
  {
    SYNC[i] = SYNC[i + 1];
  }
  SYNC[9] = syncval;
}

/***************************Concatenate New GLOB_TIME Values ***********************/

void concat_glob(unsigned long globval)
{
  for (int i = 0; i < NOE - 1; i++)
  {
    GLOB[i] = GLOB[i + 1];
  }
  GLOB[9] = globval;
}

/***************************Concatenate New T_Ard Values ***********************/

void concat_T_ard(unsigned long tval)
{
  for (int i = 0; i < NOE - 1; i++)
  {
    T_ard[i] = T_ard[i + 1];

  }
  T_ard[9] = tval;
}

/******************* Evaluate CAN in the while loop **********************/

void evaluate_can()
{
  unsigned long temp;

  if (CAN_MSGAVAIL == CAN.checkReceive())
  {
    timer_reset();
    temp = micros();
    read_can();
    if (CAN_ID1 == 83820783)
    {
      concat_T_ard(temp);
      concat_sync(read_sync_msg());
    }
    else if (CAN_ID1 == 83821039)
    {
      concat_glob(read_glob_msg());
    }
  }
}

/*******************Time Compare Function*************/

int t1_past_t2(unsigned long t1, unsigned long t2)
/* returns true, if t1 "past" t2 */
{
  return t1 - t2 < 0x80000000;
}

/********************* Get Next Trigger **************************/
unsigned long get_next_trigger()
{

  unsigned long Glob_Ard, Glob_Ard_Next;
  Lag_time = get_lag_min();
  cyc_time = get_cyc_min();
  Glob_Ard = GLOB[9] - Lag_time + calib_delay;

  n = 1;
  Glob_Ard_Next = Glob_Ard; /* n == 0 */
  /*Serial.print("GetNextTrigger: LagTime=");
    Serial.print(Lag_time);
    Serial.print("\tcycTime=");
    Serial.print(cyc_time);
    Serial.print("\tGlobArd=");
    Serial.print(Glob_Ard);
    Serial.print("\tGlobArdNext=");
    Serial.print(Glob_Ard_Next);
    Serial.print("\tmicros=");
    Serial.println(micros());*/

  while (t1_past_t2(micros(), Glob_Ard_Next))
  {
    Glob_Ard_Next = Glob_Ard + (n * cyc_time);
    n++;// Limit on N
  }
  return  Glob_Ard_Next;
}

/************** Generate Trigger Function ****************/

void gen_trigger()
{

  count++;
  digitalWrite(TRIG_PIN, HIGH);
  delay(1);
  digitalWrite(TRIG_PIN, LOW);
  Serial.print("\nTrigger Number: ");
  Serial.print(count);
  Serial.print("\tLag Time: ");
  Serial.print(Lag_time);
  Serial.print("\t");
  Serial.print(n);
  Serial.print("\t");
  Serial.print(millis());
  Serial.print("\t");
  Serial.print(repeat_frequence);
}

/*********************************Interrupt Service Routine for Timer 1*********************************/
ISR(TIMER1_COMPA_vect)
{
  RESET = 0;
}

/*********************************MAIN LOOP*********************************/

void loop() {
  RESET = 1;
  unsigned long t_now;
  unsigned long t_start = micros();
  unsigned long cnt = 0;
  get_time();
  Serial.print("I am outside the while loop");
  next_trigger_time = get_next_trigger();
  timer_start();
  while (1 && RESET)
  {
    evaluate_can();
    t_now = micros();
    if (  t1_past_t2(t_now, next_trigger_time)  )
    {
      gen_trigger();
      next_trigger_time = get_next_trigger();
    }
    cnt += 1;
    repeat_frequence = (float)cnt * 1000000.f / (float)(t_now - t_start);
    if ( t_now - t_start > 20000000 )
    {
      cnt = 0;
      t_start = t_now;
    }
  }
}

/**********************************END OF PROGRAM******************************/

/*  Task 1: IDS Camera and Arduino only to be tried first.

    Task 2: To create GitHub Repository and upload relevant library and code files.

    Task 3: Tool for Picture comparison(Image dump tool from both Cameras) -- Christoph

    Task 4: Test with Trigger with O3M and IDS Cables connected and a moving target.

    Task 5: To build a function to glow LED in case the cameras are synchronized.

    Task 6: Re-intialization after loosing the CAN messages for a pre-determined time.
*/


