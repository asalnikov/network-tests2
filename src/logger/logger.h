#ifdef DEBUG
#define log(...) { fprintf(stderr, "%s:%d :", __FILE__, __LINE__); fprintf(stderr, __VA_ARGS__); }
#else
#define log(...)
#endif
