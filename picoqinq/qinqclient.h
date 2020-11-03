/*
* Author: Christian Huitema
* Copyright (c) 2019, Private Octopus, Inc.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Private Octopus, Inc. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef picoqinq_client_H
#define picoqinq_client_H


struct st_picoqinq_client_stream_ctx_t;

/* Define stream context for callbacks
 */
typedef enum {
    picoqinq_client_stream_status_none = 0,
    picoqinq_client_stream_status_header,
    picoqinq_client_stream_status_crlf,
    picoqinq_client_stream_status_receiving,
    picoqinq_client_stream_status_finished
} picoqinq_client_stream_status_t;

#define PICOQINQ_client_FRAME_MAX 2048

typedef struct st_picoqinq_client_stream_ctx_t {
    struct st_picoqinq_client_stream_ctx_t* next_stream;
    uint64_t stream_id;
    size_t data_received;
    int is_fin_received : 1;
    int is_reset : 1;
    int hc_direction;
    picoqinq_header_compression_t* hc;
    uint8_t frame[PICOQINQ_client_FRAME_MAX];
} picoqinq_client_stream_ctx_t;


/* Define the QinQ server callback */

typedef struct st_picoqinq_client_callback_ctx_t {
    picoquic_quic_t * quic;
    picoqinq_client_stream_ctx_t* first_stream;
    size_t buffer_max;
    uint8_t* buffer;
    uint64_t receive_hcid;
    uint64_t send_hcid;
    picoqinq_header_compression_t* receive_hc;
    picoqinq_header_compression_t* send_hc;
} picoqinq_client_callback_ctx_t;

/*
 * Structures used in the hash table of connections
 */
#define PICOQINQ_MIN_CID_LENGTH 4
typedef struct st_picoqinq_cnx_id_key_t {
    uint8_t cnx_id[PICOQINQ_MIN_CID_LENGTH];
    struct st_picoqinq_client_callback_ctx_t* cnx_ctx;
    struct st_picoqinq_cnx_id_key_t* next_cnx_id;
} picoqinq_cnx_id_key_t;

/* Quic in Quic context
 */
typedef struct st_picoqinq_ctx_t {
    picoquic_quic_t* quic;
    /* TODO: hash table of CNX_ID */
    /* TODO: list of connections? */
} picoqinq_ctx_t;

picoqinq_client_stream_ctx_t * picoqinq_find_or_create_stream(
    picoquic_cnx_t* cnx,
    uint64_t stream_id,
    picoqinq_client_callback_ctx_t * ctx,
    int should_create);

int picoqinq_client_callback_data(picoquic_cnx_t* cnx, picoqinq_client_stream_ctx_t * stream_ctx, uint64_t stream_id, uint8_t* bytes, 
    size_t length, picoquic_call_back_event_t fin_or_event, picoqinq_client_callback_ctx_t* callback_ctx);

picoqinq_client_callback_ctx_t * picoqinq_client_callback_create_context(picoquic_quic_t * quic);

void picoqinq_client_callback_delete_context(picoqinq_client_callback_ctx_t * ctx);

int picoqinq_client_callback(picoquic_cnx_t* cnx,
    uint64_t stream_id, uint8_t* bytes, size_t length,
    picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);

int picoqinq_forward_outgoing_packet(picoquic_cnx_t* cnx, uint8_t* bytes, size_t length, const struct sockaddr* addr_to, uint64_t current_time);

picoquic_cnx_t* picoqinq_create_proxied_cnx(picoquic_cnx_t* cnx_proxy, struct sockaddr* addr, uint64_t start_time, uint32_t preferred_version, char const* sni, char const* alpn, picoquic_stream_data_cb_fn callback_fn, void* callback_ctx);

#endif /* DEMO_client_H */