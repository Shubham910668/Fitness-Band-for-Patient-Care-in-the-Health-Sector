


#define BLYNK_TEMPLATE_ID           "TMPL3qCXhZ9ma"
#define BLYNK_TEMPLATE_NAME         "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "dXmFHwIz21Yfak1gWPZOcIo3x3o_B66J"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include "MAX30105.h"
#include "heartRate.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
MAX30105 particleSensor;

#define BLYNK_PRINT Serial
char ssid[] = "Pie";
char pass[] = "1234567890";
BlynkTimer timer;


#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#define SW420_PIN 4
// Define a variable to store the vibration state
int vibrationState = 0;
int vibrationCount = 0;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

double new_emissivity = 0.98;
float ambient_temp=0.0;
float object_temp=0.0;

long irValue;
double beatsPerMinute;
int beatAvg;

void de(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Welcome");
  display.println("Initializing...");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void te(){
  
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
       display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 17);
    display.println("Error connecting to MLX sensor. Check wiring.");
    display.display();
    while (1);
  };
 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bpm(){
   if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x08); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0);
}
/////////////////////////////////////////////////////////////////////////////////////
void display_print(){
  if (irValue < 50000)
   { Serial.print(" No finger?");
   display.clearDisplay();
    display.print(" No finger?");
    display.display();
    delay(100);
  }else{
    display.clearDisplay();
     display.setTextSize(1);
     //display.setFont(ArialMT_Plain_16);
    display.setCursor(0, 21);
    
   // display.print("C\tObject = "); display.print(ambient_temp);
    display.println("Body Temp="); display.print(object_temp); display.println("F");

    display.setCursor(0, 7);
   // display.print("IR=");
    //display.print(irValue);
    //display.print(", BPM=");
    //display.print(beatsPerMinute);
    display.print("Heartrate=");
    display.print(beatAvg);
    //display.setCursor(0, 48);
    // display.print("Vibration state: ");
    // display.println(vibrationState);
    
    display.display();
  }
}

void serial_print(){
  Serial.print("Vibration state: ");
  Serial.println(vibrationState);
  Serial.print("Environmental temp = "); Serial.print(ambient_temp);
  Serial.print("F\tBody = "); Serial.print(object_temp); Serial.println("F");

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  Serial.println();
  
}

void hr(){
  irValue = particleSensor.getIR();
  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  //delay(10);  
}
////////////////////////////////////////////////////////
void temp_read(){
  ambient_temp=mlx.readAmbientTempF();
  object_temp=mlx.readAmbientTempC();
  //object_temp=98.6;
  //object_temp=mlx.readObjectTempC();
}
///////////////////////////////////////////////

// BLYNK_WRITE(V0)
// {
//   // Set incoming value from pin V0 to a variable
//   int value = param.asInt();
//   BLYNK_PRINT.println(value);
//   // Update state
//   //Blynk.virtualWrite(V1, value);
// }
//////////////////////////////////////////////////
void myTimerEvent()
{
  Blynk.virtualWrite(V1,vibrationCount);
  Blynk.virtualWrite(V2,beatAvg );
  Blynk.virtualWrite(V4, object_temp);
  Blynk.virtualWrite(V3, ambient_temp);
  //BLYNK_PRINT.println("uploaded");
}
///////////////////////////
void blnk(){
 Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // Setup a function to be called every second
  timer.setInterval(1100L, myTimerEvent);
  timer.setInterval(300L,temp_read);
}
///////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  while (!Serial);
  blnk();
  pinMode(SW420_PIN, INPUT);
  Serial.println("Initializing Display....");
  de();

  Serial.println("Adafruit MLX90614 test");
  te();

  Serial.println("Adafruit MAX30102 test");
  bpm();
  
  Serial.print("Emissivity = "); Serial.println(mlx.readEmissivity());
  mlx.writeEmissivity(0.95); 
  Serial.println("================================================");

  
}
long print_time=millis();
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {


  // double new_Emissivity = 0.95; // Set your new emissivity value here
  // //writeEmissivity(newEmissivity);
  // delay(1000);



  vibrationState = digitalRead(SW420_PIN);
  // Print the vibration state on the serial monitor
  // Check if vibration state is HIGH (vibration detected)
  if (vibrationState == HIGH) {
    vibrationCount++;  // Increment the vibration count

    // Check if vibration state has been HIGH for more than 30 consecutive times
    if (vibrationCount >5) 
    { Blynk.logEvent("vibration");
      Serial.println("Alert: ");
      display.clearDisplay();
      display.println("Alert: ");
      display.display();
      // You can add additional actions here, such as triggering an alarm or sending a notification.
      vibrationCount = 0;  // Reset the vibration count
    }
  } else {
    vibrationCount = 0;  // Reset the vibration count if no vibration is detected
  }
 
  
  temp_read();
  hr();
  
  if(millis()-print_time>1000){
  display_print();
  serial_print();
  print_time=millis();
  }
  else if(object_temp>100)
  {  
    Blynk.logEvent("vibration");
    
  }
  else if(beatAvg>125)
  {
    Blynk.logEvent("vibration");
  }
  // Wait for 100 milliseconds
  //delay(100);
  //delay(500);
  Blynk.run();
  timer.run();
}


