#ifdef HAVE_SLACK


/**
  Sends a ping message to Slack. Call this function immediately after establishing
  the WebSocket connection, and then every 5 seconds to keep the connection alive.
*/
void sendPing() {
  DynamicJsonDocument root(1024);
  root["type"] = "ping";
  root["id"] = slack_nextCmdId++;
  String json;
  serializeJson(root, json);
  slack_webSocket.sendTXT(json);
}



bool replyToSlackMessage(String msg)
{
  const char* host = "hooks.slack.com";
//  Serial.print("Connecting to ");
//  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClientSecure client;
  const int httpsPort = 443;
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed :-(");
    return false;
  }
//  Serial.print("Posting to URL: ");
//  Serial.println(SLACK_WEBHOOK);

  String postData="{\"text\": \"" + msg + "\"}";
    // This will send the request to the server
//  Serial.println(postData);
  client.print(String("POST ") + SLACK_WEBHOOK + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Connection: close" + "\r\n" +
               "Content-Length:" + postData.length() + "\r\n" +
               "\r\n" + postData);
//  Serial.println("Request sent");
  String line = client.readStringUntil('\n');
//  Serial.printf("Response code was: ");
//  Serial.println(line);
  if (line.startsWith("HTTP/1.1 200 OK")) {
    return true;
  } else {
    return false;
  }
}

/**
  replies with an ack
*/
void processSlackMessage(char *payload) {
//  Serial.println(String(payload));
  DynamicJsonDocument doc(2000);
  deserializeJson(doc, payload);
  const char* msgtype=doc["type"];
  const char* msgtext=doc["text"];
  int i;
  // if the message came from a bot, this key is present, otherwise NULL
  const char* bot_id=doc["bot_id"];
  if (strcmp(msgtype,"message") == 0 && bot_id == NULL) {
    if (strcmp(msgtext,"disp off") == 0) {
      max7219_current_command = none;
      replyToSlackMessage("ACK: matrix off");
    }
    else if (strcmp(msgtext,"disp right") == 0) {
      max7219_current_command = rightarrow;
      replyToSlackMessage("ACK: matrix right");
    }
    else if (strcmp(msgtext,"disp left") == 0) {
      max7219_current_command = leftarrow;
      replyToSlackMessage("ACK: matrix left");
    }
    else if (strcmp(msgtext,"disp up") == 0) {
      max7219_current_command = uparrow;
      replyToSlackMessage("ACK: matrix up");
    }
    else if (strcmp(msgtext,"disp down") == 0) {
      max7219_current_command = downarrow;
      replyToSlackMessage("ACK: matrix down");
    }
    else if (strcmp(msgtext,"layout 1") == 0) {
      tft.fillScreen(TFT_BLACK);
      st7735_current_layout = original;
      replyToSlackMessage("ACK: setting original layout");
    }
    else if (strcmp(msgtext,"layout 2") == 0) {
      tft.fillScreen(TFT_BLACK);
      st7735_current_layout = driver;
      replyToSlackMessage("ACK: setting driver layout");
    }
    else if (strcmp(msgtext,"alert") == 0) {
      max7219_current_command = alert;
      replyToSlackMessage("ACK: alert (10 flashes)");
      max7219_alertcounter=10;
    }
    else if (strncmp(msgtext,"text ", 5) == 0) {
      max7219_current_command = text;
      // Oldschool char copy
      for (i=5; i<54; i++) {
        slack_textmessage[i-5] = msgtext[i];
        if (msgtext[i] == 0) {
          i=54;  
        }
      }
      replyToSlackMessage("ACK: TFT display "+String(slack_textmessage));
      slack_textcounter=MAX7219_TEXTTIME;
    }
    else
    {
      showSlackSyntax();
      max7219_current_command = none;
    }
  }
}





/**
  Called on each web socket event. Handles disconnection, and also
  incoming messages from slack.
*/
void webSocketEvent(WStype_t type, uint8_t *payload, size_t len) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WebSocket] Disconnected :-( \n");
      slack_connected = false;
      break;

    case WStype_CONNECTED:
      Serial.printf("[WebSocket] Connected to: %s\n", payload);
      Serial.print("Websocket opened in "); 
      Serial.println(String(millis()-slack_startconnect) + " ms"); 
      showSlackSyntax();
      sendPing();
      break;

    case WStype_TEXT:
//      Serial.printf("[WebSocket] Message: %s\n", payload);
      processSlackMessage((char*)payload);
      break;
  }
}

void showSlackSyntax() {
#ifdef HAVE_MAX7219
      replyToSlackMessage("ESP32 Bot online,\nCommands:\ndisp off|right|left|up|down\nalert\ntext STRING\nlayout 1|2");
#endif
#ifndef HAVE_MAX7219
      replyToSlackMessage("ESP32 Bot online,\nCommands:\ntext STRING\nlayout 1|2");
#endif
}


/**
  Establishes a bot connection to Slack:
  1. Performs a REST call to get the WebSocket URL
  2. Conencts the WebSocket
  Returns true if the connection was established successfully.
*/
bool connectToSlack() {
  // Step 1: Find WebSocket address via RTM API (https://api.slack.com/methods/rtm.connect)
  HTTPClient http;
  http.begin("https://slack.com/api/rtm.connect?token=" SLACK_BOT_TOKEN, SLACK_SSL_FINGERPRINT);
  int httpCode = http.GET();
  Serial.printf("HTTP GET returncode %d\n", httpCode);
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP GET failed with code %d\n", httpCode);
    return false;
  }

  WiFiClient *client = http.getStreamPtr();
  client->find("wss:\\/\\/");
  String host = client->readStringUntil('\\');
  String path = client->readStringUntil('"');
  path.replace("\\/", "/");

  // Step 2: Open WebSocket connection and register event handler
  Serial.println("WebSocket Host=" + host + " Path=" + path);
  slack_webSocket.beginSSL(host, 443, path, "", "");
  slack_webSocket.onEvent(webSocketEvent);
  return true;
}

#endif
