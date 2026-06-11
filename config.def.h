void get_date(char *out);
void get_ram(char *out);
void get_disk(char *out);

typedef struct {
  const char *format;
  void (*func)(char *);
} Block;

static const char *date_format = "%a %b %d | %R";
static const char *ram_format = "%.2f/%.2fGB";
static const char *disk_format = "%.2f/%.2fGB %d%%";

static const Block blocks[] = {
    //  Format string             Function to run
    {"%s",                        get_date},
    {" | RAM: %s",                get_ram},
    {" | /: %s ",                 get_disk},
};
