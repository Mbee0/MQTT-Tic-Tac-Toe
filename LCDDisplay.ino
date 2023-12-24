#include <WiFi.h>
#include<PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define SDA 13                    //Define SDA pins
#define SCL 14                    //Define SCL pins

// this is setting the lcd variable to be manipulated
LiquidCrystal_I2C lcd(0x27, 16, 2);

// this is setting up the variables for the internet connection
const char *ssid_Router = "Megan's Iphone";
const char *password_Router = "LAURENBEE";
const char *mqtt_server = "34.28.215.180";
const char *mqtt_subscribe_topic = "transferCoords";
const char *mqtt_publish_topic = "transferSpots";
const char *mqtt_publish_topic1 = "transferRatio";

// This is setting up the variables to play the tic tac toe game
char exsNOhs[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
int availableTiles[9] = {0,1,2,3,4,5,6,7,8};
int playerXWins = 0;
int playerOWins = 0;
int numberOfDraws = 0;
int numberOfTurnsLeft = 9;
int gameCount = 0;
bool currentPlayer = true; //true means X and false means O
bool gameEnd = false;
bool notReady = true;

WiFiClient espClient;
PubSubClient client(espClient);

// THIS IS THE FIRST THING THAT RUNS
void setup() {
  Serial.begin(115200);
  client.setServer(mqtt_server, 1883);
  setup_wifi();
  client.setCallback(callback);

  //Once the client has been set up with the broker, we are going to configure and light up the lcd; this will be a way for us to know when we are connected and ready to go
  Wire.begin(SDA, SCL);           // attach the IIC pin
  if (!i2CAddrTest(0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print("p1: 0 p2: 0 d: 0");     // The print content is displayed on the LCD
  lcd.setCursor(0, 1);
  lcd.print("Game Count:  0");
}

// connecting to wifi/the router that you are using
void setup_wifi(){
  Serial.println("Setup start");
  WiFi.begin(ssid_Router, password_Router);
  Serial.println(String("Connecting to ")+ssid_Router);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected, IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Setup End");
}

// called if the connection is not established/if it is running for the first time
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("transferCoords"); // Subscribe to the desired topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// I think that this loop is the main function
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  // wait for the connection with cron
  while(notReady){
    delay(5000);
    client.loop();
  }

  // if it is connected: start the 100 game process
  while (gameCount < 100){
    grabNewCoordinates();
  }
  // once it is over, we want to notify to the script that it should also be done running the script to send random spots
  const char * someting = "Q";
  client.publish("transferSpots", someting);

  // then we want to print that the games are over to our serial monitor
  Serial.println("100 games are over");
  Serial.println("Player X Wins: " + playerXWins);
  Serial.println("Player O Wins: " + playerOWins);
  Serial.println("Number of Draws: " + numberOfDraws);
  Serial.println("Stopping the program");
  lcd.setCursor(0,1);
  lcd.print("Game 100 over :)");
  delay(100000);
}

void grabNewCoordinates(){
  // this allows the client to accept messages from the broker and keep it alive to recieve messages
  client.loop();
  
  if(currentPlayer){
    String temp = String('X') + String(numberOfTurnsLeft);
    const char * message = temp.c_str();
    // Serial.println("Held in message: " + temp);
    client.publish("transferSpots", message);
	// this goes to the callback function
  } else {
    String temp = String('O') + String(numberOfTurnsLeft);
    const char * message = temp.c_str();
    client.publish("transferSpots", message);
  }
  currentPlayer = !currentPlayer;
  // this accounts for the time it takes for the arduino to run through the code within the call back function
  delay(1000);
}

// this callback function runs automatically once a publisher publishes something to a topic that this client has already subscribed to
void callback(char* topic, byte* payload, unsigned int length) {
  if(!notReady){
    numberOfTurnsLeft--;
    char player = (char)payload[0];
    char spot = (char)payload[1];
    // Serial.print("player received: ");
    // Serial.print(player);
    // Serial.print(", spot received: ");
    // Serial.println(spot);
    // need to get the specific tile
    int spotGrabbed = spot - '0';// this will turn into an int so we dont need to do (int)spot
    int tile;
    bool grabbed = false;
    int temp[numberOfTurnsLeft];
    // need to change the number of available tiles
    for(int i = 0; i <= numberOfTurnsLeft; i++){
      if (i == spotGrabbed){
        // Serial.print("when grabbed i: ");
        // Serial.print(i);
        // Serial.print(", spotGrabbed var: ");
        // Serial.println(spotGrabbed);
        tile = availableTiles[i];
        // availableTiles[i] = availableTiles[i++];
        grabbed = true;
      } else if(grabbed){
        // availableTiles[i] = availableTiles[i++];
        availableTiles[i - 1] = availableTiles[i];
      }
    }

    // Serial.println("Available tiles: ");
    // for(int i = 0; i < 9; i++){
    //   Serial.print(availableTiles[i]);
    //   Serial.print(", ");
    // }

    // Serial.print("tile grabbed: ");
    // Serial.println(tile);
    //sets the board
    exsNOhs[tile] = player;

    // checks to see if there is a winner
    if(numberOfTurnsLeft < 5){
      checkIfGameEnd();
    }
  } else {
    notReady = false;
  }
  
}

// uses the board to process if there is a winner or not
void checkIfGameEnd(){
  Serial.println("Board: ");
  for(int i = 0; i < 9; i++){
    Serial.print(exsNOhs[i]);
    Serial.print(", ");
  }
  int lines[8][3] = {{0,1,2}, {3,4,5}, {6,7,8}, {0,3,6}, {1,4,7}, {2,5,8}, {0,4,8}, {2,4,6}};
  for (int i = 0; i < 7; i++){
    int line[3] = {lines[i][0], lines[i][1], lines[i][2]};
    // technically not correct but whatever... (:
    if(exsNOhs[line[0]] == exsNOhs[line[1]] && exsNOhs[line[0]] == exsNOhs[line[2]] && exsNOhs[line[1]] == exsNOhs[line[2]] && exsNOhs[line[0]] != ' ') {
      gameEnd = true;
      if (exsNOhs[line[0]] == 'X'){
        playerXWins++;
      } else{
        playerOWins++;
      }
      i = 7;
    }
  }
  if (numberOfTurnsLeft == 0 && !gameEnd){
    numberOfDraws++;
    gameEnd = true;
  }

  // if there is a winner or a draw, then we want to display the results, reset the variables that are being used for each game, and pass the win/loss/draw ratio to another file
  if (gameEnd){
    gameCount++;
    String messageOne = "";
    if (gameCount == 100){
      messageOne += "Q";
    } else {
    }
    messageOne += "Player X Wins: " + (String)playerXWins + " Player O Wins: " + (String)playerOWins + " Draws: " + (String)numberOfDraws;
    const char * message = messageOne.c_str();

    client.publish("transferSpots", message);
    // modify variables and show the display
    printScoresOnLCD();
    printGameCount();

    // reset the variables to be used for the next game
    for (int i = 0; i < 9; i++){
      exsNOhs[i] = ' ';
      availableTiles[i] = i;
    }
    numberOfTurnsLeft = 9;
    gameEnd = false;

    delay(250);
  }
}

bool i2CAddrTest(uint8_t addr) {
  Wire.begin();
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}

void printScoresOnLCD(){
  lcd.setCursor(0, 0);
  if(playerXWins < 10){
    lcd.print("pX: ");
  } else {
    lcd.print("pX:");
  }
  lcd.print(playerXWins);
  if(playerOWins < 10){
    lcd.print(" pO: ");
  } else {
    lcd.print(" pO:");
  }
  lcd.print(playerOWins);
  if(numberOfDraws < 10){
    lcd.print(" d: ");
  } else {
    lcd.print(" d:");
  }
  lcd.print(numberOfDraws);
}

void printGameCount(){
  lcd.setCursor(0, 1);
  if (gameCount < 10){
    lcd.print("Game Count:  ");
    lcd.print(gameCount);
  } else if (gameCount < 100){
    lcd.print("Game Count: ");
    lcd.print(gameCount);
  } else {
    lcd.print("Game Count:");
    lcd.print(gameCount);
  }
}
