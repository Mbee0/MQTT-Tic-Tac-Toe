#!/bin/bash

echo "Messaging the ESP 32"
mosquitto_pub -h "YOUR_IP_ADDRESS" -t transferCoords -m "ready"
echo "Finished the message and beginning the loop..."

while :
do
        receivedMessage=$(mosquitto_sub -h "YOUR_IP_ADDRESS" -t transferSpots -C 1)
        player=${receivedMessage:0:1}
        available=${receivedMessage:1:1}
        echo 'message received: '"$receivedMessage"
        if [ "$player" == "X" ]; then
                spotPicked=$(./playerOne.sh $available)
                echo 'Spot x picked: '"$spotPicked"
                mosquitto_pub -h "YOUR_IP_ADDRESS" -t transferCoords -m 'X'"$spotPicked"
        elif [ "$player" == "O" ]; then
                spotPicked=$(./playerTwo.sh $available)
                echo 'Spot o picked: '"$spotPicked"
                mosquitto_pub -h "YOUR_IP_ADDRESS" -t transferCoords -m 'O'"$spotPicked"
        elif [ "$player" == "Q" ]; then
                break
        else
                echo "got in"
                echo "$receivedMessage" >> resultsOfTTT.txt
                echo 'Sent message: '"$receivedMessage"
        fi
done

echo "Game finished!" >> resultsOfTTT.txt

echo "job is done :)"
