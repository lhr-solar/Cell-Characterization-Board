#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include <driver/dac.h>

// Replace with your network credentials
char ssid[17] = {0};
char password[17] = {0};

// Set up a web server on port 80
WiFiServer server(80);



// ADC Pin
const int adcPin = 32;  // Change this if using a different pin

void setup() {
  Serial.begin(115200);
  EEPROM.begin(34);
  dac_output_voltage(DAC_CHAN_1, 128);
  // read previous ssid and password from flash
  for (int j = 0; j <= 16; j++){
    ssid[j] = EEPROM.read(j);
  }
  Serial.println(ssid);
  for (int j = 0; j <= 16; j++){
    password[j] = EEPROM.read(j+17);
  }
  Serial.println(password);
  if (ssid[0] == 0 || password[0] == 0){
    Serial.println("No previous saved network, please input a new one");
    setupNewNetwork();
  }
  else {
    Serial.println("Previous saved network detected, would you like to use it again?");
    char in = -1;
    while (!(in == 'y' || in == 'n')){in = Serial.read();}
    if (in == 'n'){
      Serial.read(); // new line
      Serial.read(); // carriage return
      setupNewNetwork();
    }
  }
  
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.println("IP address: " + WiFi.localIP().toString());

  // Start mDNS
  if (!MDNS.begin("cellcharacterization")) {
    Serial.println("Error setting up mDNS responder!");
    while (1) delay(1000);
  }
  Serial.println("mDNS responder started");

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  WiFiClient client = server.available();  // Check for incoming client

  if (client) {
    Serial.println("New Client Connected.");
    String request = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (c == '\n') {  // End of request
          Serial.println(request);

          // Read ADC value
          int adcValue = analogRead(adcPin);
          float voltage = adcValue * (3.3 / 4095.0);  // Convert ADC to voltage (assuming 12-bit resolution)

          // Handle the "/adc" request separately
          if (request.indexOf("GET /adc") >= 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            client.println(adcValue);  // Send only ADC value
            break;
          }

          // Send HTML response
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<style>html { font-family: Helvetica; text-align: center; }</style>");
          
          // JavaScript for auto-updating ADC value
          client.println("<script>");
          client.println("function updateADC() {");
          client.println("  fetch('/adc').then(response => response.text()).then(data => {");
          client.println("    let voltage = (parseInt(data) * 3.3 / 4095).toFixed(2);");
          client.println("    document.getElementById('adcValue').innerText = data;");
          client.println("    document.getElementById('voltageValue').innerText = voltage + 'V';");
          client.println("  });");
          client.println("}");
          client.println("</script>");



          client.println("</head><body>");
          client.println("<h1>ESP32 ADC Web Server</h1>");
          client.println("<p>ADC Pin Value: <span id='adcValue'>---</span></p>");
          client.println("<p>Voltage: <span id='voltageValue'>---</span></p>");
          client.println("<button onclick='updateADC()'>Read ADC</button>");  // Button to trigger ADC reading
          client.println("</body></html>");

          client.println();
          
          break;
        }
      }
    }
    
    delay(10);
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void setupNewNetwork(void){
  // Connect to WiFi
  Serial.println("Input Network SSID:");
  int in = -1;
  int i = 0;
  while (in != 0x0A && i <= 16){
    in = Serial.read();
    while (in == -1){in = Serial.read();}
    ssid[i] = (char)in;
    i++;
  }
  Serial.read(); // carriage return
  ssid[i-1] = 0;
  Serial.println(ssid);
  Serial.println("Input Network Password:");
  in = -1;
  i = 0;
  while (in != 0x0A && i <= 16){
    in = Serial.read();
    while (in == -1){in = Serial.read();}
    password[i] = (char)in;
    i++;
  }
  Serial.read(); // carriage return
  password[i-1] = 0;
  Serial.println(password);
  // save new network settings to flash
  for (int j = 0; j <= 16; j++){
    EEPROM.write(j,ssid[j]);
  }
  for (int j = 0; j <= 16; j++){
    EEPROM.write(j+17,password[j]);
  }
  EEPROM.commit();
}
