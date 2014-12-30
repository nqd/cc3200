// #include "mqtt.h"
#include "mqtt/MQTTClient.h"
#include "common.h"

#include "gpio_if.h"
#include "uart_if.h"
#include "i2c_if.h"

#define MQTT_TOPIC_LEDS "ticc3200/leds/#"
#define MQTT_TOPIC_BUTTON1 "ticc3200/buttons/1"
#define MQTT_TOPIC_BUTTON2 "ticc3200/buttons/2"
#define DEVICE_STRING_SUB "ticc3200-sub"
#define DEVICE_STRING_PUB "ticc3200-pub"

//****************************************************************************
//
//! \brief ButtonNotify Task - Reads button states and publishes data to MQTT
//!                            broker.
//! \param[in]                  pvParameters is the data passed to the Task
//!
//! \return                        None
//
//****************************************************************************
int doReport(int *sensor)
{
    static unsigned int msg_id = 0;
    Network n;
    Client hMQTTClient;
    int rc = 0;
    unsigned char buf[100];
    unsigned char readbuf[100];
    

    // Wait for a connection
    // while (1) {

        NewNetwork(&n);
        UART_PRINT("Connecting Socket\n\r");
        //Works - Unsecured
        rc = ConnectNetwork(&n, "test.mosquitto.org", 1883);

        UART_PRINT("Opened TCP Port to mqtt broker with return code %d\n\r", rc);
        UART_PRINT("Connecting to MQTT broker\n\r");

        MQTTClient(&hMQTTClient, &n, 1000, buf, 100, readbuf, 100);
        MQTTPacket_connectData cdata = MQTTPacket_connectData_initializer;
        cdata.MQTTVersion = 3;
        cdata.keepAliveInterval = 10;
        cdata.clientID.cstring = (char*) DEVICE_STRING_PUB;
        rc = MQTTConnect(&hMQTTClient, &cdata);
        if (rc != 0) {
            UART_PRINT("rc from MQTT server is %d\n\r", rc);
        } else {
            UART_PRINT("connected to MQTT broker\n\r");
        }
        UART_PRINT("\tsocket=%d\n\r", n.my_socket);
    
        // while (1) {
        UART_PRINT("tick\n\r");
        UART_PRINT("SW3\n\r");

        char s_sensor[sizeof(int)+1];
        snprintf(s_sensor, sizeof(int), "%d", *sensor);

        MQTTMessage msg;
        msg.dup = 0;
        msg.id = msg_id++;
        msg.payload = s_sensor;
        msg.payloadlen = sizeof(*sensor);
        msg.qos = QOS0;
        msg.retained = 0;
        rc = MQTTPublish(&hMQTTClient, MQTT_TOPIC_BUTTON2, &msg);
        UART_PRINT("SW3, rc=%d\n\r", rc);
        if (rc != 0)
            return -1;

        rc = MQTTYield(&hMQTTClient, 10);
        if (rc != 0) {
            UART_PRINT("rc = %d\n\r", rc);
            return -1;
        }

        sl_Close(n.my_socket);

        return 0;
            // osi_Sleep(5000);
    //     }
    // }
}

