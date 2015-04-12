/***********************************************************************
 * EPLUG: Ethernet Controlled Power Plug
 ***********************************************************************
 * Author   : José Oliveira
 * Email    : jose.oliveira.83@gmail.com 
 * Blog     : http://myhowtosandprojects.blogspot.pt
 * Facebook : https://www.facebook.com/BragaLab
 * Date     : 11/04/2015
 * Version  : 1
 ***********************************************************************
 * DESCRIPTION:
 *  EPLUG is a small telnet Command Line Interface (CLI) that you can
 *  use to control multiple relays atached to Power Plugs. With this 
 *  you can turn ON and OFF just about any electronic appliance you 
 *  own from any device with telnet (PC, SmartPhone, Tablet, etc),
 *  using your Home Network or the Internet.
 *  
 *  EPLUG was built as a framework that can be used for other projects.
 *  You can easily change the commands syntax by changing the text on 
 *  the command patterns and add new commands by adding a new 
 *  "else if(..){...}" statements on the "loop()" function
 ***********************************************************************
 * LICENSE
 *  This file is part of EPLUG.
 * 
 *  EPLUG is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  EPLUG is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with EPLUG.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/ 
#include <UIPEthernet.h>
#include <UIPServer.h>
#include <UIPClient.h>
#include <string.h>


// GLOBAL VARS ///////////////////////////////////
EthernetServer server = EthernetServer(23);                //Define local server port
EthernetClient client;                                     //Currently connected client     
int       cmd_recv=0;                                      //1=command received | 0=command not received
int const cmd_len=22;                                      //Lenght off the command receive buffer - command len+2(\r\n)
char      cmd[cmd_len];                                    //Buffer to receive the command on "rcv_cmd()" 
int       relay_state[14];                                 //List of the relays state (pin nº==relay nº) 
                                                           //relay_state[0] not used: 13 pins -> relay_state[1..13] 
 
// PATTERNS TO MATCH AGAINST COMMANDS //////////// 
char SET_RELAY_ON_PATTERN[]="set on relay";                // CMD==PATTERN + PIN NUMBER (one pin)
char SET_RELAY_OFF_PATTERN[]="set off relay";              // CMD==PATTERN + PIN NUMBER (one pin)
char SHOW_RELAY_PATTERN[]="show relay";                    // CMD==PATTERN (all pins) or CMD==PATTERN + PIN NUMBER (one pin)
char EXIT_PATTERN[]="exit";                                // CMD==PATTERN
char ENTER_PATTERN[]="";                                   // CMD==PATTERN
char ABOUT_PATTERN[]="about";                              // CMD==PATTERN

void setup()
{ 
  //INIT - NEWTWORK INTERFACE (NIC) //////////////
  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};        //Arduino MAC Address  
  IPAddress myIP   (192,168,1,20);                         //Arduino IP Address 
  IPAddress dnsIP  (192,168,1,254);                        //DNS Server IP Address
  IPAddress gwIP   (192,168,1,254);                        //Gateway IP Address
  IPAddress IPmask (255,255,255,0);                        //IP Mask
  Ethernet.begin(mac, myIP, dnsIP, gwIP, IPmask);
 
  //INIT - START TCP SERVER //////////////////////
  server.begin();                                          //Start listening for clients

   
  ///////////////////////////////////////////////////////////////////
  // DO NOTE USE THESE PINS FOR RELAYS
  ///////////////////////////////////////////////////////////////////
  // Ethernet NIC HR911105A - Network Board Pins
  //  Arduino UNO pin 12  / Arduino MEGA pin 50 -- SO  (SPI MISO)
  //  Arduino UNO pin 11  / Arduino MEGA pin 51 -- SI  (SPI MOSI)
  //  Arduino UNO pin 13  / Arduino MEGA pin 52 -- SCK (SPI SCK)
  //  Arduino UNO pin 10  / Arduino MEGA pin 53 -- CS  (SPI SS)
  //  Arduino UNO pin 3v3 / Arduino MEGA 3v3    -- VCC
  //  Arduino UNO pin GND / Arduino MEGA GND    -- GND
  ///////////////////////////////////////////////////////////////////    
  // RS232 - SERIAL PORT
  //  PIN 0 - TX - RS232
  //  PIN 1 - RX - RS232
  ///////////////////////////////////////////////////////////////////
  //pinMode(0, OUTPUT);      // RELE PIN OUT     | RS232 RX
  //digitalWrite(0, HIGH);   // RELE OFF (HIGH)  | RS232 RX
  //pinMode(1, OUTPUT);      // RELE PIN OUT     | RS232 TX
  //digitalWrite(1, HIGH);   // RELE OFF (HIGH)  | RS232 TX
  pinMode(2, OUTPUT);      // RELE PIN OUT
  digitalWrite(2, HIGH);   // RELE OFF (HIGH)
  pinMode(3, OUTPUT);      // RELE PIN OUT
  digitalWrite(3, HIGH);   // RELE OFF (HIGH)
  pinMode(4, OUTPUT);      // RELE PIN OUT
  digitalWrite(4, HIGH);   // RELE OFF (HIGH)
  pinMode(5, OUTPUT);      // RELE PIN OUT
  digitalWrite(5, HIGH);   // RELE OFF (HIGH)
  pinMode(6, OUTPUT);      // RELE PIN OUT
  digitalWrite(6, HIGH);   // RELE OFF (HIGH)
  pinMode(7, OUTPUT);      // RELE PIN OUT
  digitalWrite(7, HIGH);   // RELE OFF (HIGH)
  pinMode(8, OUTPUT);      // RELE PIN OUT
  digitalWrite(8, HIGH);   // RELE OFF (HIGH)
  pinMode(9, OUTPUT);      // RELE PIN OUT
  digitalWrite(9, HIGH);   // RELE OFF (HIGH)
  //pinMode(10, OUTPUT);     // RELE PIN OUT    | ETH NIC - CS(SPI SS)
  //digitalWrite(10, HIGH);  // RELE OFF (HIGH) | ETH NIC - CS(SPI SS)
  //pinMode(11, OUTPUT);     // RELE PIN OUT    | ETH NIC - SI(SPI MOSI)
  //digitalWrite(11, HIGH);  // RELE OFF (HIGH) | ETH NIC - SI(SPI MOSI)
  //pinMode(12, OUTPUT);     // RELE PIN OUT    | ETH NIC - SO(SPI MISO)
  //digitalWrite(12, HIGH);  // RELE OFF (HIGH) | ETH NIC - SO(SPI MISO)
  //pinMode(13, OUTPUT);     // RELE PIN OUT    | ETH NIC - SCK(SPI SCK)
  //digitalWrite(13, HIGH);  // RELE OFF (HIGH) | ETH NIC - SCK(SPI SCK)
   

   
  ///////////////////////////////////////////////////////////////////
  // Pins 0-1 - RS232 | Pins 10-13 SPI (HR911105A NIC)
  //  0   - Relay on PIN in the State OFF
  //  1   - Relay on PIN in the State ON
  //  >=2 - No Relay on the Pin 
  ///////////////////////////////////////////////////////////////////
  relay_state[0]=2;   //Relay 0  --> NO RELAY (>=2) | RS232 RX
  relay_state[1]=2;   //Relay 1  --> NO RELAY (>=2) | RS232 TX
  relay_state[2]=0;   //Relay 2  --> OFF(0)
  relay_state[3]=0;   //Relay 3  --> OFF(0)
  relay_state[4]=0;   //Relay 4  --> OFF(0)
  relay_state[5]=0;   //Relay 5  --> OFF(0)
  relay_state[6]=0;   //Relay 6  --> OFF(0)
  relay_state[7]=0;   //Relay 7  --> OFF(0)
  relay_state[8]=0;   //Relay 8  --> OFF(0)
  relay_state[9]=0;   //Relay 9  --> OFF(0)
  relay_state[10]=2;  //Relay 10 --> NO RELAY (>=2) | ETH NIC - CS(SPI SS)
  relay_state[11]=2;  //Relay 11 --> NO RELAY (>=2) | ETH NIC - SI(SPI MOSI)
  relay_state[12]=2;  //Relay 12 --> NO RELAY (>=2) | ETH NIC - SO(SPI MISO)
  relay_state[13]=2;  //Relay 13 --> NO RELAY (>=2) | ETH NIC - SCK(SPI SCK)
  
}


void loop()
{
  //RECEIVE THE COMMAND /////////////////////////////////////////////
  cmd_recv=rcv_cmd(cmd,cmd_len); 
  
  if(cmd_recv==1)
  { 
    cmd_recv=0;                                            //Reset flag - received command 
  
   /******************************************************************************
    * PATTERN MATCHING - ACL LIKE STYLE
    ******************************************************************************
    * More specific matches for a command first - Exact matches (equals)
    ******************************************************************************
     cmd_equals(pattern,command)       - exact match
     cmd_startswith_l(pattern,command) - partial match (pattern len < command len)
    ******************************************************************************/
    //       MATCH PATTERN                               =====>      ACTION  
         if (cmd_startswith_l(SET_RELAY_ON_PATTERN  , cmd))  { set_on_relay_x(cmd);  } // SET ON - RELAY on PIN x
    else if (cmd_startswith_l(SET_RELAY_OFF_PATTERN , cmd))  { set_off_relay_x(cmd); } // SET OFF - RELAY ON PIN x
    else if (cmd_equals      (SHOW_RELAY_PATTERN    , cmd))  { show_relay_all();     } // SHOW RELAYs ON ALL THE PINS
    else if (cmd_startswith_l(SHOW_RELAY_PATTERN    , cmd))  { show_relay_x(cmd);    } // SHOW RELAY ON PIN x
    else if (cmd_equals      (EXIT_PATTERN          , cmd))  { exit();               } // EXIT - CLOSE CONNECTION
    else if (cmd_equals      (ENTER_PATTERN         , cmd))  { help();               } // ENTER - SHOW HELP
    else if (cmd_equals      (ABOUT_PATTERN         , cmd))  { about();              } // ABOUT - SHOW THE ABOUT
    else { client.println("Invalid Command - Press Enter"); }
  
 } // if cmd recv

  
}// void loop()


//###########################################################################################
//###########################################################################################
//## COMMAND RECEPTION AND PATTERN MATCHING #################################################
//###########################################################################################
//###########################################################################################

//////////////////////////////////////////////////////////////////////////
// Read the text command from the TCP connection
//////////////////////////////////////////////////////////////////////////
// command structure
//   <pattern>\r\n - the "\n" marks the end of the command
//                 - the "\r" is inserted by windows telnet
//   or 
//
//   <pattern>\n - the "\n" marks the end of the command
//////////////////////////////////////////////////////////////////////////
int rcv_cmd(char* cmd, int cmd_len)
{
  cmd[0]='\0';                                             //Clear the string
  int rcv_chs=0;                                           //Reset the nº of received characters
      
   
  while(1)                                                 //Receive loop - exits only on "return"
  {
     size_t size; 
  
     if (client = server.available())                      //If an incoming client connects, start receiving
     {
         
       if (client)
       {	 
  
           while((size = client.available()) > 0)          //Read command 
           {
             
               uint8_t* msg = (uint8_t*)malloc(size+1);    //The '+1' is for the string terminator
               size = client.read(msg,size);               //Read the chars on buffer
               msg[size]='\0';                             //Add a string terminator  
               
               // Serial.println(msg);                     //Return what is received
               rcv_chs=rcv_chs+size;                       //Count the received char to avoid exceding the "cmd" buffer length
                
               if(rcv_chs<cmd_len)                         //If received chars don't exceed the "cmd" buffer
               {
                 sprintf(cmd, "%s%s",cmd,msg);             //Concatenate cmd+msg
                 free(msg);                                //Release buffer
                
                 if(cmd[rcv_chs-1]=='\n')                  //End of the command found (newline - \n) ?
                 {
                   //TRIM SPECIAL CHARS FROM COMMAND
                   if(cmd[rcv_chs-2]=='\r')                //Extra \r char (telnet on windows) found ?
                    cmd[rcv_chs-2]='\0';                   //Delete the \r\n - telnet on windows (\r)
                    else cmd[rcv_chs-1]='\0';              //Delete the \n   - other
                  
                   return 1;                               //End of command => return success
                 }
                
               }else return 0;                             //Received chars are bigger than 'cmd' buffer
               
           }//while
           
           
        }//if
     }//if
  
  }//while
  
}


//////////////////////////////////////////////////////////////////////////
// Check if COMMAND starts with PATTERN (pattern len < cmd len - not equal)
//////////////////////////////////////////////////////////////////////////
int cmd_startswith_l(char* pattern, char* command) 
{
  int pattern_len=strlen(pattern);						   //Get pattern number of chars
  int cmd_len=strlen(command);                             //Get command number of chars
  
  if(pattern_len<cmd_len)                                  //Is "pattern" smaller than "command" ?
  {
    //COMPARE CHAR BY CHAR
    //IF PATTERN MATCHES THE BEGIN OF THE COMMAND 
    int i;
    for(i=0;i<pattern_len;i++)
    {
      if(pattern[i] != command[i])                         //Compare the "pattern" to the begin of "commad" - char by char
       return 0;                                           //One diferent char => return FALSE
    }
    
    //IF IT HASN'T RETURN FALSE, THEN RETURN TRUE
    return 1;                                              //All char equals (pattern len) => return TRUE
  
  }
  else return 0;	                                       //"Pattern" is not smaller then the "command"
                                                           // so its impossible for the begin of the command
														   // to be equal to the pattern => return false

}


//////////////////////////////////////////////////////////////////////////
// Check if the PATTERN is equal to COMMAND - exact match
//////////////////////////////////////////////////////////////////////////
int cmd_equals(char* pattern, char* command) 
{
  int pattern_len=strlen(pattern);						   //Get pattern number of chars
  int cmd_len=strlen(command);                             //Get command number of chars
  
  if(pattern_len==cmd_len)                                 //Is "pattern" the same size as the "command" ?
  {
  
    //COMPARE CHAR BY CHAR
    //IF PATTERN MATCHES THE COMMAND 
    int i;
    for(i=0;i<pattern_len;i++)
    {
      if(pattern[i] != command[i])                         //Compare the "pattern" to the "commad" - char by char
       return 0;                                           //One diferent char => return FALSE
    }
    
    //IF IT HASN'T RETURN FALSE, THEN RETURN TRUE
    return 1; //equal length + all char equals 
  
  }
  else return 0;	                                       //"Pattern" is not the same size as the "command"
                                                           // so its impossible for the command
														   // to be equal to the pattern => return FALSE

}





//###########################################################################################
//###########################################################################################
//## RELAY FUNCTIONS ########################################################################
//###########################################################################################
//###########################################################################################


//////////////////////////////////////////////////////////////////////////
// SET RELAY ON
//////////////////////////////////////////////////////////////////////////
int set_on_relay_x(char* command)
{
   int PIN=get_pin(command);                               // Extract the the pin number
   if(PIN!=-1)                                             // Was the pin number valid ?
   {
      if(relay_state[PIN] == 0)                            // Was relay OFF ? => ON
      {  
	    //SET RELAY ON
        digitalWrite(PIN, LOW);                            // Activate Relay
        relay_state[PIN]=1;                                // Update Pin Status to ON
      
	    //PRINT: RELAY ON
        client.print("RELAY on Pin ");                     // Print: Relay x ON
        client.print(PIN);
        client.println(" - ON\n");  
        return 1;
      }
      else if(relay_state[PIN] == 1)                       // Was relay already ON ?
      {
	    //PRINT: RELAY ALREADY ON
        client.print("RELAY on Pin ");                     // Print: Relay x already ON
        client.print(PIN);
        client.println(" - ALREADY ON\n");  
        return 0;
      }
      else  if(relay_state[PIN] >= 2)                      // No relay configured ?                                    
      {
	    //PRINT: NO RELAY
        client.print("RELAY on Pin ");                     // Print: Relay x NO RELAY
        client.print(PIN);
        client.println(" - NO RELAY\n"); 
        return 0;
      }
      return 0;                                            // Other state - return false 
	  
   } 
   else                                                    // Pin number is not valid
   {
     client.print("RELAY Pin - INVALID PIN\n");            // Print: Relay INVALID PIN
     return 0;
   }
}    


//////////////////////////////////////////////////////////////////////////
// SET RELAY OFF 
//////////////////////////////////////////////////////////////////////////
int set_off_relay_x(char* command)
{ 
    int PIN=get_pin(command);                              // Extract the the pin number
    if(PIN!=-1)                                            // Was the pin number valid ?
    {
      
       if(relay_state[PIN] == 1)                           // Was relay ON ? => OFF
       {
	     //SET RELAY OFF
         digitalWrite(PIN, HIGH);                          // Deactivate Relay
         relay_state[PIN]=0;                               // Update Pin Status to OFF
      
	     //PRINT: RELAY OFF	  
         client.print("RELAY on Pin ");                    // Print: Relay x OFF
         client.print(PIN);
         client.println(" - OFF\n");  
        
         return 1;
       }
       else if(relay_state[PIN] == 0)                      // Was relay already OFF ?
       {
	     //PRINT: RELAY ALREADY OFF
         client.print("RELAY on Pin ");                    // Print: Relay x ALREADY OFF
         client.print(PIN);
         client.println(" - ALREADY OFF\n");  
         return 0;
       }
       else if(relay_state[PIN] >= 2)                      // No relay configured ?  
       {
	     //PRINT: NO RELAY
         client.print("RELAY on Pin ");                    // Print: Relay x NO RELAY
         client.print(PIN);
         client.println(" - NO RELAY\n");  
         return 0;
       }
       return 0;                                           // Other state - return false 
      
    }
    else                                                   // Pin number is not valid
    {
       client.print("RELAY Pin - INVALID PIN\n");          // Print: Relay INVALID PIN 
       return 0;
    }
}    


//////////////////////////////////////////////////////////////////////////
// SHOW RELAY X 
//////////////////////////////////////////////////////////////////////////
int show_relay_x(char* command)
{ 
    int PIN=get_pin(command);                             // Extract the the pin number
    if(PIN!=-1)                                           // Was the pin number valid ?
    {
     
       if(relay_state[PIN] == 1)                           // Is relay ON ?
       {
         client.print("RELAY on Pin ");                    // Print: Relay x ON
         client.print(PIN);
         client.println(" - ON");      
         return 1;
       }
       else if(relay_state[PIN] == 0)                      // Is relay OFF ?
       {
         client.print("RELAY on Pin ");                    // Print: Relay x OFF
         client.print(PIN);
         client.println(" - OFF\n");  
         return 1;
       }
       else if(relay_state[PIN] >= 2)                      // No relay configured ?  
       {
         client.print("RELAY on Pin ");                    // Print: Relay x NO RELAY
         client.print(PIN);
         client.println(" - NO RELAY\n");  
         return 1;
       }
       return 0;                                           // Other state - return false 
      
    } 
    else                                                   // Pin number is not valid    
    {
       client.print("RELAY Pin - INVALID PIN\n");          // Print: Relay INVALID PIN
       return 0;
    }
    
}
 

//////////////////////////////////////////////////////////////////////////
// SHOW RELAY ALL - PIN 3-13
//////////////////////////////////////////////////////////////////////////
int show_relay_all()
{
 
  for(int PIN=0;PIN<14;PIN++)
  {
  
       if(relay_state[PIN] == 1)                           // Is relay ON ?
       {
         client.print("RELAY on Pin ");                    // Print: Relay x ON
         client.print(PIN);
         client.println(" - ON");      
       }
       else if(relay_state[PIN] == 0)                      // Is relay OFF ?
       {
         client.print("RELAY on Pin ");                    // Print: Relay x OFF
         client.print(PIN);
         client.println(" - OFF");  
       }
       else if(relay_state[PIN] >= 2)                      // No relay configured ?  
       {
         client.print("RELAY on Pin ");                    // Print: Relay x NO RELAY
         client.print(PIN);
         client.println(" - NO RELAY");  
       }
  
  }
  client.println("");                                      //Print: Newline
  return 1;

}


//////////////////////////////////////////////////////////////////////////
// GET PIN NUMBER - from the command
//////////////////////////////////////////////////////////////////////////
int get_pin(char* command)
{
  int len=strlen(command)-1;                               // chars - 1(terminator)
  
  int last_space=0;                                        // Space before the pin number 
  int pin_str_len=0;                                       // Size of chars - pin number str
  char *pin_str;                                           // Substring - pin number
  int pin=0;                                               // Converted pin number
  
  
  //SEARCH FOR THE LAST SPACE (SEARCH: END->BEGIN) 
  for(int i=len; i>0 ;i--)
  {
  
    if(command[i] == ' ' && last_space==0)                 // Was last space found ? 
    {
      last_space=i;                                        // Record last space
      pin_str_len=len-i;                                   // Get the pin number str size + 1
     
      //GET THE SUB-STRING THAT REPRESENTS THE PIN NUMBER
      pin_str = (char *) malloc(pin_str_len);              // chars + 1(terminator)
      for(int j=0; j<=pin_str_len ;j++)
      pin_str[j]=command[last_space+j+1];
  
      //CHECK FOR 0 MANUALY BECAUSE ATOI RETURNS 0 ON FAILURE
      if( pin_str[0]=='0' && pin_str_len==2)               // Pin number is 0 AND str_len=2 aka - '0'\0
      {
 	  return 0;                                            // Number is 0 => return 0
 	 }
      else                                                 // Other characters 
      {
        //CONVERT THE SUB-STRING WITH THE PIN NUMBER TO A NUMBER (int)
        pin=atoi(pin_str);                                 // Convert string to int
        free(pin_str);                                     // Free the allocated memory
                                                            
        if(pin==0)                                         // String to int failure ? 
        return -1;                                         // Yes - return failure (-1)
        else return pin;                                   // No - return pin (converted string)
       } 
     
    }//if
    
  }//for
  
  return -1;                                               //no results - return failure

}//get_pin()

 



//###########################################################################################
//###########################################################################################
//## AUXILIARY FUNCTIONS ####################################################################
//###########################################################################################
//###########################################################################################

//////////////////////////////////////////////////////////////////////////
// HELP - SHOW AVAILABLE COMMANDS
//////////////////////////////////////////////////////////////////////////
void help()
{
 client.println("EPLUG (by Jose Oliveira)\n"); 
 client.println(" set on relay x  - active relay");
 client.println(" set off relay x - deactivate relay");
 client.println(" show relay x    - show relay state");
 client.println(" show relay      - show all relay states");
 client.println(" exit            - closes the connection");
 client.println(" enter           - help (this menu)");
 client.println(" about           - info about EPLUG\n");
}


//////////////////////////////////////////////////////////////////////////
// Closes the curent client connection (TCP Session)
//////////////////////////////////////////////////////////////////////////
int exit() 
{
  client.stop();
  return 1;
}

 
//////////////////////////////////////////////////////////////////////////
// Information about EPLUG
//////////////////////////////////////////////////////////////////////////

int about() 
{
 client.println("\nAbout EPLUG"); 
 client.println(" Author   : Jose Oliveira");
 client.println(" Email    : jose.oliveira.83@gmail.com");
 client.println(" Version  : 1\n");
 return 1;
}

  
 
 
 
 
