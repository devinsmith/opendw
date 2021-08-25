// From https://github.com/dmsc/emu2/blob/master/src/keyb.c

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_C 0x2e03

static int term_raw = 0;
static int tty_fd = -1;
static int mod_state = 0;

enum mod_keys
{
    MOD_SHIFT = 1,
    MOD_RSHIFT = 2,
    MOD_CTRL = 4,
    MOD_ALT = 8
};

static void set_raw_term(int raw)
{
  static struct termios oldattr; // Initial terminal state
  if (raw == term_raw)
    return;

  term_raw = raw;
  if (term_raw) {
    struct termios newattr;
    tcgetattr(tty_fd, &oldattr);
    newattr = oldattr;
    cfmakeraw(&newattr);
    newattr.c_cc[VMIN] = 0;
    newattr.c_cc[VTIME] = 0;
    tcsetattr(tty_fd, TCSANOW, &newattr);
  } else {
    tcsetattr(tty_fd, TCSANOW, &oldattr);
  }
}

static void exit_keyboard(void)
{
  set_raw_term(0);
  close(tty_fd);
}

static void init_keyboard(void)
{
  if (tty_fd < 0) {
    tty_fd = open("/dev/tty", O_NOCTTY | O_RDONLY);
    if (tty_fd < 0) {
      perror("error at open TTY");
      exit(1);
    }
    atexit(exit_keyboard);
  }
  set_raw_term(1);
}

static int get_scancode(int i)
{
  if (i >= 'a' && i <= 'z')
    i = i - 'a' + 'A';

  switch (i) {
  case 'C': return 0x2E00;
  default: return i & 0xFF00;
  }
}


static int add_scancode(int i)
{
  // Exclude ESC, ENTER, and TAB.
  if (i < 0x20 && i != 0x1B && i != 0x0D && i != 0x09) {
    // CTRL+KEY
    mod_state |= MOD_CTRL;
    int orig = i;
    if (i == 0x1C)  i = '\\';
    else if (i == 0x1D) i = ']';
    else if (i == 0x1E) i = '6';
    else if (i == 0x1F) i = '-';
    else if (i == 0x08) orig = 0x7F;
    else i = i + 0x40; // Why does emu2 have 0x20??

    return orig | get_scancode(i);
  }
  return i;
}

static int read_key(void)
{
  char ch = 0xFF;
  // Reads first key code
  if (read(tty_fd, &ch, 1) == 0)
    return -1; // No data

  if (ch == 0x1B)
    return ch;

  mod_state = 0;
  // Normal key
  if ((ch & 0xFF) < 0x80)
    return add_scancode(ch);

  return 0;
}

int main(int argc, char *argv[])
{

  printf("Press CTRL-C to exit\n");

  init_keyboard();

  int k;

  while ((k = read_key()) != CTRL_C) {
    if (k == -1) {
      sleep(1);
    } else {
      printf("0x%04X\r\n", k);
    }
  }

  return 0;
}
