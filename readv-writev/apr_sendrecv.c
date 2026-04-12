/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Source: https://github.com/apache/apr/blob/trunk/network_io/unix/sendrecv.c
 */

#include "apr_arch_networkio.h"
#include "apr_support.h"

#if APR_HAS_SENDFILE
/* This file is needed to allow us access to the apr_file_t internals. */
#include "apr_arch_file_io.h"
#endif /* APR_HAS_SENDFILE */

/* osreldate.h is only needed on FreeBSD for sendfile detection */
#if defined(__FreeBSD__)
#include <osreldate.h>
#endif

apr_status_t apr_socket_send(apr_socket_t *sock, const char *buf,
                             apr_size_t *len)
{
    apr_ssize_t rv;

    if (sock->options & APR_INCOMPLETE_WRITE) {
        sock->options &= ~APR_INCOMPLETE_WRITE;
        goto do_select;
    }

    do {
        rv = write(sock->socketdes, buf, (*len));
    } while (rv == -1 && errno == EINTR);

    while (rv == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)
                    && (sock->timeout > 0)) {
        apr_status_t arv;
do_select:
        arv = apr_wait_for_io_or_timeout(NULL, sock, 0);
        if (arv != APR_SUCCESS) {
            *len = 0;
            return arv;
        }
        else {
            do {
                rv = write(sock->socketdes, buf, (*len));
            } while (rv == -1 && errno == EINTR);
        }
    }
    if (rv == -1) {
        *len = 0;
        return errno;
    }
    if ((sock->timeout > 0) && (rv < *len)) {
        sock->options |= APR_INCOMPLETE_WRITE;
    }
    (*len) = rv;
    return APR_SUCCESS;
}

apr_status_t apr_socket_recv(apr_socket_t *sock, char *buf, apr_size_t *len)
{
    apr_ssize_t rv;
    apr_status_t arv;

    if (sock->options & APR_INCOMPLETE_READ) {
        sock->options &= ~APR_INCOMPLETE_READ;
        goto do_select;
    }

    do {
        rv = read(sock->socketdes, buf, (*len));
    } while (rv == -1 && errno == EINTR);

    while ((rv == -1) && (errno == EAGAIN || errno == EWOULDBLOCK)
                      && (sock->timeout > 0)) {
do_select:
        arv = apr_wait_for_io_or_timeout(NULL, sock, 1);
        if (arv != APR_SUCCESS) {
            *len = 0;
            return arv;
        }
        else {
            do {
                rv = read(sock->socketdes, buf, (*len));
            } while (rv == -1 && errno == EINTR);
        }
    }
    if (rv == -1) {
        (*len) = 0;
        return errno;
    }
    if ((sock->timeout > 0) && (rv < *len)) {
        sock->options |= APR_INCOMPLETE_READ;
    }
    (*len) = rv;
    if (rv == 0) {
        return APR_EOF;
    }
    return APR_SUCCESS;
}

apr_status_t apr_socket_sendv(apr_socket_t * sock, const struct iovec *vec,
                              apr_int32_t nvec, apr_size_t *len)
{
#ifdef HAVE_WRITEV
    apr_ssize_t rv;
    apr_int32_t i;

    if (sock->options & APR_INCOMPLETE_WRITE) {
        sock->options &= ~APR_INCOMPLETE_WRITE;
        goto do_select;
    }

    do {
        rv = writev(sock->socketdes, vec, nvec);
    } while (rv == -1 && errno == EINTR);

    while ((rv == -1) && (errno == EAGAIN || errno == EWOULDBLOCK)
                      && (sock->timeout > 0)) {
        apr_status_t arv;
do_select:
        arv = apr_wait_for_io_or_timeout(NULL, sock, 0);
        if (arv != APR_SUCCESS) {
            *len = 0;
            return arv;
        }
        else {
            do {
                rv = writev(sock->socketdes, vec, nvec);
            } while (rv == -1 && errno == EINTR);
        }
    }
    if (rv == -1) {
        *len = 0;
        return errno;
    }
    if (sock->timeout > 0) {
        apr_size_t rv_len = rv;
        for (i = 0; i < nvec; ++i) {
            apr_size_t iov_len = vec[i].iov_len;
            if (rv_len < iov_len) {
                sock->options |= APR_INCOMPLETE_WRITE;
                break;
            }
            rv_len -= iov_len;
        }
    }
    (*len) = rv;
    return APR_SUCCESS;
#else
    *len = vec[0].iov_len;
    return apr_socket_send(sock, vec[0].iov_base, len);
#endif
}

#if APR_HAS_SENDFILE

/* Define a structure to pass in when we have a NULL header value */
static apr_hdtr_t no_hdtr;

#if (defined(__linux__) || defined(__GNU__)) && defined(HAVE_WRITEV)

apr_status_t apr_socket_sendfile(apr_socket_t *sock, apr_file_t *file,
                                 apr_hdtr_t *hdtr, apr_off_t *offset,
                                 apr_size_t *len, apr_int32_t flags)
{
    int nopush_set = 0, rv, i;
    apr_size_t bytes_to_send = *len;
    apr_status_t arv;
    apr_size_t n;

#if APR_HAS_LARGE_FILES && defined(HAVE_SENDFILE64)
    apr_off_t off = *offset;
#define sendfile sendfile64

#elif APR_HAS_LARGE_FILES && SIZEOF_OFF_T == 4
    off_t off;

    if ((apr_int64_t)*offset + bytes_to_send > INT_MAX) {
        *len = 0;
        return EINVAL;
    }

    off = *offset;

#else
    off_t off = *offset;

    if (sizeof(off_t) == 8 && bytes_to_send > INT_MAX) {
        bytes_to_send = INT_MAX;
    }
#endif

    /* Until further notice. */
    *len = 0;

    if (!hdtr) {
        hdtr = &no_hdtr;
    }
    else if ((hdtr->numheaders > 0 || hdtr->numtrailers > 0)
             && !apr_is_option_set(sock, APR_TCP_NOPUSH)) {
        /* cork before writing headers */
        rv = apr_socket_opt_set(sock, APR_TCP_NOPUSH, 1);
        if (rv != APR_SUCCESS) {
            return rv;
        }
        nopush_set = 1;
    }

    if (hdtr->numheaders > 0) {
        apr_size_t total_hdrbytes;

        /* Now write the headers */
        arv = apr_socket_sendv(sock, hdtr->headers, hdtr->numheaders, &n);
        *len += n;
        if (arv != APR_SUCCESS) {
            return arv;
        }

        total_hdrbytes = 0;
        for (i = 0; i < hdtr->numheaders; i++) {
            total_hdrbytes += hdtr->headers[i].iov_len;
        }
        if (n < total_hdrbytes) {
            if (nopush_set) {
                return apr_socket_opt_set(sock, APR_TCP_NOPUSH, 0);
            }
            return APR_SUCCESS;
        }
    }

    if (sock->options & APR_INCOMPLETE_WRITE) {
        sock->options &= ~APR_INCOMPLETE_WRITE;
        goto do_select;
    }

    do {
        rv = sendfile(sock->socketdes,    /* socket */
                      file->filedes, /* open file descriptor of the file to be sent */
                      &off,    /* where in the file to start */
                      bytes_to_send);   /* number of bytes to send */
    } while (rv == -1 && errno == EINTR);

    while ((rv == -1) && (errno == EAGAIN || errno == EWOULDBLOCK)
                      && (sock->timeout > 0)) {
do_select:
        arv = apr_wait_for_io_or_timeout(NULL, sock, 0);
        if (arv != APR_SUCCESS) {
            return arv;
        }
        else {
            do {
                rv = sendfile(sock->socketdes,    /* socket */
                              file->filedes, /* open file descriptor of the file to be sent */
                              &off,    /* where in the file to start */
                              bytes_to_send);    /* number of bytes to send */
            } while (rv == -1 && errno == EINTR);
        }
    }

    if (rv == -1) {
        arv = errno;
        if (nopush_set) {
            apr_socket_opt_set(sock, APR_TCP_NOPUSH, 0);
        }
        return arv;
    }

    *len += rv;

    if ((apr_size_t)rv < bytes_to_send) {
        if (nopush_set) {
            arv = apr_socket_opt_set(sock, APR_TCP_NOPUSH, 0);
        }
        else {
            arv = APR_SUCCESS;
        }
        if (rv > 0) {
            if (sock->timeout > 0) {
                sock->options |= APR_INCOMPLETE_WRITE;
            }
            return arv;
        }
        else {
            return APR_EOF;
        }
    }

    /* Now write the footers */
    if (hdtr->numtrailers > 0) {
        arv = apr_socket_sendv(sock, hdtr->trailers, hdtr->numtrailers, &n);
        *len += n;
        if (nopush_set) {
            apr_socket_opt_set(sock, APR_TCP_NOPUSH, 0);
        }
        if (arv != APR_SUCCESS) {
            return arv;
        }
    }

    return APR_SUCCESS;
}

#endif /* __linux__ */

#endif /* APR_HAS_SENDFILE */
