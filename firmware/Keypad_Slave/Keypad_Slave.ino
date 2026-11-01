#include <ArduinoBLE.h>

namespace {

constexpr char SERVICE_UUID[] = "6c6f6e6f-4d61-6372-6f2d-4c6f6e6f0001";
constexpr char EVENT_UUID[] = "6c6f6e6f-4d61-6372-6f2d-4c6f6e6f0002";
constexpr char DEVICE_NAME[] = "Lono-Keypad";

constexpr uint8_t KEY_PINS[] = {D0, D1, D2, D3};
constexpr uint8_t PAIR_PIN = D4;
constexpr uint8_t LED_PIN = D5;
constexpr uint8_t EVENT_DOWN = 1;
constexpr uint8_t EVENT_UP = 0;
constexpr unsigned long DEBOUNCE_MS = 25;

BLEService eventService(SERVICE_UUID);
BLECharacteristic eventCharacteristic(
    EVENT_UUID, BLERead | BLENotify, 2, true);

struct ButtonState {
  bool stable = HIGH;
  bool reading = HIGH;
  unsigned long changedAt = 0;
};

ButtonState keys[4];
ButtonState pairButton;
BLEDevice central;
bool advertising = false;
bool wasConnected = false;
unsigned long eventLedUntil = 0;

bool updateButton(ButtonState& button, uint8_t pin, bool& pressed) {
  const bool reading = digitalRead(pin);
  if (reading != button.reading) {
    button.reading = reading;
    button.changedAt = millis();
  }

  if (millis() - button.changedAt >= DEBOUNCE_MS &&
      button.stable != button.reading) {
    button.stable = button.reading;
    pressed = button.stable == LOW;
    return true;
  }
  return false;
}

void beginAdvertising() {
  if (advertising) {
    return;
  }
  BLE.advertise();
  advertising = true;
  Serial.println(F("Advertising; press SW8 on the master to connect"));
}

void sendKeyEvent(uint8_t type, uint8_t key) {
  if (!central || !central.connected()) {
    return;
  }
  const uint8_t packet[2] = {type, key};
  eventCharacteristic.writeValue(packet, sizeof(packet));
  eventLedUntil = millis() + 180;
}

void updateStatusLed(bool connected) {
  const unsigned long now = millis();
  if (now < eventLedUntil) {
    digitalWrite(LED_PIN, ((now / 45) % 2) == 0 ? HIGH : LOW);
  } else if (connected) {
    digitalWrite(LED_PIN, HIGH);
  } else if (advertising) {
    digitalWrite(LED_PIN, ((now / 300) % 2) == 0 ? HIGH : LOW);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void fail(const __FlashStringHelper* message) {
  Serial.println(message);
  while (true) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(250);
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < 4; ++i) {
    pinMode(KEY_PINS[i], INPUT_PULLUP);
  }
  pinMode(PAIR_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (!BLE.begin()) {
    fail(F("BLE init failed"));
  }

  BLE.setLocalName(DEVICE_NAME);
  BLE.setAdvertisedService(eventService);
  eventService.addCharacteristic(eventCharacteristic);
  BLE.addService(eventService);
  const uint8_t emptyPacket[2] = {0, 0};
  eventCharacteristic.writeValue(emptyPacket, sizeof(emptyPacket));

  Serial.println(F("Keypad slave ready; press SW5 to advertise"));
}

void loop() {
  BLE.poll();

  BLEDevice newCentral = BLE.central();
  if (newCentral) {
    central = newCentral;
    advertising = false;
  }

  const bool connected = central && central.connected();
  if (connected && !wasConnected) {
    Serial.println(F("Connected to master"));
  } else if (!connected && wasConnected) {
    Serial.println(F("Disconnected from master"));
    central = BLEDevice();
  }
  wasConnected = connected;

  bool pressed = false;
  if (!connected && updateButton(pairButton, PAIR_PIN, pressed) && pressed) {
    beginAdvertising();
  }

  for (uint8_t i = 0; i < 4; ++i) {
    if (updateButton(keys[i], KEY_PINS[i], pressed)) {
      sendKeyEvent(pressed ? EVENT_DOWN : EVENT_UP, i);
    }
  }

  updateStatusLed(connected);
  delay(2);
}
