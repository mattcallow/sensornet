/*
 * sn_messages.h
 *
 *  Created on: 23/01/2013
 *      Author: matt
 */

#ifndef SN_MESSAGES_H_
#define SN_MESSAGES_H_

#ifdef ARDUINO
#include "Arduino.h"
#endif

#ifdef __arm__
#pragma pack(1)
#endif
typedef struct {
#define SN_HEADER_LEN 5
	uint8_t source;
	uint8_t destination;
	uint8_t type;
	uint8_t seq;
	uint8_t max_hops;
} sn_header_t;

#define SN_PAYLOAD_LEN 20
typedef struct {
	sn_header_t header;
	uint8_t payload[SN_PAYLOAD_LEN];
} sn_message_t;


#define SN_MESSAGE_SENSOR_DATA		0x03
#define SN_MESSAGE_SET_CONTROL		0x04
// message type 0x05 unused
#define SN_MESSAGE_UPDATE_ROUTE		0x06
// message type 0x07 unused
#define SN_MESSAGE_DELETE_ROUTE		0x08
// message type 0x09 unused
#define SN_MESSAGE_DELETE_ALL_ROUTES	0x0A
// message type 0x0B unused

/*
 * Message definitions
 */
#define SN_MESSAGE_AYT 			0x00
// no payload

typedef struct {
#define SN_MESSAGE_STATUS		0x01
	uint8_t status;
#define STATUS_REBOOT 			0x01
#define STATUS_CONFIG_CORRUPT 	0x02
	uint8_t capabilities;
#define CAPABILITY_RX			0x01
#define CAPABILITY_TX			0x02
#define CAPABILITY_ROUTER		0x04
#define CAPABILITY_RTC			0x08
	// 0x10 unused
#define CAPABILITY_MAINS		0x20
#define CAPABILITY_BATTERY		0x40
#define CAPABILITY_SOLAR		0x80
	uint16_t supply_voltage;
	uint8_t max_routes;
} sn_payload_status_t;

typedef struct {
#define SN_MESSAGE_REQ_SENSOR_DATA	0x02
	uint8_t sensor_number;
} sn_payload_req_sensor_data_t;

typedef struct {
	uint8_t type;
	uint8_t sensor;
	union {
		uint8_t digital;
		uint16_t analog16;
		uint32_t analog32;
		uint16_t counter16;
		uint32_t counter32;
	} data;
} sn_event_t;

typedef struct {
#define SN_MESSAGE_SENSOR_DATA		0x03
	uint8_t sensor;
	uint8_t data_valid;
	union {
		uint8_t digital;
		uint16_t analog16;
		uint32_t analog32;
		uint16_t counter16;
		uint32_t counter32;
		sn_event_t event;
	} data;
} sn_payload_sensor_data_t;

#ifdef __arm__
#pragma pack()
#endif
/*
 * End of message definitions
 */
#endif /* SN_MESSAGES_H_ */
