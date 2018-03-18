#include "RestClient.h"

#ifdef HTTP_DEBUG
#define HTTP_DEBUG_PRINT(string) (Serial.print(string))
#endif

#ifndef HTTP_DEBUG
#define HTTP_DEBUG_PRINT(string)
#endif

RestClient::RestClient(const char* _host){
  host = _host;
  port = 80;
  num_headers = 0;
  contentType = "application/x-www-form-urlencoded";	// default
  timeout = 5000;
  use_https = false;
  fingerprint = "";
}

RestClient::RestClient(const char* _host, int _port){
  host = _host;
  port = _port;
  num_headers = 0;
  contentType = "application/x-www-form-urlencoded";	// default
  timeout = 5000;
  use_https = false;
  fingerprint = "";
}

int RestClient::begin(const char* ssid, const char* pass) {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  return WiFi.status();
}

// GET path
int RestClient::get(const char* path){
  return request("GET", path, NULL, NULL);
}

//GET path with response
int RestClient::get(const char* path, String* response){
  return request("GET", path, NULL, response);
}

// POST path and body
int RestClient::post(const char* path, const char* body){
  return request("POST", path, body, NULL);
}

// POST path and body with response
int RestClient::post(const char* path, const char* body, String* response){
  return request("POST", path, body, response);
}

// PUT path and body
int RestClient::put(const char* path, const char* body){
  return request("PUT", path, body, NULL);
}

// PUT path and body with response
int RestClient::put(const char* path, const char* body, String* response){
  return request("PUT", path, body, response);
}

// DELETE path
int RestClient::del(const char* path){
  return request("DELETE", path, NULL, NULL);
}

// DELETE path and response
int RestClient::del(const char* path, String* response){
  return request("DELETE", path, NULL, response);
}

// DELETE path and body
int RestClient::del(const char* path, const char* body ){
  return request("DELETE", path, body, NULL);
}

// DELETE path and body with response
int RestClient::del(const char* path, const char* body, String* response){
  return request("DELETE", path, body, response);
}

void RestClient::write(const char* string){
  HTTP_DEBUG_PRINT(string);
  if(use_https) {
    client_s.print(string);
  } else {
    client.print(string);
  }
}

void RestClient::setHeader(const char* header){
  headers[num_headers] = header;
  num_headers++;
}

void RestClient::setContentType(const char* contentTypeValue){
  contentType = contentTypeValue;
}

void RestClient::setTimeout(unsigned int timeoutMilliseconds) {
    timeout = timeoutMilliseconds;
}

void RestClient::setSecureConnection(bool secureConn){
  use_https = secureConn;
}

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
// EX: CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C
void RestClient::setFingerprint(const char* fingerPrint){
  fingerprint = fingerPrint;
}

// The mother- generic request method.
//
int RestClient::request(const char* method, const char* path,
                  const char* body, String* response){

  HTTP_DEBUG_PRINT("HTTP: connect\n");

  if(use_https) {
    //Secure connection
    if(!client_s.connect(host, port)){
      HTTP_DEBUG_PRINT("HTTPS Connection failed\n");
      return 0;
    }

    if (client_s.verify(fingerprint, host)) {
      HTTP_DEBUG_PRINT("SSL fingerprint certificate matches!");
    } else {
      HTTP_DEBUG_PRINT("SSL fingerprint certificate doesn't match!");
      return 0;
    }

  } else {
    //Normal connection
    client.setTimeout(timeout);

    if(!client.connect(host, port)){
      HTTP_DEBUG_PRINT("HTTP Connection failed\n");
      return 0;
    }
  }

  HTTP_DEBUG_PRINT("HTTP: connected\n");
  HTTP_DEBUG_PRINT("REQUEST: \n");

  // Make a HTTP request line:
  write(method);
  write(" ");
  write(path);
  write(" HTTP/1.1\r\n");
  for(int i=0; i<num_headers; i++){
    write(headers[i]);
    write("\r\n");
  }
  write("Host: ");
  write(host);
  write("\r\n");
  write("Connection: close\r\n");

  if(body != NULL){
    char contentLength[30];
    sprintf(contentLength, "Content-Length: %d\r\n", strlen(body));
    write(contentLength);

  write("Content-Type: ");
  write(contentType);
  write("\r\n");
  }

  write("\r\n");

  if(body != NULL){
    write(body);
    write("\r\n");
    write("\r\n");
  }

  //make sure you write all those bytes.
  delay(100);

  HTTP_DEBUG_PRINT("HTTP: call readResponse\n");
  int statusCode = readResponse(response);
  HTTP_DEBUG_PRINT("HTTP: return readResponse\n");

  //cleanup
  HTTP_DEBUG_PRINT("HTTP: stop client\n");
  num_headers = 0;

  if(use_https) {
    client_s.stop();
  } else {
    client.stop();
  }

  delay(50);
  HTTP_DEBUG_PRINT("HTTP: client stopped\n");

  return statusCode;
}

int RestClient::readResponse(String* response) {

  // an http request ends with a blank line
  boolean currentLineIsBlank = true;
  boolean httpBody = false;
  boolean inStatus = false;

  char statusCode[4];
  int i = 0;
  int code = 0;

  if (use_https) {
    //Secure connection
    if(response == NULL){
      HTTP_DEBUG_PRINT("HTTPS: NULL RESPONSE POINTER: \n");
    }else{
      HTTP_DEBUG_PRINT("HTTPS: NON-NULL RESPONSE POINTER: \n");
    }

    HTTP_DEBUG_PRINT("HTTPS: RESPONSE: \n");

    while (client_s.connected()) {
      HTTP_DEBUG_PRINT(".");

      if (client_s.available()) {
        HTTP_DEBUG_PRINT(",");

        char c = client_s.read();
        HTTP_DEBUG_PRINT(c);

        if(c == ' ' && !inStatus){
          inStatus = true;
        }

        if(inStatus && i < 3 && c != ' '){
          statusCode[i] = c;
          i++;
        }
        if(i == 3){
          statusCode[i] = '\0';
          code = atoi(statusCode);
        }

        if(httpBody){
          //only write response if its not null
          if(response != NULL) response->concat(c);
        }
        else
        {
            if (c == '\n' && currentLineIsBlank) {
              httpBody = true;
            }

            if (c == '\n') {
              // you're starting a new line
              currentLineIsBlank = true;
            }
            else if (c != '\r') {
              // you've gotten a character on the current line
              currentLineIsBlank = false;
            }
        }
      }
    }

    HTTP_DEBUG_PRINT("HTTPS: return readResponse3\n");
    return code;

  } else {
    //Normal connection
    if(response == NULL){
      HTTP_DEBUG_PRINT("HTTP: NULL RESPONSE POINTER: \n");
    }else{
      HTTP_DEBUG_PRINT("HTTP: NON-NULL RESPONSE POINTER: \n");
    }

    HTTP_DEBUG_PRINT("HTTP: RESPONSE: \n");
    while (client.connected()) {
      HTTP_DEBUG_PRINT(".");

      if (client.available()) {
        HTTP_DEBUG_PRINT(",");

        char c = client.read();
        HTTP_DEBUG_PRINT(c);

        if(c == ' ' && !inStatus){
          inStatus = true;
        }

        if(inStatus && i < 3 && c != ' '){
          statusCode[i] = c;
          i++;
        }
        if(i == 3){
          statusCode[i] = '\0';
          code = atoi(statusCode);
        }

        if(httpBody){
          //only write response if its not null
          if(response != NULL) response->concat(c);
        }
        else
        {
            if (c == '\n' && currentLineIsBlank) {
              httpBody = true;
            }

            if (c == '\n') {
              // you're starting a new line
              currentLineIsBlank = true;
            }
            else if (c != '\r') {
              // you've gotten a character on the current line
              currentLineIsBlank = false;
            }
        }
      }
    }

    HTTP_DEBUG_PRINT("HTTP: return readResponse3\n");
    return code;
  }
}
