#include <ArduinoBLE.h>

namespace {

constexpr char SERVICE_UUID[] = "6c6f6e6f-4d61-6372-6f2d-4c6f6e6f0001";
constexpr char EVENT_UUID[] = "6c6f6e6f-4d61-6372-6f2d-4c6f6e6f0002";
constexpr char DEVICE_NAME[] = "Lono-Rotary";

constexpr uint8_t LED_PIN = D1;
constexpr uint8_t ENCODER_A_PIN = D2;
constexpr uint8_t ENCODER_B_PIN = D3;
constexpr uint8_t ENCODER_BUTTON_PIN = D4;
constexpr uint8_t PAIR_PIN = D5;
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

ButtonState encoderButton;
ButtonState pairButton;
BLEDevice central;
bool advertising = false;
bool wasConnected = false;
unsigned long eventLedUntil = 0;
uint8_t previousEncoderState = 0;
int8_t encoderAccumulator = 0;

constexpr int8_t ENCODER_TRANSITIONS[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0};

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

void sendEvent(uint8_t type, uint8_t code) {
  if (!central || !central.connected()) {
    return;
  }
  const uint8_t packet[2] = {type, code};
  eventCharacteristic.writeValue(packet, sizeof(packet));
  eventLedUntil = millis() + 180;
}

void sendTap(uint8_t code) {
  sendEvent(EVENT_DOWN, code);
  sendEvent(EVENT_UP, code);
}

void updateEncoder() {
  const uint8_t current =
      static_cast<uint8_t>((digitalRead(ENCODER_A_PIN) << 1) |
                           digitalRead(ENCODER_B_PIN));
  const uint8_t transition =
      static_cast<uint8_t>((previousEncoderState << 2) | current);
  encoderAccumulator += ENCODER_TRANSITIONS[transition];
  previousEncoderState = current;

  if (encoderAccumulator >= 4) {
    sendTap(0);
    encoderAccumulator = 0;
  } else if (encoderAccumulator <= -4) {
    sendTap(1);
    encoderAccumulator = 0;
  }
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
  pinMode(LED_PIN, OUTPUT);
  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PAIR_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);
  previousEncoderState =
      static_cast<uint8_t>((digitalRead(ENCODER_A_PIN) << 1) |
                           digitalRead(ENCODER_B_PIN));

  if (!BLE.begin()) {
    fail(F("BLE init failed"));
  }

  BLE.setLocalName(DEVICE_NAME);
  BLE.setAdvertisedService(eventService);
  eventService.addCharacteristic(eventCharacteristic);
  BLE.addService(eventService);
  const uint8_t emptyPacket[2] = {0, 0};
  eventCharacteristic.writeValue(emptyPacket, sizeof(emptyPacket));

  Serial.println(F("Rotary slave ready; press SW7 to advertise"));
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

  updateEncoder();
  if (updateButton(encoderButton, ENCODER_BUTTON_PIN, pressed)) {
    sendEvent(pressed ? EVENT_DOWN : EVENT_UP, 2);
  }

  updateStatusLed(connected);
  delay(2);
}
