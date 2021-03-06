/*
    Copyright (c) 2007-2014 Contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "testutil.hpp"

#define MSG_SIZE 20

#ifdef _WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#define usleep(a) Sleep((a) / 1000)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

int main (void)
{
    int rc;
 
    setup_test_environment();
    //  Create the infrastructure
    void *ctx = zmq_ctx_new ();
    assert (ctx);

    void *rep = zmq_socket (ctx, ZMQ_REP);
    assert (rep);
    void *req = zmq_socket (ctx, ZMQ_REQ);
    assert (req);

    rc = zmq_bind(rep, "tcp://127.0.0.1:5560");
    assert (rc == 0);

    rc = zmq_connect(req, "tcp://127.0.0.1:5560");
    assert (rc == 0);

    char tmp[MSG_SIZE];
    zmq_send(req, tmp, MSG_SIZE, 0);

    zmq_msg_t msg;
    rc = zmq_msg_init(&msg);
    assert (rc == 0);

    zmq_recvmsg(rep, &msg, 0);
    assert(zmq_msg_size(&msg) == MSG_SIZE);
  
	  // get the messages source file descriptor
    int srcFd = zmq_msg_get(&msg, ZMQ_SRCFD);
    assert(srcFd >= 0);

	  // get the remote endpoint
    struct sockaddr_storage ss;
    socklen_t addrlen = sizeof ss;
    rc = getpeername (srcFd, (struct sockaddr*) &ss, &addrlen);
    assert (rc == 0);

    char host [NI_MAXHOST];
    rc = getnameinfo ((struct sockaddr*) &ss, addrlen, host, sizeof host, NULL, 0, NI_NUMERICHOST);
    assert (rc == 0);

	  // assert it is localhost which connected
    assert (strcmp(host, "127.0.0.1") == 0);

    rc = zmq_close (rep);
    assert (rc == 0);
    rc = zmq_close (req);
    assert (rc == 0);

	  // sleep a bit for the socket to be freed
	  usleep(30000);
	
	  // getting name from closed socket will fail
    rc = getpeername (srcFd, (struct sockaddr*) &ss, &addrlen);
    assert (rc == -1);
    assert (errno == EBADF);
    
    rc = zmq_ctx_term (ctx);
    assert (rc == 0);

    return 0 ;
}

