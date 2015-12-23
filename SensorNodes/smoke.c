#include <RFM69.h>
#include <SPI.h>

#define NODEID        13    //unique for each node on same network
#define NETWORKID     101  //the same on all nodes that talk to each other
#define GATEWAYID     1
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "thisIsEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack
#define LED           9  // Moteinos have LEDs on D9
#define SERIAL_BAUD   115200  //must be 9600 for GPS, use whatever if no GPS

int TRANSMITPERIOD = 300; //transmit a packet to gateway so often (in ms)
char buff[20];
byte sendSize=0;
boolean requestACK = false;
RFM69 radio;

//struct for wireless data transmission
typedef struct {    
  int       nodeID;     
  int       deviceID;   
  unsigned long   uptime;     
  float     var2_float;     
  float     var3_float;   
} Payload;
Payload theData;

//end RFM69 ------------------------------------------

const int gasPin = A0;
const int rgb3 = 6;
const int alm = 5;
long lastPeriod = -1;

void setup()
{
  Serial.begin(SERIAL_BAUD);          //  setup serial
  Serial.println("starting");
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  theData.nodeID = NODEID;  //this node id should be the same for all devices in this node 
  pinMode(rgb3, OUTPUT);
  pinMode(alm, OUTPUT); 
}


void loop()
{  
  int gasMonitor = analogRead(gasPin);
  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod) {
    Serial.print("\ngasMonitor=");
    Serial.println(gasMonitor);

    //send data
    theData.deviceID = 6;
    theData.uptime = millis();
    theData.var2_float = gasMonitor;
    theData.var3_float = 0;

    Serial.print("Sending struct (");
    Serial.print(sizeof(theData));
    Serial.print(" bytes) ... ");
    if (radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData))) {
      Serial.print(" ok!");
    } else {
      Serial.print(" nothing...");
    }
    while(gasMonitor > 100){
      theData.var3_float = 1;                                                     //Alarm signal
      radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData));
      Serial.println("ALARM!! sending alarm");
      playTone(100, 2600);
      analogWrite(rgb3, 255);
      delay(100);
      digitalWrite(rgb3, LOW);
      gasMonitor = analogRead(gasPin);
    }
    analogWrite(alm, 0);
    lastPeriod=currPeriod;
  }
  delay(1000);
}

void playTone(long duration, int freq) {
    duration *= 1000;
    int period = (1.0 / freq) * 1000000;
    long elapsed_time = 0;
    while (elapsed_time < duration) {
        digitalWrite(alm,HIGH);
        delayMicroseconds(period / 2);
        digitalWrite(alm, LOW);
        delayMicroseconds(period / 2);
        elapsed_time += (period);
    }
}