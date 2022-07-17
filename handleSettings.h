server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
  String content = "<!DOCTYPE html>";
  __P("<head>")
  __P("<style type='text/css'>table {border-collapse:collapse;border:1px solid #FF0000;}");
  __P("table td { border:1px solid #FF0000; text-align:center;}</style>");
  __P("</head>");
  __P("<html>");
  __P("<h1>Settings</h1>");
  __P("<form action='newsettings'>");
  __P("<label for un>Temperature units</label><select name=un><option value='C' %s>&#8451</option><option value='F' %s>&#8457</option></select><br>", units ? "" : "selected", units ? "selected" : "");
  __P("<label for nz>Number of Zones   </label><input type='number' max=%i min=1 name=nz value=%i><br>", max_zones, num_zones);
  __P("<label for np>Number of Pumps  </label><input type='number' max=%i min=1 name=np value=%i><br>", max_pumps, num_pumps);
  __P("<label for nb>Number of Boilers </label><input type='number' max=%i min=1 name=nb value=%i><br>", max_boilers, num_boilers);
  #ifdef USE_DS2482
  __P("<label for dp>DS2482 reset pin </label><input type='number' max=39 min=-1 name=dp value=%i> (-1 if not used)<br><br>", ds2482_reset);
  #endif
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
    __P("<option value='0000000000000000' %s>no sensor</option>", (boilers[j].f_sensor.channel == -1) ? "selected" : "");
    for (int i = 0; i < num_sensors; i++) {
      __P("<option value=%s %s>%s</option>", all_sensors[i].str_address, 
          memcmp(boilers[j].f_sensor.address, all_sensors[i].address, 8) ? "" : "selected",
          all_sensors[i].dot_address);
    }
    __P("</select><br>");
    __P("<label for=br%i>Return</label><select name=br%i>", j, j);
        __P("<option value='0000000000000000' %s>no sensor</option>", (boilers[j].r_sensor.channel == -1) ? "selected" : "");
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
  __P("<br><br><input type='submit' value='Submit' style='height:50px; width:50px'>");
  __P("</form>");
  __P("</html>");
  if (!request->authenticate(http_user, http_pass)) return request->requestAuthentication();
  request->send(200, "text/html", content);
});


server.on("/newsettings", HTTP_GET, [](AsyncWebServerRequest *request){
  String d = "&";
  int i;
  for (i = 0; i < request->args(); i++) {
    d = d + request->argName(i) + "=" + request->arg(i) + "&";
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
  request->redirect("/");
});
