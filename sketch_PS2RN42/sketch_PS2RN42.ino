// http://www.quadibloc.com/comp/scan.htm
// http://www.computer-engineering.org/ps2keyboard/scancodes2.html
// http://www-ug.eecg.toronto.edu/msl/nios_devices/datasheets/PS2%20Protocol.htm

#include <Arduino.h> // for attachInterrupt, FALLING
#include <SoftwareSerial.h>

#define KEYBOARD_DATA_PIN 3
#define KEYBOARD_IRQ_PIN 2
#define KEYBOARD_IRQ_NUM 0
#define BUFFER_SIZE 45

static volatile uint8_t buffer[BUFFER_SIZE];
static volatile uint8_t head = 0, tail = 0;

void ps2interrupt(void)
{
  static uint8_t bitcount = 0;
  static uint8_t incoming = 0;
  static uint32_t prev_ms = 0;
  uint32_t now_ms;
  uint8_t n, val;

  val = digitalRead(KEYBOARD_DATA_PIN);
  now_ms = millis();
  if (now_ms - prev_ms > 250) {
    bitcount = 0;
    incoming = 0;
  }
  prev_ms = now_ms;
  n = bitcount - 1;
  if (n <= 7) {
    incoming |= (val << n);
  }
  bitcount++;
  if (bitcount == 11) {
    uint8_t i = head + 1;
    if (i >= BUFFER_SIZE) i = 0;
    if (i != tail) { 
      buffer[i] = incoming;
      head = i;
    }
    bitcount = 0;
    incoming = 0;
  }
}

void PS2Keyboard_init() {
  pinMode(KEYBOARD_IRQ_PIN, INPUT_PULLUP);
  pinMode(KEYBOARD_DATA_PIN, INPUT_PULLUP);
  attachInterrupt(KEYBOARD_IRQ_NUM, ps2interrupt, FALLING);
}

static inline uint8_t get_scan_code(void)
{
  uint8_t c, i;

  i = tail;
  if (i == head) return 0;
  i++;
  if (i >= BUFFER_SIZE) i = 0;
  c = buffer[i];
  tail = i;
  return c;
}

bool PS2Keyboard_available() {
  return (tail != head);
}

SoftwareSerial blue(4, 5); // Rx,Tx

void setup()
{
  delay(1000);
  PS2Keyboard_init();
  blue.begin(115200);
  Serial.begin(115200);
}

uint8_t sendbuf[6] = {0xFD, 0x04, 0x01, 0x00, 0x00, 0x04};
//uint8_t sendbuf[6] = {0xFD, 0x04, 0x01, 0x01, 0x00, 0x00};

uint8_t update_scan_code(uint8_t c)
{
  static int released = 0;
  uint8_t zero = 0;

  if(released)
  {
    released = 0;
    blue.write(0xFD);
    blue.write(0x09);
    blue.write(0x01);
    blue.write(zero);
    blue.write(zero);
    blue.write(zero);

    blue.write(zero);
    blue.write(zero);
    blue.write(zero);
    blue.write(zero);
    blue.write(zero);
    return 0xFF;
  }

  if(c == 0xF0)
  {
    released = 1;
    return 0xFF;
  }
  else if(c == 0x1C)
  {
    blue.write(sendbuf, 6);
    //blue.write(0x61);
    return 0xFF;
  }
  else if(c == 0x1B)
  {
    blue.write(0xE1);
    return 0xFF;
  }
  
  return 0xFF;
}

void send_raw_data(uint8_t d)
{
  static int logcnt = 0;
  Serial.print("[");
  Serial.print(logcnt++);
  Serial.print("]");
  Serial.print(" Send:");
  Serial.println(d, HEX);
  blue.write(d);
}

void loop()
{
  static int logcnt = 0;
  uint8_t c, d;
  if (PS2Keyboard_available()) {
    c = get_scan_code();
    Serial.print("[");
    Serial.print(logcnt++);
    Serial.print("]");
    Serial.print(" Raw:");
    Serial.println(c, HEX);
    
    if((d = update_scan_code(c)) != 0xFF)
      send_raw_data(d);
  }
}
