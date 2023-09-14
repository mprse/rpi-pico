/*
 * Copyright (c) 2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SRC_BLESERVER_H_
#define SRC_BLESERVER_H_

void BleServer_SetupBLE(void);

void BleServer_Init(void);

void send_data(uint16_t value);

#endif /* SRC_BLESERVER_H_ */