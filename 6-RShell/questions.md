1. **How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?**

> **Answer:**  
The client detects the end of a command's output using a special delimiter character (e.g., EOF or null character). Techniques include looping `recv()` calls until this delimiter is received to handle partial reads effectively.

2. **This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?**

> **Answer:**  
The protocol should explicitly use special markers (such as null characters or EOF) to signal the end of a command/message. Without these markers, messages might combine or fragment, causing commands to be misinterpreted or incomplete.

3. **Describe the general differences between stateful and stateless protocols.**

> **Answer:**  
Stateful protocols maintain connection/session state information between requests, enabling context (like TCP). Stateless protocols treat each request independently, not retaining any session information (like HTTP).

4. **Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?**

> **Answer:**  
UDP provides lower latency and overhead, making it useful for real-time applications such as streaming video/audio, games, or scenarios where speed matters more than reliability.

5. **What interface/abstraction is provided by the operating system to enable applications to use network communications?**

> **Answer:**  
Sockets provide the interface/abstraction allowing applications to perform network communications.
