/*
  ESP8266MQTT.cpp - A MQTT library for ESP8266 AT
  Tobias Schaber, codecentric AG
*/

#include "ESP8266MQTT.h"
#include "Arduino.h"

ESP8266MQTT::ESP8266MQTT() {

}

uint8_t* ESP8266MQTT::getConnectMessage(const char *id) {
    return getConnectMessage(id,NULL,NULL,0,0,0,0);
}

uint8_t* ESP8266MQTT::getConnectMessage(const char *id, const char *user, const char *pass) {
    return getConnectMessage(id,user,pass,0,0,0,0);
}

uint8_t* ESP8266MQTT::getConnectMessage(const char *id, const char *user, const char *pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage) {


		// Leave room in the buffer for header and variable length field
		uint16_t length = 5;
		unsigned int j;

		/* this is for MQTT Protocol Version 3.1.1. For 3.1, change things here! */
		uint8_t d[7] = {0x00,0x04,'M','Q','T','T',MQTT_VERSION};
		uint8_t MQTT_HEADER_VERSION_LENGTH = 7;

		for (j = 0;j<MQTT_HEADER_VERSION_LENGTH;j++) {
			buffer[length++] = d[j];
		}

		uint8_t v;
		if (willTopic) {
			v = 0x06|(willQos<<3)|(willRetain<<5);
		} else {
			v = 0x02;
		}

		if(user != NULL) {
			v = v|0x80;

			if(pass != NULL) {
				v = v|(0x80>>1);
			}
		}

		buffer[length++] = v;
		
		/* define mqtt keepalive in sec */
		//uint8_t MQTT_KEEPALIVE = 15;

		buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
		buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);
		length = writeString(id,buffer,length);
		if (willTopic) {
			length = writeString(willTopic,buffer,length);
			length = writeString(willMessage,buffer,length);
		}

		if(user != NULL) {
			length = writeString(user,buffer,length);
			if(pass != NULL) {
				length = writeString(pass,buffer,length);
			}
		}


		return write(MQTTCONNECT,buffer,length-5);
		
/* connect ack lesen? 
 
  		uint8_t llen;
		uint16_t len = readPacket(&llen);
		
		if (len == 4) {
			if (buffer[3] == 0) {
				lastInActivity = millis();
				pingOutstanding = false;
				_state = MQTT_CONNECTED;
				return true;
			} else {
				_state = buffer[3];
			}
		}
		_client->stop();
	} else {
		_state = MQTT_CONNECT_FAILED;
	}
	return false;
	
	
	*/

}

/*
 * (MQTTCONNECT,buffer,length-5);
 */
 
uint8_t* ESP8266MQTT::write(uint8_t header, uint8_t* buf, uint16_t length) {
    uint8_t lenBuf[4];
    uint8_t llen = 0;
    uint8_t digit;
    uint8_t pos = 0;
    uint16_t rc;
    uint16_t len = length;
    do {
        digit = len % 128;
        len = len / 128;
        if (len > 0) {
            digit |= 0x80;
        }
        lenBuf[pos++] = digit;
        llen++;
    } while(len>0);

    buf[4-llen] = header;
    for (int i=0;i<llen;i++) {
        buf[5-llen+i] = lenBuf[i];
        //Serial.print("x:");
        //Serial.println(buf[5-llen+i]);
    }

    // TODO: THIS REPLACED! rc = _client->write(buf+(4-llen),length+1+llen);
    return buf+(4-llen);
    
    //return (rc == 1+llen+length);

}

uint16_t ESP8266MQTT::writeString(const char* string, uint8_t* buf, uint16_t pos) {
    const char* idp = string;
    uint16_t i = 0;
    pos += 2;
    while (*idp) {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos-i-2] = (i >> 8);
    buf[pos-i-1] = (i & 0xFF);
    return pos;
}

