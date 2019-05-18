/*
 * REVISION HISTORY
 * 
 * 300
 * Original INO file lost
 * Rewiritten on 13/05/2017
 * Eric Brouwer
 * 
 * To accomodate the SIM00L on an Arduino UNO, SIM800 is now
 * connected to a software serial port.
 * Changes to wiring is needed.
 * SIM800 RX pin to Arduino UNO pin 2
 * SIM800 TX pin to Arduino UNO pin 3
 * SIM800 RESET  to Arduino UNO pin 4 (although this pin is not realy required)
 * 
 * 
*/

//========================================================================================
//========================================================================================
//
// ! ! ! NOTE ! ! !
// ================
//
//   BUFFER IN SOFTWARE SERIAL LIBRARY TO BE CHANGED TO 200
//
//   To use Software Serial, the user should also change the Software Serial BUFFER to 200.
//   This must be edited in the following file:
//
//   C:/Program Files/Arduino x.x.x/Hardware/Arduino/AVR/Libraries/SoftwareSerial/src/SoftWareSerial.h
//  
//   Change buffer as below:
//
//   #ifndef _SS_MAX_RX_BUFF
//   #define _SS_MAX_RX_BUFF 200
//   // RX buffer size
//   #endif
//
//========================================================================================
//========================================================================================

#include <SoftwareSerial.h>

// ! ! !  CHANGE THIS NUMBER TO YOUR OUW NUMBER  ! ! !
//----------------------------------------------------
#define Your_Nr "xxxxxxxxxx"

// Define I/O pins
//-----------------------------------------------
#define SIM800_Tx      2  //SIM800 serial port Tx
#define SIM800_Rx      3  //SIM800 Serial port Rx
#define SIM800_Reset   4  //SIM800 Reset pin

// Create an instance of software serial port
//-----------------------------------------------
SoftwareSerial SSerial(SIM800_Tx, SIM800_Rx); // RX, TX

#define Input1 A5  //Input 1
#define Input2 A4  //Input 2
#define Input3 A3  //Input 3
#define Input4 A2  //Input 4
#define Output1 5  //Output 1 
#define Output2 6  //Output 2
#define Output3 7  //Output 3
#define Output4 8  //Output 4

#define LedG    9  //Tx LED
#define LedR   10  //Rx LED 

#define Req_Update    F("????")               // update request 
#define Req_Reset     F("####")               // Save channels that must pulse

// Define program variables
//-----------------------------------------------
String        RxString  = "";
char          RxChar    = ' ';
int           Counter   = 0;
boolean       Connected = false;
String        GSM_Nr    = "100000000000000";
String        GSM_Msg   = "300000000000000";
String        Master_Nr = Your_Nr;

boolean       In1       = 0;
boolean       In1_old   = 0;
boolean       In1_A     = 0;
boolean       In2       = 0;
boolean       In2_old   = 0;
boolean       In2_A     = 0;
boolean       In3       = 0;
boolean       In3_old   = 0;
boolean       In3_A     = 0;
boolean       In4       = 0;
boolean       In4_old   = 0;
boolean       In4_A     = 0;

boolean       UpdateSMS = 0;
String        StatusString;

//###############################################################################################
// SETUP
// 
//###############################################################################################
void setup() {

  // Setup I/Os
  //-----------
  pinMode(Input1,INPUT_PULLUP);
  pinMode(Input2,INPUT_PULLUP);
  pinMode(Input3,INPUT_PULLUP);
  pinMode(Input4,INPUT_PULLUP);
  pinMode(Output1,OUTPUT);
  pinMode(Output2,OUTPUT);
  pinMode(Output3,OUTPUT);
  pinMode(Output4,OUTPUT);
  pinMode(LedR,OUTPUT);
  pinMode(LedG,OUTPUT);
  
  digitalWrite(Output1,LOW);
  digitalWrite(Output2,LOW);
  digitalWrite(Output3,LOW);
  digitalWrite(Output4,LOW);
  digitalWrite(LedR,LOW);
  digitalWrite(LedG,LOW);

  // Start SSerial ports
  //-------------------
  SSerial.begin(9600);
  Serial.begin(9600);
    
  // Initialise SIM800
  //------------------
  InitGSM();

  // Read inputs for reference
  //--------------------------
  // read startup status
  In1 = !digitalRead(Input1);
  In1_old = In1;
  In1_A = 0;
  In2 = !digitalRead(Input2);
  In2_old = In2;
  In2_A = 0;
  In3 = !digitalRead(Input3);
  In3_old = In3;
  In3_A = 0;
  In4 = !digitalRead(Input4);
  In4_old = In4;
  In4_A = 0;

  UpdateSMS = 0;
  delay(5000);
}


//###############################################################################################
// MAIN LOOP
// 
//###############################################################################################
void loop() {

  // scan for data from software SSerial port
  //-----------------------------------------------
  RxString = "";
  Counter = 0;
  while(SSerial.available()){
    digitalWrite(LedR,HIGH);
    delay(1);  // short delay to give time for new data to be placed in buffer
    // get new character
    RxChar = char(SSerial.read());
    //add first 200 character to string
    if (Counter < 200) {
      RxString.concat(RxChar);
      Counter = Counter + 1;
    }
  }
  if (digitalRead(LedR) == 1) {
    delay(50);
    digitalWrite(LedR,LOW);
  }

  // Is there a new incoming call? 
  //-----------------------------------------------
  if (Received(F("RING")) ) GetCall();
  
  // Is there a new SMS?
  //-----------------------------------------------
  if (Received(F("CMT:")) ) GetSMS();
  
  // Scan inputs
  //-----------------------------------------------
  ScanInputs();

  //See if Update must be send
  //-----------------------------------------------
  if (UpdateSMS == 1) {
    UpdateSMS = 0;
    SendSMS();  
  }

}


//###############################################################################################
// Scan Inputs
// 
//###############################################################################################
void ScanInputs() {

  //Input 1
  //=======
  In1 = !digitalRead(Input1);
  if (In1 != In1_old) {
    In1_old = In1;
    if (In1 == 1) {
      if (In1_A == 0) {
        In1_A = 1;  //turn alarm status on
        UpdateSMS = 1;
      }
    }
  }
  //Input 2
  //=======
  In2 = !digitalRead(Input2);
  if (In2 != In2_old) {
    In2_old = In2;
    if (In2 == 1) {
      if (In2_A == 0) {
        In2_A = 1;  //turn alarm status on
        UpdateSMS = 1;
      }
    }
  }
  //Input 3
  //=======
  In3 = !digitalRead(Input3);
  if (In3 != In3_old) {
    In3_old = In3;
    if (In3 == 1) {
      if (In3_A == 0) {
        In3_A = 1;  //turn alarm status on
        UpdateSMS = 1;
      }
    }
  }
  //Input 4
  //=======
  In4 = !digitalRead(Input4);
  if (In4 != In4_old) {
    In4_old = In4;
    if (In4 == 1) {
      if (In4_A == 0) {
        In4_A = 1;  //turn alarm status on
        UpdateSMS = 1;
      }
    }
  }

}


//###############################################################################################
// Get SMS Content
// 
//###############################################################################################
void GetSMS() {
  
  //Get SMS number
  //================================================
  GSM_Nr  = RxString;
  //get number
  int t1 = GSM_Nr.indexOf('"');
  GSM_Nr.remove(0,t1 + 1);
  t1 = GSM_Nr.indexOf('"');
  GSM_Nr.remove(t1);
   
  // Get SMS message
  //================================================
  GSM_Msg = RxString;
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  t1 = GSM_Msg.indexOf('"');
  GSM_Msg.remove(0,t1 + 1);
  GSM_Msg.remove(0,1);
  GSM_Msg.trim();

  RxString = GSM_Msg;

  //Output 1
  //========
  if (Received("o1:1")){
    digitalWrite(Output1,HIGH);
    UpdateSMS = 1;
  }
  if (Received("o1:0")){
    digitalWrite(Output1,LOW);
    UpdateSMS = 1;
  }
  if (Received("o1:p")){
    digitalWrite(Output1,HIGH);
    delay(1000);
    digitalWrite(Output1,LOW);
    UpdateSMS = 1;
  }
  //Output 2
  //========
  if (Received("o2:1")){
    digitalWrite(Output2,HIGH);
    UpdateSMS = 1;
  }
  if (Received("o2:0")){
    digitalWrite(Output2,LOW);
    UpdateSMS = 1;
  }
  if (Received("o2:p")){
    digitalWrite(Output2,HIGH);
    delay(1000);
    digitalWrite(Output2,LOW);
    UpdateSMS = 1;
  }
  //Output 3
  //========
  if (Received("o3:1")){
    digitalWrite(Output3,HIGH);
    UpdateSMS = 1;
  }
  if (Received("o3:0")){
    digitalWrite(Output3,LOW);
    UpdateSMS = 1;
  }
  if (Received("o3:p")){
    digitalWrite(Output3,HIGH);
    delay(1000);
    digitalWrite(Output3,LOW);
    UpdateSMS = 1;
  }
  //Output 4
  //========
  if (Received("o4:1")){
    digitalWrite(Output4,HIGH);
    UpdateSMS = 1;
  }
  if (Received("o4:0")){
    digitalWrite(Output4,LOW);
    UpdateSMS = 1;
  }
  if (Received("o4:p")){
    digitalWrite(Output4,HIGH);
    delay(1000);
    digitalWrite(Output4,LOW);
    UpdateSMS = 1;
  }

  //Status Request
  //==============
  if (Received("????")){
    UpdateSMS = 1;
  }

  //Reset Alarms
  //============
  if (Received("####")){
    In1_A = !digitalRead(Input1);
    In2_A = !digitalRead(Input2);
    In3_A = !digitalRead(Input3);
    In4_A = !digitalRead(Input4);
    UpdateSMS = 1;
  }  
}


//###############################################################################################
// Send SMS 
// 
//###############################################################################################
void SendSMS() {
  
  digitalWrite(LedG,HIGH);
  
  //Send Status SMS
  StatusString = " ";
  
  //Alarm 1
  //=======
  if (In1_A == 1) StatusString = StatusString + "i1:1 "; else StatusString = StatusString + "i1:0 ";

  //Alarm 2
  //=======
  if (In2_A == 1) StatusString = StatusString + "i2:1 "; else StatusString = StatusString + "i2:0 ";
 
  //Alarm 3
  //=======
  if (In3_A == 1) StatusString = StatusString + "i3:1 "; else StatusString = StatusString + "i3:0 ";
 
  //Alarm 4
  //=======
  if (In4_A == 1) StatusString = StatusString + "i4:1 "; else StatusString = StatusString + "i4:0 ";

  
  //Output 1
  //========
  if (digitalRead(Output1) == 0) StatusString = StatusString + "o1:0 "; else StatusString = StatusString + "o1:1 ";
 
  //Output 2
  //========
  if (digitalRead(Output2) == 0) StatusString = StatusString + "o2:0 "; else StatusString = StatusString + "o2:1 ";
  
  //Output 3
  //========
  if (digitalRead(Output3) == 0) StatusString = StatusString + "o3:0 "; else StatusString = StatusString + "o3:1 ";
 
  //Output 4
  //========
  if (digitalRead(Output4) == 0) StatusString = StatusString + "o4:0 "; else StatusString = StatusString + "o4:1 ";
   
    // Configure to send SMS
  //-----------------------------------------------
  SSerial.print(F("AT+CMGS=\""));
  // Send SMS number
  //-----------------------------------------------
  SSerial.print(Master_Nr); 
  SSerial.print(F("\"\r\n"));
  delay(50);
  
  // Send SMS message
  //-----------------------------------------------
  SSerial.print(StatusString);
  delay(50);
  
  // Send Ctrl+Z / ESC to denote SMS message is complete
  //-----------------------------------------------
  SSerial.write((char)26);
  WaitOK();
  
  digitalWrite(LedG,LOW);
}


//###############################################################################################
// Respond to incomming calls
// 
//###############################################################################################
void GetCall() {
  // Drop incoming call
  //-----------------------------------------------
  SSerial.print(F("ATH\n\r"));
}


//###############################################################################################
// Init GSM Module
// 
//###############################################################################################
void InitGSM() {
  
  digitalWrite(LedR,HIGH);
  
  // Setup I/Os
  //-----------------------------------------------
  pinMode(SIM800_Reset,OUTPUT);
  
  // Reboot SIM800
  //-----------------------------------------------
  digitalWrite(SIM800_Reset,LOW);
  delay(250);
  digitalWrite(SIM800_Reset,HIGH);
  //wait for GSM module to reboot (about 5 seconds)
  delay(5000);
  
  //Scan for GSM Module
  //-----------------------------------------------
  SSerial.print(F("AT\r\n"));
  WaitOK();
  
  // Wait for network connectivity
  //-----------------------------------------------
  while(!Connected) {
    SSerial.print(F("AT+CREG?\r\n"));
    delay(1000);
    RxString = "";
    while(SSerial.available()){
      RxString = RxString + char(SSerial.read());
    }
    if (Received(F("+CREG: 0,1"))) {
      //network connected
      Connected = true;
    }
  }
  
  // Set SMS mode to ASCII
  //-----------------------------------------------
  SSerial.print(F("AT+CMGF=1\r\n"));
  WaitOK();
  
  // Set device to read SMS if available and print to SSerial
  //-----------------------------------------------
  SSerial.print(F("AT+CNMI=1,2,0,0,0\r\n"));
  WaitOK();
  
  // Delete old SMS
  //-----------------------------------------------
  SSerial.print(F("AT+CMGD=1,4\r\n"));
  WaitOK();  
  
  // Set phone book to SIM
  //-----------------------------------------------
  SSerial.print(F("AT+CPBS=\"SM\"\r\n"));
  WaitOK();

  // Set local time to network time
  //-----------------------------------------------
  SSerial.print(F("AT+COPS=2\r\n"));
  WaitOK();
  SSerial.print(F("AT+CLTS=1\r\n"));
  WaitOK();
  SSerial.print(F("AT+COPS=0\r\n"));
  WaitOK();  
  
  //wait for network operator to be selected
  //-----------------------------------------------
  while(!SSerial.available()) {
  }
  //wait for time to be set
  delay(5000);
  while(SSerial.available()) {
    char c1 = char(SSerial.read());
    c1 = c1;
  }
  
  // Setup USSD function
  //-----------------------------------------------
  SSerial.print(F("AT+CUSD=1\r\n"));
  WaitOK();


  // Read SSerial buffer to empty
  //-----------------------------------------------
  while(SSerial.available()){
    char c = char(SSerial.read());
    c = c;
  }
  digitalWrite(LedR,LOW);

}


//###############################################################################################
// Wait for the OK response from software SSerial port
// 
//###############################################################################################
void WaitOK() {
  char c1= ' ';
  char c2 = ' ';
  boolean OK = false;
  // Wait for data in rx buffer
  //-----------------------------------------------
  while(!SSerial.available()){
  }
  // Read data from buffer
  //-----------------------------------------------
  while (!OK) {
    while(SSerial.available()){
      c1 = c2;
      c2 = char(SSerial.read());
      if ( (c1 == 'O') and (c2 == 'K') ) {
        // OK received
        //-----------------------------------------------
        OK = true;
      }
    }
  }
}


//###############################################################################################
// Search for specific characters inside RxString
// 
//###############################################################################################
boolean Received(String S) {
  S.trim();
  S.toLowerCase();
  RxString.toLowerCase();
  if (RxString.indexOf(S) >= 0) return true; else return false;
}



