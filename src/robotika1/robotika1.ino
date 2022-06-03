#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <IRremote.h>

//Infra
int RECV_PIN = 7;
IRrecv irrecv(RECV_PIN);
decode_results results;
//INFRA END

//NFC
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;      


#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
//NFC END
//DISPLAY
const int trigPin = 5;
const int echoPin = 4;
//DISPLAY END
String loginKey = "47 2B 37 53";
//String loginKey="C3 8A 1B 24";
bool infraon = false;
bool gotcard = false;
bool test = false;
String content = "";
bool edit = false;
String editable = loginKey;


float duration, distance;
int roundCount = 0;
int editedCharNumber = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  irrecv.enableIRIn();//infra
  SPI.begin();
  mfrc522.PCD_Init();//initialize the nfc reader
  lcd.init();// initialize the lcd
  lcd.backlight();
  lcdFirstRow("Nobody is here");


}


void loop()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * .0343) / 2;
  if (roundCount == 100) //600
  {
    roundCount = 0;
    infraon = false;
    gotcard = false;
    Serial.println((String)infraon);
  }
  if (infraon && !edit)
  {
    infra();
    delay(50);
    roundCount++;
    if (roundCount != 100)
    {
      Serial.println(roundCount);
    }
  }
  if (infraon && edit)
  {
    Serial.println("editelek");
    openMenu();
  }
  if (distance < 40 && !infraon)
  {
    lcdFirstRow("Wait for card");
    waitingForCard();
  }

  if (distance > 40 && !infraon)
  {
    lcdFirstRow("Wait for user");
    Serial.print("Distance: ");
    Serial.println(distance);
    lcdSecondRow((String)distance);
  }
  delay(30);


}

void lcdFirstRow(String toWrite)
{
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(toWrite);
  lcd.setCursor(0, 0);
}
void lcdSecondRow(String toWrite)
{
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(toWrite);
  lcd.setCursor(0, 1);
}



void waitingForCard()
{
  for (int i = 0; i < 150; i++)
  {
    if (i % 10 == 0)
    {
      lcdSecondRow((String)(15 - i / 10) + " sec left");
    }
    if (nfcCheck())
    {
      Serial.println("No card");
    }
    else
    {
      Serial.println("Yes card");
      break;
    }
    delay(100);
  }
}

bool nfcCheck()
{
  //nfc check
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return true;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return true;
  }
  Serial.println("Most olvasom be");
  readCard();
  return false;

}

void readCard()
{
  Serial.print("UID tag :");
  content = "";
  byte letter;
  Serial.println(mfrc522.uid.size);
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i],HEX);
    Serial.println("**************************");
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.println("----------------------------");
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println("Content"+content);
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if ((content.substring(1) == loginKey)) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    lcdFirstRow("Authorized");
    lcdSecondRow("");
    editable = loginKey;
    Serial.println();
    delay(2000);
    infraon = true;
    roundCount = 1;
    gotcard = true;
    Serial.println("doorOpen on");
    doorOpen();
  }

  else   {

    Serial.println("Access denied");
    for (int i = 5; i > 0; i--)
    {
      String temp = (String)i;
      lcdFirstRow("Access denied " + temp);
      delay(1000);
    }
    lcdFirstRow("Wait for card");
  }
}

void doorOpen()
{
  lcdFirstRow("Door open");
  lcdSecondRow("Press * to edit");
}


void infra()
{
  if (irrecv.decode(&results)) {
    switch (results.value)
    {
      case 0xFF42BD://1 Button
        Serial.print("* Pressed");
        edit = true;
        lcdFirstRow("# save * cancel");
        break;
      default:
        Serial.print("Undefined code received: 0x");
        Serial.println(results.value, HEX);

        break;
    }
    irrecv.resume();
  }
}

void openMenu()
{
  lcdSecondRow(editable);
 
  delay(200);
  lcd.setCursor(editedCharNumber, 1);
  lcd.print(" ");
  lcd.setCursor(editedCharNumber, 1);
  editable[editedCharNumber];


  Serial.println(editable[0]);
  if (irrecv.decode(&results)) {
    switch (results.value)
    {
      case 0xFFC23D://Jobbra
        if (editedCharNumber == 1 || editedCharNumber == 4 || editedCharNumber == 7)
        {
          editedCharNumber += 2;
        }
        else if (editedCharNumber == 10)
        {
          editedCharNumber = 0;
        }
        else
          editedCharNumber++;
        Serial.print("right Pressed");

        break;
      case 0xFF22DD://Balra
        Serial.print("left Pressed");
        if (editedCharNumber == 3 || editedCharNumber == 6 || editedCharNumber == 9)
        {
          editedCharNumber -= 2;
        }
        else if (editedCharNumber == 0)
        {
          editedCharNumber = 10;
        }
        else
          editedCharNumber--;

        break;

      case 0xFF629D://Fel
        Serial.print("Up Pressed");
        //editedCharNumber
        //editable
        editable[editedCharNumber] = hexaModify(editable[editedCharNumber], true);
        break;

      case 0xFFA857://Le
        Serial.print("Down Pressed");
        editable[editedCharNumber] = hexaModify(editable[editedCharNumber], false);
        break;

      case 0xFF42BD://*
        Serial.print("Cancel Pressed");
        edit = false;
        infraon = false;
        gotcard = false;
        return;
        break;
      case 0xFF52AD://#
        Serial.print("Save Pressed");
        
        
        int blockNum = 0;  
        Serial.println(editable);
        writeWithNFC(editable);
        
        
        edit = false;
        infraon = false;
        gotcard = false;
        //majd itt még mentek
        return;

        break;
      default:
        Serial.print("Undefined code received: 0x");
        Serial.println(results.value, HEX);

        break;
    }
    irrecv.resume();
  }

}

char hexaModify(char editedChar, bool positive)
{
  switch (editedChar)
  {
    case '0':
      if (positive)
        return '1';
      return 'F';
      break;
    case '1':
      if (positive)
        return '2';
      return '0';
      break;
    case '2':
      if (positive)
        return '3';
      return '1';
      break;
    case '3':
      if (positive)
        return '4';
      return '2';
      break;
    case '4':
      if (positive)
        return '5';
      return '3';
      break;
    case '5':
      if (positive)
        return '6';
      return '4';
      break;
    case '6':
      if (positive)
        return '7';
      return '5';
      break;
    case '7':
      if (positive)
        return '8';
      return '6';
      break;
    case '8':
      if (positive)
        return '9';
      return '7';
      break;
    case '9':
      if (positive)
        return 'A';
      return '8';
      break;
    case 'A':
      if (positive)
        return 'B';
      return '9';
      break;
    case 'B':
      if (positive)
        return 'C';
      return 'A';
      break;
    case 'C':
      if (positive)
        return 'D';
      return 'B';
      break;
    case 'D':
      if (positive)
        return 'E';
      return 'C';
      break;
    case 'E':
      if (positive)
        return 'F';
      return 'D';
      break;
    case 'F':
      if (positive)
        return '0';
      return 'E';
      break;
  }
}

void writeWithNFC(String toWrite)
{
  byte blockData[4];
  
  Serial.println("\n nEZ KELL NEKÜNK: ");
  
  for(int i=0,j=0;i<4;i++,j+=3)
  {
    //Serial.println(j);
    byte byte1 = (byte)intFromChar(toWrite[j],toWrite[j+1]);
    Serial.println(byte1,HEX);
    blockData[i] = byte1;
  }

  if(mfrc522.MIFARE_SetUid(blockData,(byte)4,true))
  {
     lcdFirstRow("UID changed to");
     lcdSecondRow(editable+" :)");
     delay(6000);
     loginKey = editable;
  }
  else
  {
    lcdFirstRow("Cant change uid");
    lcdSecondRow("of this card :(");
    delay(6000);
  }
  
 
  
}


int intFromChar (char l, char h)
{
  int toRet= 0;
  switch(h)
  {
    case '0': toRet+=0; break;
    case '1': toRet+=1; break;
    case '2': toRet+=2; break;
    case '3': toRet+=3; break;
    case '4': toRet+=4; break;
    case '5': toRet+=5; break;
    case '6': toRet+=6; break;
    case '7': toRet+=7; break;
    case '8': toRet+=8; break;
    case '9': toRet+=9; break;
    case 'A': toRet+=10; break;
    case 'B': toRet+=11; break;
    case 'C': toRet+=12; break;
    case 'D': toRet+=13; break;
    case 'E': toRet+=14; break;
    case 'F': toRet+=15; break;
  }


   switch(l)
  {
    case '0': toRet+=0; break;
    case '1': toRet+=16; break;
    case '2': toRet+=32; break;
    case '3': toRet+=48; break;
    case '4': toRet+=64; break;
    case '5': toRet+=16*5; break;
    case '6': toRet+=16*6; break;
    case '7': toRet+=16*7; break;
    case '8': toRet+=16*8; break;
    case '9': toRet+=16*9; break;
    case 'A': toRet+=16*10; break;
    case 'B': toRet+=16*11; break;
    case 'C': toRet+=16*12; break;
    case 'D': toRet+=16*13; break;
    case 'E': toRet+=16*14; break;
    case 'F': toRet+=16*15; break;
  }


   return toRet;
}
