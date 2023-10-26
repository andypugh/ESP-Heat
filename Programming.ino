
// Programming Screen
void program(AsyncWebServerRequest *request, int z) {
  int ro = 200, ri = 150;
  if (z >= 0 && z < num_zones) {
    String content = "<!DOCTYPE html>";
    __P("<head> </head>");
    __P("<html>");
    __P("<link rel='icon' href='data:,'>");
    __P("<body>");
    __P("<svg svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' version='1.1' width='297mm' height='210mm'>");
    // Header
    __P("<text y='80' x='250' font-size='30' fill='0' text-anchor='middle' font-family='Times' >%s</text>", zones[z].name);
    // Setting clock faces
    for (int h = 0; h < 24; h++) {
      __P("<a xlink:href='hour?zone=%02d&h=%02d'><path d='M %d %d ", z, h, (int)(ri * sin(0.2618 * h)), (int)(-ri * cos(0.2618 * h)));
      __P("L %d %d ",                                               (int)(ro * sin(0.2618 * h)), (int)(-ro * cos(0.2618 * h)));
      __P("A %d %d 0 0 1 %d %d ",                                   ro, ro, (int)(ro * sin(0.2618 * (h + 1))), (int)(-ro * cos(0.2618 * (h + 1))));
      __P("L %d %d ",                                               (int)(ri * sin(0.2618 * (h + 1))), (int)(-ri * cos(0.2618 * (h + 1))));
      __P("A %d %d 0 0 0 %d %d '",                                  ri, ri, (int)(ri * sin(0.2618 * h)), (int)(-ri * cos(0.2618 * h)));       
      __P("style='fill:%s;stroke:#000000;stroke-opacity:1 stroke-width:2' transform='translate(250, 350)' /></a>", 
          (zones[z].hours & (1 << 24 | 1 << h) && !(zones[z].hours & 1 << 25)) ? on_colour : off_colour);
            
      __P("<text x='%d' y='%d' font-size='15' fill='#000000' dominant-baseline='middle' text-anchor='middle' font-family='Times' transform='translate(250, 350)'> %02d </text>",
          (int)((ro + 20) * sin(0.2618 * h)), (int)(-(ro + 20) * cos(0.2618 * h)), h);
    }
    __P("<path d='M 0 %d l 10 10 l -20 0 z' transform='rotate (%d 250 350) translate(250 350)'/>", -ri,  (60 * timeinfo.tm_hour + timeinfo.tm_min)/4);
    Serial.println((60 * timeinfo.tm_hour + timeinfo.tm_min) * 0.25);    // Current time indicator
    // Set / display on temperature
    __P("<rect y='310' x='150' height='80' width='80' style='fill:%s;stroke:#000000;stroke-width:3'/>", on_colour);
    __P("<text y='350' x='190' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='middle' font-family='Times'> %d.%1d </text>",
        (int)(zones[z].on_temp / 2), (int)(zones[z].on_temp * 5) % 10);
    __P("<a xlink:href='on_step?zone=%02d&d=10'> <path d='M 149 305 l 54 0 l -27 -25 z' /></a>", z);
    __P("<a xlink:href='on_step?zone=%02d&d=-10'><path d='M 149 395 l 54 0 l -27  25 z' /></a>", z);
    __P("<a xlink:href='on_step?zone=%02d&d=1'>  <path d='M 204 305 l 27 0 l -14 -13 z' /></a>", z);
    __P("<a xlink:href='on_step?zone=%02d&d=-1'> <path d='M 204 395 l 27 0 l -14  13 z' /></a>", z);
    // Set / display off temperature
    __P("<rect y='310' x='270' height='80' width='80' style='fill:%s;stroke:#000000;stroke-width:3'/>", off_colour);
    __P("<text y='350' x='310' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='middle' font-family='Times'> %d.%1d </text>",
        (int)(zones[z].off_temp / 2), (int)(zones[z].off_temp * 5) % 10);
    __P("<a xlink:href='off_step?zone=%02d&d=10'> <path d='M 269 305 l 54 0 l -27 -25 z' /></a>", z);
    __P("<a xlink:href='off_step?zone=%02d&d=-10'><path d='M 269 395 l 54 0 l -27  25 z' /></a>", z);
    __P("<a xlink:href='off_step?zone=%02d&d=1'>  <path d='M 324 305 l 27 0 l -14 -13 z' /></a>", z);
    __P("<a xlink:href='off_step?zone=%02d&d=-1'> <path d='M 324 395 l 27 0 l -14  13 z' /></a>", z);
    // AUTO Button
    __P("<a xlink:href='auto?zone=%d&a=0'><rect y='460' x='475' height='25' width='25' style='fill:%s;stroke:#000000;stroke-width:3'/></a>", z, (zones[z].hours& 0x3000000) ? "#FFFFFF" : "#FF0000");
    __P("<text y='472' x='515' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='left' font-family='Times'> AUTO </text>");
    // Manual ON button
    __P("<a xlink:href='auto?zone=%d&a=1'><rect y='500' x='475' height='25' width='25' style='fill:%s;stroke:#000000;stroke-width:3'/></a>", z,  (zones[z].hours& 0x1000000) ? "#FF0000" : "#FFFFFF");
    __P("<text y='512' x='515' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='left' font-family='Times'> Manual ON </text>");
    // Manual OFF button
    __P("<a xlink:href='auto?zone=%d&a=2'><rect y='540' x='475' height='25' width='25' style='fill:%s;stroke:#000000;stroke-width:3'/></a>", z,  (zones[z].hours& 0x2000000) ? "#FF0000" : "#FFFFFF");
    __P("<text y='562' x='515' font-size='30' fill='#000000' dominant-baseline='middle' text-anchor='left' font-family='Times'> Manual OFF </text>");
    // Back link
    __P("<a xlink:href='../'><text y='650' x='60' font-size='20' fill='#0000FF' font-family='Times' >Back to Main Screen</text></a>");
    __P("</svg>");
    __P("</body></html>");
    if (!request->authenticate(http_user, http_pass)) return request->requestAuthentication();
    request->send(200, "text/html", content);
  }
}



int get_int(int *p) {
  int result = 0;
  int sign = 1;
  while (1) {
    char c = EEPROM.read((*p)++);
    switch (c) {
      case '=':
        break;
      case '&':
        (*p)--;
        return sign * result;        
      case '-':
        sign = -1;
      default:
        result *= 10;
        result += (c - '0');
    }
  }
}

void set_string(char* dest, int *p) {
  char c = EEPROM.read((*p)++);
  dest[0] = 0;
  while (c != '&' && *p < EEPROM_SIZE) {

    strncat(dest, &c, 1);
    c = EEPROM.read((*p)++);
  }
  (*p)--;
}

void write_defaults() {
  String d = "&nz=5&np=3&nb=1&dp=-1&zn0=Far End&zn1=Solar&zn2=Hall&zn3=Downstairs&zn4=Upstairs&zo0=25&zo1=26&zo2=27"
             "&zo3=14&zo4=19&zi0=36&zi1=39&zi2=34&zi3=35&zi4=32&zt0=0000000000000000&zt1=0000000000000000"
             "&zt2=0000000000000000&zt3=0000000000000000&zt4=0000000000000000&bo0=33&bm0=1&bm0=2&bm0=4&bm0=8"
             "&bm0=16&po0=4&pm0=1&po1=5&pm1=4&pm1=8&po2=12&pm2=1&pm2=2&pm2=4&pm2=8&pm2=16&df3=1"
             "&zs0=M60 350 h190 v190 h-190 z&zs1=M250 350 h180 v190 h-180 z&zs2=M390 350 h200 v190 h-200 z"
             "&zs3=M590 350 h180 v270 h-180 z&zs4=M690 50 h180 v270 h-180 z&\0";
  for (int i = 0; i < d.length(); i++) {
    EEPROM.write(EEPROM_BASE + i, d.charAt(i));
  }
  EEPROM.commit();
}

void parse_svg(int z){
  // Find where to place captions on an SVG shape
  int p = 0;
  int x, y;
  int t1, t2;
  int maxx = -10000;
  int minx = 10000;
  int maxy = -10000;
  int miny = 10000;
  int ret;

  // FIXME: Do curves better, and arcs at all. 
  
  while (p < 50) {
    switch (zones[z].shape[p]){
    case 'M':
      ret = sscanf(zones[z].shape + p, "M%i %i", &x, &y);
      break;
    case 'm':
      ret = sscanf(zones[z].shape + p, "m%i %i", &t1, &t2);
      x = x + t1;
      y = y + t2;
      break;
    case 'H':
      ret = sscanf(zones[z].shape + p, "H%i", &x);
      break;
    case 'h':
      ret = sscanf(zones[z].shape + p, "h%i", &t1);
      x = x + t1;
      break;
    case 'V':
      ret = sscanf(zones[z].shape + p, "V%i", &y);
      break;
    case 'v':
      ret = sscanf(zones[z].shape + p, "v%i", &t1);
      y = y + t1;
      break;
    case 'L':
      ret = sscanf(zones[z].shape + p, "L%i %i", &x, &y);
      break;
    case 'l':
      ret = sscanf(zones[z].shape + p, "l%i %i", &t1, &t2);
      x = x + t1;
      y = y + t2;
      break;
    case 'C':
      ret = sscanf(zones[z].shape + p, "C%*i %*i,%*i %*i,%i %i", &x, &y);
      break;
    case 'c':
      ret = sscanf(zones[z].shape + p, "c%*i %*i,%*i %*i,%i %i", &t1, &t2);
      x = x + t1;
      y = y + t2;
      break;
    case 0: // end of string
      zones[z].middle = (minx + maxx) / 2;
      zones[z].top = miny;
      zones[z].bottom = maxy;
      break;
    }
    maxx = max(x, maxx);
    minx = min(x, minx);
    maxy = max(y, maxy);
    miny = min(y, miny);
    
    p++;
    
  }
}

void debounce(AsyncWebServerRequest *request, int z){
  // The authentication results in all URLS being called twice, which is bad
  // with the programming screen temperatures and clock segments.
  char ret[14];
  snprintf(ret, 14, "/set?zone=%02i", z);
  request->redirect(ret);
  program(request, z);
}

void write_EEPROM(int base, AsyncWebServerRequest *request){
  String d = "&";
  int i;
  for (i = 0; i < request->args(); i++) {
    d = d + request->argName(i) + "=" + request->arg(i) + "&";
  }
  Serial.println("Writing to EEPROM");
  Serial.println(d);
  for (i = 0; i < d.length(); i++) {
    EEPROM.write(base + i, d.charAt(i));
  }
  for (; i < d.length() + 8; i++) {
    EEPROM.write(base + i, 0);
  }
  EEPROM.commit();    
}
void read_EEPROM(int p, bool init) {
  int z;
  char c;
  if (init){
    for (z = 0; z < max_pumps;  pumps[z++].mask = 0);  
    for (z = 0; z < max_boilers;  boilers[z++].mask = 0);
    for (z = 0; z < max_zones; zones[z++].default_state = 0);
  }
  
  Serial.print("\nReading EEPROM\n");
  if (EEPROM.read(p) != '&') { // no initial setup
    write_defaults();
  }
  while ((c = EEPROM.read(p++)) > 0 && c < 255) {
    if (c == '&') {
      switch (256 * EEPROM.read(p++) + EEPROM.read(p++)) {
        case 0:
          Serial.printf("Total bytes used %i / %i\n", p, EEPROM_SIZE);
          return;
        case 25712: // dp
          ds2482_reset = get_int(&p);
          Serial.printf("ds2482_reset set to %i\n", ds2482_reset);
          break;
        case 30062: // un
          p++; // skip the =
          units = (EEPROM.read(p++) == 'F') ? 1 : 0;
          Serial.printf("Units set to %c\n", (units)?'F':'C');
          break;
        case 28282: // nz
          num_zones = get_int(&p);
          Serial.printf("num zones set to %i\n", num_zones);
          break;
        case 28272: // np
          num_pumps = get_int(&p);
          Serial.printf("num pumps set to %i\n", num_pumps);
          break;
        case 28258: // nb
          num_boilers = get_int(&p);
          Serial.printf("num boilers set to %i\n", num_boilers);
          break;
        case 31342: // zn
          z = EEPROM.read(p++) - '0';
          p++; // skip the =
          set_string(zones[z].name, &p);
          Serial.printf("Zone name %i set to %s\n", z, zones[z].name);
          break;
        case 31343: // zo
          z = EEPROM.read(p++) - '0';
          zones[z].out_pin = get_int(&p);
          Serial.printf("Zone %i out pin set to %i\n", z, zones[z].out_pin);
          break;
        case 31337: // zi
          z = EEPROM.read(p++) - '0';
          zones[z].in_pin = get_int(&p);
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
          boilers[z].out_pin = get_int(&p);
          Serial.printf("Boiler %i out pin set to %i\n", z, boilers[z].out_pin);
          break;
        case 25197: // bm
          z = z = EEPROM.read(p++) - '0';
          boilers[z].mask |= get_int(&p);
          Serial.printf("Boiler %i mask set to %i\n", z, boilers[z].mask);
          break;
        case 28783: // po
          z = EEPROM.read(p++) - '0';
          pumps[z].out_pin = get_int(&p);
          Serial.printf("Pump %i out pin set to %i\n", z, pumps[z].out_pin);
          break;
        case 28781: // pm
          z = EEPROM.read(p++) - '0';
          pumps[z].mask |= get_int(&p);
          Serial.printf("Pump %i mask set to %i\n", z, pumps[z].mask);
          break;
        case 25702: // df
          z = EEPROM.read(p++) - '0';
          zones[z].default_state = get_int(&p);
          Serial.printf("Zone %i default state set to %i\n", z, zones[z].default_state);
          break;
        case 31347: // zs
          z = EEPROM.read(p++) - '0';
          p++; // skip the =
          set_string(zones[z].shape, &p);
          parse_svg(z);
          Serial.printf("Zone %i shape %s. Mid %i Top %i Btm %i\n", z, zones[z].shape, zones[z].middle, zones[z].top, zones[z].bottom);
          break;
        case 26734: // hn
          p++; // skip the =
          set_string(hostname, &p);
          Serial.printf("Hostname set to %s\n", hostname);
          break;
        case 29555: // ss
          p++; // skip the =
          set_string(ssid, &p);
          Serial.printf("ssid set to %s\n", ssid);
          break;
        case 30576: // wp
          p++; // skip the =
          set_string(password, &p);
          Serial.printf("Password set to %s\n", password);
          break;
        case 28275: // ns
          p++; // skip the =
          set_string(ntpServer, &p);
          Serial.printf("NTP Server set to %s\n", ntpServer);
          break;
        case 29818: // tz
          p++; // skip the =
          set_string(TZstr, &p);
          Serial.printf("Timezone String set to %s\n", TZstr);
          break;
        case 28019: // ms
          p++; // skip the =
          set_string(error_subject, &p);
          Serial.printf("Error Subject set to %s\n", error_subject);
          break;
        case 28005: // me
          p++; // skip the =
          set_string(error_email, &p);
          Serial.printf("Error email sender set to %s\n", error_email);
          break;
        case 28018: // mr
          p++; // skip the =
          set_string(error_recipient, &p);
          Serial.printf("Error recipient set to %s\n", error_recipient);
          break;
        case 28016: // mp
          p++; // skip the =
          set_string(error_password, &p);
          Serial.printf("Error mail server password set to %s\n", error_password);
          break;
        case 29549: // sm
          p++; // skip the =
          set_string(smtp_server, &p);
          Serial.printf("Error mail server address set to %s\n", smtp_server);
          break;
        case 29552: // sp
          smtp_port = get_int(&p);
          Serial.printf("Error mail server port set to %i\n", smtp_port);
          break;
        case 26741: // hu
          p++; // skip the =
          set_string(http_user, &p);
          Serial.printf("Settings Username set to %s\n", http_user);
          break;
        case 26736: // hp
          p++; // skip the =
          set_string(http_pass, &p);
          Serial.printf("Settings password set to %s\n", http_pass);
          break;
        default:
          Serial.printf("Case %i %c%c not handled\n", 256 * EEPROM.read(p - 2) + EEPROM.read(p - 1), EEPROM.read(p - 2), EEPROM.read(p - 1));
          break;
      }
    }
  }
}
