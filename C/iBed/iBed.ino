#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <String.h>
#include <stdio.h>
#include <FastLED.h>

#define DEBUG 1
#define END_CHAR '/'
#define ID 2

#define LED_PIN     2
#define NUM_LEDS    144
#define UPDATES_PER_SECOND 100

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];


#define __SET 0
#define __GET 1

String SERVER = "192.168.33.246"; // To complete by real address

int BRIGHTNESS = 60;
int __current_state = 0;
int __current_color[3] = {41, 128, 185}; // rgb(41, 128, 185)

const char *ssid = "Appartouze_City_Gang";
const char *password = "Cafsouris220";

ESP8266WebServer server(80);
HTTPClient http;


CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


void setup() {

    delay(3000);

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);


    delay(5000);

    while(WiFi.status() != WL_CONNECTED){
        delay(500);

        Serial.print('.');
    }

    Serial.println();

    server.on("/set", handleNewGetRequest);  // setData?deviceID=$ID&command=$cmd
    server.on("/get", handleNewGetRequest);  // setData?deviceID=$ID&command=$cmd
    server.onNotFound ( handleNotFound );
	server.begin();

    Serial.println(WiFi.localIP().toString());
    
}

void loop() {
    server.handleClient();

    ChangePalettePeriodically(__current_state);
    
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    
    FillLEDsFromPaletteColors(startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void handleNewGetRequest() {

    String power = "-1";
    String deviceID = "";

    if (server.args() > 0 && server.uri() == "/set") { // 
        
        if (server.argName(0) == "power") {
            deviceID = server.arg(0);
            powerManager(server.arg(0).toInt());
        }

        if (server.argName(0) == "state") {
            deviceID = server.arg(0);
            __current_state = server.arg(0).toInt();
        }

        if (server.argName(0) == "r" && server.argName(1) == "g" && server.argName(2) == "b") {
            deviceID = server.arg(0);

            __current_color[0] = server.arg(0).toInt();
            __current_color[1] = server.arg(1).toInt();
            __current_color[2] = server.arg(2).toInt();

            __current_state = 25;
        }

        if (deviceID == "-1") {
            handleNotFound(); 
        }

        Serial.println();

        handleRoot(__SET);
        
    } else if (server.uri() == "/get") {
        handleRoot(__GET);
    } else {
        handleNotFound();
    }
}

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

    if (DEBUG) {
        server.send (404, "text/plain", message );
    } else {
        server.send(404, "text/plain", "{\"message\" : \"request_aborted\"}");
    }
}

void handleRoot(int __params) {
    if (__params == __SET) {
        server.send(200, "text/html", "{\"message\" : \"request_success\"}");
    } else if (__params == __GET) {
        String message = "{\"message\" : \"request_success\",";
        message += "\"state\" : ";
        message += __current_state;
        message += ",\"brightness\" : ";
        message += BRIGHTNESS;
        message += "}";
        server.send(200, "text/html", message);
    } else {
        handleNotFound();
    }
    
}

void executeCommand(String command) {
    char socket[80];

    int buffer_l = command.length() + 1;
    char buffer[buffer_l];

    command.toCharArray(buffer, buffer_l);

    sscanf(buffer, "deviceID=0&cmd=%s", socket);

    String url = "/setData?deviceID=0";
        url += "&command=";
        url += socket;

        sendData(SERVER, url);  
    
}

void sendData(String ip, String url) {

    ip += url;

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    Serial.print("[HTTP] URL :: http://");
    Serial.println(ip);

    if (http.begin(client, "http://" + ip)) {


      Serial.print("[HTTP] GET...\n");

      int httpCode = http.GET();

      if (httpCode > 0) {
        
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP] Unable to connect\n");
    }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;

    if (__current_state == 25) {
        fill_solid(leds, 144, CRGB(__current_color[0], __current_color[1], __current_color[2]));
        return;
    }
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically(int __value)
{
    
        if( __value ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( __value == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( __value == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( __value == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( __value == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( __value == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( __value == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( __value == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( __value == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( __value == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }

}


// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};


void powerManager(int brightness) {

    if (brightness > 100)
        brightness = 100;

    if (BRIGHTNESS < brightness) {
        for (int i = BRIGHTNESS; i < brightness; i++) {
            FastLED.setBrightness(map(i, 0, 100, 0, 255));
            FastLED.show();
            delay(1);
        }
    } else {
        for (int i = BRIGHTNESS; i > brightness; i--) {
            FastLED.setBrightness(map(i, 0, 100, 0, 255));
            FastLED.show();
            delay(1);
        }
    }

    if (brightness == 0) 
        FastLED.setBrightness(0); FastLED.show();

    if (brightness == 100)
        FastLED.setBrightness(255); FastLED.show();

    BRIGHTNESS = brightness;

    return;

    FastLED.setBrightness(  BRIGHTNESS );
}