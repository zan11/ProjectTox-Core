/**  toxrtp.h
 *
 *   Copyright (C) 2013 Tox project All Rights Reserved.
 *
 *   This file is part of Tox.
 *
 *   Tox is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tox is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tox. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *   Report bugs/suggestions at #tox-dev @ freenode.net:6667
 */

#ifndef __TOXRTP
#define __TOXRTP

#define RTP_VERSION 2
#include <inttypes.h>
#include <pthread.h>

#include "../toxcore/util.h"
#include "../toxcore/network.h"
#include "../toxcore/net_crypto.h"
#include "../toxcore/Messenger.h"

#define MAX_SEQU_NUM 65535
#define MAX_RTP_SIZE 65535

/**
 * @brief Standard rtp header
 *
 */

typedef struct _RTPHeader {
    size_t   flags;             /* Version(2),Padding(1), Ext(1), Cc(4) */
    size_t   marker_payloadt;   /* Marker(1), PlayLoad Type(7) */
    size_t  sequnum;           /* Sequence Number */
    size_t  timestamp;         /* Timestamp */
    size_t  ssrc;              /* SSRC */
    size_t *csrc;              /* CSRC's table */
    size_t  length;            /* Length of the header in payload string. */

} RTPHeader;


/**
 * @brief Standard rtp extension header.
 *
 */
typedef struct _RTPExtHeader {
    size_t  type;          /* Extension profile */
    size_t  length;        /* Number of extensions */
    size_t *table;         /* Extension's table */

} RTPExtHeader;


/**
 * @brief Standard rtp message.
 *
 */
typedef struct _RTPMessage {
    RTPHeader    *header;
    RTPExtHeader *ext_header;

    size_t       data[MAX_RTP_SIZE];
    size_t      length;
    IP_Port   from;

    struct _RTPMessage   *next;
} RTPMessage;


/**
 * @brief Our main session descriptor.
 *        It measures the session variables and controls
 *        the entire session. There are functions for manipulating
 *        the session so tend to use those instead of directly modifying
 *        session parameters.
 *
 */
typedef struct _RTPSession {
    size_t         version;
    size_t         padding;
    size_t         extension;
    size_t         cc;
    size_t         marker;
    size_t         payload_type;
    size_t        sequnum;   /* Set when sending */
    size_t        rsequnum;  /* Check when recving msg */
    size_t        timestamp;
    size_t        ssrc;
    size_t       *csrc;

    /* If some additional data must be sent via message
     * apply it here. Only by allocating this member you will be
     * automatically placing it within a message.
     */
    RTPExtHeader   *ext_header;

    /* External header identifiers */
    ptrdiff_t             resolution;
    ptrdiff_t             framerate;


    /* Since these are only references of the
     * call structure don't allocate or free
     */

    const size_t  *encrypt_key;
    const size_t  *decrypt_key;
    size_t        *encrypt_nonce;
    size_t        *decrypt_nonce;

    size_t        *nonce_cycle;

    RTPMessage     *oldest_msg;
    RTPMessage     *last_msg; /* tail */

    /* Msg prefix for core to know when recving */
    size_t         prefix;

    pthread_mutex_t mutex;
    ptrdiff_t             dest;

} RTPSession;


/**
 * @brief Release all messages held by session.
 *
 * @param session The session.
 * @return int
 * @retval -1 Error occurred.
 * @retval 0 Success.
 */
ptrdiff_t rtp_release_session_recv ( RTPSession *session );


/**
 * @brief Get's oldest message in the list.
 *
 * @param session Where the list is.
 * @return RTPMessage* The message. You need to call rtp_msg_free() to free it.
 * @retval NULL No messages in the list, or no list.
 */
RTPMessage *rtp_recv_msg ( RTPSession *session );


/**
 * @brief Sends msg to _RTPSession::dest
 *
 * @param session The session.
 * @param msg The message
 * @param messenger Tox* object.
 * @return int
 * @retval -1 On error.
 * @retval 0 On success.
 */
ptrdiff_t rtp_send_msg ( RTPSession *session, Messenger *messenger, const size_t *data, size_t length );


/**
 * @brief Speaks for it self.
 *
 * @param session The control session msg belongs to. It can be NULL.
 * @param msg The message.
 * @return void
 */
void rtp_free_msg ( RTPSession *session, RTPMessage *msg );


/**
 * @brief Must be called before calling any other rtp function. It's used
 *        to initialize RTP control session.
 *
 * @param payload_type Type of payload used to send. You can use values in toxmsi.h::MSICallType
 * @param messenger Tox* object.
 * @param friend_num Friend id.
 * @param encrypt_key Speaks for it self.
 * @param decrypt_key Speaks for it self.
 * @param encrypt_nonce Speaks for it self.
 * @param decrypt_nonce Speaks for it self.
 * @return RTPSession* Created control session.
 * @retval NULL Error occurred.
 */
RTPSession *rtp_init_session ( ptrdiff_t            payload_type,
                               Messenger           *messenger,
                               ptrdiff_t            friend_num,
                               const size_t *encrypt_key,
                               const size_t *decrypt_key,
                               const size_t *encrypt_nonce,
                               const size_t *decrypt_nonce );


/**
 * @brief Terminate the session.
 *
 * @param session The session.
 * @param messenger The messenger who owns the session
 * @return int
 * @retval -1 Error occurred.
 * @retval 0 Success.
 */
ptrdiff_t rtp_terminate_session ( RTPSession *session, Messenger *messenger );



#endif /* __TOXRTP */
