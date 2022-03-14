#include <WiFi.h>                 
#include <PubSubClient.h>       
#include <WiFiClientSecure.h>    
#include "DHT.h"               
     
#define Light_sensor 35
#define DHTPIN 33
#define Soilmoisture_sensor 34          
#define DHTTYPE DHT22
#define MSG_BUFFER_SIZE (50)

const char* ssid = "nazevWifi";
const char* password = "hesloWifi";

const char* mqtt_server = "5df194823s1ag52ad1c9b0aafbdbba686c7.s1.eu.hivemq.cloud";
const char* mqtt_username = "uzivatelBoker";
const char* mqtt_password = "hesloBroker";
const int mqtt_port =8883;
          
const int Pump = 27;
const int LED = 26;
const int Ventilator = 32;
const char* sensorVlhkosti_topic = "sensorVlhkosti";
const char* sensorVlhkostiPudy_topic = "sensorVlhkostiPudy";
const char* sensorIntenzity_topic = "sensorIntenzity";
const char* sensorTeploty_topic = "sensorTeploty";
const char* cerpadloCyklus_topic = "cerpadloCyklus";          
const char* osvetleniCyklus_topic = "osvetleniCyklus";
const char* ventilatorCyklus_topic = "ventilatorCyklus";
const char* manualOsvetleni_topic = "manualOsvetleni";
const char* manualVentilator_topic = "manualVentilator";
const char* manualCerpadlo_topic = "manualCerpadlo";

bool isAutomaticLightOn;
bool isAutomaticPumpOn;
bool isAutomaticVentOn;
bool manualLight;
bool manualVent;
bool manualPump;
int sensorVlhkosti;
int sensorVlhkostiPudy;
int sensorIntenzity;
int sensorTeploty;
int cerpadloCyklus;
int osvetleniCyklus;
int ventilatorCyklus;
int manualOsvetleni;
int manualVentilator;
int manualCerpadlo;
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];

WiFiClientSecure espClient; 
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      client.subscribe(cerpadloCyklus_topic);
      client.subscribe(osvetleniCyklus_topic);
      client.subscribe(ventilatorCyklus_topic);
      client.subscribe(manualOsvetleni_topic);
      client.subscribe(manualVentilator_topic);
      client.subscribe(manualCerpadlo_topic);
      }
      else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(1);
  setup_wifi();

  dht.begin();
  
  pinMode(Pump, OUTPUT);              
  pinMode(LED, OUTPUT);               
  pinMode(Ventilator, OUTPUT);        

  espClient.setCACert(root_ca);       
   
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  int soilValue = analogRead(Soilmoisture_sensor);
  soilValue = map(soilValue,0,2320,0,100);             

  if(isAutomaticPumpOn){
    if (soilValue < 40){
      digitalWrite(Pump, HIGH);
    }
    else{                         
      digitalWrite(Pump, LOW);
    }
  }
  else{
    digitalWrite(Pump, LOW);
  }
  
  if(!isAutomaticPumpOn and manualPump){
      digitalWrite(Pump, HIGH);
  }
  else if(!manualPump and soilValue > 40){
    digitalWrite(Pump, LOW);
  }

  int itensityValue = analogRead(Light_sensor);
  itensityValue = map(itensityValue,4095,0,0,100);

  if(isAutomaticLightOn){
     if (itensityValue < 20){
      digitalWrite(LED, HIGH);
     }
     else{
      digitalWrite(LED, LOW);
     }
   }
    else{
      digitalWrite(LED, LOW);
    }
  
  if(!isAutomaticLightOn and manualLight){
      digitalWrite(LED, HIGH);
  }
  else if(!manualLight and itensityValue > 20){
    digitalWrite(LED, LOW);
  }

  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();

    if(isAutomaticVentOn){
    if (humidity > 40 or temperature > 28){
      digitalWrite(Ventilator, HIGH);
    }
    else{
      digitalWrite(Ventilator, LOW);
    }
    }
    else{
      digitalWrite(Ventilator, LOW);
    }
  
  if(!isAutomaticVentOn and manualVent){
      digitalWrite(Ventilator, HIGH);
  }
  else if(!manualVent and humidity < 40 and temperature < 28){
    digitalWrite(Ventilator, LOW);
  }
 
  if (!client.connected()){ 
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    sensorVlhkosti = humidity;
    sensorVlhkostiPudy = soilValue;
    sensorIntenzity = itensityValue;
    sensorTeploty = temperature;

    publishMessage(sensorVlhkosti_topic,String(sensorVlhkosti),true);    
    publishMessage(sensorVlhkostiPudy_topic,String(sensorVlhkostiPudy),true);
    publishMessage(sensorIntenzity_topic,String(sensorIntenzity),true);
    publishMessage(sensorTeploty_topic,String(sensorTeploty),true);  
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];
  
  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);
                                          
    if( strcmp(topic,cerpadloCyklus_topic) == 0)
   {
     isAutomaticPumpOn = incommingMessage.equals("1");
   }

  else if( strcmp(topic,osvetleniCyklus_topic) == 0)               
  {
    isAutomaticLightOn = incommingMessage.equals("1");  
  }

   else  if( strcmp(topic,ventilatorCyklus_topic) == 0)
  {
     isAutomaticVentOn = incommingMessage.equals("1");
  }

     else  if( strcmp(topic,manualOsvetleni_topic) == 0)
  {
     manualLight = incommingMessage.equals("1");
  }

       else  if( strcmp(topic,manualVentilator_topic) == 0)
  {
     manualVent = incommingMessage.equals("1");
  }

       else  if( strcmp(topic,manualCerpadlo_topic) == 0)
  {
     manualPump = incommingMessage.equals("1");
  }
  
}

void publishMessage(const char* topic, String payload , boolean retained){
  client.publish(topic, (byte*) payload.c_str(), 10, true);
  Serial.println("Message publised ["+String(topic)+"]: "+payload);
}
