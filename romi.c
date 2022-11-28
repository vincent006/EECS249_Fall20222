#include "romi.h"
#include <math.h>
#include <string.h> // Defines memcpy
#include <stdlib.h> // Defines abs
#include <stdint.h> // Defines uint8_t, etc.

#include <wiringPi.h>
#include <wiringSerial.h>
#include <cstdio>
#include <errno.h>
#include <unistd.h>

// uart instance for rasp pi
static int serial_port;


// See romi.h for function documentation.

// NOTE: The implementation here uses code in the buckler repo.
// Eventually, it should replace all that code.

///////////////////////////////////////////////////////////////////////////////////////
//// Private functions. Not documented in romi.h. Used in public functions below.
//// Not intended to be called by users.

/**
 * @brief Combine bytes into a 16 bit unsigned integer.
 * @param low Low-order byte.
 * @param high High-order byte.
 * @return The combined bytes.
 */
static uint16_t to_uint16(uint8_t low, uint16_t high)
{
    return ((uint16_t)high << 8) | low;
}

/**
 * @brief Calculate and return a checksum for the specified buffer.
 * This is based on checkSum() in the buckler repo.
 *
 * @param buffer The buffer.
 * @param length The length of the buffer.
 * @return The XOR of the bytes in the buffer.
 */
static uint8_t _romi_checksum(uint8_t *buffer, int length)
{
    uint8_t cs = 0x00;
    // printf("\n l = %d \n",length);
    for (int i = 2; i < length; i++)
    {
        // printf("%d ",buffer[i]);
        cs ^= buffer[i];
    }
    // printf("\n last[%d]=%d cs=%d",i,buffer[i],cs);
    return cs;
}

/**
 * @brief Send the specified payload over UART to the Romi robot.
 * This function adds a header (0xAA and 0x55) and a length to
 * the front of the byte stream that is sent, plus a checksum at
 * the end.
 * This is based on kobukiSendPayload from the buckler repo,
 * converted to use lower-level UART functions.
 *
 * @param payload The payload.
 * @param length The size of the payload.
 * @return An error code that should be checked using the macro APP_ERROR_CHECK.
 */
void _romi_send_payload(uint8_t *payload, uint8_t length)
{
    uint8_t writeData[256] = {0};

    // Write move payload
    writeData[0] = 0xAA;
    writeData[1] = 0x55;
    writeData[2] = length;
    memcpy(writeData + 3, payload, length);
    writeData[3 + length] = _romi_checksum(writeData, 3 + length);

    // nrfx_uart_tx(&nrfx_uart, writeData, length + 4);
    // ssize_t write(int fildes, const void *buf, size_t nbyte);
    printf("%d, %d, %d, %d\n", writeData[0], writeData[1], writeData[2], writeData[3]);
    write(serial_port, writeData, (size_t)(length + 4));
}

/**
 * @brief Send a drive command with specified radius and speed to the Romi.
 * This is based on kobukiDriveRadius from the buckler repo.
 *
 * @param radius The radius.
 * @param speed The speed.
 * @return An error code that should be checked using the macro APP_ERROR_CHECK.
 */
void _romi_drive_radius(int16_t radius, int16_t speed)
{
    uint8_t payload[6];
    payload[0] = 0x01;
    payload[1] = 0x04;
    memcpy(payload + 2, &speed, 2);
    memcpy(payload + 4, &radius, 2);

    _romi_send_payload(payload, 6);
}

/**
 * @brief Initialize the UART to a baud rate of 115,200.
 * @return An error code that should be checked using the macro APP_ERROR_CHECK.
 */
int _romi_uart_init()
{
    if ((serial_port = serialOpen ("/dev/ttyAMA0", 115200)) < 0){	/* open serial port */
        printf("Unable to open serial device\n");
        return 1 ;
    } else {
        printf("Serial opened\n");
    }
    if (wiringPiSetup () == -1)					/* initializes wiringPi setup */
    {
        fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
        return 1 ;
    }
    return 0;
}

int rasp_uart_rx(int fd, uint8_t* p_data, size_t length) {
    int ret = 0;
    uint8_t buf = 0;
    for (int i = 0; i < length; i++) {
        // ret = read(fd, &buf, sizeof(p_data[i]));
        // p_data[i] = (char) buf;
        // printf("read: %d %d\n", p_data[i], buf);
        // if (ret == 0) {
        //     return 0;
        // }

        buf = serialGetchar(fd);
        p_data[i] = buf;
        printf("read %d: %d\n", i, buf);

    }
    return 1;
}

/**
 * @brief Read a packet from the serial port.
 * Read from the serial port until a header AA55 is encountered.
 * Then read the length of the packet.
 * Then read the packet into the specified buffer.
 *
 * @param packetBuffer The buffer into which to put the result.
 * @param len The length of the buffer.
 * @return int32_t
 */
int _romi_read_serial(uint8_t *packetBuffer, uint8_t len)
{
    // States of the state machine depending on what is read next.
    // Header should start with AA55.
    typedef enum
    {
        wait_until_header,
        read_length,
        read_payload,
        read_checksum
    } state_type;

    state_type state = wait_until_header;
    uint8_t header_buf[2];
    uint8_t char_buf;
    uint8_t payloadSize = 0;
    uint8_t calcuatedCS = 0;
    uint8_t byteBuffer = 0;
    int ret;
    int err_cnt = 0;
    bool done = false;


    int num_checksum_failures = 0;

    // if (len <= 4)
    // {
    //     ret = NRF_ERROR_NO_MEM;
    //     done = true;
    // }

    while (!done)
    {
        switch (state)
        {
        case wait_until_header:
            // ret = nrfx_uart_rx(&nrfx_uart, header_buf, 2);
            ret = read(serial_port, header_buf, 2);
            if (ret == 0)
            {
                err_cnt++;
                break;
            }

            if (header_buf[0] == 0xAA && header_buf[1] == 0x55)
            {
                state = read_length;
            }
            else
            {
                state = wait_until_header;
            }
            break;

        case read_length:
            //ret = nrfx_uart_rx(&nrfx_uart, &payloadSize, sizeof(payloadSize));
            ret = read(serial_port, &payloadSize, sizeof(payloadSize));
            if (ret == 0)
            {
                err_cnt++;
                state = wait_until_header;
                break;
            }
            if (len < payloadSize + 3)
            {
                done = true;
                ret = -1;
            }
            state = read_payload;
            break;

        case read_payload:
            // ret = nrfx_uart_rx(&nrfx_uart, packetBuffer + 3, payloadSize + 1);
            ret = read(serial_port, packetBuffer + 3, payloadSize + 1);
            //ret = rasp_uart_rx(serial_port, packetBuffer + 3, payloadSize + 1);
            if (ret == 0)
            {
                err_cnt++;
                state = wait_until_header;
                break;
            }
            state = read_checksum;
            break;

        case read_checksum:
            memcpy(packetBuffer, header_buf, 2);
            packetBuffer[2] = payloadSize;

            calcuatedCS = _romi_checksum(packetBuffer, payloadSize + 3);
            byteBuffer = (packetBuffer)[payloadSize + 3];
            done = true;
            if (calcuatedCS == byteBuffer)
            {
                num_checksum_failures = 0;
                ret = 1;
            }
            else
            {
                state = wait_until_header;
                if (num_checksum_failures == 3)
                {
                    ret = -1500;
                }
                num_checksum_failures++;
            }
            break;

        default:
            break;
        }

        // If we get too many errors abort the sensor reading.
        if (ret == 0 && err_cnt >= 20)
        {
            done = true;
        }
    }
    serialFlush(serial_port);

    return ret;
}

/**
 * @brief Parse the sensor data sent over the UART by the robot.
 * This is based on kobukiParseSensorPacket in kobukiSensors.c in the buckler repo.
 *
 * @param packet The raw sensor data.
 * @param sensors The struct into which to insert the data.
 */
static void _romi_parse_sensor_packet(const uint8_t *packet, romi_sensors_t *sensors)
{
    uint8_t payload_length = packet[2];
    uint8_t subpayload_length = 0;

    uint8_t i = 3;
    while (i < payload_length + 3)
    {

        uint8_t id = packet[i];

        subpayload_length = packet[i + 1];

        printf("id: %d\n", id);

        switch (id)
        {
        case 0x01:
            // There's an ambiguity in the documentation where
            // it says there are two headers with value 0x01:
            // basic sensor data and controller info - although it
            // says elsewhere that controller info has ID 0x15
            // so we'll just check here to make sure it's the right length

            if (subpayload_length == 0x0F)
            {
                sensors->time_stamp = to_uint16(packet[i + 2], packet[i + 3]);

                sensors->bumps.right = packet[i + 4] & 0x01;
                sensors->bumps.center = (packet[i + 4] & 0x02);
                sensors->bumps.left = (packet[i + 4] & 0x04);

                sensors->reflectance.right = (packet[i + 6] & 0x01);
                sensors->reflectance.center = (packet[i + 6] & 0x02);
                sensors->reflectance.left = (packet[i + 6] & 0x04);

                sensors->encoders.left = to_uint16(packet[i + 7], packet[i + 8]);
                sensors->encoders.right = to_uint16(packet[i + 9], packet[i + 10]);
                printf("left packet: %d %d\n", packet[i + 7], packet[i + 8]);
                printf("encoder left: %d\n", to_uint16(packet[i + 7], packet[i + 8]));

                sensors->buttons.right = (bool)(packet[i + 13] & 0x01);
                sensors->buttons.left = (bool)(packet[i + 13] & 0x02);

                i += subpayload_length + 2; // + 2 for header and length
            }
            else
            {
                i += payload_length + 3; // add enough to terminate the outer while loop
            }

            break;

        default:
            printf("Unexpected message type over UART: %d", id);
            break;
        }
    }
    // Checksum has already been checked.
    return;
}

///////////////////////////////////////////////////////////////////////////////////////
//// Public functions. Documented in romi.h. Intended to be called by users.

bool romi_button_pressed(romi_sensors_t *const sensors)
{
    // save previous states of buttons
    static bool previous_left = false;
    static bool previous_right = false;

    bool result = false;

    // Check left button.
    bool current_left = sensors->buttons.left;
    if (current_left && previous_left != current_left)
    {
        result = true;
    }
    previous_left = current_left;

    // check right
    bool current_right = sensors->buttons.right;
    if (current_right && previous_right != current_right)
    {
        result = true;
    }
    previous_right = current_right;

    return result;
}

// Based on kobukiDriveDirect from the buckler repo, but with a number of fixes.
void romi_drive_direct(int16_t left_wheel_speed, int16_t right_wheel_speed)
{
    // Convert the independent wheel speeds into an approximate radius and speed.
    int32_t speed;
    int32_t radius;
    int32_t left_right;

    // Choose the larger of the two speeds.
    if (abs(right_wheel_speed) > abs(left_wheel_speed))
    {
        speed = right_wheel_speed;
        left_right = 1;
    }
    else
    {
        speed = left_wheel_speed;
        left_right = -1;
    }

    if (right_wheel_speed == left_wheel_speed)
    {
        radius = 0; // Special case 0 commands Kobuki travel with infinite radius.
    }
    else
    {
        double estimate = (right_wheel_speed + left_wheel_speed)
        		/ (2.0 * (right_wheel_speed - left_wheel_speed) / 123.0);
        // The value 123 was determined experimentally to work, and is approximately 1/2 the wheelbase in mm.

        radius = round(estimate);
        if (radius == 0)
            radius = left_right; // Avoid special case 0 unless want infinite radius.
        // if the above statement overflows a signed 16 bit value, set radius=0 for infinite radius.
        if (radius > 32767)
            radius = 0;
        if (radius < -32768)
            radius = 0;
    }
    /* Original had the following rather odd adjustment with no comment.
    if (radius == 1){
        speed = speed * -1;
    }
    */

    _romi_drive_radius(radius, speed);
}

int romi_init()
{
    uint32_t err_code = 0;

    err_code = _romi_uart_init();
    if (err_code != 0)
    {
        return err_code;
    }

    // Make sure the robot is stopped.
    romi_drive_direct(0, 0);

    return err_code;
}

int romi_sensors_poll(romi_sensors_t *const sensors)
{
    // initialize communications buffer
    // We know that the maximum size of the packet is less than 140 based on documentation
    uint8_t packet[140] = {0};
    int status = _romi_read_serial(packet, 140);
    if (status == 0)
    {
        printf("Read Serial Error\n");
        return status;
    }

    // parse response
    _romi_parse_sensor_packet(packet, sensors);

    return status;
}
