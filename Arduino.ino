#include <FastLED.h>
#include "DFRobotDFPlayerMini.h"

// Конфигурация матрицы
#define WIDTH 32
#define HEIGHT 8
#define NUM_LEDS WIDTH * HEIGHT
#define DATA_PIN 5

CRGB leds[NUM_LEDS];
int currentBrightness = 50;
int ordersCount = 0;

// DFPlayer
DFRobotDFPlayerMini myDFPlayer;
int currentVolume = -1;

// Массив для отображения цифр (5x3)
const bool digitPatterns[10][5][3] = {
  { // 0
    {1, 1, 1},
    {1, 0, 1},
    {1, 0, 1},
    {1, 0, 1},
    {1, 1, 1}
  },
  { // 1
    {0, 1, 0},
    {1, 1, 0},
    {0, 1, 0},
    {0, 1, 0},
    {1, 1, 1}
  },
  { // 2
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1},
    {1, 0, 0},
    {1, 1, 1}
  },
  { // 3
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1}
  },
  { // 4
    {1, 0, 1},
    {1, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {0, 0, 1}
  },
  { // 5
    {1, 1, 1},
    {1, 0, 0},
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1}
  },
  { // 6
    {1, 1, 1},
    {1, 0, 0},
    {1, 1, 1},
    {1, 0, 1},
    {1, 1, 1}
  },
  { // 7
    {1, 1, 1},
    {0, 0, 1},
    {0, 0, 1},
    {0, 0, 1},
    {0, 0, 1}
  },
  { // 8
    {1, 1, 1},
    {1, 0, 1},
    {1, 1, 1},
    {1, 0, 1},
    {1, 1, 1}
  },
  { // 9
    {1, 1, 1},
    {1, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1}
  }
};

// Массив для отображения букв (5x3)
const bool textPatterns[8][5][3] = {
  { // З
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1}
  },
  { // А
    {1, 1, 1},
    {1, 0, 1},
    {1, 1, 1},
    {1, 0, 1},
    {1, 0, 1}
  },
  { // К
    {1, 0, 1},
    {1, 0, 1},
    {1, 1, 0},
    {1, 0, 1},
    {1, 0, 1}
  },
  { // А
    {1, 1, 1},
    {1, 0, 1},
    {1, 1, 1},
    {1, 0, 1},
    {1, 0, 1}
  },
  { // З
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1},
    {0, 0, 1},
    {1, 1, 1}
  },
  { // :
    {0, 0, 0},
    {0, 1, 0},
    {0, 0, 0},
    {0, 1, 0},
    {0, 0, 0}
  }
};

void printDetail(uint8_t type, int value);

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(currentBrightness);
  displayOrderCount(0); // Показываем "Заказов: 0" при старте
  FastLED.show();

  Serial.println("Arduino Mega готов к работе");

  // Инициализация DFPlayer
  Serial1.begin(9600);
  if (!myDFPlayer.begin(Serial1, true, true)) {
    Serial.println(F("Ошибка запуска DFPlayer"));
    while (true) delay(1);
  }

  Serial.println(F("DFPlayer Mini онлайн"));
  myDFPlayer.volume(15);
}

void loop() {
  while (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.length() > 0) {
      Serial.print("Получена команда: ");
      Serial.println(command);

      if (command.startsWith("BRIGHTNESS:")) {
        currentBrightness = command.substring(11).toInt();
        FastLED.setBrightness(currentBrightness);
        FastLED.show();
      }
      else if (command.startsWith("TRACK:")) {
        int trackNumber = command.substring(6).toInt() + 1;
        if (trackNumber >= 1 && trackNumber <= 999) {
          Serial.print("Проигрываю трек: ");
          Serial.println(trackNumber);
          myDFPlayer.play(trackNumber);
          
          ordersCount = trackNumber - 1;
          if (ordersCount < 0) ordersCount = 0;
          if (ordersCount > 9) ordersCount = 9;
          
          displayOrderCount(ordersCount);
        }
      }
    }
  }

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());
  }

  int analogValue = analogRead(A0);
  int volume = map(analogValue, 0, 1023, 0, 30);

  if (volume != currentVolume) {
    currentVolume = volume;
    myDFPlayer.volume(volume);
    Serial.print("Громкость установлена: ");
    Serial.println(volume);
  }

  delay(10);
}

// Функция для отображения текста "Заказов:" и числа
void displayOrderCount(int number) {
  if (number < 0 || number > 9) return;
  
  FastLED.clear();
  
  // Отображаем "Заказов:" (первые 8 символов)
  for (int charIndex = 0; charIndex < 8; charIndex++) {
    int startX = charIndex * 4;
    for (int y = 0; y < 5; y++) {
      for (int x = 0; x < 3; x++) {
        if (textPatterns[charIndex][y][x]) {
          int ledIndex = getIndex(startX + x, y);
          if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
            leds[ledIndex] = CRGB::Blue; // Синий цвет текста
          }
        }
      }
    }
  }
  
  // Отображаем число (смещаем на 8*4=32 пикселя, но у нас WIDTH=32)
  // Поэтому будем отображать число правее текста с небольшим отступом
  int numberStartX = 24; // Начинаем с 24-го пикселя (8 букв * 3 пикселя + пробелы)
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 3; x++) {
      if (digitPatterns[number][y][x]) {
        int ledIndex = getIndex(numberStartX + x, y);
        if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
          leds[ledIndex] = CRGB::Green; // Зеленый цвет цифры
        }
      }
    }
  }
  
  FastLED.show();
}

int getIndex(int x, int y) {
  return (x % 2 == 0) ? (x * HEIGHT + y) : (x * HEIGHT + (HEIGHT - 1 - y));
}

void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
