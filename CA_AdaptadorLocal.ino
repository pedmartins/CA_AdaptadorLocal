#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

  MDNSResponder mdns;
  
  // Network credentials
  //const char* ssid = "SMC";
  const char* ssid = "Host_FunWorks";
  const char* password = "033e6fd5f7a0";
  
  const char* hostAdapter = "dooradapter";
  const char* hostModule = "raspberrypi.home";
  //String line = "";
  
  // Connect to the site defined in "hostModule"
  WiFiClient client;
  
  ESP8266WebServer server(80);
  
  //const char* mdns_hostname = "raspberrypi.home";
  
  //#define WIFI_WAIT 1
  //#define ADAPTER_WAIT 2
  //#define ARMED 3
  //#define DISARMED 4

  const int WIFI_WAIT = 0;
  const int ADAPTER_WAIT = 1;
  const int ARMED = 2;
  const int DISARMED = 3;
  const int OPENLCK1 = 4;
  
  const int onvalue = 1;
  const int offvalue = 0;
  
  #define LEDON LOW
  #define SWITCHON LOW
  
  #define WifiLEDInterval 200
  #define mDNSLEDInterval 1500
  #define DISARMInterval 1000
  
  const int Reed = D1; // GPIO5
  const int Button = D2; // GPIO4
  const int RedLed = D8; // GPIO12
  const int OrangeLed = D7; // GPIO13
  const int Door1 = D5; //GPIO14 - D5
  const int Door2 = D0; //GPIO16 - D0

  
  const int idxSwitchDoor = 15;
  const int idxSwitchButton = 16;
  const int idxArmStatus = 1;
  
  // Status LED Setup
  unsigned long previousMillis = 0;
  int orangeLedState = LEDON;
  int redLedState = LEDON;

  // End of Status LED Setup
  /*enum status {
    WIFI_WAIT = 0,
    ADAPTER_WAIT = 1,
    ARMED = 2,
    DISARMED = 3
  };*/

  //char *lightSwitch;

  // System and Switch states
  int doorState;
  int buttonState;

  //status currentStatus = WIFI_WAIT;
  int currentStatus = WIFI_WAIT;
  int defaultArmStatus = -1;

  bool checkWifi = false;
  bool serverInitialized = false;
  bool endpArm = false;
  bool endpDisarm = false;
  bool endpOpenLock1 = false;
  bool endpOpenLock2 = false;


  // Toggle Red Status LED
  void toggleRedLed() {
     redLedState = !redLedState;
     digitalWrite(RedLed, redLedState);
     delay(500);
  }

  // Toggle Red LED Status 
  void switchOnRedLed() {
     digitalWrite(RedLed, HIGH);
     delay(500);
  }

  // Toggle Blue Status LED
  void toggleOrangeLed() {
     orangeLedState = !orangeLedState;
     digitalWrite(OrangeLed, orangeLedState);
     delay(500);
  }

  // Toggle Orange LED Status 
  void switchOnOrangeLed() {
     digitalWrite(OrangeLed, HIGH);
     delay(500);
  }

  void sendDoorState(int switchval) {

    char* getStr;
    sprintf(getStr, "GET /json.htm?type=command&param=switchlight&idx=%d&switchcmd=%d HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", idxSwitchDoor, switchval, hostModule);
    
    if ( ! client.connect(hostModule, 8085) ) {
        Serial.println("Connection failed :-(");
    }

    Serial.println("Connected to host");
    Serial.println("Sending DoorState request...");

    client.print(getStr);

    delay(100);
  }

void getButtonArmStatus_BAK() {

    String getStr = "";
    StaticJsonBuffer<400> jsonBuffer;
    
    // Define number of pieces
    const int numberOfPieces = 5;
    String pieces[numberOfPieces];

    // Keep track of current position in array
    int counter = 0;

    // Keep track of the last comma so we know where to start the substring
    int lastIndex = 0;

    getStr += "GET /json.htm?type=command&param=getuservariable&idx=";
    getStr += String(idxArmStatus);
    getStr += " HTTP/1.1\r\nHost: ";
    getStr += hostModule;
    getStr += "\r\nConnection: close\r\n\r\n";

 
    //sprintf(getStr, "GET /json.htm?type=command&param=getuservariable&idx=%d HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", idxArmStatus, hostModule);
        
    if ( ! client.connect(hostModule, 8085) ) {
        Serial.println("Connection failed :-(");
    } else {
      Serial.println("Connected to host");
      Serial.println("Sending ModuloLocalStatus request...");
      Serial.println(getStr);
     
      client.print(getStr);
  
      delay(100);

      String header = client.readStringUntil('[');
      String lines;
      Serial.println(header);
      //int totalLength = 0
      
       // if there are incoming bytes available
      // from the server, read them and print them:
      while (client.available()) {

        lines = client.readStringUntil(']');
        char json[lines.length()];
        JsonObject& root = jsonBuffer.parseObject(json);

        lines.toCharArray(json,lines.length());
        //totalLength += line.length();
        
        const char* lastUpdate = root["LastUpdate"];
        const char* charname = root["Name"];
        const char* chartype = root["Type"];
        const int intValue = atoi(root["Value"]);
        const char* charidx = root["idx"];
        
        Serial.println("LastUpdate:"+String(lastUpdate));
        Serial.println("Name>"+String(charname)+"<");
        Serial.println("Type>"+String(chartype)+"<");
        Serial.println("Value>"+String(intValue)+"<");
        Serial.println("idx>"+String(charidx)+"<");

        //Serial.println(strcmp(root["Value"], onvalue));
        //Serial.println(strcmp(defaultArmStatus, onvalue));

       
        // defines initial ARM status adapter
        if ( intValue == onvalue ) {
        //strcpy(defaultArmStatus, root["Value"]);
          toggleRedLed();
          defaultArmStatus = onvalue;
          Serial.println("(A)");
        } else if ( intValue == offvalue ) {
          Serial.println("(B)");
          toggleOrangeLed();
          defaultArmStatus = offvalue;
        }

       
      } 
    }
  }
  
  void getDefaultArmStatus() {

    String getStr = "";
    
    // Define number of pieces
    const int numberOfPieces = 5;
    String pieces[numberOfPieces];

    // Keep track of current position in array
    int counter = 0;

    // Keep track of the last comma so we know where to start the substring
    int lastIndex = 0;

    getStr += "GET /json.htm?type=command&param=getuservariable&idx=";
    getStr += String(idxArmStatus);
    getStr += " HTTP/1.1\r\nHost: ";
    getStr += hostModule;
    getStr += "\r\nConnection: close\r\n\r\n";

 
    //sprintf(getStr, "GET /json.htm?type=command&param=getuservariable&idx=%d HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", idxArmStatus, hostModule);
        
    if ( ! client.connect(hostModule, 8085) ) {
        Serial.println("Connection failed :-(");
    } else {
      Serial.println("Connected to host");
      Serial.println("Sending ModuloLocalStatus request...");
      Serial.println(getStr);
     
      client.print(getStr);
  
      delay(100);

      String header = client.readStringUntil('{');
      Serial.println(header);
        
       // if there are incoming bytes available
      // from the server, read them and print them:
      while (client.available()) {

        String line = client.readStringUntil('\r');
        //char charLine[line.length()];
        //line.toCharArray(charLine,line.length());

        // defines initial ARM status adapter
        if ( line.indexOf("\"Value\" : \"1\"") != -1 ) {
        //strcpy(defaultArmStatus, root["Value"]);
          toggleRedLed();
          defaultArmStatus = onvalue;
          Serial.println("Default(On)");
        } else if ( line.indexOf("\"Value\" : \"0\"") != -1 ) {
          toggleOrangeLed();
          defaultArmStatus = offvalue;
          Serial.println("Default(Off)");
        }
      }
        /*
        for (int i = 0; i < line.length(); i++) {
          // Loop through each character and check if it's a "
          //char c[1]; 
          //String(line.substring(i, i+1)).toCharArry(c,1);
          if ( charLine[i] == '"') {
            // Grab the piece from the last index up to the current position and store it
            String token = line.substring(lastIndex, i);
          
            // Exclude control characters
            if ( token.length() > 0 && token != " : " && token != "[" && token != "]" ) {
              pieces[counter] = token;
  
              // Increase the position in the array that we store into
              counter++;
            }
          }
            
          // Update the last position and add 1, so it starts from the next character
          lastIndex = i + 1;
              
          // If we're at the end of the string (no more commas to stop us)
          if (i == (line.length() - 1)) {
            // Grab the last part of the string from the lastIndex to the end
            pieces[counter] = line.substring(lastIndex, i);
            counter++;
          }
        }         

        //Serial.println(line);  
        for ( int n = 0 ; n < counter ; n++ ) {
          Serial.println(pieces[n]);  
        }
        counter = 0;
        lastIndex = 0;
      }
      */
      
      Serial.println("Request sent...");
      delay(1000);   
    }  
  }

  void sendButtonArmStatus(int switchval) {

    String getStr = "";
    const char* switchStr;

    switch ( switchval ) {
      case 0:
        switchStr = "Off";
        break;
      case 1:
        switchStr = "On";
        break;
    }

    getStr += "GET /json.htm?type=command&param=switchlight&idx=13&switchcmd=";
    getStr += String(switchStr);
    getStr += " HTTP/1.1\r\nHost: ";
    getStr += hostModule;
    getStr += "\r\nConnection: close\r\n\r\n";

    if ( ! client.connect(hostModule, 8085) ) {
        Serial.println("Connection failed :-(");
    } else {
      Serial.println("Connected to host");
      Serial.println("Sending ButtonState request...");

      Serial.print(getStr);  
      client.print(getStr);
  /*
      delay(100);
      
       // if there are incoming bytes available
      // from the server, read them and print them:
      while (client.available()) {
        String line = client.readStringUntil('\r');
        //Serial.print(line);  
      }
      
      Serial.println("Request sent...");
      delay(1000);   
      */
    }  
  }
  
  void sendButtonArmStatus_BAK(int switchval) {

    String getStr = "";

    getStr += "GET /json.htm?type=command&param=updateuservariable&vname=ARMED&vtype=0&vvalue=";
    getStr += String(switchval);
    getStr += " HTTP/1.1\r\nHost: ";
    getStr += hostModule;
    getStr += "\r\nConnection: close\r\n\r\n";

    if ( ! client.connect(hostModule, 8085) ) {
        Serial.println("Connection failed :-(");
    } else {
      Serial.println("Connected to host");
      Serial.println("Sending ButtonState request...");
  
      client.print(getStr);
  
      delay(100);
      
       // if there are incoming bytes available
      // from the server, read them and print them:
      while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);  
      }
      
      Serial.println("Request sent...");
      delay(1000);   
    }  
  }

  void initializeWebServer() {

      server.stop();
      //digitalWrite(OrangeLed, LEDON);
      //orangeLedState = LEDON;
  }

  void handleArm() {

      Serial.println("Handling Arm request...");
      setCurrentStatus(ARMED);
      server.send(200, "text/plain", "Comando ARMAR confirmado\r");
  }

  void handleDisarm() {

      Serial.println("Handling Disarm request...");
      setCurrentStatus(DISARMED);
      server.send(200, "text/plain", "Comando DESARMAR confirmado\r");
  }

  void handleOpenLock1() {

      Serial.println("Handling OpenLock1 request...");
      setCurrentStatus(OPENLCK1);
      server.send(200, "text/plain", "Comando OPENLOCK1 confirmado\r");
  }

  void handleOpenLock2() {
    
    Serial.println("Handling OpenLock2 request...");

      server.send(200, "text/plain", "Comando OPENLOCK2 confirmado\r");
  }

  void handleNotFound() {

    String message = "Comando indisponível\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
      
    //char notFoundStr[80]; 
    Serial.println("Handling NotFound request...");
    //sprintf(notFoundStr, "Comando indisponível: %s\r", cmd);
    server.send(404, "text/plain", message);
  }

  
  void openEndPoints() {
    // Setup Server Routes
    if ( endpArm ) {
        server.on("/arm", handleArm);
    }    

    if ( endpDisarm ) {
        server.on("/disarm", handleDisarm);
    }    

    if( currentStatus == ARMED ) {
  
      if ( endpOpenLock1 ) {
          server.on("/openlock1", handleOpenLock1);
      }    
  
      if ( endpOpenLock2 ) {
          server.on("/openlock2", handleOpenLock2);
      }    
    }
    
    server.onNotFound(handleNotFound);
  }
    
  void setCurrentStatus( const int newStatus ) {
  
    char str[50];

    serverInitialized = false;
    endpArm = false;
    endpDisarm = false;
    endpOpenLock1 = false;
    endpOpenLock2 = false;

    sprintf(str, "CurrentStatus[%d] -> NewStatus[%d]", currentStatus, newStatus);
    Serial.println(str);
    
    switch ( newStatus ) {

      case ARMED:
        endpArm = true;
        endpDisarm = true;
        endpOpenLock1 = true;
        endpOpenLock2 = true;

        currentStatus = newStatus;
        break;
        
      case DISARMED:
        endpArm = true;
        endpDisarm = true;
        
        currentStatus = newStatus;
        break;

      case OPENLCK1:
        if ( currentStatus == ARMED ) {
          digitalWrite(Door1, LOW);
          delay(3000);
          digitalWrite(Door1, HIGH);
          sprintf(str, "CurrentStatus[%d] -> NewStatus[%d]", newStatus, currentStatus );
          Serial.println(str);
        }
        break;

      default:
        currentStatus = newStatus;
        break;
    }
  
  }
    
  void setup() {
  
    //wdtDisable();
  
    pinMode(Reed, INPUT);
    pinMode(Button, INPUT);
    pinMode(RedLed, OUTPUT);
    pinMode(OrangeLed, OUTPUT);
    pinMode(Door1, OUTPUT);
    pinMode(Door2, OUTPUT);
  
    digitalWrite(Door1, HIGH);
    digitalWrite(Button, LOW);

    // Set Initial Switch states
    doorState = digitalRead(Reed);
    buttonState = digitalRead(Button);
  
    Serial.begin(115200);
    
    //delay(1000);  // give you some time to start the Serial Monitor
  
    // Connecting to wifi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
  
    checkWifi = true;
    
    // Wait until we're connected
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
   
    // Report success
    Serial.println("");
    Serial.print("Connected to wifi with IP ");  
    Serial.println(WiFi.localIP());
  
    checkWifi = false;
    //currentStatus = MDNSWait;
  }
  
  void loop() {
  
    yield();
   
   //Serial.println("*1"); 
      // Check if Wifi Disconnected
      if( !checkWifi && serverInitialized) {
        if(WiFi.status() != WL_CONNECTED) {
          server.stop();
          checkWifi = true;
          setCurrentStatus(WIFI_WAIT);
          //currentStatus=WIFI_WAIT;
        }
      }
      
    //Serial.println("*2"); 
      // WifiWait State, waiting for Wifi to initialize and connect
      if( currentStatus == WIFI_WAIT ) {
        // If Wifi is not Initialized start the WiFi connection
        if(checkWifi) {
          WiFi.begin(ssid, password);
          checkWifi = false;
        }
        // When the Wifi Connects move to the MDNSWait State
        if(WiFi.status() == WL_CONNECTED) {
          serverInitialized = false;
        } else {
          // Waiting for Wifi to connect, flash LED accordingly
          unsigned long currentMillis = millis();
          unsigned long elapsedTime = currentMillis - previousMillis;
          if(elapsedTime >= WifiLEDInterval) {
            previousMillis = currentMillis;
            toggleOrangeLed(); 
          }
        }
      } 
    
      // INICIALIZA MDNS
      //Serial.println("*3"); 
      // MDNSWait Sate, waiting for MDNS to initialize
      if( currentStatus == WIFI_WAIT && serverInitialized == false ) {
        //Serial.println("*31"); 
        // When the MDNS initializes move to Ready State
        if (MDNS.begin(hostAdapter)) {
          //serverInitialized = true;
  //Serial.println("*32");   
          setCurrentStatus(ADAPTER_WAIT);
          //currentStatus=&ADAPTER_WAIT;
        } else {
          // Waiting for MDNS to Init, flash LED accordingly
          unsigned long currentMillis = millis();
          unsigned long elapsedTime = currentMillis - previousMillis;
        //Serial.println("*33");   
          if(elapsedTime >= mDNSLEDInterval) {
            previousMillis = currentMillis;
            toggleOrangeLed(); 
          }
        //Serial.println("*34");             
        }
      }
  
 
      //Serial.println("*5"); 
      // INPUT Reed - Controla estado da porta
      // Check Door State and turn on or off based on it
      int new_doorState = digitalRead(Reed);
      if(new_doorState!= doorState) {
  
        doorState = new_doorState;
        sendDoorState(doorState);
      }
  
  
      //Controla estado do adaptador ARMADO / DESARMADO
      if( currentStatus == ADAPTER_WAIT ) {
        switchOnOrangeLed();
        switchOnRedLed();
        //Serial.println("*6");   
  
        getDefaultArmStatus();
        Serial.println("(A)");
        
        if ( defaultArmStatus == onvalue ) {
          toggleOrangeLed();
          setCurrentStatus(ARMED);
          sendButtonArmStatus(1);
      //    toggleRedLed();
        } else if ( defaultArmStatus == offvalue ) {
          toggleRedLed();        
          setCurrentStatus(DISARMED);
          sendButtonArmStatus(0);
       //   toggleOrangeLed();
        }  
      } 
  
      //Serial.println("*7"); 
      if( currentStatus == ARMED || currentStatus == DISARMED ) {
  
        if(serverInitialized == false) {
  
          //Initialize Webserver
          //Serial.println("*7"); 
          initializeWebServer();
  
          openEndPoints();
          // Start Server
          server.begin();
          
          serverInitialized = true;
        } else {
          //Serial.println("*71"); 
          server.handleClient();
          //yield();
        }

      // INPUT Botão - comunica ativação do botão armar
      // Check Button State and turn on or off based on it
      int new_buttonState = digitalRead(Button);
        if(new_buttonState!= buttonState) {

          unsigned long currentMillis = millis();
          unsigned long elapsedTime = currentMillis - previousMillis;
          
          Serial.println(String("*72"+new_buttonState));   
          if ( ! new_buttonState ) {
            if( elapsedTime >= DISARMInterval) {
              Serial.println(String("*721"));   
              setCurrentStatus(DISARMED);
              sendButtonArmStatus(0);
  
            } else {
              Serial.println(String("*722"));   
              buttonState = new_buttonState;
              setCurrentStatus(ARMED);
              sendButtonArmStatus(1);
            }

          }
            previousMillis = currentMillis;
            buttonState = new_buttonState;
        }
      }
  }
  

