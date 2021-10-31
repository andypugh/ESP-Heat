// This function handles blink-codes and watches for a new web interface connection
// returns a true if there is a connection to handle, NULL otherwise
void do_status() {
  static int old_error;
  int i;
  static int c_counter;
  if (WiFi.status() != WL_CONNECTED){
    error_flag = 2;
    c_counter +=1;
    if (c_counter > 30){ // wait 5 mins between connection attempts
      Serial.println("Reconnection attempt");
      WiFi.begin(ssid, password);
      c_counter = 0;
    }
  }
  for (i = 0; i < 10; i++) {
    if (WiFi.status() == WL_CONNECTED) server.handleClient();
    if (i < error_flag) {
      digitalWrite(BLINK_LED, HIGH);
      delay(200);
      digitalWrite(BLINK_LED, LOW);
      delay(200);
    } else {
      digitalWrite(BLINK_LED, LOW);
      delay(400);
    }
  }

  // send an email message about the error
    
  if (old_error == error_flag || error_flag <= 1) return; // don't send same email twice
  if (WiFi.status() != WL_CONNECTED) return; // don't attempt to send an email with no connection
  
  if (error_email != "\0") {
    SMTPData smtpData;
    smtpData.setLogin(smtp_server, smtp_port, error_email,error_password);
    smtpData.setSender("Heating Controller", error_email);
    smtpData.addRecipient(error_recipient);
    smtpData.setSubject("Heating fault code detected " + WiFi.localIP().toString());
    String content = "Current status of controller:\n\n";
    
    HTTPClient http;
    http.begin("https://api.ipify.org/?format=text");
    int httpCode = http.GET();
    __P("External IP = http://");
    content += http.getString();
    http.end();

    __P("\n\nError code %i\n\n", error_flag);
    for (int i = 0; i < num_boilers; i++) { __P("Boiler %d %s\n", i, boiler_states[boilers[i].state]);}
    __P("\n");
    for (int i = 0; i < num_zones; i++) { __P("Valve %d %s\n", i, valve_states[zones[i].state]);}
    __P("\n");
    for (int i = 0; i < num_pumps; i++) { __P("Pump %d %s\n", i, (zone_on & pumps[i].mask)?"On":"Off");}
    smtpData.setMessage(content, false);
    MailClient.sendMail(smtpData);
  }
  if (error_flag == 2) error_flag = 1; // reconnected
  old_error = error_flag;
}

double get_temp(DS18B20 s){
  
#ifdef USE_DS2482
    if (s.channel <= 7 && s.channel >=0 ){
      oneWire.setChannel(s.channel);
    }
#endif

    sensors.requestTemperatures();
    
    if (units) {
      return sensors.getTempF(s.address);
    } else {
      return sensors.getTempC(s.address);
    }
}
