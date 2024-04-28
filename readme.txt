Costea Razvan-Stefan, 324CC
Communication protocols - Homework 2

My implementation was heavily inspired by my SO asynchronous web server
assignment, especially the parts relating to epoll and TCP sockets. As such, the
boilerplate code for connecting to a server, creating a socket and others were
taken from there. To that, I've added code to make ports reusable and to disable
Nagle's algorithm, using the setsockopt() function.

General:

For handling UDP datagrams, I've implemented two data types:
- enum data_type, which represents the data type of the value inside the UDP
datagram
- struct message, with a field for the topic, the data type and the payload

For handling the posibility of TCP messages being concatenated, I've implemented
send and receive functions that send and respectively receive the size of the
message (represented as an int) and then the message itself. This way, I
can send and receive one message at a time, without worrying about them being
concatenated. 

Server:

The server creates a TCP listener, a UDP listener and an epoll instance, waiting
for input from stdin, UDP clients and TCP clients. The server receives a
client's ID when it connects to the server or a command (subscribe or unsub-
scribe).

I use an unordered set to keep track of all currently connected clients.
I use an unordered map to map sockets to client IDs so I know which client ID to
erase from the connected set when I receive the notification that a socket was
closed.
I use an unordered map to map client IDs to sockets so I which socket to send
messages from UDP clients.
I use a vector of pairs to keep track of all topics mentioned thus far and every
client subscribed to them.

I handle topics containing wildcard character as follows: I replace every '*'
with ".*" (any character any ammount of times) and every '+' with "[^/]*" (any
character except for '/' any ammount of times) and use regex matching.
When I receive a topic from a UDP client, I try to match it with all topics in
my vector and send a message to all clients subscribed to the matched topic. To
avoid sending multiple messages to the same client about the same topic, I used
an unordered_set which keeps track of whether I've already sent a message to a
client.
For unsubscribing, I unsubscribe the respective client from all topics that
match the topic he gave me.

When the server receives 'exit' from stdin, it closes all open sockets, the TCP
and UDP listeners and the epoll instance.

Subscriber:

The TCP client connects to the server, to which it sends its ID, and can receive
input from stdin or from the server itself.

From stdin, the client can only receive three commands: exit, subscribe and
unsubscribe.
For exit, it closes the socket and the epoll instance.
For the other two, the client sends the server the command received from stdin.

When the client receives a message from the server, it checks the data type of
the data inside the payload, parses it, switches it from network order to host
order if needed, and prints it.