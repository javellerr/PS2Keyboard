// http://www.quadibloc.com/comp/scan.htm
// http://www.computer-engineering.org/ps2keyboard/scancodes2.html
// http://www-ug.eecg.toronto.edu/msl/nios_devices/datasheets/PS2%20Protocol.htm
// https://www.basic4mcu.com/bbs/board.php?bo_table=gesiyo12&wr_id=140&page=8https://www.basic4mcu.com/bbs/board.php?bo_table=gesiyo12&wr_id=140&page=8

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

uint8_t keyset[1];
uint8_t keymap[] = {
  1, 0xF0, 0x14, 2, 0xF8, 0xF0, 0x14,
  1, 0xF1, 0x12, 2, 0xF9, 0xF0, 0x12,
  1, 0xF2, 0x11, 2, 0xFA, 0xF0, 0x11,
  2, 0xF3, 0xE0, 0x1F, 3, 0xFB, 0xE0, 0xF0, 0x1F,
  2, 0xF4, 0xE0, 0x14, 3, 0xFC, 0xE0, 0xF0, 0x14,
  1, 0xF5, 0x59, 2, 0xFD, 0xF0, 0x59,
  2, 0xF6, 0xE0, 0x11, 3, 0xFE, 0xE0, 0xF0, 0x11,
  2, 0xF7, 0xE0, 0x27, 3, 0xFF, 0xE0, 0xF0, 0x27,

  1, 0x04, 0x1C, 2, 0x00, 0xF0, 0x1C,
  1, 0x06, 0x21, 2, 0x00, 0xF0, 0x21,

  0,
};

uint8_t update_valid_hcode(uint8_t *hbuf, uint8_t size)
{
  // 1. table 검색을 하여
  uint8_t *list = keymap;

  while(*list)
  {
    Serial.print("*");
    if(*list == size && memcmp(list + 2, hbuf, size) == 0)
    {
      Serial.print("\n");
      return *(list + 1);
    }

    list += *list + 2;
  }

  Serial.print("\n");
  return 0x44;
}

void process_and_get_scancode(uint8_t c)
{
  uint8_t hcode;
  static uint8_t hbuf[8];
  static uint8_t hidx = 0;
  static uint8_t mod = 0;
  uint8_t valid;

  hbuf[hidx++] = c;

  // 1. 키를 식별 한다.
  hcode = update_valid_hcode(hbuf, hidx);

  if (hcode == 0x44) return;

  // 2. 메타 키이면, 메타 키 상태를 업데이트
  if (hcode >= 0xF0 && hcode <= 0xFF)
  {
    if(hcode < 0xF8)
    {
      mod |= 1 << (hcode & 0x0F);
    }
    else 
    {
      mod &= ~(1 << ((hcode & 0x0F) - 8));
    }
    hcode = 0;
  }

  // 3. 식별한 키를 전송 한다.
  send_raw_report(mod, hcode);

  hidx = 0;
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
