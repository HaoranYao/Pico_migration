/*
* Author: Christian Huitema
* Copyright (c) 2017, Private Octopus, Inc.
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

/*
 * Management of transport parameters for PicoQUIC.
 */

#include "picoquic_internal.h"
#include "tls_api.h"
#include <string.h>

uint8_t* picoquic_transport_param_varint_encode(uint8_t* bytes, const uint8_t* bytes_max, uint64_t n64) 
{
    if (bytes + 2 > bytes_max) {
        bytes = NULL;
    }
    else {
        uint8_t * byte_l;
        size_t l;

        *bytes++ = 0;
        byte_l = bytes;
        *bytes++ = 0;
        l = picoquic_varint_encode(bytes, bytes_max - bytes, n64);
        if (l == 0) {
            bytes = NULL;
        }
        else {
            *byte_l = (uint8_t) l;
            bytes += l;
        }
    }

    return bytes;
}

uint64_t picoquic_transport_param_varint_decode(picoquic_cnx_t * cnx, uint8_t* bytes, uint16_t extension_length, int* ret) 
{
    uint64_t n64 = 0;
    size_t l_v = picoquic_varint_decode(bytes, extension_length, &n64);

    if (l_v == 0 || l_v != extension_length) {
        *ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
    }

    return n64;
}

uint8_t* picoquic_transport_param_type_varint_encode(uint8_t* bytes, const uint8_t* bytes_max, picoquic_tp_enum tp_type, uint64_t n64)
{
    if (bytes != NULL && bytes + 2 <= bytes_max) {
        picoformat_16(bytes, (uint16_t)tp_type);
        bytes = picoquic_transport_param_varint_encode(bytes + 2, bytes_max, n64);
    }
    else {
        bytes = NULL;
    }
    return bytes;
}

uint64_t picoquic_decode_transport_param_stream_id(uint64_t rank, int extension_mode, int stream_type) 
{
    uint64_t stream_id = 0xFFFFFFFFFFFFFFFFull;
    
    if (rank > 0) {
        stream_id = stream_type;
        stream_id += extension_mode^1;

        stream_id += 4 * (rank - 1);
    }

    return stream_id;
}

uint64_t picoquic_prepare_transport_param_stream_id(uint64_t stream_id) 
{
    uint64_t rank = 0;

    if (stream_id != 0xFFFFFFFFFFFFFFFFll) {
        rank = (uint64_t)1 +  (stream_id / 4);
    }

    return rank;
}

uint8_t * picoquic_encode_transport_param_prefered_address(uint8_t * bytes, uint8_t * bytes_max,
    picoquic_tp_prefered_address_t * prefered_address)
{
    /* first compute the length */
    uint16_t coded_length = 4 + 2 + 16 + 2 + 1 + prefered_address->connection_id.id_len + 16;

    if (bytes == NULL || bytes + coded_length > bytes_max) {
        bytes = NULL;
    } else {
        picoformat_16(bytes, picoquic_tp_server_preferred_address);
        bytes += 2;
        picoformat_16(bytes, coded_length);
        bytes += 2;
        memcpy(bytes, prefered_address->ipv4Address, 4);
        bytes += 4;
        picoformat_16(bytes, prefered_address->ipv4Port);
        bytes += 2;
        memcpy(bytes, prefered_address->ipv6Address, 16);
        bytes += 16;
        picoformat_16(bytes, prefered_address->ipv4Port);
        bytes += 2;
        *bytes++ = prefered_address->connection_id.id_len;
        bytes += picoquic_format_connection_id(bytes, bytes_max - bytes,
            prefered_address->connection_id);
        memcpy(bytes, prefered_address->statelessResetToken, 16);
        bytes += 16;
    }

    return bytes;
}

uint8_t * picoquic_encode_transport_param_prefered_address_old(uint8_t * bytes, uint8_t * bytes_max,
    picoquic_tp_prefered_address_t * prefered_address)
{
    /* first compute the length */
    uint16_t coded_length;
    /* First compute which version to send */
    uint8_t ip_version = 0;
    int ip_v4_defined = 0;
    int ip_v6_defined = 0;
    uint8_t ip_length = 0;
    uint8_t * ip_address = NULL;
    uint16_t port = 0;

    for (int i = 0; i < 4; i++) {
        if (prefered_address->ipv4Address[i] != 0) {
            ip_v4_defined = 1;
        }
    }

    for (int i = 0; i < 16; i++) {
        if (prefered_address->ipv6Address[i] != 0) {
            ip_v6_defined = 1;
        }
    }

    if (ip_v6_defined) {
        ip_version = 6;
        ip_length = 16;
        ip_address = prefered_address->ipv6Address;
        port = prefered_address->ipv6Port;
    }
    else if (ip_v4_defined) {
        ip_version = 4;
        ip_length = 4;
        ip_address = prefered_address->ipv6Address;
        port = prefered_address->ipv6Port;
    }
    else {
        /* No version defined */
        return bytes;
    }

    coded_length = 4 + 1 + 1 + ip_length + 2 +
        1 + prefered_address->connection_id.id_len + 16;

    if (bytes == NULL || bytes + coded_length > bytes_max) {
        bytes = NULL;
    }
    else {
        picoformat_16(bytes, picoquic_tp_server_preferred_address);
        bytes += 2;
        picoformat_16(bytes, coded_length);
        bytes += 2;
        *bytes++ = ip_version;
        *bytes++ = ip_length;
        memcpy(bytes, ip_address, ip_length);
        bytes += ip_length;
        picoformat_16(bytes, port);
        bytes += 2;
        *bytes++ = prefered_address->connection_id.id_len;
        bytes += picoquic_format_connection_id(bytes, bytes_max - bytes,
            prefered_address->connection_id);
        memcpy(bytes, prefered_address->statelessResetToken, 16);
        bytes += 16;
    }

    return bytes;
}

size_t picoquic_decode_transport_param_prefered_address(uint8_t * bytes, size_t bytes_max,
    picoquic_tp_prefered_address_t * prefered_address)
{
    /* first compute the minimal length */
    size_t byte_index = 0;
    uint8_t cnx_id_length = 0;
    size_t minimal_length = 4 + 2 + 16 + 2 + 1 /* + prefered_address->connection_id.id_len */ + 16;
    size_t ret = 0;

    if (bytes_max >= minimal_length) {
        memcpy(prefered_address->ipv4Address, bytes + byte_index, 4);
        byte_index += 4;
        prefered_address->ipv4Port = PICOPARSE_16(bytes + byte_index);
        byte_index += 2;
        memcpy(prefered_address->ipv6Address, bytes + byte_index, 16);
        byte_index += 16;
        prefered_address->ipv6Port = PICOPARSE_16(bytes + byte_index);
        byte_index += 2;
        cnx_id_length = bytes[byte_index++];
        if (byte_index + cnx_id_length + 16 <= bytes_max &&
            cnx_id_length > 0 &&
            cnx_id_length == picoquic_parse_connection_id(bytes + byte_index, cnx_id_length,
                &prefered_address->connection_id)){
            byte_index += cnx_id_length;
            memcpy(prefered_address->statelessResetToken, bytes + byte_index, 16);
            byte_index += 16;
            ret = byte_index;
            prefered_address->is_defined = 1;
        }
    }

    return ret;
}

size_t picoquic_decode_transport_param_prefered_address_old(uint8_t * bytes, size_t bytes_max,
    picoquic_tp_prefered_address_t * prefered_address)
{
    /* first compute the minimal length */
    size_t byte_index = 0;
    uint8_t ip_length = 0;
    uint8_t cnx_id_length = 0;
    size_t minimal_length = (size_t)1 + 1u + ip_length + 2u + 1u + cnx_id_length + 16u;
    size_t ret = 0;
    int ip_version;

    memset(prefered_address, 0, sizeof(picoquic_tp_prefered_address_t));

    if (bytes_max >= minimal_length) {
        ip_version = bytes[byte_index++];
        ip_length = bytes[byte_index++];
        if (ip_length == 4 && ip_version == 4) {
            memcpy(prefered_address->ipv4Address, bytes + byte_index, ip_length);
            byte_index += ip_length;
            prefered_address->ipv4Port = PICOPARSE_16(bytes + byte_index);
            byte_index += 2;
        }
        else if (ip_length == 16 && ip_version == 6) {
            memcpy(prefered_address->ipv6Address, bytes + byte_index, ip_length);
            byte_index += ip_length;
            prefered_address->ipv6Port = PICOPARSE_16(bytes + byte_index);
            byte_index += 2;
        }
        cnx_id_length = bytes[byte_index++];
        if (byte_index + cnx_id_length + 16 <= bytes_max &&
            cnx_id_length == picoquic_parse_connection_id(bytes + byte_index, cnx_id_length,
                &prefered_address->connection_id)) {
            byte_index += cnx_id_length;
            memcpy(prefered_address->statelessResetToken, bytes + byte_index, 16);
            byte_index += 16;
            ret = byte_index;
        }
    }

    return ret;
}

int picoquic_prepare_transport_extensions(picoquic_cnx_t* cnx, int extension_mode,
    uint8_t* bytes, size_t bytes_length, size_t* consumed)
{
    int ret = 0;
    uint8_t * bytes_zero = bytes;
    uint8_t * bytes_param_start = bytes;
    uint8_t * bytes_max = bytes + bytes_length;

    if (bytes != NULL && bytes + 2 <= bytes_max) {
        bytes_param_start = bytes;
        bytes += 2;
    }

    bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_initial_max_stream_data_bidi_local,
        cnx->local_parameters.initial_max_stream_data_bidi_local);

    bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_initial_max_data,
        cnx->local_parameters.initial_max_data);

    if (cnx->local_parameters.initial_max_stream_id_bidir > 0) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_initial_max_streams_bidi,
            picoquic_prepare_transport_param_stream_id(
                cnx->local_parameters.initial_max_stream_id_bidir));
    }

    if (cnx->local_parameters.idle_timeout > 0) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_idle_timeout,
            cnx->local_parameters.idle_timeout);
    }

    bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_max_packet_size,
        cnx->local_parameters.max_packet_size);

    if (extension_mode == 1) {
        if (bytes != NULL && bytes + 4 + PICOQUIC_RESET_SECRET_SIZE < bytes_max) {
            picoformat_16(bytes, picoquic_tp_stateless_reset_token);
            bytes += 2;
            picoformat_16(bytes, PICOQUIC_RESET_SECRET_SIZE);
            bytes += 2;
            (void)picoquic_create_cnxid_reset_secret(cnx->quic, cnx->path[0]->local_cnxid, bytes);
            bytes += PICOQUIC_RESET_SECRET_SIZE;
        }
    }

    if (cnx->local_parameters.ack_delay_exponent != 3) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_ack_delay_exponent,
            cnx->local_parameters.ack_delay_exponent);
    }

    if (cnx->local_parameters.initial_max_stream_id_unidir > 0) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_initial_max_streams_uni,
            picoquic_prepare_transport_param_stream_id(cnx->local_parameters.initial_max_stream_id_unidir));
    }

    if (cnx->local_parameters.prefered_address.is_defined) {
        bytes = picoquic_encode_transport_param_prefered_address(
            bytes, bytes_max, &cnx->local_parameters.prefered_address);
    }

    if (cnx->local_parameters.migration_disabled != 0 && bytes != NULL) {
        if (bytes + 4 > bytes_max) {
            bytes = NULL;
        }
        else {
            picoformat_16(bytes, picoquic_tp_disable_migration);
            bytes += 2;
            picoformat_16(bytes, 0);
            bytes += 2;
        }
    }

    if (cnx->local_parameters.initial_max_stream_data_bidi_remote > 0) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_initial_max_stream_data_bidi_remote,
            cnx->local_parameters.initial_max_stream_data_bidi_remote);
    }

    if (cnx->local_parameters.initial_max_stream_data_uni > 0) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_initial_max_stream_data_uni,
            cnx->local_parameters.initial_max_stream_data_uni);
    }

    if (cnx->local_parameters.active_connection_id_limit > 0) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_active_connection_id_limit,
            cnx->local_parameters.active_connection_id_limit);
    }

    if (cnx->local_parameters.max_ack_delay != PICOQUIC_ACK_DELAY_MAX_DEFAULT) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_max_ack_delay,
            (cnx->local_parameters.max_ack_delay + 999) / 1000); /* Max ACK delay in milliseconds */
    }

    if (extension_mode == 1 && cnx->original_cnxid.id_len > 0 && bytes != NULL) {
        if (bytes + 4 + cnx->original_cnxid.id_len > bytes_max) {
            bytes = NULL;
        }
        else {
            picoformat_16(bytes, picoquic_tp_original_connection_id);
            bytes += 2;
            picoformat_16(bytes, cnx->original_cnxid.id_len);
            bytes += 2;
            memcpy(bytes, cnx->original_cnxid.id, cnx->original_cnxid.id_len);
            bytes += cnx->original_cnxid.id_len;
        }
    }

    if (!cnx->client_mode && cnx->local_parameters.max_datagram_size == 0 &&
        cnx->remote_parameters.max_datagram_size > 0) {
        cnx->local_parameters.max_datagram_size = PICOQUIC_MAX_PACKET_SIZE;
    }

    if (cnx->local_parameters.max_datagram_size > 0 && bytes != NULL) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_max_datagram_size,
            cnx->local_parameters.max_datagram_size);
    }

    if (cnx->grease_transport_parameters) {
        /* Do not use a purely random value, so we can repetitive tests */
        int n = 31 * (cnx->initial_cnxid.id[0] + cnx->client_mode) + 27;
        uint64_t v = cnx->initial_cnxid.id[1];
        while (n == picoquic_tp_test_large_chello ||
            n == picoquic_tp_enable_loss_bit_old ||
            n == picoquic_tp_enable_one_way_delay){
            n += 31;
        }
        v = (v << 8) + cnx->initial_cnxid.id[2];
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, n, v);
    }

    if (cnx->test_large_chello && bytes != NULL) {
        if (bytes + 4 + 1200 > bytes_max) {
            bytes = NULL;
        }
        else {
            picoformat_16(bytes, picoquic_tp_test_large_chello);
            bytes += 2;
            picoformat_16(bytes, 1200);
            bytes += 2;
            memset(bytes, 'Q', 1200);
            bytes += 1200;
        }
    }

    if (cnx->local_parameters.enable_loss_bit > 0 && bytes != NULL) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_enable_loss_bit,
            (cnx->local_parameters.enable_loss_bit > 1) ? 1 : 0);
    }

    if (cnx->local_parameters.enable_one_way_delay > 0 && bytes != NULL) {
        if (bytes + 4 > bytes_max) {
            bytes = NULL;
        }
        else {
            picoformat_16(bytes, picoquic_tp_enable_one_way_delay);
            bytes += 2;
            picoformat_16(bytes, 0);
            bytes += 2;
        }
    }

    if (bytes != NULL && cnx->local_parameters.min_ack_delay > 0) {
        bytes = picoquic_transport_param_type_varint_encode(bytes, bytes_max, picoquic_tp_min_ack_delay,
            cnx->local_parameters.min_ack_delay);
    }

    /* Finally, update the parameters length */
    if (bytes == NULL) {
        *consumed = 0;
        ret = PICOQUIC_ERROR_EXTENSION_BUFFER_TOO_SMALL;
    }
    else {
        picoformat_16(bytes_param_start, (uint16_t)(bytes - bytes_param_start - 2));
        *consumed = bytes - bytes_zero;

        if (cnx->quic->F_log) {
            picoquic_log_transport_extension(cnx->quic->F_log, cnx, 0, 1, bytes_zero, *consumed);
        }
    }

    return ret;
}


void picoquic_clear_transport_extensions(picoquic_cnx_t* cnx)
{
    cnx->remote_parameters.initial_max_stream_data_bidi_local = 0;
    picoquic_update_stream_initial_remote(cnx);
    cnx->remote_parameters.initial_max_stream_data_bidi_remote = 0;
    picoquic_update_stream_initial_remote(cnx);
    cnx->remote_parameters.initial_max_stream_data_uni = 0;
    picoquic_update_stream_initial_remote(cnx);
    cnx->remote_parameters.initial_max_data = 0;
    cnx->maxdata_remote = cnx->remote_parameters.initial_max_data;
    cnx->remote_parameters.initial_max_stream_id_bidir = 0;
    cnx->max_stream_id_bidir_remote = 0;
    cnx->remote_parameters.idle_timeout = 0;
    cnx->remote_parameters.max_packet_size = 1500;
    cnx->remote_parameters.ack_delay_exponent = 3;
    cnx->remote_parameters.initial_max_stream_id_unidir = 0;
    cnx->max_stream_id_unidir_remote = 0;
    cnx->remote_parameters.migration_disabled = 0;
    cnx->remote_parameters.max_ack_delay = PICOQUIC_ACK_DELAY_MAX_DEFAULT;
    cnx->remote_parameters.max_datagram_size = 0;
    cnx->remote_parameters.active_connection_id_limit = 0;
    cnx->remote_parameters.enable_loss_bit = 0;
    cnx->remote_parameters.enable_one_way_delay = 0;
}

int picoquic_receive_transport_extensions(picoquic_cnx_t* cnx, int extension_mode,
    uint8_t* bytes, size_t bytes_max, size_t* consumed)
{
    int ret = 0;
    size_t byte_index = 0;
    uint64_t present_flag = 0;
    picoquic_connection_id_t original_connection_id = picoquic_null_connection_id;

    cnx->remote_parameters_received = 1;
    picoquic_clear_transport_extensions(cnx);

    if (cnx->quic->F_log) {
        picoquic_log_transport_extension(cnx->quic->F_log, cnx, 1, 1, bytes, bytes_max);
    }

    if (byte_index + 2 > bytes_max) {
        ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
    }
    else {
        uint16_t extensions_size = PICOPARSE_16(bytes + byte_index);
        size_t extensions_end;
        byte_index += 2;
        extensions_end = byte_index + extensions_size;

        /* Set the parameters to default value zero */
        memset(&cnx->remote_parameters, 0, sizeof(picoquic_tp_t));
        /* Except for ack_delay_exponent, whose default is 3 */
        cnx->remote_parameters.ack_delay_exponent = 3;

        if (extensions_end > bytes_max) {
            ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
        }
        else {
            while (ret == 0 && byte_index < extensions_end) {
                if (byte_index + 4 > extensions_end) {
                    ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                }
                else {
                    uint16_t extension_type = PICOPARSE_16(bytes + byte_index);
                    uint16_t extension_length = PICOPARSE_16(bytes + byte_index + 2);
                    byte_index += 4;

                    if (byte_index + extension_length > extensions_end) {
                        ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                    }
                    else {
                        if (extension_type < 64) {
                            if ((present_flag & (1ull << extension_type)) != 0) {
                                /* Malformed, already present */
                                ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                            }
                            else {
                                present_flag |= (1ull << extension_type);
                            }
                        }

                        switch (extension_type) {
                        case picoquic_tp_initial_max_stream_data_bidi_local:
                            cnx->remote_parameters.initial_max_stream_data_bidi_local = 
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);

                            /* If we sent zero rtt data, the streams were created with the
                             * old value of the remote parameter. We need to update that.
                             */
                            picoquic_update_stream_initial_remote(cnx);
                            break;
                        case picoquic_tp_initial_max_stream_data_bidi_remote:
                            cnx->remote_parameters.initial_max_stream_data_bidi_remote = 
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            /* If we sent zero rtt data, the streams were created with the
                            * old value of the remote parameter. We need to update that.
                            */
                            picoquic_update_stream_initial_remote(cnx);
                            break;
                        case picoquic_tp_initial_max_stream_data_uni:
                            cnx->remote_parameters.initial_max_stream_data_uni = 
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            /* If we sent zero rtt data, the streams were created with the
                            * old value of the remote parameter. We need to update that.
                            */
                            picoquic_update_stream_initial_remote(cnx);
                            break;
                        case picoquic_tp_initial_max_data:
                            cnx->remote_parameters.initial_max_data = 
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            cnx->maxdata_remote = cnx->remote_parameters.initial_max_data;
                            break;
                        case picoquic_tp_initial_max_streams_bidi: {
                            uint64_t old_limit = cnx->max_stream_id_bidir_remote;
                            cnx->remote_parameters.initial_max_stream_id_bidir =
                                picoquic_decode_transport_param_stream_id(
                                    picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret), extension_mode,
                                    PICOQUIC_STREAM_ID_BIDIR);

                            cnx->max_stream_id_bidir_remote =
                                (cnx->remote_parameters.initial_max_stream_id_bidir == 0xFFFFFFFF) ? 0 : cnx->remote_parameters.initial_max_stream_id_bidir;

                            picoquic_add_output_streams(cnx, old_limit, cnx->max_stream_id_bidir_remote, 1);
                            break;
                        }
                        case picoquic_tp_idle_timeout:
                            cnx->remote_parameters.idle_timeout = (uint32_t)
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            break;

                        case picoquic_tp_max_packet_size:
                            cnx->remote_parameters.max_packet_size = (uint32_t)
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            break;
                        case picoquic_tp_stateless_reset_token:
                            if (extension_mode != 1) {
                                ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                            }
                            else if (extension_length != PICOQUIC_RESET_SECRET_SIZE) {
                                ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                            }
                            else {
                                memcpy(cnx->path[0]->reset_secret, bytes + byte_index, PICOQUIC_RESET_SECRET_SIZE);
                            }
                            break;
                        case picoquic_tp_ack_delay_exponent:
                            cnx->remote_parameters.ack_delay_exponent = (uint8_t)
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            break;
                        case picoquic_tp_initial_max_streams_uni: {
                            uint64_t old_limit = cnx->max_stream_id_unidir_remote;
                            cnx->remote_parameters.initial_max_stream_id_unidir =
                                picoquic_decode_transport_param_stream_id(
                                    picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret), extension_mode,
                                    PICOQUIC_STREAM_ID_UNIDIR);

                            cnx->max_stream_id_unidir_remote =
                                (cnx->remote_parameters.initial_max_stream_id_unidir == 0xFFFFFFFF) ? 0 : cnx->remote_parameters.initial_max_stream_id_unidir;
                            picoquic_add_output_streams(cnx, old_limit, cnx->max_stream_id_unidir_remote, 0);
                            break;
                        }
                        case picoquic_tp_server_preferred_address:
                        {
                            size_t coded_length = picoquic_decode_transport_param_prefered_address(
                                bytes + byte_index, extension_length, &cnx->remote_parameters.prefered_address);

                            if (coded_length != extension_length) {
                                ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                            }
                            break;
                        }
                        case picoquic_tp_disable_migration:
                            if (extension_length != 0) {
                                ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                            }
                            else {
                                cnx->remote_parameters.migration_disabled = 1;
                            }
                            break;
                        case picoquic_tp_max_ack_delay:
                            cnx->remote_parameters.max_ack_delay = (uint32_t)
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret) * 1000;
                            if (cnx->remote_parameters.max_ack_delay > PICOQUIC_MAX_ACK_DELAY_MAX_MS * 1000) {
                                ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                            }
                            break;
                        case picoquic_tp_original_connection_id:
                            if (extension_length >= 256) {
                                ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                            }
                            else {
                                original_connection_id.id_len = (uint8_t)picoquic_parse_connection_id(bytes + byte_index, (uint8_t)extension_length, &original_connection_id);
                                if (original_connection_id.id_len == 0) {
                                    ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                                }
                            }
                            break;
                        case picoquic_tp_active_connection_id_limit:
                            cnx->remote_parameters.active_connection_id_limit = (uint32_t)
                            picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            /* TODO: may need to check the value, but conditions are unclear */
                            break;
                        case picoquic_tp_max_datagram_size:
                            cnx->remote_parameters.max_datagram_size = (uint32_t)
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            break;
                        case picoquic_tp_enable_loss_bit_old:
                            /* The old loss bit definition is obsolete */
                            break;
                        case picoquic_tp_enable_loss_bit: {
                            uint64_t enabled = picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            if (ret == 0) {
                                if (enabled == 0) {
                                    /* Send only variant of loss bit */
                                    cnx->remote_parameters.enable_loss_bit = 1;
                                }
                                else if (enabled == 1) {
                                    /* Both send and receive are enabled */
                                    cnx->remote_parameters.enable_loss_bit = 2;
                                }
                                else {
                                    /* Only values 0 and 1 are expected */
                                    ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                                }
                            }
                            break;
                        }
                        case picoquic_tp_enable_one_way_delay:
                            if (extension_length != 0) {
                                ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
                            }
                            else {
                                cnx->remote_parameters.enable_one_way_delay = 1;
                            }
                            break;
                        case picoquic_tp_min_ack_delay:
                            cnx->remote_parameters.min_ack_delay = 
                                picoquic_transport_param_varint_decode(cnx, bytes + byte_index, extension_length, &ret);
                            /* Values of 0 and values larger that 2^24 are not expected */
                            if (ret == 0 &&
                                (cnx->remote_parameters.min_ack_delay == 0 ||
                                    cnx->remote_parameters.min_ack_delay > PICOQUIC_ACK_DELAY_MIN_MAX_VALUE)) {
                                ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PROTOCOL_VIOLATION, 0);
                            }
                            else {
                                if (cnx->local_parameters.max_ack_delay > 0) {
                                    cnx->is_ack_frequency_negotiated = 1;
                                }
                            }
                            break;
                        default:
                            /* ignore unknown extensions */
                            break;
                        }

                        if (ret == 0) {
                            byte_index += extension_length;
                        }
                    }
                }
            }
        }
    }

    /* Compute the negotiated version of the time out 
     */
    cnx->idle_timeout = cnx->local_parameters.idle_timeout;
    if (cnx->idle_timeout == 0 || 
        (cnx->remote_parameters.idle_timeout > 0 && cnx->remote_parameters.idle_timeout < cnx->idle_timeout)) {
        cnx->idle_timeout = cnx->remote_parameters.idle_timeout;
    }
    if (cnx->idle_timeout == 0) {
        cnx->idle_timeout = UINT64_MAX;
    }
    else {
        cnx->idle_timeout *= 1000;
    }

    if (ret == 0 && (present_flag & (1ull << picoquic_tp_max_ack_delay)) == 0) {
        cnx->remote_parameters.max_ack_delay = PICOQUIC_ACK_DELAY_MAX_DEFAULT;
    }

    if (ret == 0 && (present_flag & (1ull << picoquic_tp_active_connection_id_limit)) == 0) {
        if (cnx->path[0]->local_cnxid.id_len == 0) {
            cnx->remote_parameters.active_connection_id_limit = 0;
        }
        else {
            cnx->remote_parameters.active_connection_id_limit = PICOQUIC_NB_PATH_DEFAULT;
        }
    }

    /* Clients must not include reset token, server address, or original cid  */

    if (ret == 0 && extension_mode == 0 &&
        ((present_flag & (1ull << picoquic_tp_stateless_reset_token)) != 0 ||
        (present_flag & (1ull << picoquic_tp_server_preferred_address)) != 0 ||
        (present_flag & (1ull << picoquic_tp_original_connection_id)) != 0)) {
        ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
    }

    /* Original connection ID should be NULL at the client and at the server if
     * there was no retry, should exactly match otherwise. Mismatch is trated
     * as a transport parameter error */
    if (ret == 0 && extension_mode == 1) {
        if (cnx->original_cnxid.id_len != 0 &&
            picoquic_compare_connection_id(&cnx->original_cnxid, &original_connection_id) != 0) {
            ret = picoquic_connection_error(cnx, PICOQUIC_TRANSPORT_PARAMETER_ERROR, 0);
        }
    }

    /* Loss bit is only enabled if negotiated by both parties */
    cnx->is_loss_bit_enabled_outgoing = (cnx->local_parameters.enable_loss_bit > 1) && (cnx->remote_parameters.enable_loss_bit > 0);
    cnx->is_loss_bit_enabled_incoming = (cnx->local_parameters.enable_loss_bit > 0) && (cnx->remote_parameters.enable_loss_bit > 1);

    /* One way delay only enabled if asked by client and accepted by server */
    if (cnx->client_mode) {
        cnx->is_one_way_delay_enabled = cnx->local_parameters.enable_one_way_delay && cnx->remote_parameters.enable_one_way_delay;
    }
    else if (cnx->remote_parameters.enable_one_way_delay) {
        cnx->local_parameters.enable_one_way_delay = 1;
        cnx->is_one_way_delay_enabled = 1;
    }

    /* ACK Frequency is only enabled on server if negotiated by client */
    if (!cnx->client_mode && !cnx->is_ack_frequency_negotiated) {
        cnx->local_parameters.min_ack_delay = 0;
    }

    *consumed = byte_index;

    return ret;
}
