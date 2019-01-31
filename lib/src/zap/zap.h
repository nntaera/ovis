/* -*- c-basic-offset: 8 -*-
 * Copyright (c) 2013-2017 National Technology & Engineering Solutions
 * of Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with
 * NTESS, the U.S. Government retains certain rights in this software.
 * Copyright (c) 2013-2017 Open Grid Computing, Inc. All rights reserved.
 *
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the U.S. Government.
 * Export of this program may require a license from the United States
 * Government.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the BSD-type
 * license below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *      Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *      Neither the name of Sandia nor the names of any contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *      Neither the name of Open Grid Computing nor the names of any
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *      Modified source versions must be plainly marked as such, and
 *      must not be misrepresented as being the original software.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \mainpage Zap
 *
 * Zap is a transport independent API for RDMA capable transports. It
 * is a thin wrapper layer that allows user-mode applications to use
 * RDMA operations without regard to network address or API
 * differences between transports. The Zap connection model is similar
 * to sockets for SOCK_STREAM connections. On the passive side, an
 * endpoint is created and the application 'listens' on a local
 * address and port. On the active side, an endpoint is created and a
 * connection is requested.
 *
 * The Connection Management API are as follows:
 *
 * \li \b zap_new() Create a new Zap endpoint.
 * \li \b zap_listen() Listen for incoming connect requests on a Zap endpoint.
 * \li \b zap_connect() Request a connection with a remote peer.
 * \li \b zap_accept() Accept a connection from a remote peer.
 * \li \b zap_close() Close a connection with a remote peer on a Zap endpoint
 *
 * Zap is event-driven. Application will be notifiedt via a callback function
 * per occurring events (e.g. read complete, data received, connection
 * requested, etc). Please see \c ::zap_cb_fn_t for more information.
 */
#ifndef __ZAP_H__
#define __ZAP_H__
#include <inttypes.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define ZAP_MAX_TRANSPORT_NAME_LEN 16

typedef struct zap_ep *zap_ep_t;
typedef struct zap *zap_t;
typedef void (*zap_log_fn_t)(const char *fmt, ...);
typedef struct zap_map *zap_map_t;

#define ZAP_EVENT_BAD -1

typedef enum zap_event_type {
	/*! An incoming connect request is ready to be accepted or rejected. */
	ZAP_EVENT_CONNECT_REQUEST = 1,
	/*! A connect request failed due to a network error. */
	ZAP_EVENT_CONNECT_ERROR,
	/*! An active request initiated with \c zap_connect() was accepted. */
	ZAP_EVENT_CONNECTED,
	/*! A connect request was rejected by the peer.  */
	ZAP_EVENT_REJECTED,
	/*! A connection endpoint has been disconnected. */
	ZAP_EVENT_DISCONNECTED,
	/*! Data has been received. */
	ZAP_EVENT_RECV_COMPLETE,
	/*! An \c zap_read() request has completed. */
	ZAP_EVENT_READ_COMPLETE,
	/*! An \c zap_write() request has completed. */
	ZAP_EVENT_WRITE_COMPLETE,
	/*! The peer has shared a buffer with \c zap_share(). */
	ZAP_EVENT_RENDEZVOUS,
	/*! Last event (dummy) */
	ZAP_EVENT_LAST
} zap_event_type_t;

typedef enum zap_err_e {
	/*! No error. */
	ZAP_ERR_OK,
	/*! Invalid parameters. */
	ZAP_ERR_PARAMETER,
	/*! Miscellaneous transport error. */
	ZAP_ERR_TRANSPORT,
	/*! Miscellaneous endpoint error. */
	ZAP_ERR_ENDPOINT,
	/*! Address resolve error. */
	ZAP_ERR_ADDRESS,
	/*! Route resolve error. */
	ZAP_ERR_ROUTE,
	/*! Memory mapping error. */
	ZAP_ERR_MAPPING,
	/*! Miscellaneous resource error (usually out of memory) */
	ZAP_ERR_RESOURCE,
	/*! Required resource busy (for example, trying to listen to the same
	 * port twice). */
	ZAP_ERR_BUSY,
	/*! Not enough space in the transport buffer. */
	ZAP_ERR_NO_SPACE,
	/*! Invalid map type. */
	ZAP_ERR_INVALID_MAP_TYPE,
	/*! Connection error. */
	ZAP_ERR_CONNECT,
	/*! Endpoint not connected. */
	ZAP_ERR_NOT_CONNECTED,
	/*! Host unreachable. */
	ZAP_ERR_HOST_UNREACHABLE,
	/*! Mapping access error due to the length of local-side mapping. */
	ZAP_ERR_LOCAL_LEN,
	/*! Miscellaneous operation error on local-side memory mapping. */
	ZAP_ERR_LOCAL_OPERATION,
	/*! Mapping access error due to local-side mapping permission. */
	ZAP_ERR_LOCAL_PERMISSION,
	/*! Remote-side map handle is invalid. */
	ZAP_ERR_REMOTE_MAP,
	/*! Mapping access error due to the length of remote-side mapping. */
	ZAP_ERR_REMOTE_LEN,
	/*! Mapping access error due to remote-side mapping permission. */
	ZAP_ERR_REMOTE_PERMISSION,
	/*! Miscellaneous operation error on remote-side memory mapping. */
	ZAP_ERR_REMOTE_OPERATION,
	/*! Retry exceed error. */
	ZAP_ERR_RETRY_EXCEEDED,
	/*! Connection time out error. */
	ZAP_ERR_TIMEOUT,
	/*! Transport flush error. */
	ZAP_ERR_FLUSH,
	/*! Last error (dummy). */
	ZAP_ERR_LAST
} zap_err_t;

static char *__zap_err_str[] = {
	"ZAP_ERR_OK",
	"ZAP_ERR_PARAMETER",
	"ZAP_ERR_TRANSPORT",
	"ZAP_ERR_ENDPOINT",
	"ZAP_ERR_ADDRESS",
	"ZAP_ERR_ROUTE",
	"ZAP_ERR_MAPPING",
	"ZAP_ERR_RESOURCE",
	"ZAP_ERR_BUSY",
	"ZAP_ERR_NO_SPACE",
	"ZAP_ERR_INVALID_MAP_TYPE",
	"ZAP_ERR_CONNECT",
	"ZAP_ERR_NOT_CONNECTED",
	"ZAP_ERR_HOST_UNREACHABLE",
	"ZAP_ERR_LOCAL_LEN",
	"ZAP_ERR_LOCAL_OPERATION",
	"ZAP_ERR_LOCAL_PERMISSION",
	"ZAP_ERR_REMOTE_MAP",
	"ZAP_ERR_REMOTE_LEN",
	"ZAP_ERR_REMOTE_OPERATION",
	"ZAP_ERR_REMOTE_PERMISSION",
	"ZAP_ERR_RETRY_EXCEEDED",
	"ZAP_ERR_TIMEOUT",
	"ZAP_ERR_FLUSH",
	"ZAP_ERR_LAST"
};

static inline
const char* zap_err_str(enum zap_err_e e)
{
	if (e < 0 || e > ZAP_ERR_LAST)
		return "ZAP_ERR_UNKNOWN";
	return __zap_err_str[e];
}

/**
 * Convert a given errno \c e into ::zap_err_e.
 * \param e The errno.
 * \returns ::zap_err_e.
 */
enum zap_err_e zap_errno2zerr(int e);

typedef struct zap_event {
	/*! Event type */
	zap_event_type_t type;
	/*! Event status in case of error */
	zap_err_t status;
	/*! Mapping information for \c ::ZAP_EVENT_RENDEZVOUS event */
	zap_map_t map;
	/**
	 * Pointer to message data. This buffer is owned by Zap and
	 * may be freed when the callback returns.
	 */
	unsigned char *data;
	/*! The length of \c #data in bytes */
	size_t data_len;
	/*! Application provided context */
	void *context;
} *zap_event_t;

/**
 * Zap callback function.
 *
 * When an event (please see \c ::zap_event) occur to an endpoint, zap will call
 * the associated callback function to notify the application. Data associated
 * with the event is accessible via various fields of \b ev.
 *
 * For all event types, \c ev->type determines the event type (see zap_event_t),
 * and \c ev->status determines the status of the operation.
 *
 * The event-specific data is as follows:
 *
 * - \c ::ZAP_EVENT_CONNECT_REQUEST
 *   - N/A
 * - \c ::ZAP_EVENT_CONNECT_ERROR
 *   - N/A
 * - \c ::ZAP_EVENT_CONNECTED
 *   - N/A
 * - \c ::ZAP_EVENT_REJECTED
 *   - N/A
 * - \c ::ZAP_EVENT_DISCONNECTED
 *   - N/A
 * - \c ::ZAP_EVENT_RECV_COMPLETE
 *   - \b\c ev->data contains the received data. zap own \c ev->data, and
 *     application can only read this memory.
 *   - \b\c ev->data_len indicates the length of the received data.
 * - \c ::ZAP_EVENT_READ_COMPLETE
 *   - \b\c ev->context refers to application-provided context when
 *     \c ::zap_read() is called.
 * - \c ::ZAP_EVENT_WRITE_COMPLETE
 *   - \b\c ev->context refers to application-provided context when
 *     \c ::zap_write() is called.
 * - \c ::ZAP_EVENT_RENDEZVOUS
 *   - \b\c ev->map refers to automatically-created remote memory map associated
 *     witllthe event. The application owns the map and has to free it when it
 *     is not used.
 *   - \b\c ev->data contain a tag-along message of \c zap_share() operation.
 *   - \b\c ev->data_len is the length of the tag-along message.
 */
typedef void (*zap_cb_fn_t)(zap_ep_t zep, zap_event_t ev);

#define zap_ptr_(_t, _p, _o) (_t *)&((char *)_p)[_o]

typedef struct zap_mem_info {
	void *start;
	size_t len;
} *zap_mem_info_t;
typedef zap_mem_info_t (*zap_mem_info_fn_t)(void);

/**
 * \brief Get the context in the endpoint
 *
 * \param ep The handle to the endpoint which contains the ucontext
 */
void *zap_get_ucontext(zap_ep_t ep);

/**
 * \brief Set the context in the endpoint
 *
 * \param ep The handle to the endpoint in which the ucontext will be set.
 * \param context The user context
 */
void zap_set_ucontext(zap_ep_t ep, void *context);

/** \brief Get a handle to a transport.
 *
 * Get a transport handle to a transport.
 *
 * \param name	The transport name, e.g. 'rdma', 'sock', etc...
 * \param log_fn Pointer to the function to uuse for logging errors or status
 * \param mfn	Pointer to a function that returns the mapped memory info
 * \return Pointer to the transport or NULL if the transport does not exist.
 */
zap_t zap_get(const char *name, zap_log_fn_t log_fn, zap_mem_info_fn_t map_info_fn);

/** \brief Returns the max send message size.
 *
 * Returns the max message size supported by the transport. Messages larger
 * than the returned value cannot be sent using zap_send or received from the
 * remote peer.
 *
 * \param t	The transport handle.
 * \return !0	The max message size. A zero value indicates a bad transport handle.
 */
size_t zap_max_msg(zap_t z);

/** \brief Create a new endpoint on a transport.
 *
 * Create an endpoint and initialize the reference count to 1. The
 * application should call zap_free() when it is finished with the
 * endpoint.
 *
 * \param z	The Zap transport handle
 * \param cb	Ponter to a function to receive asynchronous events on the endpoint.
 * \return Pointer to the new endpoint or NULL if there was an error
 */
zap_ep_t zap_new(zap_t z, zap_cb_fn_t cb);

#define ZAP_EP_PRIO_NORMAL	0
#define ZAP_EP_PRIO_HIGH	1

/** \brief Set endpoint priority
 *
 * An endpoint can be HIGH or NORMAL priority. By default all
 * endpoints are NORMAL priority. Events on HIGH priority endpoints
 * are queued ahead of events for NORMAL priority endpoints.
 *
 * A non-zero value for the \c prio argument will set the endpoint to
 * HIGH priority.
 *
 * \param ep	The Zap endpoint handle
 * \param prio	The priority level
 */
void zap_set_priority(zap_ep_t ep, int prio);

/** \brief Release an endpoint
 *
 * Drop the implicit zap_new() reference. This is functionally
 * equivalent to zap_put_ep().
 *
 * Note that outstanding I/O may hold references on the endpoint and
 * this does not initiate a disconnect. See the zap_close() function
 * for initiating a disconnect on an endpoint.
 *
 * \param ep	The endpoint handle
 */
void zap_free(zap_ep_t ep);

/** \brief Request a connection with a remote peer.
 *
 * \param ep	The transport handle.
 * \param sa	Pointer to a sockaddr containing the address of the
 *		remote peer.
 * \param sa_len Size of the sockaddr in bytes.
 * \param data	Buffer containing connect data for peer.
 * \param data_len Size of message in bytes.
 * \return 0	Success
 * \return !0	A Zap error code. See zap_err_t.
 */
zap_err_t zap_connect(zap_ep_t ep, struct sockaddr *sa, socklen_t sa_len,
		      char *data, size_t data_len);

/**
 * A syncrhonous version of ::zap_connect()
 *
 * \param ep	The transport handle.
 * \param sa	Pointer to a sockaddr containing the address of the
 *		remote peer.
 * \param sa_len Size of the sockaddr in bytes.
 * \param data  Connect data.
 * \param data_len Connect data length in bytes.
 * \return 0	Success
 * \return !0	A Zap error code. See zap_err_t.
 */
zap_err_t zap_connect_sync(zap_ep_t ep, struct sockaddr *sa, socklen_t sa_len,
			   char *data, size_t data_len);

/**
 * Synchronous connect by name
 *
 * \param ep	The transport handle.
 * \param host  The host name
 * \param port  The service/port number
 * \param data  Connect data.
 * \param data_len Connect data length in bytes.
 *
 * \return 0	Success
 * \return !0	A Zap error code. See zap_err_t.
 */
zap_err_t zap_connect_by_name(zap_ep_t ep, const char *host_port, const char *port,
			      char *data, size_t data_len);

/** \brief Accept a connection request from a remote peer.
 *
 * In case of an error, zap will cleanup the endpoint. Application doesn't
 * need to do anything.
 *
 * \param ep	The endpoint handle.
 * \param cb	Pointer to a function to receive asynchronous events on the endpoint.
 * \param data	Pointer to connection data.
 * \param data_len The size of the connection data in bytes.
 *
 * \return 0	Success
 * \return !0	A Zap error code. See zap_err_t.
 */
zap_err_t zap_accept(zap_ep_t ep, zap_cb_fn_t cb, char *data, size_t data_len);

/** \brief Return the local and remote addresses for an endpoint.
 *
 * \param ep	The endpoint handle.
 * \return 0	Success
 * \return !0	A Zap error code. See zap_err_t.
 */
zap_err_t zap_get_name(zap_ep_t ep, struct sockaddr *local_sa,
		       struct sockaddr *remote_sa, socklen_t *sa_len);

/**
 * \brief Take a reference on the endpoint
 *
 * Increment the endpoint's internal reference count. The transport
 * cannot be destroyed while the reference count is non-zero.
 *
 * \param ep	The endpoint handle
 */
void zap_get_ep(zap_ep_t ep);

/**
 * \brief Drop a reference on the endpoint
 *
 * Decrement the endpoint's internal reference count. If the
 * transport's internal reference count goes to zero, the endpoint
 * will be destroyed.
 *
 * \param ep	The endpoint handle
 */
void zap_put_ep(zap_ep_t ep);

/** \brief Reject a connection request from a remote peer.
 *
 * Disconnect from the peer and drop the implicit reference created by
 * the transport when this endpoint was created in response to the
 * peer's connection request.
 *
 * In case of an error, Zap will cleanup the endpoint. Application doesn't
 * need to do anything.
 *
 * \param ep	The transport handle.
 * \param data	Pointer to connection data.
 * \param data_len The size of the connection data in bytes.
 *
 * \return 0	Success
 * \return !0	A Zap error code. See zap_err_t.
 */
zap_err_t zap_reject(zap_ep_t ep, char *data, size_t data_len);

/** \brief Listen for incoming connection requests
 *
 * \param ep	The endpoint handle
 * \param sa	The sockaddr for the local interface on which to
 *		listen. Note that a zero for s_addr implies listen on all
 *		interfaces.
 * \param ep	The size of the sockaddr in bytes
 * \retval 0	Success
 * \retval EINVAL The specified endpoint or address was invalid
 * \retval EINUSE The specified port number is in use
*/
zap_err_t zap_listen(zap_ep_t ep, struct sockaddr *sa, socklen_t sa_len);

/** Close the connection
 *
 * Initiate a close of a connection with the remote peer. Any
 * outstanding, incomplete I/O will be completed with ZAP_EVENT_FLUSH.
 * If the endpoint was connected, the application will receive a
 * ZAP_EVENT_DISCONNECTED event when the close is complete.
 *
 * The zap_close() function will release all outstanding mappings all
 * outstanding active mappings associated with the endpoint. After
 * calling this function, the application must not touch any mappings
 * that were created directly via zap_map() or indirectly through the
 * receipt of a rendezvous from a remote peer on this endpoint.
 *
 * \param ep	The endpoint handle
 */
zap_err_t zap_close(zap_ep_t ep);

/** \brief Send data to the peer
 *
 * \param ep	The endpoint handle
 * \param buf	Pointer to the buffer to send
 * \param sz	The number of bytes from the buffer to send
 * \returns	ZAP_ERR_OK on success, or a zap_err_t value indicating the
 *		reason for failure.
 */
zap_err_t zap_send(zap_ep_t ep, void *buf, size_t sz);

/** \brief RDMA write data to a remote buffer */
zap_err_t zap_write(zap_ep_t t,
		    zap_map_t src_map, void *src,
		    zap_map_t dst_map, void *dst, size_t sz,
		    void *context);

/** \brief RDMA read data from a remote buffer */
zap_err_t zap_read(zap_ep_t z,
		   zap_map_t src_map, char *src,
		   zap_map_t dst_map, char *dst, size_t sz,
		   void *context);

/** \brief Zap buffer mapping access rights. */
typedef enum zap_access {
	ZAP_ACCESS_NONE = 0,	/*! Only local access is allowed */
	ZAP_ACCESS_WRITE = 1,	/*! Remote write is granted  */
	ZAP_ACCESS_READ = 2,	/*! Remote read is granted */
} zap_access_t;

typedef enum zap_map_type {
	ZAP_MAP_LOCAL = 1,	  /*! A local map acquired with zap_map */
	ZAP_MAP_REMOTE = 2,  /*! A map received from a remote peer vis zap_share  */
} zap_map_type_t;

/** \brief Allocate a remote buffer
 *
 * Map an area of local memory so that it can be used in RDMA
 * operations. This mapping can be shared with a remote peer using the
 * \c zap_share interface.
 *
 * A buffer must be mapped if it is either the data source or the data
 * sink of an RDMA WRITE request submitted with \c zap_write_start or
 * an RDMA READ request submitted with \c zap_read_start. Note that a
 * buffer does not need to be shared using the \c zap_share_map if it
 * is only used locally, i.e. it is not the target of an RDMA WRITE or
 * the source of an RDMA READ.
 *
 * \param ep	The endpoint handle
 * \param pm	Pointer to the map handle
 * \param addr	The memory address of the buffer
 * \param sz	The size in bytes of the buffer
 * \param acc	The remote access flags of the buffer
 * \return 0	Success, or a non-zero zap_err_t error code.
 */
zap_err_t zap_map(zap_ep_t ep, zap_map_t *pm,
		  void *addr, size_t sz, zap_access_t acc);

/**
 * Add a reference to a zap mapping
 *
 * This is needed if the mapping is going to be shared by multiple
 * clients and it is expected that zap_unmap() may be called multiple
 * times.
 *
 * The application must call zap_unmap() once for each time it obtains
 * a map from zap_map(), zap_get() or ZAP_EVENT_RENDEZVOUS.
 *
 * \param map The zap map obtained from zap_map()
 */
zap_map_t zap_map_get(zap_map_t map);

/** \brief Return the length of the mapped buffer.
 *
 * \param map	The map handle.
 * \returns !0	The size of of the mapped buffer in bytes.
 * \returns 0	The map is invalid..
 */
size_t zap_map_len(zap_map_t map);

/** \brief Return the address of the mapped buffer.
 *
 * Returns the address of the start of the mapped buffer as a void
 * pointer. The rationale for void * is to allow assignment to a
 * pointer to a data structure without an explicit cast.
 *
 * \param map	The map handle.
 * \returns !0	Pointer to the start of the mapped buffer.
 * \returns NULL The map is invalid.
 */
char *zap_map_addr(zap_map_t map);

/** \brief Unmap a buffer previously mapped with \c zap_map_buf
 *
 * Unmap a buffer previously mapped with \c zap_map_buf. The buffer
 * will no longer be accessible to the remote peer. Note that the peer is
 * not notified that the mapping has been removed.
 *
 * \param ep	The endpoint handle
 * \param map	The buffer mapping returned by a previous call to \c
 *		zap_map_buf
 * \return 0	Success, or a non-zero zap_err_t error code.
 */
zap_err_t zap_unmap(zap_ep_t ep, zap_map_t map);

/** \brief Share a mapping with a remote peer
 *
 * Send the remote peer your zap_buf_t information. If successful, the
 * peer will be able to RDMA READ and/or WRITE into the buffer. Note
 * that the access they are granted depends on the zap_access_t flags
 * provide when the buffer was mapped with \c zap_map_buf.
 *
 * \param ep The endpoint handle
 * \param m A Zap buffer mapping returned by \c zap_map_buf.
 * \param msg The message that will tag along with share operation.
 * \param msg_len The length of the message.
 *
 * \returns ZAP_ERR_OK if there is no errors.
 * \returns zap_error_code on error.
 */
zap_err_t zap_share(zap_ep_t ep, zap_map_t m, const char *msg, size_t msg_len);

const char* zap_event_str(enum zap_event_type e);

#endif
