#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

class RestClient {

  public:
    RestClient(const char* host);
    RestClient(const char* _host, int _port);

    //Client Setup
    int begin(const char* ssid, const char* pass);

    //Generic HTTP Request
    int request(const char* method, const char* path,
                const char* body, String* response);
    // Set a Request Header
    void setHeader(const char*);
    // Set Content-Type Header
    void setContentType(const char*);
    // Set Request timeout
    void setTimeout(unsigned int);


    //Set to use a secure connection
    void setSecureConnection(bool secureConn);
    //Set the fingerprint of SSL certificate to use secure connection
    void setFingerprint(const char* fingerPrint);

    // GET path
    int get(const char*);
    // GET path and response
    int get(const char*, String*);

    // POST path and body
    int post(const char* path, const char* body);
    // POST path and body and response
    int post(const char* path, const char* body, String*);

    // PUT path and body
    int put(const char* path, const char* body);
    // PUT path and body and response
    int put(const char* path, const char* body, String*);

    // DELETE path
    int del(const char*);
    // DELETE path and body
    int del(const char*, const char*);
    // DELETE path and response
    int del(const char*, String*);
    // DELETE path and body and response
    int del(const char*, const char*, String*);

  private:
    WiFiClient client;
    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client_s;
    int readResponse(String*);
    void write(const char*);
    const char* host;
    int port;
    int num_headers;
    const char* headers[10];
    bool use_https;
    const char* fingerprint;
	const char* contentType;
    unsigned int timeout;
};
