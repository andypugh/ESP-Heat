
// Programming Screen
void program(AsyncWebServerRequest *request, int z) {
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
    if (!request->authenticate(http_user, http_pass)) return request->requestAuthentication();
    request->send(200, "text/html", content);
  }
}



int get_int(int p) {
  int result = 0;
  while (1) {
    char c = EEPROM.read(p++);
    switch (c) {
      case '=':
        break;
      case '&':
        return result;
      default:
        result *= 10;
        result += (c - '0');
    }
  }
}

void set_string(char* dest, int p) {
  char c = EEPROM.read(p++);
  dest[0] = 0;
  while (c != '&') {
    strncat(dest, &c, 1);
    c = EEPROM.read(p++);
  }
}

void write_defaults() {
  String d = "&nz=5&np=3&nb=1&dp=15&zn0=Far End&zn1=Solar&zn2=Hall&zn3=Downstairs&zn4=Upstairs&zo0=25&zo1=26&zo2=27"
             "&zo3=14&zo4=19&zi0=36&zi1=39&zi2=34&zi3=35&zi4=32&zt0=0000000000000000&zt1=0000000000000000"
             "&zt2=0000000000000000&zt3=0000000000000000&zt4=0000000000000000&bo0=33&bm0=1&bm0=2&bm0=4&bm0=8"
             "&bm0=16&po0=4&pm0=1&po1=5&pm1=4&pm1=8&po2=12&pm2=1&pm2=2&pm2=4&pm2=8&pm2=16&df3=1&";
  for (int i = 0; i < d.length(); i++) {
    EEPROM.write(EEPROM_BASE + i, d.charAt(i));
  }
  EEPROM.commit();
}

void read_EEPROM(int p) {
  int z;
  char c;

  for (z = 0; z < max_pumps;  pumps[z++].mask = 0);
  for (z = 0; z < max_boilers;  boilers[z++].mask = 0);

  Serial.print("\nReading EEPROM\n");
  if (EEPROM.read(p) != '&') { // no initial setup
    write_defaults();
  }
  while ((c = EEPROM.read(p++)) > 0 && c < 255) {
    if (c == '&') {
      switch (256 * EEPROM.read(p++) + EEPROM.read(p++)) {
        case 0:
          return;
        case 25712: // dp
          ds2482_reset = get_int(p);
          Serial.printf("ds2482_reset set to %i\n", ds2482_reset);
          break;
        case 30062: // un
          p++; // skip the =
          units = (EEPROM.read(p++) == 'F') ? 1 : 0;
          Serial.printf("Units set to %c\n", (units)?'F':'C');
          break;
        case 28282: // nz
          num_zones = get_int(p);
          Serial.printf("num zones set to %i\n", num_zones);
          break;
        case 28272: // np
          num_pumps = get_int(p);
          Serial.printf("num pumps set to %i\n", num_pumps);
          break;
        case 28258: // nb
          num_boilers = get_int(p);
          Serial.printf("num boilers set to %i\n", num_boilers);
          break;
        case 31342: // zn
          z = EEPROM.read(p++) - '0';
          p++; // skip the =
          set_string(zones[z].name, p);
          Serial.printf("Zone name %i set to %s\n", z, zones[z].name);
          break;
        case 31347: // zs
          break;
        case 31343: // zo
          z = EEPROM.read(p++) - '0';
          zones[z].out_pin = get_int(p);
          Serial.printf("Zone %i out pin set to %i\n", z, zones[z].out_pin);
          break;
        case 31337: // zi
          z = EEPROM.read(p++) - '0';
          zones[z].in_pin = get_int(p);
          Serial.printf("Zone %i in pin set to %i\n", z, zones[z].in_pin);
          break;
        case 31348: // zt
          z = EEPROM.read(p++) - '0';
          p++; // skip the '='
          Serial.printf("Zone %i sensor address set to ", z);
          for (int i = 0; i < 8; i++) {
            char h[3] = {0};
            int v;
            h[0] = EEPROM.read(p++);
            h[1] = EEPROM.read(p++);
            v = strtol(h, NULL, 16);
            Serial.printf("%02x.", v);
            zones[z].sensor.address[i] = v;
          }
          Serial.printf("\n");
          break;
        case 25190: // bf
          z = EEPROM.read(p++) - '0';
          p++; // skip the '='
          Serial.printf("Boiler %i flow address set to ", z);
          for (int i = 0; i < 8; i++) {
            char h[3] = {0};
            int v;
            h[0] = EEPROM.read(p++);
            h[1] = EEPROM.read(p++);
            v = strtol(h, NULL, 16);
            Serial.printf("%02x.", v);
            boilers[z].f_sensor.address[i] = v;
          }
          Serial.printf("\n");
          break;
        case 25202: // br
          z = EEPROM.read(p++) - '0';
          p++; // skip the '='
          Serial.printf("Boiler %i return address set to ", z);
          for (int i = 0; i < 8; i++) {
            char h[3] = {0};
            int v;
            h[0] = EEPROM.read(p++);
            h[1] = EEPROM.read(p++);
            v = strtol(h, NULL, 16);
            Serial.printf("%02x.", v);
            boilers[z].r_sensor.address[i] = v;
          }
          Serial.printf("\n");
          break;
        case 25199: // bo
          z = EEPROM.read(p++) - '0';
          boilers[z].out_pin = get_int(p);
          Serial.printf("Boiler %i out pin set to %i\n", z, boilers[z].out_pin);
          break;
        case 25197: // bm
          z = z = EEPROM.read(p++) - '0';
          boilers[z].mask |= get_int(p);
          Serial.printf("Boiler %i mask set to %i\n", z, boilers[z].mask);
          break;
        case 28783: // po
          z = EEPROM.read(p++) - '0';
          pumps[z].out_pin = get_int(p);
          Serial.printf("Pump %i out pin set to %i\n", z, pumps[z].out_pin);
          break;
        case 28781: // pm
          z = EEPROM.read(p++) - '0';
          pumps[z].mask |= get_int(p);
          Serial.printf("Pump %i mask set to %i\n", z, pumps[z].mask);
          break;
        case 25702: // df
          z = EEPROM.read(p++) - '0';
          zones[z].default_state = get_int(p);
          Serial.printf("Zone %i default state set to %i\n", z, zones[z].default_state);
          break;
        default:
          Serial.printf("Case %i %c%c not handled\n", 256 * EEPROM.read(p - 2) + EEPROM.read(p - 1), EEPROM.read(p - 2), EEPROM.read(p - 1));
          break;
      }
    }
  }
}
