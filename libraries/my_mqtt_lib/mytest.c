


/* ======================================================================================================================== */
/* TCP Verbindung aufbauen */
/* ======================================================================================================================== */
        wifi.createTCP(HOST_NAME, HOST_PORT)



/* ======================================================================================================================== */
/* TCP Daten senden: CONNECT */
/* ======================================================================================================================== */

        /* Was wir haben: */

                char *hello = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n";
                wifi.send((const uint8_t*)hello, strlen(hello));


        /* Wie die neue Library aufgerufen wird: */

                if (client.connect("arduinoClient")) {
                        Serial.println("connected");
                }

        /* Wie die neue Library funktioniert: */

                boolean PubSubClient::connect(const char *id, const char *user, const char *pass) {
                        // user/pass kann auch NULL/NULL sein!
                    return connect(id,user,pass,0,0,0,0);

                }

                -->

                /* DIESE METHODE WIRD FÜR DEN CONNECT AUFGERUFEN. VIELLEICHT KÖNNEN WIR DIE 1:! NUTZEN UND DAFÜR EINFACH DEN buffer[] ZURÜCKGEBEN? */

                







        

/* ======================================================================================================================== */
/* TCP Daten empfangen */
/* ======================================================================================================================== */
        uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);


