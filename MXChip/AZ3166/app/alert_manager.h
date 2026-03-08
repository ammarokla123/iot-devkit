/* 
 * Copyright (c) 2024 Eclipse Foundation
 * 
 *  This program and the accompanying materials are made available 
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 * 
 *  SPDX-License-Identifier: MIT
 * 
 *  Contributors: 
 *     <Your Name> - Alert manager for Environmental Monitoring demo.
 */

#ifndef _ALERT_MANAGER_H
#define _ALERT_MANAGER_H

#include "tx_api.h"

/* Thread entry point */
void alert_manager_thread_entry(ULONG parameter);

#endif /* _ALERT_MANAGER_H */
