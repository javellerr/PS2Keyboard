// http://www.quadibloc.com/comp/scan.htm
// http://www.computer-engineering.org/ps2keyboard/scancodes2.html
// http://www-ug.eecg.toronto.edu/msl/nios_devices/datasheets/PS2%20Protocol.htm

#include <Arduino.h> // for attachInterrupt, FALLING
#include <SoftwareSerial.h>

#define KEYBOARD_DATA_PIN 3
#define KEYBOARD_IRQ_PIN 2
#define KEYBOARD_IRQ_NUM 0
#define BUFFER_SIZE 45

#define DBG

static volatile uint8_t buffer[BUFFER_SIZE];
static volatile uint8_t head = 0, tail = 0;
SoftwareSerial blue(4, 5); // Rx,Tx

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

void process_and_get_scancode(uint8_t c)
{
  static int8_t flag_release = 0;
  static int8_t flag_ext = 0;

  if (flag_release)
  {
    flag_release = 0;
    send_raw_report(0x00, 0x00);
    return;
  }
  else if (flag_ext)
  {
    flag_ext = 0;
    if (c == 0x1F)
    {
      send_raw_report(0x08, 0x00);
    }
    return;
  }
  else if (c == 0x1C)
  {
    send_raw_report(0x00, 0x04);
    return;
  }
  else if (c == 0x5A)
  {
    send_raw_report(0x00, 0x28);
    return;
  }
  else if (c == 0xE0)
  {
    flag_ext = 1;
    return;
  }
  else if (c == 0xF0)
  {
    flag_release = 1;
    return;
  }
}

#ifdef DBG
static char logbuf[64];
static int logcnt = 0;
#endif

void send_raw_report(uint8_t modifier, uint8_t scancode)
{
  static uint8_t sendbuf[6] =
  {
    0xFD, // start byte
    0x04, // length
    0x01, // descriptor
    0x00, // modifier
    0x00, // 0x00
    0x00 // scan code 1
  };

#ifdef DBG
  sprintf(logbuf, "[%d] send_raw_report : (%02X)\n", logcnt++, scancode);
  Serial.print(logbuf);
#endif

  sendbuf[3] = modifier;
  sendbuf[5] = scancode;

  blue.write(sendbuf, sizeof(sendbuf));
}

void setup()
{
  delay(1000);
  PS2Keyboard_init();
  blue.begin(115200);
  Serial.begin(115200);
}

void loop()
{
  static int logcnt = 0;
  uint8_t c, d;
  if (PS2Keyboard_available()) {
    c = get_scan_code();

#ifdef DBG
    sprintf(logbuf, "[%d] get_scan_code : (%02X)\n", logcnt++, c);
    Serial.print(logbuf);
#endif

    process_and_get_scancode(c);
  }
}
