#include "transaction/deserialize.h"
#include "transaction/tx_types.h"

#include <stddef.h>
#include <stdint.h>
#include "format.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    buffer_t buf = {.offset = 0, .ptr = Data, .size = Size};
    transaction_t tx = {0};
    parser_status_e status;
    char nonce[21] = {0};
    status = transaction_deserialize(&buf, &tx);
    if (status == PARSING_OK) {
        format_u64(nonce, sizeof(nonce), tx.header.nonce);
        printf("nonce: %s\n", nonce);
    }
    return 0;
}