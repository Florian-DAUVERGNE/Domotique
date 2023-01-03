#include <ESP32Time.h>

//Bibliothèque utilisée pour le serveur
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

//Bibliothèque utilisée pour le TI
#include <EmonLib.h>

//Biblioth.èque utilisée pour le thermomètre
#include <OneWire.h>
#include <DallasTemperature.h>

ESP32Time rtc(3600);


//Déclaration des paramètres du serveur
AsyncWebServer server(80);
const char *ssid = "";
const char *password = "";
IPAddress local_IP(192,168,1,22);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(192,168,1,1);
IPAddress secondaryDNS(192,168,1,1);

//Déclaration des paramètres du TI
#define pin_TI 35
EnergyMonitor emon1;
float tension = 230;
float courant = 0;
float puissance = 0;
float consommationELECTRICITE=0;

//Déclaration des paramètres du thermomètre
#define pin_TEMPERATURE 32
OneWire oneWire(pin_TEMPERATURE);
DallasTemperature sensors(&oneWire);
float temperature=0;

//Déclaration des paramètres de la cellule crépusculaire
#define celluleCrepusculaire 33
int luminosite;

//Déclaration des paramètres du débitmètre
#define pin_DEBITMETRE 34
float consommationEAU=0;
int etatActuelDebitmetre = 0;
int etatMemoriseDebitmetre = 0;
int NombresRotations = 0;
float debit= 0;
unsigned long TempsActuel;
unsigned long TempsAncien;

unsigned long TempsDebutProgramme;
float TempsEcoule;
unsigned long TempsFinProgramme;
unsigned long Timer;
unsigned long TimerJourNuit;


//Déclaration des paramètres du module relai
#define pin_relai1 19
#define pin_relai2 18
#define pin_relai3 5
#define pin_relai4 17
#define pin_relai5 16
#define pin_relai6 4
#define pin_relai7 0
#define pin_relai8 2

float consommationEauAnnuelle[12]={};
float consommationElectriciteAnnuelle[12]={};
int date[6]={};

void initialisationRelai(int relai){
  pinMode(relai, OUTPUT);
  digitalWrite(relai, HIGH);
}

String transformerTableauEnFichier(float tableau[12])
{
  String temp=String(tableau[0])+","+String(tableau[1])+","+String(tableau[2])+","+String(tableau[3])+","+String(tableau[4])+","+String(tableau[5])+",";
  temp+=String(tableau[6])+","+String(tableau[7])+","+String(tableau[8])+","+String(tableau[9])+","+String(tableau[10])+","+String(tableau[11]);
  return temp;
}

void transformerFichierEnTableau(String fichier,float tableau[12]){
String temp[12]={};
int j =0;
for(int i=0;i<fichier.length();i++){
  if(fichier[i]!=','){
    temp[j]+=fichier[i];
    }
   else if(fichier[i]==','){
    j++;
    }
    else{break;}
  }

  for(int i=0;i<12;i++){
   tableau[i]=temp[i].toFloat();
  }
}

void recupererFichiers()
{
  if(!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  File fichier ;

  if(fichier = SPIFFS.open("/consoEau.txt","r"))
  {
    String contenuFichier=fichier.readStringUntil('n');
    
    transformerFichierEnTableau(contenuFichier,consommationEauAnnuelle);
    
    fichier.close();
  }  
  
  if(fichier = SPIFFS.open("/consoElec.txt","r"))
  {
    String contenuFichier=fichier.readStringUntil('n');
    
    transformerFichierEnTableau(contenuFichier,consommationElectriciteAnnuelle);
    
    fichier.close();
  }

    if(fichier = SPIFFS.open("/date.txt","r"))
  {
    String contenuFichier=fichier.readStringUntil('n');
    String temp[6]={};
    int j =0;
      
      for(int i=0;i<contenuFichier.length();i++){
        if(contenuFichier[i]!=','){
          temp[j]+=contenuFichier[i];
          }
         else if(contenuFichier[i]==','){
          j++;
          }
          else{break;}
        }
      
        for(int i=0;i<6;i++){
         date[i]=temp[i].toInt();
        }
    fichier.close();
  }

}

void calculerDebitEau()
{
   TempsAncien = millis();
  
  while(TempsActuel<TempsAncien+1000)
  {
    TempsActuel = millis();
    
    etatActuelDebitmetre = digitalRead(pin_DEBITMETRE);
    
    if (etatActuelDebitmetre != etatMemoriseDebitmetre)
    {
      etatMemoriseDebitmetre = etatActuelDebitmetre;
      
      if (etatActuelDebitmetre == 0)
      {
        NombresRotations++;
      }
    }
  }
  debit=NombresRotations*0.00208;
  NombresRotations=0;
  TempsAncien=millis(); 
}

void demarrerServeur()
{
  if(!WiFi.config(local_IP,gateway,subnet,primaryDNS,secondaryDNS))
  {
    Serial.println("Wifi failed");
  }
  
  WiFi.begin(ssid, password);
  
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }
  
  server.begin();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/ConsoEau", HTTP_GET, [](AsyncWebServerRequest *request)
  {    
    request->send(200,"text/plain",transformerTableauEnFichier(consommationEauAnnuelle));
  });

  server.on("/ConsoElec", HTTP_GET, [](AsyncWebServerRequest *request)
  {    
    request->send(200,"text/plain",transformerTableauEnFichier(consommationElectriciteAnnuelle));
  });

  server.on("/JourNuit",HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(200, "text/plain",String(digitalRead(pin_relai8)));
  });

  server.on("/EtatRelais", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    String EtatRelais=String(digitalRead(pin_relai1))+","+String(digitalRead(pin_relai2))+","+String(digitalRead(pin_relai3))+","+String(digitalRead(pin_relai4))+",";
    
    EtatRelais+=String(digitalRead(pin_relai5))+","+String(digitalRead(pin_relai6))+","+String(digitalRead(pin_relai7))+","+String(digitalRead(pin_relai8));
    
    request->send(200, "text/plain",EtatRelais);
  });

    server.on("/Puissanceinstantanee", HTTP_GET, [](AsyncWebServerRequest *request)
  {
     courant = emon1.calcIrms(1480);
     puissance= courant*tension;
     request->send(200, "text/plain",String(puissance));
  });

  server.on("/Temperature", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    sensors.requestTemperatures();
    temperature=sensors.getTempCByIndex(0);
    request->send(200, "text/plain",String(temperature,1));
  });
  
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    int Relai=request->getParam("Relai")->value().toInt();
    
    switch (Relai)
    {
    case 1:Relai=pin_relai1;break;
    case 2:Relai=pin_relai2;break;
    case 3:Relai=pin_relai3;break;
    case 4:Relai=pin_relai4;break;
    case 5:Relai=pin_relai5;break;
    case 6:Relai=pin_relai6;break;
    case 7:Relai=pin_relai7;break;
    case 8:Relai=pin_relai8;break;
    }

    if(Relai==pin_relai1 || Relai==pin_relai3)
    {
     request->send(200,"text/plain","Impulsion");
     digitalWrite(Relai,LOW);
     delay(1000);
     digitalWrite(Relai,HIGH);
    }

    else
    {
      if(digitalRead(Relai)==1)
      {
        digitalWrite(Relai,LOW);
      }
      else
      {
        digitalWrite(Relai,HIGH);
      }
      request->send(200,"text/plain",String(digitalRead(Relai)));
    }
   });
}

void ressourcesHTML(){
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
     request->send(SPIFFS, "/script.js", "text/javascript");
  });

    server.on("/jquery.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/jquery.js", "text/javascript");
  });

   server.on("/chart.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/chart.js", "text/javascript");
  });

     server.on("/LedAllume.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/LedAllume.png", "image/png");
  });

        server.on("/home.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/home.png", "image/png");
  });
          server.on("/LedHS.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/LedHS.png", "image/png");
  });
            server.on("/sun.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/sun.png", "image/png");
  });
            server.on("/moon.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/moon.png", "image/png");
  });

        server.on("/LedEteinte.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/LedEteinte.png", "image/png");
  });
}

void setup()
{
 Serial.begin(9600);
 pinMode(pin_DEBITMETRE,INPUT);
 pinMode(celluleCrepusculaire,INPUT);
 
 emon1.current(pin_TI,6.5);

 initialisationRelai(pin_relai1);
 initialisationRelai(pin_relai2);
 initialisationRelai(pin_relai3);
 initialisationRelai(pin_relai4);
 initialisationRelai(pin_relai5);
 initialisationRelai(pin_relai6);
 initialisationRelai(pin_relai7);
 initialisationRelai(pin_relai8);

 recupererFichiers();
 demarrerServeur();
 ressourcesHTML();
 for(int i=0;i<=5;i++)
 {
  emon1.calcIrms(1480);
 }
 Timer=millis();
 TimerJourNuit=millis();
 rtc.setTime(date[0],date[1],date[2],date[3],date[4],date[5]);
}
void loop()
{  
 TempsFinProgramme = millis()-TempsDebutProgramme;
 TempsDebutProgramme = millis();
 TempsEcoule = TempsFinProgramme*0.001;
 
 calculerDebitEau();
 
 consommationEAU+=debit*TempsEcoule;
  
 courant = emon1.calcIrms(1480);
 puissance= courant*tension;

 consommationELECTRICITE+=(puissance/3600)*TempsEcoule;
  
  if(millis()>=Timer+30000)
 {
  Timer=millis();
  
    
  consommationEauAnnuelle[rtc.getMonth()]+=consommationEAU;      

  consommationEAU=0;
  
  File file=SPIFFS.open("/consoEau.txt","w");
  file.println(transformerTableauEnFichier(consommationEauAnnuelle));
  file.close();

  consommationElectriciteAnnuelle[rtc.getMonth()]+=consommationELECTRICITE;

  consommationELECTRICITE=0;

  file=SPIFFS.open("/consoElec.txt","w");
  file.println(transformerTableauEnFichier(consommationElectriciteAnnuelle));
  file.close();

  file=SPIFFS.open("/date.txt","w");
  file.println(String(rtc.getSecond())+","+String(rtc.getMinute())+","+String(rtc.getHour(true))+","+String(rtc.getDay())+","+String((rtc.getMonth()+1))+","+String(rtc.getYear()));
  file.close();
 }

   if(millis()>=TimerJourNuit+3600000)
 {
  TimerJourNuit=millis();
  luminosite=analogRead(celluleCrepusculaire);
  if(luminosite<=1000)
  {
    Serial.println(luminosite);
    digitalWrite(pin_relai8,LOW);
    Serial.println("Nuit");
  }
  else
  {
    Serial.println(luminosite);
    digitalWrite(pin_relai8,HIGH);
    Serial.println("Jour");
  }
  
 }
}
