/*
 * Lock.cpp
 *
 *  Created on: 17 gen 2024
 *      Author: maurizio
 */

#include "Lock.h"

Lock::Lock() {
	 xSemaphore = xSemaphoreCreateBinary();
	 if (xSemaphore) xSemaphoreGive(xSemaphore);
}

void Lock::take() {
	if (xSemaphore) xSemaphoreTake( xSemaphore,portMAX_DELAY);
}

void Lock::give() {
	if (xSemaphore) xSemaphoreGive(xSemaphore);
}

Lock::~Lock() {
	if (xSemaphore)	vSemaphoreDelete(xSemaphore);
}

