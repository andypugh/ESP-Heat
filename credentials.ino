int AP_mode(){
  WiFi.softAP("ESP-WIFI-MANAGER", NULL);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP); 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    get_credentials(request);
  });
  server.on("/network", HTTP_GET, [](AsyncWebServerRequest *request){
    write_EEPROM(CREDENTIALS_BASE, request);
    read_EEPROM(CREDENTIALS_BASE, false);
    request->send(200, "text/html", "Credentials Updated. <br>You can close this page.<br>The controller will now reboot.");
    ESP.restart();
  });
  server.begin();
  while (1){
    Serial.print(":");
    delay(100);
    digitalWrite(BLINK_LED, LOW);
    delay(100);
    digitalWrite(BLINK_LED, HIGH);    
  }
}

void get_credentials(AsyncWebServerRequest *request){
  Serial.println("Credentials Page");
  String content = "<!DOCTYPE html>";
  __P("<head>")
  __P("<style type='text/css'>table {border-collapse:collapse;border:1px solid #FF0000;}");
  __P("table td { border:1px solid #FF0000; text-align:center;}</style>");
  __P("</head>");
  __P("<html>");
  __P("<h1>Network Settings</h1>");
  __P("<form action='network'>");
  __P("<tr height='40px'>");
  __P("<td><label for hn>Hostname</label><input type='text' name='hn' value='%s' maxlength=32></td><br>", hostname);
  __P("<tr height='40px'>");  
  __P("<td><label for ss>Wifi Network SSID</label><input type='text' name='ss' value='%s' maxlength=32></td>", ssid);
  __P("<td><label for wp>WiFi Password</label><input type='text' name='wp' value='%s' maxlength=32></td><br>", password);
  __P("<tr height='40px'>");
  __P("<td><label for ns>Network time (NTP) server</label><input type='text' name='ns' value='%s' maxlength=32></td>", ntpServer);
  __P("<td><label for tz>TimeZone definition</label><input type='text' name='tz' value='%s' maxlength=32></td><br>", TZstr);
  __P("<tr height='40px'>");  
  __P("<td><label for ms>Error report email Subject</label><input type='text' name='ms' value='%s' maxlength=32></td><br>", error_subject);
  __P("<tr height='40px'>");
  __P("<td><label for me>Error email From: account</label><input type='text' name='me' value='%s' maxlength=32></td>", error_email);
  __P("<td><label for mr>Error email To: address</label><input type='text' name='mr' value='%s' maxlength=32></td><br>", error_recipient);
  __P("<tr height='40px'>");
  __P("<td><label for sm>SMTP Server</label><input type='text' name='sm' value='%s' maxlength=32></td>", smtp_server);
  __P("<td><label for mp>SMTP Password</label><input type='text' name='mp' value='%s' maxlength=32></td>", error_password);
  __P("<td><label for sp>SMTP Port</label><input type='text' name='sp' value='%i' maxlength=4></td><br>", smtp_port);
  __P("<tr height='60px'>");  
  __P("<td><label for hu>Admin Username</label><input type='text' name='hu' value='%s' maxlength=32></td>", http_user);
  __P("<td><label for hp>Admin Password</label><input type='text' name='hp' value='%s' maxlength=32></td><br>", http_pass);
  __P("</table>");
  __P("<br><br><input type='submit' value='Submit' style='height:50px; width:50px'>");
  __P("</form>");
  __P("</html>");
  //if (!request->authenticate(http_user, http_pass)) return request->requestAuthentication();
  request->send(200, "text/html", content);
  Serial.println("Client Connected");
}
