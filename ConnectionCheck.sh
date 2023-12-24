#!/bin/bash

MQTT_BROKER="YOUR_IP_ADDRESS"
MQTT_TOPIC=test

if mosquitto_pub -h $MQTT_BROKER -t $MQTT_TOPIC -m "ping" > /dev/null 2>&1; then
        # ESP32 is connected
        echo "ESP32 is connected to MQTT broker."
        crontab -r
        /path/to/gameplay/script
    else
        # ESP32 is not connected
        echo "ESP32 is not connected to MQTT broker. Exiting."

    fi

done 
