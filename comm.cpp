#include "comm.h"
#include "string.h"
#include <QDebug>

//uint8_t comm_raw[FRAME_SIZE];
Comm_Frame_t cPack;
// -----------------------------
// Float decode (little endian)
// -----------------------------
float bytes_to_float(int8_t *data)
{
    float value;
    memcpy(&value, data, sizeof(float));
    return value;
}

const char* topic_to_string(uint8_t topic)
{
    switch(topic)
    {
        case HEARTBEAT: return "HEARTBEAT";
        case CMD: return "CMD";
        case SENSOR_DATA: return "SENSOR";
        case JDEBUG: return "DEBUG";
        default:   return "UNKNOWN";
    }
}

const char* type_to_string(uint8_t type)
{
    switch(type)
    {
        case SPEED: return "SPEED";
        case STEERING: return "STEERING";
        case VOLTAGE: return "VOLTAGE";
        case EMERGENCY_STOP: return "EMERGENCY_STOP";
        default:   return "UNKNOWN";
    }
}

uint16_t crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;	//x^16 + x^15 + x^2 + 1 and LSB-first so 1000 0000 0000 0101 -> 0101 0000 0000 0001 = 0xA001
            else
                crc >>= 1;
        }
    }
    return crc;
}

uint16_t cobs_encode(const uint8_t *input, uint16_t length, uint8_t *output)
{
    uint16_t read_index = 0;
    uint16_t write_index = 1;
    uint16_t code_index = 0;
    uint8_t data_length = 1;	//section data

    while (read_index < length) {
        if (input[read_index] == 0) {	//section end
            output[code_index] = data_length;	//refill length
            data_length = 1;					//reset
            code_index = write_index++;			//move to next section index
        } else {
            output[write_index++] = input[read_index];
            data_length++;
            if (data_length == 0xFF) {		//section max is 255 so need new section
                output[code_index] = data_length;
                data_length = 1;
                code_index = write_index++;
            }
        }
        read_index++;
    }

    output[code_index] = data_length;
    return write_index;
}

void cobs_decode(const QByteArray &input, uint16_t length, QByteArray &output)
{
    uint16_t read_index = 0;
    //uint16_t write_index = 0;
    output.clear();
    while (read_index < length) {
        //uint8_t data_length = input[read_index];
        uint8_t data_length = input.at(read_index);
        if (read_index + data_length > length && data_length != 1)
            return ; // error

        read_index++;

        for (uint8_t i = 1; i < data_length; i++)
            output.append(input.at(read_index++));
            //output[write_index++] = input[read_index++];

        if (data_length != 0xFF && read_index < length)
            output.append(static_cast<char>(0));
            //output[write_index++] = 0;
    }

    //return write_index;
}

void send_packet(Comm_Frame_t *cFrame)
{
    uint8_t encoded[FRAME_SIZE];

    uint16_t idx = serialize(comm_raw, cFrame);

    // CRC
    uint16_t crc = crc16(comm_raw, idx);
    comm_raw[idx++] = crc & 0xFF;
    comm_raw[idx++] = (crc >> 8);

    // COBS encode
    uint16_t enc_len = cobs_encode(comm_raw, idx, encoded);

    // Framing (COBS standard)
    uint8_t zero = 0x00;
//    HAL_UART_Transmit(&COMM_UART_HANDLE, &zero, 1, 10);
//    HAL_UART_Transmit(&COMM_UART_HANDLE, encoded, enc_len, 100);
//    HAL_UART_Transmit(&COMM_UART_HANDLE, &zero, 1, 10);
}

/*void recv_packet(QByteArray &pack)
{
    uint8_t encoded[FRAME_SIZE];

    uint16_t enc_len = cobs_decode(const uint8_t *input, uint16_t length, uint8_t *output);
    uint16_t idx = serialize(comm_raw, cFrame);

    // CRC
    uint16_t crc = crc16(comm_raw, idx);
    comm_raw[idx++] = crc & 0xFF;
    comm_raw[idx++] = (crc >> 8);

    // COBS encode
    uint16_t enc_len = cobs_encode(comm_raw, idx, encoded);

    // Framing (COBS standard)
    uint8_t zero = 0x00;
    //    HAL_UART_Transmit(&COMM_UART_HANDLE, &zero, 1, 10);
    //    HAL_UART_Transmit(&COMM_UART_HANDLE, encoded, enc_len, 100);
    //    HAL_UART_Transmit(&COMM_UART_HANDLE, &zero, 1, 10);
}*/

uint16_t serialize(uint8_t* raw, Comm_Frame_t *cFrame)
{
    memset(raw, 0, FRAME_SIZE);

    uint16_t idx = 0;
    raw[idx++] = cFrame->topic;
    raw[idx++] = cFrame->type;
    raw[idx++] = cFrame->len;
    raw[idx++] = cFrame->seq;
    memcpy(&raw[idx], cFrame->payload, cFrame->len);
    //pack.append(reinterpret_cast<const char*>(cFrame->payload), cFrame->len); //fill in payload
    idx += cFrame->len;

    return idx;
}

uint16_t deserialize(QByteArray &pack, Comm_Frame_t *cFrame)
{
    memset(cFrame, 0, sizeof(Comm_Frame_t));

    uint16_t idx = 0;
    cFrame->topic = pack.at(idx++);
    cFrame->type = pack.at(idx++);
    cFrame->len = pack.at(idx++);
    cFrame->seq = pack.at(idx++);
    if (idx + cFrame->len > pack.size()) return 0xFFFF;
        memcpy(cFrame->payload, pack.constData()+idx, cFrame->len);
    idx += cFrame->len;
    cFrame->crc_lo = pack.at(idx++);
    cFrame->crc_hi = pack.at(idx++);
    uint16_t crc_calc = crc16((const uint8_t*)pack.constData(), 4 + cFrame->len);; // header + payload
    uint16_t crc_recv = (uint8_t)cFrame->crc_lo | ((uint16_t)(cFrame->crc_hi << 8));

    qDebug() << "crc_calc:" <<  Qt::hex << crc_calc;
    qDebug() << "crc_recv:" <<  Qt::hex << crc_recv;
    if (crc_calc != crc_recv)
    {
        return 0xFFFF; // error
    }
    return crc_calc;
    //return idx;
}
