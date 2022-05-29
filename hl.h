#include <stdint.h>

typedef struct {
	char *id;
	char *uri;
	uint8_t hov;
	uint32_t refs;
} Hyperlink;

void openlink(char *);
