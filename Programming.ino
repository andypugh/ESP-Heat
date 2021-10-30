
// Programming Screen
void program(int z) {
  if (z >= 0 && z < num_zones) {
    String content = "<!DOCTYPE html>";
    __P("<head> </head>");
    __P("<html>");
    __P("<link rel='icon' href='data:,'>");
    __P("<body>");
    __P("<svg svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' version='1.1' width='297mm' height='210mm'>");
    // Header
    __P("<text y='100' x='60' font-size='30' fill='0' font-family='Times' >%s</text>", zones[z].name);
    // Setting clock faces
    for (int h = 0; h < 24; h++) {
      int r = 100;
      __P("<a xlink:href='hour?zone=%02d&h=%02d'><path d='M 0 0 L %d %d A %d %d 0 0 1 %d %d L 0 0' style='fill:%s;stroke:#000000;stroke-opacity:1' transform='translate(%d,300)' /></a>",
          z, h,
          (int)(r * sin(0.5236 * h)), (int)(-r * cos(0.5236 * h)),
          r, r, (int)(r * sin(0.5236 * (h + 1))), (int)(-r * cos(0.5236 * (h + 1))),
          (zones[z].hours& (1 << 24 | 1 << h) && !(zones[z].hours& 1 << 25)) ? on_colour : off_colour,
          (h < 12) ? 200 : 500);
      __P("<text x='%d' y='%d' font-size='15' fill='#000000' dominant-baseline='middle' text-anchor='middle' font-family='Times' transform='translate(%d,300)'> %02d </text>",
          (int)((r + 20) * sin(0.5236 * h)), (int)(-(r + 20) * cos(0.5236 * h)),
          (h < 12) ? 200 : 500,
          h);
    }
    // Set / display on temperature
    __P("<rect y='480' x='130' height='80' width='80' style='fill:%s;stroke:#000000;stroke-width:3'/>", on_colour);
    __P("<text y='520' x='170' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='middle' font-family='Times'> %d.%1d </text>",
        (int)(zones[z].on_temp / 2), (int)(zones[z].on_temp * 5) % 10);
    __P("<a xlink:href='on_step?zone=%02d&d=10'> <path d='M 129 475 l 54 0 l -27 -25 z' /></a>", z);
    __P("<a xlink:href='on_step?zone=%02d&d=-10'><path d='M 129 565 l 54 0 l -27  25 z' /></a>", z);
    __P("<a xlink:href='on_step?zone=%02d&d=1'>  <path d='M 184 475 l 27 0 l -14 -13 z' /></a>", z);
    __P("<a xlink:href='on_step?zone=%02d&d=-1'> <path d='M 184 565 l 27 0 l -14  13 z' /></a>", z);
    // Set / display off temperature
    __P("<rect y='480' x='290' height='80' width='80' style='fill:%s;stroke:#000000;stroke-width:3'/>", off_colour);
    __P("<text y='520' x='330' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='middle' font-family='Times'> %d.%1d </text>",
        (int)(zones[z].off_temp / 2), (int)(zones[z].off_temp * 5) % 10);
    __P("<a xlink:href='off_step?zone=%02d&d=10'> <path d='M 289 475 l 54 0 l -27 -25 z' /></a>", z);
    __P("<a xlink:href='off_step?zone=%02d&d=-10'><path d='M 289 565 l 54 0 l -27  25 z' /></a>", z);
    __P("<a xlink:href='off_step?zone=%02d&d=1'>  <path d='M 344 475 l 27 0 l -14 -13 z' /></a>", z);
    __P("<a xlink:href='off_step?zone=%02d&d=-1'> <path d='M 344 565 l 27 0 l -14  13 z' /></a>", z);
    // AUTO Button
    __P("<a xlink:href='auto?zone=%d&a=0'><rect y='460' x='460' height='25' width='25' style='fill:%s;stroke:#000000;stroke-width:3'/></a>", z, (zones[z].hours& 0x3000000) ? "#FFFFFF" : "#FF0000");
    __P("<text y='472' x='500' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='left' font-family='Times'> AUTO </text>");
    // Manual ON button
    __P("<a xlink:href='auto?zone=%d&a=1'><rect y='500' x='460' height='25' width='25' style='fill:%s;stroke:#000000;stroke-width:3'/></a>", z,  (zones[z].hours& 0x1000000) ? "#FF0000" : "#FFFFFF");
    __P("<text y='512' x='500' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='left' font-family='Times'> Manual ON </text>");
    // Manual OFF button
    __P("<a xlink:href='auto?zone=%d&a=2'><rect y='540' x='460' height='25' width='25' style='fill:%s;stroke:#000000;stroke-width:3'/></a>", z,  (zones[z].hours& 0x2000000) ? "#FF0000" : "#FFFFFF");
    __P("<text y='562' x='500' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='left' font-family='Times'> Manual OFF </text>");
    // Back link
    __P("<a xlink:href='../'><text y='650' x='60' font-size='20' fill='#0000FF' font-family='Times' >Back to Main Screen</text></a>");
    __P("</svg>");
    __P("</body></html>");
    server.send(200, "text/html", content);
  }
}

void handle_OnHour() {
  int z = server.arg("zone").toInt();
  int h = server.arg("h").toInt();
  if (z < 0 || z > num_zones - 1 || h < 0 || h > 23) return;
  zones[z].hours&= 0x00FFFFFF; // set to auto mode if user tries to program
  zones[z].hours^= (1L << h);
  EEPROM.write(z * 8 + 1, (zones[z].hours& 0x00FF0000) >> 16);
  EEPROM.write(z * 8 + 2, (zones[z].hours& 0x0000FF00) >>  8);
  EEPROM.write(z * 8 + 3, (zones[z].hours& 0x000000FF));
  EEPROM.commit();
  program(z);
}
void handle_OnAuto() {
  int z = server.arg("zone").toInt();
  int a = server.arg("a").toInt();
  if (z < 0 || z > num_zones - 1 || a < 0 || a > 2) return;
  zones[z].hours&= 0x00FFFFFF;
  zones[z].hours += (long)a * 0x1000000;
  EEPROM.write(z * 8 + 0, (zones[z].hours& 0xFF000000) >> 24);
  EEPROM.commit();
  program(z);
}
void handle_OnStep() {
  int z = server.arg("zone").toInt();
  int d = server.arg("d").toInt();
  if (z < 0 || z > num_zones - 1) return;
  zones[z].on_temp += d;
  EEPROM.write(z * 8 + 4, zones[z].on_temp);
  EEPROM.commit();
  program(z);
}
void handle_OffStep() {
  int z = server.arg("zone").toInt();
  int d = server.arg("d").toInt();
  if (z < 0 || z > num_zones - 1) return;
  counter[0] = counter[1] = counter[3] = 0;
  zones[z].off_temp += d;
  EEPROM.write(z * 8 + 5, zones[z].off_temp);
  EEPROM.commit();
  program(z);
}

void handle_UpdateTemp() {
  int z = server.arg("zone").toInt();
  int t = server.arg("temp").toFloat();
  if (z < 0 || z > num_zones - 1) return;
  zones[z].temp = t;
  String content = "<!DOCTYPE html>";
  __P("Zone %d set to temperature %d\n", z, t);
  server.send(200, "text/html", content);
}
