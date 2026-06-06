void get_date(char *out);

typedef struct {
  const char *format;
  void (*func)(char *);
} Block;

static const char *date_format = "%a %b %d | %R";

static const Block blocks[] = {
    // note that you need to familiriaze yourself with the returned data type
    //  Format string             Function to run
    {"%s",                        get_date},
};
