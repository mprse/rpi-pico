#ifndef SRC_PAD_H_
#define SRC_PAD_H_

#include "pico/stdlib.h"

typedef void (*pad_event_handler_t)(uint16_t);

void pad_init(pad_event_handler_t event_handler);

#endif /* SRC_PAD_H_ */