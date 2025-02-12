/*******************************************************************************
 *   XAH Wallet
 *   (c) 2017 Ledger
 *   (c) 2020 Towo Labs
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

#include <string.h>

#include "flags.h"
#include "readers.h"
#include "sign_transaction.h"
#include "transaction_types.h"
#include "fmt.h"

#define HAS_FLAG(value, flag) ((value) & (flag)) == flag

bool is_flag(const field_t *field) {
    return field->data_type == STI_UINT32 &&
           (field->id == XAH_UINT32_FLAGS || field->id == XAH_UINT32_SET_FLAG ||
            field->id == XAH_UINT32_CLEAR_FLAG);
}

bool is_flag_hidden(const field_t *field) {
    if (is_flag(field)) {
        uint32_t value = field->data.u32;

        return value == 0 || value == TF_FULLY_CANONICAL_SIG;
    }

    return false;
}

static size_t set_error_value(field_value_t *out) {
    strncpy(out->buf, "[ERROR: FAILED TO APPEND ITEM]", sizeof(out->buf));
    return sizeof(out->buf);
}

static size_t append_item(field_value_t *out, size_t offset, const char *in) {
    size_t len = strlen(in);

    if (offset != 0) {
        if (sizeof(out->buf) - offset < 3) {
            return set_error_value(out);
        }
        out->buf[offset + 0] = ',';
        out->buf[offset + 1] = ' ';
        out->buf[offset + 2] = '\x00';
        offset += 2;
    }

    if (len >= sizeof(out->buf) - offset) {
        return set_error_value(out);
    }

    strncpy(out->buf + offset, in, sizeof(out->buf) - offset);

    return offset + len;
}

static void format_account_set_transaction_flags(uint32_t value, field_value_t *dst) {
// AccountSet flags
#define TF_REQUIRE_DEST_TAG  0x00010000u
#define TF_OPTIONAL_DEST_TAG 0x00020000u
#define TF_REQUIRE_AUTH      0x00040000u
#define TF_OPTIONAL_AUTH     0x00080000u
#define TF_DISALLOW_XAH      0x00100000u
#define TF_ALLOW_XAH         0x00200000u

    size_t offset = 0;
    if (HAS_FLAG(value, TF_REQUIRE_DEST_TAG)) {
        offset = append_item(dst, offset, "Require Dest Tag");
    }

    if (HAS_FLAG(value, TF_OPTIONAL_DEST_TAG)) {
        offset = append_item(dst, offset, "Optional Dest Tag");
    }

    if (HAS_FLAG(value, TF_REQUIRE_AUTH)) {
        offset = append_item(dst, offset, "Require Auth");
    }

    if (HAS_FLAG(value, TF_OPTIONAL_AUTH)) {
        offset = append_item(dst, offset, "Optional Auth");
    }

    if (HAS_FLAG(value, TF_DISALLOW_XAH)) {
        offset = append_item(dst, offset, "Disallow XAH");
    }

    if (HAS_FLAG(value, TF_ALLOW_XAH)) {
        append_item(dst, offset, "Allow XAH");
    }
}

static const char *format_account_set_field_flags(uint32_t value) {
// AccountSet flags for fields SetFlag and ClearFlag
#define ASF_REQUIRE_DEST                    1
#define ASF_REQUIRE_AUTH                    2
#define ASF_DISALLOW_XAH                    3
#define ASF_DISABLE_MASTER                  4
#define ASF_ACCOUNT_TXN_ID                  5
#define ASF_NO_FREEZE                       6
#define ASF_GLOBAL_FREEZE                   7
#define ASF_DEFAULT_RIPPLE                  8
#define ASF_DEPOSIT_AUTH                    9
#define ASF_AUTH_TOKEN_MINTER               10
#define ASF_TSH_COLLECT                     11
#define ASF_DISALLOW_INCOMING_NFTOKEN_OFFER 12
#define ASF_DISALLOW_INCOMING_CHECK         13
#define ASF_DISALLOW_INCOMING_PAYCHAN       14
#define ASF_DISALLOW_INCOMING_TRUSTLINE     15
#define ASF_DISALLOW_INCOMING_REMIT         16

    // Logic is different because only one flag is allowed per field
    switch (value) {
        case ASF_REQUIRE_DEST:
            return "Require Dest";
        case ASF_REQUIRE_AUTH:
            return "Require Auth";
        case ASF_DISALLOW_XAH:
            return "Disallow XAH";
        case ASF_DISABLE_MASTER:
            return "Disable Master";
        case ASF_ACCOUNT_TXN_ID:
            return "Track Txn ID";
        case ASF_NO_FREEZE:
            return "No Freeze";
        case ASF_GLOBAL_FREEZE:
            return "Global Freeze";
        case ASF_DEFAULT_RIPPLE:
            return "Default Ripple";
        case ASF_DEPOSIT_AUTH:
            return "Deposit Auth";
        case ASF_AUTH_TOKEN_MINTER:
            return "Auth NFToken Minter";
        case ASF_TSH_COLLECT:
            return "TSH Collect";
        case ASF_DISALLOW_INCOMING_NFTOKEN_OFFER:
            return "Disallow NFToken Offer";
        case ASF_DISALLOW_INCOMING_CHECK:
            return "Disallow Check";
        case ASF_DISALLOW_INCOMING_PAYCHAN:
            return "Disallow PayChan";
        case ASF_DISALLOW_INCOMING_TRUSTLINE:
            return "Disallow Trustline";
        case ASF_DISALLOW_INCOMING_REMIT:
            return "Disallow Remit";
        default:
            return NULL;
    }
}

static void format_account_set_flags(field_t *field, uint32_t value, field_value_t *dst) {
    if (field->id == XAH_UINT32_FLAGS) {
        format_account_set_transaction_flags(value, dst);
    } else {
        const char *flag = format_account_set_field_flags(value);
        if (flag != NULL) {
            strncpy(dst->buf, flag, sizeof(dst->buf));
        } else {
            snprintf(dst->buf, sizeof(dst->buf), "Unknown flag: %u", value);
        }
    }
}

static void format_offer_create_flags(uint32_t value, field_value_t *dst) {
// OfferCreate flags
#define TF_PASSIVE             0x00010000u
#define TF_IMMEDIATE_OR_CANCEL 0x00020000u
#define TF_FILL_OR_KILL        0x00040000u
#define TF_SELL                0x00080000u

    size_t offset = 0;
    if (HAS_FLAG(value, TF_PASSIVE)) {
        offset = append_item(dst, offset, "Passive");
    }

    if (HAS_FLAG(value, TF_IMMEDIATE_OR_CANCEL)) {
        offset = append_item(dst, offset, "Immediate or Cancel");
    }

    if (HAS_FLAG(value, TF_FILL_OR_KILL)) {
        offset = append_item(dst, offset, "Fill or Kill");
    }

    if (HAS_FLAG(value, TF_SELL)) {
        append_item(dst, offset, "Sell");
    }
}

static void format_payment_flags(uint32_t value, field_value_t *dst) {
// Payment flags
#define TF_NO_RIPPLE_DIRECT 0x00010000u
#define TF_PARTIAL_PAYMENT  0x00020000u
#define TF_LIMIT_QUALITY    0x00040000u

    size_t offset = 0;
    if (HAS_FLAG(value, TF_NO_RIPPLE_DIRECT)) {
        offset = append_item(dst, offset, "No Direct Ripple");
    }

    if (HAS_FLAG(value, TF_PARTIAL_PAYMENT)) {
        offset = append_item(dst, offset, "Partial Payment");
    }

    if (HAS_FLAG(value, TF_LIMIT_QUALITY)) {
        append_item(dst, offset, "Limit Quality");
    }
}

static void format_trust_set_flags(uint32_t value, field_value_t *dst) {
// TrustSet flags
#define TF_SETF_AUTH       0x00010000u
#define TF_SET_NO_RIPPLE   0x00020000u
#define TF_CLEAR_NO_RIPPLE 0x00040000u
#define TF_SET_FREEZE      0x00100000u
#define TF_CLEAR_FREEZE    0x00200000u

    size_t offset = 0;
    if (HAS_FLAG(value, TF_SETF_AUTH)) {
        offset = append_item(dst, offset, "Set Auth");
    }

    if (HAS_FLAG(value, TF_SET_NO_RIPPLE)) {
        offset = append_item(dst, offset, "Set No Ripple");
    }

    if (HAS_FLAG(value, TF_CLEAR_NO_RIPPLE)) {
        offset = append_item(dst, offset, "Clear No Ripple");
    }

    if (HAS_FLAG(value, TF_SET_FREEZE)) {
        offset = append_item(dst, offset, "Set Freeze");
    }

    if (HAS_FLAG(value, TF_CLEAR_FREEZE)) {
        append_item(dst, offset, "Clear Freeze");
    }
}

static void format_payment_channel_claim_flags(uint32_t value, field_value_t *dst) {
// PaymentChannelClaim flags
#define TF_RENEW 0x00010000u
#define TF_CLOSE 0x00020000u

    size_t offset = 0;
    if (HAS_FLAG(value, TF_RENEW)) {
        offset = append_item(dst, offset, "Renew");
    }

    if (HAS_FLAG(value, TF_CLOSE)) {
        append_item(dst, offset, "Close");
    }
}

static void format_set_hook_flags(uint32_t value, field_value_t *dst) {
// SetHook flags
#define HSF_OVERRIDE 0x00000001u
#define HSF_NSDELETE 0x0000002u
#define HSF_COLLECT  0x00000004u

    size_t offset = 0;
    if (HAS_FLAG(value, HSF_OVERRIDE)) {
        offset = append_item(dst, offset, "Override");
    }

    if (HAS_FLAG(value, HSF_NSDELETE)) {
        offset = append_item(dst, offset, "Namespace Delete");
    }

    if (HAS_FLAG(value, HSF_COLLECT)) {
        append_item(dst, offset, "Collect");
    }
}

static void format_uritoken_mint_flags(uint32_t value, field_value_t *dst) {
// URITokenMint flags
#define TF_BURNABLE 0x00000001u

    size_t offset = 0;
    if (HAS_FLAG(value, TF_BURNABLE)) {
        append_item(dst, offset, "Burnable");
    }
}

static void format_claim_reward_flags(uint32_t value, field_value_t *dst) {
// ClaimReward flags
#define TF_OPT_IN 0x00000001u

    size_t offset = 0;
    if (HAS_FLAG(value, TF_OPT_IN)) {
        append_item(dst, offset, "Opt Out");
    }
}

void format_flags(field_t *field, field_value_t *dst) {
    uint32_t value = field->data.u32;
    switch (parse_context.transaction_type) {
        case TRANSACTION_ACCOUNT_SET:
            format_account_set_flags(field, value, dst);
            break;
        case TRANSACTION_OFFER_CREATE:
            format_offer_create_flags(value, dst);
            break;
        case TRANSACTION_PAYMENT:
            format_payment_flags(value, dst);
            break;
        case TRANSACTION_TRUST_SET:
            format_trust_set_flags(value, dst);
            break;
        case TRANSACTION_PAYMENT_CHANNEL_CLAIM:
            format_payment_channel_claim_flags(value, dst);
            break;
        case TRANSACTION_SET_HOOK:
            format_set_hook_flags(value, dst);
            break;
        case TRANSACTION_URITOKEN_MINT:
            format_uritoken_mint_flags(value, dst);
            break;
        case TRANSACTION_CLAIM_REWARD:
            format_claim_reward_flags(value, dst);
            break;
        case TRANSACTION_REMIT:
            format_uritoken_mint_flags(value, dst);
            break;
        default:
            snprintf(dst->buf,
                     sizeof(dst->buf),
                     "No flags for transaction type %d",
                     parse_context.transaction_type);
            return;
    }

    // Check if no flags were found (despite is_flag_hidden returning false) and respond
    // appropriately
    if (dst->buf[0] == 0x00) {
        strncpy(dst->buf, "Unsupported value", sizeof(dst->buf));
    }
}
