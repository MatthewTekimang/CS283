1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

_The remote client determines the end of an output by reading delimiters like "\n". To handle partial reads, the client should continue reading in a loop until the termination signal is received or the maximum length is reached._

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

_Delimiters are used by a networked shell protocol along with null terminators. The commands might split incorrectly and create new commands_

3. Describe the general differences between stateful and stateless protocols.

_We would use it since stateful protocol is maintained over multiple requests and therefore the server can keep track of the client's context. WHileas, the server treats every request as independent with the Stateless protocols and therefore requires it to carry all necessary information, so it simplifies server design and scaling but may increase overhead on the client._

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

_While it is unreliable, UDP might be preferred for applications where low latency is more important than guaranteed delivery, such as video streaming, real-time communications, or online gaming. These applications often handle error corrections themselves, so the reduced overhead and faster transmission make UDP ideal for them._

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

_The operating system provides the sockets API to enable network communications. This abstraction offers functions like socket(), bind(), listen(), accept(), connect(), send(), and recv() to simplify the process of establishing and managing network connections regardless of the underlying transport protocol._
