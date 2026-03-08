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
 *     <Your Name> - NUCLEO-F429ZI NetX Duo configuration.
 */

#ifndef NX_USER_H
#define NX_USER_H

/* Disable IPv6 — this starter application uses IPv4 only */
#define NX_DISABLE_IPV6

/* Override the packet header to include space for Ethernet framing */
#define NX_PHYSICAL_HEADER  14

/* Disable FileX integration in NetX Duo servers */
#define NX_DISABLE_RESET_DISCONNECT

#endif /* NX_USER_H */
