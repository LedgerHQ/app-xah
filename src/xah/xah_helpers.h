/*******************************************************************************
 *   XAH Wallet
 *   (c) 2017 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#pragma once

#include <stdbool.h>

#include "os.h"
#include "cx.h"
#include "fields.h"

#define XAH_PUBKEY_SIZE  33
#define XAH_ADDRESS_SIZE 41

typedef struct {
    uint8_t buf[XAH_PUBKEY_SIZE];
} xah_pubkey_t;

typedef union {
    xah_pubkey_t pubkey;
    xah_account_t account;
} xah_pubkey_or_account;

typedef struct {
    char buf[XAH_ADDRESS_SIZE];
} xah_address_t;

cx_err_t xah_public_key_hash160(xah_pubkey_t *pubkey, uint8_t *out);

size_t xah_public_key_to_encoded_base58(xah_pubkey_t *pubkey,
                                        xah_account_t *account,
                                        xah_address_t *out,
                                        uint16_t version);

void xah_compress_public_key(cx_ecfp_public_key_t *public_key, xah_pubkey_t *out);

void get_address(cx_ecfp_public_key_t *pubkey, xah_address_t *address);

int xah_print_amount(uint64_t amount, char *out, size_t outlen);

bool parse_bip32_path(uint8_t *path,
                      size_t path_length,
                      uint32_t *path_parsed,
                      size_t path_parsed_length);
