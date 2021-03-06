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

//#define DBG

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

uint8_t keymap[256] = {
  // 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  0x00, 0x42, 0x00, 0x3E, 0x3C, 0x3A, 0x3B, 0x45, 0x00, 0x43, 0x41, 0x3F, 0x3D, 0x2B, 0x35, 0x00, // 0x00 ~ 0x0F
  0x00, 0xF2, 0xF1, 0x00, 0xF0, 0x14, 0x1E, 0x00, 0x00, 0x00, 0x1D, 0x16, 0x04, 0x1A, 0x1F, 0x00, // 0x10 ~ 0x1F
  0x00, 0x06, 0x1B, 0x07, 0x08, 0x21, 0x20, 0x00, 0x00, 0x2C, 0x19, 0x09, 0x17, 0x15, 0x22, 0x00, // 0x20 ~ 0x2F
  0x00, 0x11, 0x05, 0x0B, 0x0A, 0x1C, 0x23, 0x00, 0x00, 0x00, 0x10, 0x0D, 0x18, 0x24, 0x25, 0x00, // 0x30 ~ 0x3F
  0x00, 0x36, 0x0E, 0x0C, 0x12, 0x27, 0x26, 0x00, 0x00, 0x37, 0x38, 0x0F, 0x33, 0x13, 0x2D, 0x00, // 0x40 ~ 0x4F
  0x00, 0x00, 0x34, 0x00, 0x2F, 0x2E, 0x00, 0x00, 0x39, 0xF5, 0x28, 0x30, 0x00, 0x31, 0x00, 0x00, // 0x50 ~ 0x5F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x60 ~ 0x6F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, // 0x70 ~ 0x7F
  0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 ~ 0x8F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 ~ 0x9F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xA0 ~ 0xAF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB0 ~ 0xBF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xC0 ~ 0xCF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xD0 ~ 0xDF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xE0 ~ 0xEF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xF0 ~ 0xFF
};

uint8_t keymap_ext0[256] = {
  // 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x00 ~ 0x0F
  0x00, 0xF6, 0x00, 0x00, 0xF4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF3, // 0x10 ~ 0x1F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, // 0x20 ~ 0x2F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x30 ~ 0x3F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x40 ~ 0x4F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x50 ~ 0x5F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x00, 0x50, 0x4A, 0x00, 0x00, 0x00, // 0x60 ~ 0x6F
  0x49, 0x4C, 0x51, 0x00, 0x4F, 0x52, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x46, 0x4B, 0x00, 0x00, // 0x70 ~ 0x7F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 ~ 0x8F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 ~ 0x9F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xA0 ~ 0xAF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB0 ~ 0xBF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xC0 ~ 0xCF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xD0 ~ 0xDF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xE0 ~ 0xEF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xF0 ~ 0xFF
};


uint8_t keymap_ext1[256] = {
  // 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x00 ~ 0x0F
  0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x10 ~ 0x1F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x20 ~ 0x2F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x30 ~ 0x3F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x40 ~ 0x4F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x50 ~ 0x5F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x60 ~ 0x6F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x70 ~ 0x7F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 ~ 0x8F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 ~ 0x9F
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xA0 ~ 0xAF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB0 ~ 0xBF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xC0 ~ 0xCF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xD0 ~ 0xDF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xE0 ~ 0xEF
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xF0 ~ 0xFF
};

void gen_hcode(uint8_t scode, uint8_t *hcode, uint8_t *keyst)
{
  static uint8_t status_ext0 = 0;
  static uint8_t status_ext1 = 0;
  static uint8_t status_key = 0;

  if (scode == 0xE0)
    status_ext0 = 1;
  else if (scode == 0xE1)
    status_ext1 = 1;
  else if (scode == 0xF0)
    status_key = 1;
  else
  {
    if (status_ext0)
      *hcode = keymap_ext0[scode];
    else if (status_ext1)
      *hcode = keymap_ext1[scode];
    else
      *hcode = keymap[scode];

    *keyst = status_key;

    status_ext0 = 0;
    status_ext1 = 0;
    status_key = 0;
  }
}

SoftwareSerial blue(4, 5); // Rx,Tx

void send_hid_code(uint8_t hcode, uint8_t keyst)
{
  static uint8_t sendbuf[12] =
  {
    0xFD, // start byte
    0x09, // length
    0x01, // descriptor
    0x00, // modifier
    0x00, // 0x00
    0x00, // scan code 0
    0x00, // scan code 1
    0x00, // scan code 2
    0x00, // scan code 3
    0x00, // scan code 4
    0x00, // scan code 5
    0x00, // padding
  };
  static uint8_t *cbuf = &sendbuf[5];
  static uint8_t *mod = &sendbuf[3];
  static int cidx = 0;
  int i;

  if (keyst == 0) // pressed
  {
    if ((hcode & 0xF0) == 0xF0) // modifier
    {
      mod[0] |= 1 << (hcode & 0x0F);
      send_raw_report(sendbuf);
    }
    else
    {
      if (cidx > 5)
      {
        return;
      }
      for (i = 0; i < cidx; i++)
      {
        if (cbuf[i] == hcode)
        {
          return;
        }
      }
      cbuf[cidx++] = hcode;
      send_raw_report(sendbuf);
    }
  }
  else // released
  {
    if ((hcode & 0xF0) == 0xF0) // modifier
    {
      mod[0] &= ~(1 << (hcode & 0x0F));
      send_raw_report(sendbuf);
    }
    else
    {
      for (i = 0; i < cidx; i++)
      {
        if (cbuf[i] == hcode)
        {
          for (; i < cidx; i++)
            cbuf[i] = cbuf[i + 1];
          cidx--;
          send_raw_report(sendbuf);
        }
      }
    }
  }
}

void send_raw_report(uint8_t *buf)
{
  blue.write(buf, 11);

#ifdef DBG
  static char logbuf[128];
  sprintf(logbuf, "> %02X %02X %02X %02X %02X | %02X %02X %02X %02X %02X %02X\n",
          buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10]);
  Serial.print(logbuf);
#endif
}

void setup()
{
  delay(1000);
  PS2Keyboard_init();
  blue.begin(115200);
  Serial.begin(115200);

  Serial.print("start ps22hid\n");
}

void loop()
{
  uint8_t scode = 0;
  uint8_t hcode = 0;
  uint8_t keyst = 0;
  static uint8_t hcode_prev = 0;
  static uint8_t keyst_prev = 0;

  if (PS2Keyboard_available()) {
    scode = get_scan_code();

#ifdef DBG
    static int logcnt = 0;
    static char logbuf[128];
    sprintf(logbuf, "[%d] scode : %02X\n", logcnt++, scode);
    Serial.print(logbuf);
#endif

    gen_hcode(scode, &hcode, &keyst);

    if (hcode &&
        (hcode != hcode_prev || keyst != keyst_prev))
    {
      send_hid_code(hcode, keyst);

      hcode_prev = hcode;
      keyst_prev = keyst;
    }
  }
}
