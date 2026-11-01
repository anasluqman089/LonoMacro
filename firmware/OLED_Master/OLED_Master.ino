#include <ArduinoBLE.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keyboard.h>
#include <Wire.h>

namespace {

constexpr char SERVICE_UUID[] = "6c6f6e6f-4d61-6372-6f2d-4c6f6e6f0001";
constexpr char EVENT_UUID[] = "6c6f6e6f-4d61-6372-6f2d-4c6f6e6f0002";
constexpr char KEYPAD_NAME[] = "Lono-Keypad";
constexpr char ROTARY_NAME[] = "Lono-Rotary";

constexpr uint8_t PAIR_PIN = D0;
constexpr uint8_t LED_PIN = D1;
constexpr uint8_t EVENT_DOWN = 1;
constexpr uint8_t EVENT_UP = 0;
constexpr uint8_t KEYPAD_KEYS = 4;
constexpr uint8_t ROTARY_KEYS = 3;
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

struct ButtonState {
  bool stable = HIGH;
  bool reading = HIGH;
  unsigned long changedAt = 0;
};

struct SlaveLink {
  BLEDevice device;
  BLECharacteristic events;
  bool ready = false;
  bool held[KEYPAD_KEYS] = {};
};

ButtonState pairButton;
SlaveLink links[2];
bool scanning = false;
bool displayReady = false;
unsigned long eventLedUntil = 0;
unsigned long lastDisplayUpdate = 0;
char lastAction[28] = "Press SW8 to connect";

bool wasPressed(ButtonState& button, uint8_t pin) {
  const bool reading = digitalRead(pin);
  if (reading != button.reading) {
    button.reading = reading;
    button.changedAt = millis();
  }

  if (millis() - button.changedAt >= 25 && button.stable != button.reading) {
    button.stable = button.reading;
    return button.stable == LOW;
  }
  return false;
}

int linkIndexForName(const String& name) {
  if (name == KEYPAD_NAME) {
    return 0;
  }
  if (name == ROTARY_NAME) {
    return 1;
  }
  return -1;
}

uint8_t keyCountForLink(uint8_t index) {
  return index == 0 ? KEYPAD_KEYS : ROTARY_KEYS;
}

uint8_t usbKeyForEvent(uint8_t index, uint8_t code) {
  if (index == 0 && code < KEYPAD_KEYS) {
    return static_cast<uint8_t>('1' + code);
  }
  if (index == 1) {
    if (code == 0) {
      return KEY_UP_ARROW;
    }
    if (code == 1) {
      return KEY_DOWN_ARROW;
    }
    if (code == 2) {
      return KEY_RETURN;
    }
  }
  return 0;
}

void setLastAction(uint8_t index, uint8_t code, uint8_t type) {
  if (index == 0) {
    snprintf(lastAction, sizeof(lastAction), "Key %u %s",
             static_cast<unsigned>(code + 1),
             type == EVENT_DOWN ? "pressed" : "released");
  } else {
    const char* action = code == 0 ? "Up" : (code == 1 ? "Down" : "Enter");
    snprintf(lastAction, sizeof(lastAction), "%s %s", action,
             type == EVENT_DOWN ? "pressed" : "released");
  }
}

void releaseHeldKeys(uint8_t index) {
  const uint8_t keyCount = keyCountForLink(index);
  for (uint8_t code = 0; code < keyCount; ++code) {
    if (links[index].held[code]) {
      const uint8_t key = usbKeyForEvent(index, code);
      if (key != 0) {
        Keyboard.release(key);
      }
      links[index].held[code] = false;
    }
  }
}

void disconnectLink(uint8_t index) {
  releaseHeldKeys(index);
  links[index].device = BLEDevice();
  links[index].events = BLECharacteristic();
  links[index].ready = false;
}

uint8_t connectedCount() {
  uint8_t count = 0;
  for (uint8_t i = 0; i < 2; ++i) {
    if (links[i].ready && links[i].device.connected()) {
      ++count;
    }
  }
  return count;
}

void startScan() {
  BLE.scanForUuid(SERVICE_UUID);
  scanning = true;
  snprintf(lastAction, sizeof(lastAction), "Searching for slaves");
}

void stopScan() {
  BLE.stopScan();
  scanning = false;
}

void drawDisplay() {
  if (!displayReady) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("LonoMacro Master"));
  display.println();
  display.print(F("Keypad: "));
  display.println(links[0].ready && links[0].device.connected()
                      ? F("connected")
                      : F("disconnected"));
  display.print(F("Rotary: "));
  display.println(links[1].ready && links[1].device.connected()
                      ? F("connected")
                      : F("disconnected"));
  display.println();
  display.println(lastAction);
  display.display();
}

void handleEvent(uint8_t index, const uint8_t packet[2]) {
  const uint8_t type = packet[0];
  const uint8_t code = packet[1];
  const uint8_t keyCount = keyCountForLink(index);
  if ((type != EVENT_DOWN && type != EVENT_UP) || code >= keyCount) {
    Serial.println(F("Ignored invalid BLE event"));
    return;
  }

  const uint8_t key = usbKeyForEvent(index, code);
  if (key == 0) {
    Serial.println(F("Ignored unmapped BLE event"));
    return;
  }

  if (type == EVENT_DOWN && !links[index].held[code]) {
    Keyboard.press(key);
    links[index].held[code] = true;
  } else if (type == EVENT_UP && links[index].held[code]) {
    Keyboard.release(key);
    links[index].held[code] = false;
  }

  setLastAction(index, code, type);
  eventLedUntil = millis() + 180;
  drawDisplay();
}

void connectDiscoveredSlave(BLEDevice peripheral) {
  if (!peripheral.hasLocalName()) {
    return;
  }

  const int index = linkIndexForName(peripheral.localName());
  if (index < 0 || (links[index].ready && links[index].device.connected())) {
    return;
  }

  stopScan();
  if (!peripheral.connect()) {
    Serial.println(F("BLE connection failed"));
    startScan();
    return;
  }
  if (!peripheral.discoverAttributes() ||
      !peripheral.hasService(SERVICE_UUID)) {
    Serial.println(F("Slave BLE service not found"));
    peripheral.disconnect();
    startScan();
    return;
  }

  BLECharacteristic events = peripheral.characteristic(EVENT_UUID);
  if (!events || !events.canSubscribe() || !events.subscribe()) {
    Serial.println(F("Could not subscribe to slave events"));
    peripheral.disconnect();
    startScan();
    return;
  }

  links[index].device = peripheral;
  links[index].events = events;
  links[index].ready = true;
  snprintf(lastAction, sizeof(lastAction), "%s connected",
           index == 0 ? "Keypad" : "Rotary");
  Serial.println(lastAction);
  drawDisplay();

  if (connectedCount() < 2) {
    startScan();
  }
}

void updateConnections() {
  for (uint8_t i = 0; i < 2; ++i) {
    if (links[i].ready && !links[i].device.connected()) {
      disconnectLink(i);
      snprintf(lastAction, sizeof(lastAction), "%s disconnected",
               i == 0 ? "Keypad" : "Rotary");
      Serial.println(lastAction);
    }
  }

  if (scanning) {
    BLEDevice peripheral = BLE.available();
    if (peripheral) {
      connectDiscoveredSlave(peripheral);
    }
  }

  for (uint8_t i = 0; i < 2; ++i) {
    if (!links[i].ready || !links[i].device.connected() ||
        !links[i].events.valueUpdated()) {
      continue;
    }

    uint8_t packet[2] = {};
    if (links[i].events.readValue(packet, sizeof(packet)) ==
        static_cast<int>(sizeof(packet))) {
      handleEvent(i, packet);
    } else {
      Serial.println(F("Could not read BLE event"));
    }
  }
}

void updateStatusLed() {
  const unsigned long now = millis();
  const uint8_t connected = connectedCount();
  if (now < eventLedUntil) {
    digitalWrite(LED_PIN, ((now / 45) % 2) == 0 ? HIGH : LOW);
  } else if (connected == 2) {
    digitalWrite(LED_PIN, HIGH);
  } else if (scanning) {
    digitalWrite(LED_PIN, ((now / 300) % 2) == 0 ? HIGH : LOW);
  } else if (connected == 1) {
    digitalWrite(LED_PIN, ((now / 700) % 2) == 0 ? HIGH : LOW);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void showFatalError(const __FlashStringHelper* message) {
  Serial.println(message);
  if (displayReady) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("LonoMacro error"));
    display.println(message);
    display.display();
  }
  while (true) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(250);
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(PAIR_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin();
  displayReady = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!displayReady) {
    showFatalError(F("SSD1306 init failed"));
  }

  if (!BLE.begin()) {
    showFatalError(F("BLE init failed"));
  }

  Keyboard.begin();
  drawDisplay();
  Serial.println(F("Master ready; press SW8 to scan"));
}

void loop() {
  BLE.poll();

  if (wasPressed(pairButton, PAIR_PIN)) {
    startScan();
  }

  updateConnections();
  updateStatusLed();

  if (millis() - lastDisplayUpdate >= 250) {
    lastDisplayUpdate = millis();
    drawDisplay();
  }
}
