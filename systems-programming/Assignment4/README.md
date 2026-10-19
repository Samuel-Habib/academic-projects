1. Project Members
    - Samuel Habib (smh389)
    - Victor Peng (vmp134)

2. Testing Plan
    - Integration Tests (./chatd and ./raw)
        - Server Startup & Connection
            - Ran ."/chatd 15151", and "./raw localhost 15151", connected as "1|NAM|4|Bob|" and received "Recv  39 [1|MSG|30|#all|Bob|Welcome to the chat!|]"
            - Ran "./raw localhost 15151" and tried "1|SET|17|Smiling politely|", and received error 0
        - Messaging
            - Ran "./raw localhost 15151", connected as "1|NAM|4|Bob|", connected as "1|NAM|5|Jill|", ran "1|MSG|20||#all|Hello, world!|" as Bob, confirmed "Recv  32 [1|MSG|23|Bob|#all|Hello, world!|]" as Jill
            - Ran "./raw localhost 15151", connected as "1|NAM|6|Alice", ran "1|MSG|32||Alice|Private message to Alice|" as Bob, confirmed "Recv  44 [1|MSG|35|Bob|Alice|Private message to Alice|]" as Alice and not Jill
            - Ran "1|SET|6|Happy|" as Bob, confirmed "Recv  38 [1|MSG|29|#all|#all|Bob is now "Happy"|]" as Bob, Alice, and Jill
        - Queries
            - Ran "1|SET|4|Sad|" as Alice, ran "1|SET|4|Mad" as Jill, ran "1|WHO|5|#all|" as Alice, confirmed "Recv  43 [#all|Alice|Bob: Happy^JAlice: Sad^JJill: Mad|]"
            - Ran "1|WHO|4|Bob" as Alice, confirmed "Recv  31 [1|MSG|22|#all|Alice|Bob: Happy|]"
        - Error Checking
            - Ran "./raw localhost 15151", connected as "1|NAM|4|Bob|", Confirmed "Recv  23 [1|ERR|14|1|Name in use|]"
            - Ran "1|MSG|9||Bill|Hi|" as Bob, Confirmed "Recv  29 [1|ERR|20|2|Unknown recipient|]"
            - Ran "./raw localhost 15151", connected as "1|NAM|4|$$$|", Confirmed "Recv  29 [1|ERR|20|3|Illegal character|]"
            - Ran "1|MSG|100||#all|Hi Guys this is a very long message to mess with the program I hope this works ok anyways huh|" as Bob, Confirmed "Recv  20 [1|ERR|11|4|Too long|]"
    
3. Design Plan
    - Clients and Polling
        - For each client, we made a struct with their data
            - We store their file descriptor, name, status, and state
            - State determines if they have connected (0) or if they have logged in (1)
        - We decided to implement a fixed array of clients and pollfds
            - Both are of same length (SOMAXCONN + 1)
            - When a client disconnects or triggers error 0, we close the socket and shift elements in the arrays to use space efficiently
    - Message Handling
        - We use multiple functions to check correctness
            - parseLength and parseBody ensure message lengths are correct, message types, usage of the '|' char, etc.
        - We route messages based on recipient
            - We iterate through client array if recipient is #all to send data
    - Shutdown Handling
        - We monitor interrupt and terminate for any unexpected shutdowns 
            - We use signal(SIGINT, ...) and a volatile sig_atomic_t set to 1 as the condition for the while() loop
            - Once the server shuts down this way, we close all fds and exit, ensuring all resources are cleaned up
    - Error handling
        - We follow the instructions in p4.pdf
            - We ensure that error 0 is fatal while errors 1-4 are recoverable
            - We send the error packet and remove the fd
            - We ensure NAM is the first message received, error 0 otherwise