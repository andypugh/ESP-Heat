
void handle_Settings() {
  String content = "<!DOCTYPE html>";
  __P("<head>")
  __P("<style type='text/css'>table {border-collapse:collapse;border:1px solid #FF0000;}");
  __P("table td { border:1px solid #FF0000; text-align:center;}</style>");
  __P("</head>");
  __P("<html>");
  __P("<h1>Settings</h1>");
  __P("<form action='newsettings'>");
  __P("<label for=un>Temperature units</label><select name=un><option value=0 %s>&#8451</option><option value=1 %s>&#8457</option></select><br>", units ? "" : "selected", units ? "selected" : "");
  __P("<label for nz>Number of Zones   </label><input type='number' max=%i min=1 name=nz value=%i><br>", max_zones, num_zones);
  __P("<label for np>Number of Pumps  </label><input type='number' max=%i min=1 name=np value=%i><br>", max_pumps, num_pumps);
  __P("<label for nb>Number of Boilers </label><input type='number' max=%i min=1 name=nb value=%i><br><br>", max_boilers, num_boilers);
  __P("<table>");

  __P("<tr height='20px'>");
  __P("<th></th>")
  for (int i = 0; i < num_zones; i++) {
    __P("<th>Zone %i</th>", i);
  }
  __P("</tr>");

  __P("<tr height='20px'>");
  __P("<td>Zone Name</td>");
  for (int i = 0; i < num_zones; i++) {
    __P("<td><input type='text' name='zn%i' value='%s' maxlength=20></td>", i, zones[i].name);
  }
  __P("</tr>")

  __P("<tr height='20px'>");
  __P("<td>Zone Shape</td>");
  for (int i = 0; i < num_zones; i++) {
    __P("<td><textarea name='zs%i' rows='3' cols='15' maxlength=50>%s</textarea></td>", i, "Not yet used");
  }
  __P("</tr>")

  __P("<tr height='20px'>");
  __P("<td>Output Pin</td>");
  for (int i = 0; i < num_zones; i++) {
    __P("<td><input type='number' name=zo%i min=0 max=39 value='%i'></td>", i, zones[i].out_pin);
  }
  __P("</tr>")

  __P("<tr height='20px'>");
  __P("<td>Input Pin</td>");
  for (int i = 0; i < num_zones; i++) {
    __P("<td><input type='number' name=zi%i min=0 max=39 value='%i'></td>", i, zones[i].in_pin);
  }
  __P("</tr>")

  __P("<tr height='20px'>");
  __P("<td>Sensor</td>");

  for (int j = 0; j < num_zones; j++) {
    __P("<td><select name=zt%i>", j);
    __P("<option value='0000000000000000' %s>network data</option>", (zones[j].sensor.channel == -1) ? "selected" : "");
    for (int i = 0; i < num_sensors; i++) {
      __P("<option value=%s %s>%s</option>", all_sensors[i].str_address,
          memcmp(zones[j].sensor.address, all_sensors[i].address, 8) ? "" : "selected",
          all_sensors[i].dot_address);
    } 
    __P("</select></td>");
  }
  __P("</tr>");

  for (int j = 0; j < num_boilers; j++) {
    __P("<tr height='20px'>")
    __P("<td><label for=bo%i>Boiler %i: Pin </label><input type='number' name=bo%i min=0 max=39 value=%i><br>", j, j, j, boilers[j].out_pin);
    
    __P("<label for=bf%i>Feed</label><select name=bf%i>", j, j);
    __P("<option value='0000000000000000' %s>network data</option>", (boilers[j].f_sensor.channel == -1) ? "selected" : "");
    for (int i = 0; i < num_sensors; i++) {
      __P("<option value=%s %s>%s</option>", all_sensors[i].str_address, 
          memcmp(boilers[j].f_sensor.address, all_sensors[i].address, 8) ? "" : "selected",
          all_sensors[i].dot_address);
    }
    __P("</select><br>");
    __P("<label for=br%i>Return</label><select name=br%i>", j, j);
        __P("<option value='0000000000000000' %s>network data</option>", (boilers[j].r_sensor.channel == -1) ? "selected" : "");
    for (int i = 0; i < num_sensors; i++) {
      __P("<option value=%s %s>%s</option>", all_sensors[i].str_address, 
          memcmp(boilers[j].r_sensor.address, all_sensors[i].address, 8) ? "" : "selected",
          all_sensors[i].dot_address);
    }
    __P("</select></td>");
    for (int i = 0; i < num_zones; i++) {
      __P("<td><input type='checkbox' name=bm%i value=%i %s></td>", j, 1 << i, (boilers[j].mask & 1 << i) ? "checked" : "")
    }
    __P("</tr>");
  }
  for (int j = 0; j < num_pumps; j++) {
    __P("<tr height='20px'>")
    __P("<td><label for=po%i>Pump %i: Pin </label><input type='number' name=po%i min=0 max=39 value=%i></td>", j, j, j, pumps[j].out_pin);
    for (int i = 0; i < num_zones; i++) {
      __P("<td><input type='checkbox' name=pm%i value=%i %s></td>", j, 1 << i, (pumps[j].mask & 1 << i) ? "checked" : "")
    }
    __P("</tr>");
  }

  __P("<tr height='20px'>")
  __P("<td>Default ON</td>");
  for (int i = 0; i < num_zones; i++) {
    __P("<td><input type='checkbox' name=df%i value=1 %s></td>", i, (zones[i].default_state) ? "checked" : "")
  }
  __P("</tr>");

  __P("</table>");
  __P("<input type='submit' value='Submit'>");
  __P("</form>");
  __P("</html>");
  server.send(200, "text/html", content);
}

void handle_OnSet() {
  int z = server.arg("zone").toInt();
  Serial.printf("Setting Screen: setting zone %d\n", z);
  program(z);
}

void handle_newsettings() {
  String d = "&";
  int i;
  for (i = 0; i < server.args(); i++) {
    d = d + server.argName(i) + "=" + server.arg(i) + "&";
  }
  Serial.println("Writing to EEPROM");
  Serial.println(d);
  for (i = 0; i < d.length(); i++) {
    EEPROM.write(EEPROM_BASE + i, d.charAt(i));
  }
  for (; i < d.length() + 8; i++) {
    EEPROM.write(EEPROM_BASE + i, 0);
  }
  EEPROM.commit();
  read_EEPROM(EEPROM_BASE);
  handle_OnConnect();
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
  String d = "&nz=5&np=3&nb=1&zn0=Far End&zn1=Solar&zn2=Hall&zn3=Downstairs&zn4=Upstairs&zo0=25&zo1=26&zo2=27"
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
        case 30062: // un
          units = EEPROM.read(p++) - '0';
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
            char h[2];
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
            char h[2];
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
            char h[2];
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
