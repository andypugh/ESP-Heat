server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
  int z = request->arg("zone").toInt();
  Serial.printf("Setting Screen: setting zone %d\n", z);
  program(request, z);
});

server.on("/hour", HTTP_GET, [](AsyncWebServerRequest *request){
  int z = request->arg("zone").toInt();
  int h = request->arg("h").toInt();
  if (z < 0 || z > num_zones - 1 || h < 0 || h > 23) return;
  zones[z].hours &= 0x00FFFFFF; // set to auto mode if user tries to program
  zones[z].hours ^= (1L << h);
  EEPROM.write(z * 8 + 1, (zones[z].hours & 0x00FF0000) >> 16);
  EEPROM.write(z * 8 + 2, (zones[z].hours & 0x0000FF00) >>  8);
  EEPROM.write(z * 8 + 3, (zones[z].hours & 0x000000FF));
  EEPROM.commit();
  debounce(request, z);
});

server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request){
  int z = request->arg("zone").toInt();
  int a = request->arg("a").toInt();
  if (z < 0 || z > num_zones - 1 || a < 0 || a > 2) return;
  zones[z].hours&= 0x00FFFFFF;
  zones[z].hours += (long)a * 0x1000000;
  EEPROM.write(z * 8 + 0, (zones[z].hours& 0xFF000000) >> 24);
  EEPROM.commit();
  debounce(request, z);
});

server.on("/on_step", HTTP_GET, [](AsyncWebServerRequest *request){
  int z = request->arg("zone").toInt();
  int d = request->arg("d").toInt();
  if (z < 0 || z > num_zones - 1) return;
  zones[z].on_temp += d;
  EEPROM.write(z * 8 + 4, zones[z].on_temp);
  EEPROM.commit();
  debounce(request, z);
});

server.on("/off_step", HTTP_GET, [](AsyncWebServerRequest *request){
  int z = request->arg("zone").toInt();
  int d = request->arg("d").toInt();
  if (z < 0 || z > num_zones - 1) return;
  counter[0] = counter[1] = counter[3] = 0;
  zones[z].off_temp += d;
  EEPROM.write(z * 8 + 5, zones[z].off_temp);
  EEPROM.commit();
  debounce(request, z);
});

server.on("/update_temp", HTTP_GET, [](AsyncWebServerRequest *request){
  int z = request->arg("zone").toInt();
  int t = request->arg("temp").toFloat();
  if (z < 0 || z > num_zones - 1) return;
  zones[z].temp = t;
  String content = "<!DOCTYPE html>";
  __P("Zone %d set to temperature %d\n", z, t);
  request->send(200, "text/html", content);
});
