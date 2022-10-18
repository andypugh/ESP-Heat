#include <HTTPClient.h>

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
  
  if (strlen(error_email) > 6) {
    SMTPSession smtp;
    ESP_Mail_Session session;
    SMTP_Message message;
    
    session.server.host_name = smtp_server;
    session.server.port = smtp_port;
    session.login.email = error_email;
    session.login.password = error_password;
    session.login.user_domain = "";
    
    message.sender.name = "Heating Controller";
    message.sender.email = error_email;
    message.subject = error_subject;
    message.addRecipient("", error_recipient);
    String content = "Current status of controller:\n\n";
    
    HTTPClient http;
    http.begin("http://api.ipify.org/?format=text");
    int httpCode = http.GET();
    __P("External IP = http://");
    content += http.getString();
    content += ":8008";
    http.end();

    __P("\n\nError code %i\n\n", error_flag);
    for (int i = 0; i < num_boilers; i++) { __P("Boiler %d %s\n", i, boiler_states[boilers[i].state]);}
    __P("\n");
    for (int i = 0; i < num_zones; i++) { __P("Valve %d %s\n", i, valve_states[zones[i].state]);}
    __P("\n");
    for (int i = 0; i < num_pumps; i++) { __P("Pump %d %s\n", i, (zone_on & pumps[i].mask)?"On":"Off");}
    
    message.text.content = content.c_str();
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;
    
    if (!smtp.connect(&session)){
      Serial.println("smtp login failed");
      return;
    }
  
    /* Start sending Email and close the session */
    if (!MailClient.sendMail(&smtp, &message))  Serial.println("Error sending Email, " + smtp.errorReason());
    
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
    delay (100);
    
    if (units) {
      return sensors.getTempF(s.address);
    } else {
      return sensors.getTempC(s.address);
    }
}
