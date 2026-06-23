#ifndef COMM_H
#define COMM_H
#include <stdint.h>
#include "qbytearray.h"

/**
 * @brief Custom UART Communication Frame Structure
 * * Use Case: Communication between MCU (STM32) and Jetson Nano/PC.
 * * Transport using Consistent Overhead Byte Stuffing
 * * [ Frame Format ]
 * +-------+--------+--------+--------+------------+----------+----------+
 * | TOPIC |  TYPE  |  LEN   |  SEQ   |  PAYLOAD   |  CRC_LO  |  CRC_HI	 |
 * |  (1B) |  (1B)  |  (1B)  |  (1B)  | (N Bytes)  |   (1B)   |   (1B)	 |
 * +-------+--------+--------+--------+------------+----------+----------+
 */
/* --- Field Descriptions --- */
//
// 2. TOPIC (1 byte): Defines the TOPIC of data
//    - 0x01: HEARTBEAT
//    - 0x10: CMD
//    - 0x20: SENSOR_DATA
//    - 0xF0: DEBUG
//
// 2. TYPE (1 byte): Defines the sub type of data
//    - 0x01: Speed
//    - 0x02: Steering
//    - 0x03: Emergency Stop
//
// 3. LEN (1 byte): Payload Length
//    - Range: 0–255. Represents the total number of bytes in the PAYLOAD field.
//
// 4. SEQ (1 byte): Sequence Number
//    - Increments with every packet.
//    - Used to detect dropped packets (Packet Loss) and for debugging.
//
// 5. PAYLOAD (N bytes): The actual data content
//    - Parsed based on the TYPE and LEN values.
//
// 6. CRC16 (2 bytes): Cyclic Redundancy Check (CRC-16-IBM)
//    - Verifies data integrity.
//    - Prevents corrupted packets caused by UART noise or electrical interference.
//
//		[Application Layer]   TOPIC / TYPE / PAYLOAD
//        		↓
//		[Protocol Layer]      serialize / CRC
//        		↓
//		[Transport Layer]     COBS(Consistent Overhead Byte Stuffing) + UART

#define PAYLOAD_MAX_SIZE	255
#define HEARTBEAT_HEAD1		0xAA
#define HEARTBEAT_HEAD2		0x55
#define FRAME_SIZE			(PAYLOAD_MAX_SIZE + 6)

inline uint8_t comm_raw[FRAME_SIZE];

/*enum {
    IDX_TOPIC = 0,
    IDX_TYPE,
    IDX_LEN,
    IDX_SEQ
};*/

typedef enum{
    HEARTBEAT = 0x01,	//Jetson -> Heart beat
    CMD = 0x10, 		//Jetson -> Command
    SENSOR_DATA = 0x20,	//STM32	 -> Data
    JDEBUG = 0xF0		//debug
}Topic;

typedef enum{
    SPEED = 0x01,
    STEERING = 0x02,
    VOLTAGE = 0x03,
    EMERGENCY_STOP = 0x04
}Type;

typedef struct{
    int8_t topic;
    int8_t type;
    int8_t len;
    int8_t seq;
    int8_t payload[PAYLOAD_MAX_SIZE];
    int8_t crc_lo;
    int8_t crc_hi;
}Comm_Frame_t;

extern Comm_Frame_t cPack;
uint16_t crc16(const uint8_t *data, uint16_t len);
float bytes_to_float(int8_t *data);
const char* topic_to_string(uint8_t topic);
const char* type_to_string(uint8_t type);
uint16_t serialize(uint8_t* raw, Comm_Frame_t *cFrame);
uint16_t deserialize(QByteArray &pack, Comm_Frame_t *cFrame);
void cobs_decode(const QByteArray &input, uint16_t length, QByteArray &output);
void send_packet(Comm_Frame_t *cFrame);
#endif // COMM_H
