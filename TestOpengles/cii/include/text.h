#ifndef TEXT_INCLUDE
#define TEXT_INCLUDE

#define T text_t

typedef struct T {
	int         len;
	const char  *str;
} T;

typedef struct text_save_t *text_save_t;

extern  const T text_cset;
extern  const T text_ascii;
extern  const T text_ucase;
extern  const T text_lcase;
extern  const T text_digits;
extern  const T text_null;

extern  T       text_put(const char *str);
extern  char   *text_get(char *str, int size, T text);
extern  T       text_box(const char *str, int len);


extern  int text_pos(T text, int i);

extern  T       text_sub(T text, int i, int j);
extern  T       text_cat(T text1, T text2);
extern  T       text_dup(T text, int n);
extern  T       text_reverse(T text);
extern  T       text_map(T text, const T *from, const T *to);


extern  int     text_cmp(T text1, T text2);


extern  int text_chr(T text, int i, int j, int c);
extern  int text_rchr(T text, int i, int j, int c);
extern  int text_upto(T text, int i, int j, T set);
extern  int text_rupto(T text, int i, int j, T set);
extern  int text_any(T text, int i, T set);
extern  int text_many(T text, int i, int j, T set);
extern  int text_rmany(T text, int i, int j, T set);


extern  int text_find(T text, int i, int j, T str);
extern  int text_rfind(T text, int i, int j, T str);
extern  int text_match(T text, int i, int j, T str);
extern  int text_rmatch(T text, int i, int j, T str);


extern  text_save_t text_save(void);
extern  void        text_restore(text_save_t *save);

#undef T

#endif /*TEXT_INCLUDE*/
