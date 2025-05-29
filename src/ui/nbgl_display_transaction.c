/*****************************************************************************
 *   Ontology Ledger App
 *   (c) 2025 Ontology
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
 *****************************************************************************/

#ifdef HAVE_NBGL

#include <stdbool.h>  // bool
#include <string.h>   // memset, strcmp

#include "os.h"
#include "glyphs.h"
#include "os_io_seproxyhal.h"
#include "nbgl_use_case.h"
#include "io.h"
#include "bip32.h"
#include "format.h"

#include "display.h"
#include "constants.h"
#include "globals.h"
#include "sw.h"
#include "address.h"
#include "validate.h"
#include "tx_types.h"
#include "menu.h"
#include "types.h"
#include "../transaction/contract.h"
#include "../transaction/utils.h"
#include "tx_init.h"

#define MAX_PUBKEY_DISPLAY 3 //must be smaller than UINT8_MAX
#define AMOUNT_SIZE        50

// g_buffers[PARAMETERS_MAX_NUM * MAX_BUFFER_LEN]: total amount
// g_buffers[PARAMETERS_MAX_NUM * MAX_BUFFER_LEN + AMOUNT_SIZE]: gas fee
// g_buffers[(PARAMETERS_MAX_NUM + 1)* MAX_BUFFER_LEN]: signer
char g_buffers[NUM_PAIRS * MAX_BUFFER_LEN];
nbgl_contentTagValue_t g_pairs[NUM_PAIRS];
nbgl_contentTagValueList_t g_pairList;


// Unified parameters handler function
static bool handle_params(transaction_t *tx,
                          const method_display_t *method,
                          nbgl_contentTagValue_t *tag_pairs,
                          uint8_t *nbPairs) {
    if (tx == NULL || tag_pairs == NULL || nbPairs == NULL || method == NULL ||
        method->configs == NULL) {
        PRINTF("Error: Null pointer in handle_params\n");
        return false;
    }

    const param_config_t *configs = method->configs;
    *nbPairs = method->config_count;
    for (uint8_t i = 0; i < *nbPairs; i++) {
        if (!convert_param_to_chars(tx, i, &g_buffers[i * MAX_BUFFER_LEN], MAX_BUFFER_LEN)) {
            return false;
        }
        tag_pairs[configs[i].pos].item = configs[i].item;
        tag_pairs[configs[i].pos].value = &g_buffers[i * MAX_BUFFER_LEN];
    }

    // Perform special handling for some methods in Native contracts
    if (tx->contract.type != NATIVE_CONTRACT) {
        return true;
    }

    // When registering a candidate node, the staking fee of 500 NG needs to be displayed
    if (methodcmp(&tx->method.name, METHOD_REGISTER_CANDIDATE)) {
        tag_pairs[*nbPairs].item = STAKE_FEE;
        tag_pairs[*nbPairs].value = STAKE_FEE_ONG;
        (*nbPairs)++;
    }

    // The parameters of these three methods of the governance contract are of the pk_num_list type.
    // Only the first three public keys and the total amount are displayed.
    // If there are more than three public keys, the number of remaining public keys is displayed.
    if (methodcmp(&tx->method.name, METHOD_AUTHORIZE_FOR_PEER) ||
        methodcmp(&tx->method.name, METHOD_UNAUTHORIZE_FOR_PEER) ||
        methodcmp(&tx->method.name, METHOD_WITHDRAW)) {
        uint64_t pubkey_num = 0;
        if (!convert_param_to_uint64_le(&tx->method.parameters[1], &pubkey_num) ||
            pubkey_num == 0) {
            return false;
        }

        const char *amout_item = NULL;
        const char *total_amount_item;
        if (methodcmp(&tx->method.name, METHOD_AUTHORIZE_FOR_PEER)) {
            amout_item = STAKE_AMOUNT;
            total_amount_item = TOTAL_PLUS STAKE_AMOUNT;
        } else if (methodcmp(&tx->method.name, METHOD_UNAUTHORIZE_FOR_PEER)) {
            amout_item = UNSTAKE_AMOUNT;
            total_amount_item = TOTAL_PLUS UNSTAKE_AMOUNT;
        } else if (methodcmp(&tx->method.name, METHOD_WITHDRAW)) {
            amout_item = WITHDRAW_AMOUNT;
            total_amount_item = TOTAL_PLUS WITHDRAW_AMOUNT;
        }

        size_t curr = *nbPairs;
        uint8_t max_display_num = (pubkey_num < MAX_PUBKEY_DISPLAY) ? (uint8_t)pubkey_num : MAX_PUBKEY_DISPLAY;
        for (uint8_t i = 0; i < max_display_num; i++) {
            const char *label_pk;
            if (i == 0 && pubkey_num == 1) {
                label_pk = PEER_PUBKEY;
            } else {
                snprintf(&g_buffers[curr * MAX_BUFFER_LEN],
                         MAX_BUFFER_LEN,
                         "%s %d",
                         PEER_PUBKEY,
                         i + 1);
                label_pk = &g_buffers[(curr++) * MAX_BUFFER_LEN];
            }
            if (!convert_param_to_chars(tx,
                                        i + 2,
                                        &g_buffers[curr * MAX_BUFFER_LEN],
                                        MAX_BUFFER_LEN)) {
                return false;
            }
            tag_pairs[(*nbPairs)].item = label_pk;
            tag_pairs[(*nbPairs)++].value = &g_buffers[(curr++) * MAX_BUFFER_LEN];

            if (pubkey_num <= MAX_PUBKEY_DISPLAY) {
                const char *label_amount;
                if (i == 0 && pubkey_num == 1) {
                    label_amount = amout_item;
                } else {
                    snprintf(&g_buffers[curr * MAX_BUFFER_LEN],
                             MAX_BUFFER_LEN,
                             "%s %d",
                             amout_item,
                             i + 1);
                    label_amount = &g_buffers[(curr++) * MAX_BUFFER_LEN];
                }
                if (!convert_param_to_chars(tx,
                                            i + 2 + pubkey_num,
                                            &g_buffers[curr * MAX_BUFFER_LEN],
                                            MAX_BUFFER_LEN)) {
                    return false;
                }
                tag_pairs[(*nbPairs)].item = label_amount;
                tag_pairs[(*nbPairs)++].value = &g_buffers[(curr++) * MAX_BUFFER_LEN];
            }
        }

        if (pubkey_num > MAX_PUBKEY_DISPLAY) {
            format_u64(&g_buffers[curr * MAX_BUFFER_LEN],
                       MAX_BUFFER_LEN,
                       pubkey_num - MAX_PUBKEY_DISPLAY);
            tag_pairs[*nbPairs].item = NODE_AMOUNT;
            tag_pairs[(*nbPairs)++].value = &g_buffers[(curr++) * MAX_BUFFER_LEN];

            uint64_t amount = 0;
            for (size_t i = 0; i < pubkey_num; i++) {
                uint64_t tmp_amount = 0;
                if (!convert_param_to_uint64_le(&tx->method.parameters[i + 2 + pubkey_num],
                                                &tmp_amount)) {
                    return false;
                }
                amount += tmp_amount;
            }
            format_u64(&g_buffers[PARAMETERS_MAX_NUM * MAX_BUFFER_LEN], AMOUNT_SIZE, amount);
            strlcat(&g_buffers[PARAMETERS_MAX_NUM * MAX_BUFFER_LEN], " ", AMOUNT_SIZE);
            strlcat(&g_buffers[PARAMETERS_MAX_NUM * MAX_BUFFER_LEN], tx->contract.ticker, AMOUNT_SIZE);
            tag_pairs[*nbPairs].item = total_amount_item;
            tag_pairs[(*nbPairs)++].value = &g_buffers[PARAMETERS_MAX_NUM * MAX_BUFFER_LEN];
        }
    }

    // The transfer/transferV2 methods for native tokens can involve multi-to-multi operations,
    // meaning there can be a series of 'from', 'to', and 'amount' fields that need to be iterated
    // over.
    if (methodcmp(&tx->method.name, METHOD_TRANSFER) ||
        methodcmp(&tx->method.name, METHOD_TRANSFER_V2)) {
        uint8_t state_num = 1;
        while (tx->method.parameters[3 * state_num].data != NULL &&
               3 * state_num + 2 < PARAMETERS_MAX_NUM) {
            for (uint8_t i = 0; i < 3; i++) {
                uint8_t index = i + 3 * state_num;
                uint8_t pos = configs[i].pos + 3 * state_num;
                if (!convert_param_to_chars(tx,
                                            index,
                                            &g_buffers[index * MAX_BUFFER_LEN],
                                            MAX_BUFFER_LEN)) {
                    return false;
                }
                tag_pairs[pos].item = configs[i].item;
                tag_pairs[pos].value = &g_buffers[index * MAX_BUFFER_LEN];
            }
            state_num++;
            *nbPairs += 3;
        }
    }
    return true;
}

static bool calc_gas_chars(char *buffer, size_t buffer_len) {
    uint64_t gp = G_context.tx_info.transaction.header.gas_price;
    uint64_t gl = G_context.tx_info.transaction.header.gas_limit;
    if (buffer == NULL || !format_fpu64_trimmed(buffer, buffer_len, gp * gl, ONG_DECIMALS)) {
        return false;
    }
    strlcat(buffer, " ", buffer_len);
    strlcat(buffer, ONG_TICKER, buffer_len);

    return true;
}

static void review_choice(bool confirm) {
    validate_transaction(confirm);
    nbgl_useCaseReviewStatus(
        confirm ? STATUS_TYPE_TRANSACTION_SIGNED : STATUS_TYPE_TRANSACTION_REJECTED,
        ui_menu_main);
}

static int ui_display_bs_transaction() {
    g_pairs[g_pairList.nbPairs].item = BLIND_SIGN_TX;
    g_pairs[g_pairList.nbPairs++].value = BLIND_SIGNING;

    size_t addr_len = G_context.tx_info.transaction.contract.addr.len;
    uint8_t *addr = G_context.tx_info.transaction.contract.addr.data;
    format_hex(addr, addr_len, &g_buffers[0], MAX_BUFFER_LEN);
    g_pairs[g_pairList.nbPairs].item = CONTRACT_ADDRESS;
    g_pairs[g_pairList.nbPairs++].value = &g_buffers[0];

    g_pairs[g_pairList.nbPairs].item = GAS_FEE;
    g_pairs[g_pairList.nbPairs++].value = &g_buffers[PARAMETERS_MAX_NUM * MAX_BUFFER_LEN + AMOUNT_SIZE];

    g_pairs[g_pairList.nbPairs].item = SIGNER;
    g_pairs[g_pairList.nbPairs++].value = &g_buffers[(PARAMETERS_MAX_NUM + 1) * MAX_BUFFER_LEN];

    nbgl_useCaseReviewBlindSigning(TYPE_TRANSACTION,
                                   &g_pairList,
                                   &ICON_APP_ONTOLOGY,
                                   BLIND_SIGNING_TITLE,
                                   NULL,
#ifdef SCREEN_SIZE_WALLET
                                   BLIND_SIGNING_CONTENT,
#else
                                    NULL,
#endif
                                    NULL,
                                   review_choice);
    return 0;
}

static int ui_display_normal_transaction() {
    const method_display_t *method = init_dipslay_pos_and_item(&G_context.tx_info.transaction);
    if (method == NULL) {
        return io_send_sw(SW_INVALID_TRANSACTION);
    }
    if (!handle_params(&G_context.tx_info.transaction, method, g_pairs, &g_pairList.nbPairs)) {
        return io_send_sw(SW_INVALID_TRANSACTION);
    }

    g_pairs[g_pairList.nbPairs].item = GAS_FEE;
    g_pairs[g_pairList.nbPairs++].value = &g_buffers[PARAMETERS_MAX_NUM * MAX_BUFFER_LEN + AMOUNT_SIZE];

    g_pairs[g_pairList.nbPairs].item = SIGNER;
    g_pairs[g_pairList.nbPairs++].value = &g_buffers[(PARAMETERS_MAX_NUM + 1) * MAX_BUFFER_LEN];

    nbgl_useCaseReview(TYPE_TRANSACTION,
                       &g_pairList,
                       &ICON_APP_ONTOLOGY,
                       method->title,
                       NULL,
#ifdef SCREEN_SIZE_WALLET
                       method->finish_title,
#else
                        NULL,
#endif
                       review_choice);
    return 0;
}

int ui_display_transaction(bool is_blind_signed) {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    explicit_bzero(&g_buffers, sizeof(g_buffers));

    explicit_bzero(&g_pairList, sizeof(g_pairList));
    g_pairList.pairs = g_pairs;
    g_pairList.nbPairs = 0;

    if (!calc_gas_chars(&g_buffers[PARAMETERS_MAX_NUM * MAX_BUFFER_LEN + AMOUNT_SIZE],
                        MAX_BUFFER_LEN)) {
        return io_send_sw(SW_INVALID_TRANSACTION);
    }

    if (!derive_address_from_bip32_path(&g_buffers[(PARAMETERS_MAX_NUM + 1) * MAX_BUFFER_LEN],
                                        MAX_BUFFER_LEN)) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }

    if (is_blind_signed) {
        return ui_display_bs_transaction();
    } else {
        return ui_display_normal_transaction();
    }
    return 0;
}

#endif