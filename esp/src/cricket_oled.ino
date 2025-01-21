#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <U8g2lib.h>

#include "config.h"

// Used for SPI are: D5, D6, D7
// Args are u8g2(rotation, cs = D8, dc = anything use D1, reset = anything use D4)
U8G2_SSD1305_128X32_ADAFRUIT_F_4W_HW_SPI u8g2(U8G2_R0, D8, D1, D4);
const int display_width = u8g2.getDisplayWidth();
unsigned long scrolling_time = 0;
unsigned long match_display_time = 0;
unsigned long last_button_press_time = 0;
unsigned long current_millis;
int times_round_counter = 0;
int goes_round = 0;
JsonDocument doc;
JsonArray arr;
JsonArray::iterator it;

struct match_data_t{
  String m_d;
  String m_s;
  String t1_d;
  String t2_d;

  int width_md;
  int width_ms;
  int width_t1;
  int width_t2;
} match_data;

struct offsets_t{
  int offset_md;
  int offset_ms;
  int offset_t1;
  int offset_t2;
} offsets;

match_data_t get_match_data(JsonObject repo){
  String match_details = repo["match_details"];
  String match_status = repo["match_status"];
  String team_1_score = repo["team_1_score"];
  String team_1_name = repo["team_1_name"];
  String team_1_details = team_1_name + ": " + team_1_score;
  String team_2_score = repo["team_2_score"];
  String team_2_name = repo["team_2_name"];
  String team_2_details = team_2_name + ": " + team_2_score;
  Serial.println("Got details");
  Serial.print("Match details: ");
  Serial.println(match_details);
  Serial.print("Match status: ");
  Serial.println(match_status);
  Serial.print("Team 1 name: ");
  Serial.println(team_1_name);
  Serial.print("Team 1 score: ");
  Serial.println(team_1_score);
  Serial.print("Team 1 details: ");
  Serial.println(team_1_details);
  Serial.print("Team 2 name: ");
  Serial.println(team_2_name);
  Serial.print("Team 2 score: ");
  Serial.println(team_2_score);
  Serial.print("Team 2 details: ");
  Serial.println(team_2_details);

  int width_md = u8g2.getStrWidth((char*)match_details.c_str());
  int width_ms = u8g2.getStrWidth((char*)match_status.c_str());
  int width_t1 = u8g2.getStrWidth((char*)team_1_details.c_str());
  int width_t2 = u8g2.getStrWidth((char*)team_2_details.c_str());

  if (width_md > display_width){
    match_details.concat(" ");
    width_md = u8g2.getStrWidth((char*)match_details.c_str());
  }
      
  if (width_ms > display_width){
    match_status.concat(" ");
    width_ms = u8g2.getStrWidth((char*)match_status.c_str());
  }
      
  if (width_t1 > display_width){
    team_1_details.concat(" ");
    width_t1 = u8g2.getStrWidth((char*)team_1_details.c_str());
  }
      
  if (width_t2 > display_width){
    team_2_details.concat(" ");
    width_t2 = u8g2.getStrWidth((char*)team_2_details.c_str());
  }

  match_data_t match_data = {
    match_details,
    match_status,
    team_1_details,
    team_2_details,
    width_md,
    width_ms,
    width_t1,
    width_t2
  };
  return match_data;
}

void draw_line(int &offset, int y_pos, int width, const char* text){
  if (width > display_width){
    int x = offset;
    do {
      u8g2.drawStr(x, y_pos, text);
      x += width;
    } while (x < display_width);

    offset -= 1;
    if (offset < -width) {
      offset = 0;
    }
  } else {
    u8g2.drawStr(0, y_pos, text);
  }
}

int get_goes_round(int num_el){
  int goes_round;
  Serial.print("num_el: ");
  Serial.println(num_el);
  Serial.println("");
  if( num_el > 10) {
    goes_round = num_el;
  } else{
    goes_round = num_el;
  }
  return goes_round;
}

void scroll_match_details(offsets_t &offsets, match_data_t &match_data){
  unsigned long cm_scroll = millis();
  const char* m_d = (char*)match_data.m_d.c_str();
  const char* m_s = (char*)match_data.m_s.c_str();
  const char* t1_d = (char*)match_data.t1_d.c_str();
  const char* t2_d = (char*)match_data.t2_d.c_str();
    
  if (cm_scroll - scrolling_time >= 50){
    scrolling_time = cm_scroll;
    u8g2.clearBuffer();

    draw_line(offsets.offset_md, 7, match_data.width_md, m_d);
    draw_line(offsets.offset_ms, 15, match_data.width_ms, m_s);
    draw_line(offsets.offset_t1, 23, match_data.width_t1, t1_d);
    draw_line(offsets.offset_t2, 31, match_data.width_t2, t2_d);

    u8g2.sendBuffer();
  }
}

JsonDocument get_json(){
  JsonDocument doc;
  HTTPClient http;
  WiFiClient client;
  http.useHTTP10(true);
  http.begin(client, endpoint);
  int httpCode = http.GET();
  Serial.println("httpCode below");
  Serial.println(httpCode);
  Serial.println("------");
  Serial.println("Did request");

  // TODO: Error handling
  DeserializationError err = deserializeJson(doc, http.getStream());
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.f_str());
  }
  Serial.println("Did decode");
  http.end();

  return doc;
}

void setup() {
  pinMode(D3, INPUT_PULLUP);
  u8g2.begin();
  u8g2.setFont(u8g2_font_5x7_tf); // Reasonable choice

  Serial.begin(115200);
  Serial.println("Remote test!");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  WiFi.persistent(false);

  WiFi.begin(ssid, password);

  delay(1000);
  char spinny[5] = "|/-\\";
  int spinnyCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    Serial.print(WiFi.status());
    u8g2.clearBuffer();
    u8g2.setCursor(0, 8);
    u8g2.print(spinny[spinnyCounter]);
    u8g2.sendBuffer();
    if(spinnyCounter == 3){
      spinnyCounter = 0;
    }
    else{
      spinnyCounter++;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {

  if (times_round_counter == goes_round){
    doc = get_json();
    arr = doc.as<JsonArray>();
    Serial.println("Got array");
    goes_round = get_goes_round(arr.size());
    Serial.print("goes_round is: ");
    Serial.println(goes_round);
    it = arr.begin();
    match_data = get_match_data(*it);
    offsets = {0, 0, 0, 0};
    times_round_counter = 0;
    match_display_time = millis();
  }

  current_millis = millis();
  if (current_millis - match_display_time >= 60000) {
    ++it;
    if (it == arr.end()){
      it = arr.begin();
    }
    match_display_time = current_millis;
    match_data = get_match_data(*it);
    offsets = {0, 0, 0, 0};
    times_round_counter++;
    Serial.print("times_round_counter is: ");
    Serial.println(times_round_counter);
  } else {
    scroll_match_details(offsets, match_data);
  }

  current_millis = millis();
  if((digitalRead(D3) == LOW)
     && (current_millis - last_button_press_time >= 500)){
    last_button_press_time = current_millis;
    ++it;
    if (it == arr.end()){
      it = arr.begin();
    }
    times_round_counter++;
    match_display_time = current_millis;
    match_data = get_match_data(*it);
    offsets = {0, 0, 0, 0};
  }
}
