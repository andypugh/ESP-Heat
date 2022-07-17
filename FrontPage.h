server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  // Display the status web page
  char timeStr[50];
  strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  String content = "<!DOCTYPE html>";
  __P("<head><meta http-equiv='refresh' content='60'></head>");
  __P("<html>");
  __P("<link rel='icon' href='data:,'>");
  __P("<body>");
  __P("<svg svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' version='1.1' width='297mm' height='210mm'>");
  __P("<text y='70' x='60' font-size='40' fill='#ao5a2c' font-family='Times' > %s </text>", timeStr);
  for (int i = 0; i < num_boilers; i++) {
    __P("<text y='%d' x='80' font-size='30' dominant-baseline='middle' text-anchor='left' fill='0' font-family='Times' >Boiler %d %s",
        35 * i + 110, i, boiler_states[boilers[i].state]);
    if (boilers[i].f_sensor.address[0]) {
      __P("&nbsp;&nbsp;&nbsp;Flow: %d.%1d%s", (int)boilers[i].f_temp, (int)(boilers[i].f_temp * 10) % 10, units ? "&#8457" : "&#8451");
    }
    if (boilers[i].r_sensor.address[0]) {
      __P("&nbsp;&nbsp;&nbsp;Ret: %d.%1d%s", (int)boilers[i].r_temp, (int)(boilers[i].r_temp * 10) % 10, units ? "&#8457" : "&#8451");
    }
    __P("</text>");
  }
  for (int i = 0; i < num_zones; i++) {
    __P("<text y='%d' x='80' font-size='30' dominant-baseline='middle' text-anchor='left' fill='0' font-family='Times' >Valve  %d %s</text>",
        35 * i + 130 + 30 * num_boilers, i, valve_states[zones[i].state]);
  }
  for (int i = 0; i < num_pumps; i++) {
    __P("<text y='%d' x='475' font-size='30' dominant-baseline='middle' text-anchor='left' fill='0' font-family='Times' >Pump  %d %s</text>",
        35 * i + 130 + 30 * num_boilers, i, digitalRead(pumps[i].out_pin)?"Off":"On");
  }
  for (int i = 0; i < num_zones; i++) {
    __P("<a xlink:href='set?zone=%d'><rect y='%d' x='%d' height='%d' width='%d' style='fill:%s;stroke:#000000;stroke-width:3'/></a>",
        i, zone_pos[i][0], zone_pos[i][1], zone_pos[i][2], zone_pos[i][3], bitRead(zone_on, i) ? on_colour : off_colour);
    __P("<text y='%d' x='%d' font-size='20' dominant-baseline='middle' text-anchor='middle' fill='0' font-family='Times' >%s</text>",
        zone_pos[i][0] + zone_pos[i][2] - 20,
        zone_pos[i][1] + zone_pos[i][3] / 2,
        zones[i].name);
    __P("<text y='%d' x='%d' font-size='40' dominant-baseline='middle' text-anchor='middle' fill='0' font-family='Times' >%d.%1d%s</text>",
        zone_pos[i][0] + 60,
        zone_pos[i][1] + zone_pos[i][3] / 2,
        (int)zones[i].temp, abs((int)(zones[i].temp * 10) % 10),
        units ? "&#8457" : "&#8451");
  }
  __P("<a xlink:href='settings'><text y='650' x='60' font-size='20' fill='#0000FF' font-family='Times' >Setup</text></a>");
  __P("</svg>");
  __P("</body></html>");
  if (!request->authenticate(http_user, http_pass)) return request->requestAuthentication();
   request->send(200, "text/html", content);
});

server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
  request->redirect("/");
  ESP.restart();
});


/*

// There is probably a better way, these functions find the middle of the bounding box of an SVG path.
int midx(char* p){
  int x, y;
  char *c;
  int xmax = 0;
  int xmin = 1000;
  c = strtok(p, " \0");
  while (c != NULL) {
    switch (*c){
      case 'M':
      sscanf(strtok(NULL, " \0"), "%d", &x);
    }
    
  }
}
*/
