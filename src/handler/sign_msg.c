/*****************************************************************************
 *   Ontology Ledger App
 *   (c) 2025 OGD
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

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"
#include "../sw.h"
#include "../globals.h"
#include "../ui/display.h"
#include "sign_msg.h"
#include "../message/types.h"
#include "../transaction/utils.h"
#include "../address.h"

#define MSG_LEN_MAX_CHARS 8
int handler_sign_message(buffer_t *cdata, uint8_t chunk, bool more) {
    if (chunk == 0) {  // first APDU, parse BIP32 path
        explicit_bzero(&G_context, sizeof(G_context));
        G_context.req_type = CONFIRM_MESSAGE;
        G_context.state = STATE_NONE;

        if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
            !buffer_read_bip32_path(cdata,
                                    G_context.bip32_path,
                                    (size_t) G_context.bip32_path_len)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }
        if (!is_valid_bip44_prefix(G_context.bip32_path, G_context.bip32_path_len)) {
            return io_send_sw(SW_INVALID_PATH);
        }
        return io_send_sw(SW_OK);
    } else {
        if (G_context.req_type != CONFIRM_MESSAGE) {
            return io_send_sw(SW_BAD_STATE);
        }
        if (G_context.msg_info.raw_msg_len + cdata->size > sizeof(G_context.msg_info.raw_msg)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }
        if (!buffer_move(cdata,
                         G_context.msg_info.raw_msg + G_context.msg_info.raw_msg_len,
                         cdata->size)) {
            return io_send_sw(SW_PERSONAL_MSG_PARSING_FAIL);
        }
        G_context.msg_info.raw_msg_len += cdata->size;
        if (more) {
            // more APDUs with message siging request part are expected.
            // Send a SW_OK to signal that we have received the chunk
            return io_send_sw(SW_OK);

        } else {
            // last APDU for this message siging request, let's display and request a sign
            // confirmation
            G_context.state = STATE_PARSED;

            cx_sha256_t cx_sha256;
            cx_sha256_init(&cx_sha256);

            char len[MSG_LEN_MAX_CHARS];
            snprintf(len, sizeof(len), "%u", G_context.msg_info.raw_msg_len);

            if (cx_hash_update((cx_hash_t *) &cx_sha256, SIGN_MAGIC, sizeof(SIGN_MAGIC)) != CX_OK ||
                cx_hash_update((cx_hash_t *) &cx_sha256, (uint8_t *) len, strlen(len)) != CX_OK ||
                cx_hash_update((cx_hash_t *) &cx_sha256,
                               G_context.msg_info.raw_msg,
                               G_context.msg_info.raw_msg_len) != CX_OK ||
                cx_hash_final((cx_hash_t *) &cx_sha256, G_context.msg_info.m_hash) != CX_OK) {
                return io_send_sw(SW_HASH_FAILED);
            }

            PRINTF("Hash: %.*H\n", sizeof(G_context.msg_info.m_hash), G_context.msg_info.m_hash);
            return ui_display_message();
        }
    }
    return 0;
}