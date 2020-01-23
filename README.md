# NAT
Network addresss translation flow tracker. It takes input from **NAT.txt** which has all the allowed translation from input IP to output IP. Then there is a **FLOW.txt** file which contains IP addresses to be resolved.

## Compile and Run
```
$ cd /path/to/repo/
$ make  # This should generate an executable nat_flow
$ ./nat_flow
```
After reading NAT.txt and FLOW.txt it will generate **output.txt** in the same directory which will contain the possible flows from given NAT directives.

# Current implementation
* The implementation is done in *C* language which contains two main structures
```
/* Stores a single IP address */
typedef struct ip_addr {
    char ip[MAXIPLEN];
    char port[MAXPORTLEN];
} ip_addr_t;

/* Stores a NAT pair from input file */
typedef struct nat_pair {
    ip_addr_t input_ip;
    ip_addr_t output_ip;
    struct nat_pair *next;
} nat_pair_t;
```
* Code reuse: Keeping these two structures made code reuse possible. For example, _parse_ip_ fuction wsa used by both _parse_nat_file_ and _process_flow_file_.
* Under 90 minutes I did not get a chance to write a testscript separately instead I made changes in nat.txt file to include erroneous values.
  * __inet_pton__ (inbuilt C function) was used to check the correctness of IP field of the string which was a time saviour!!
  * IP can only be acceptable if is a digit and value ranges from 0-255.
* __Assumption__: If we have two NAT pairs whose inputs are for example `*:51 -> 10.0.0.1:8080` and `192.168.1.1:51 -> 8.8.8.8:80` in chronological fashion, then for the flow for example `192.168.1.1:51` we get `10.0.0.1:8080`. This is because code checks for the first matching pair in the NAT which is `*:51 -> 10.0.0.1:8080` in this case.

## Possible optimizations and enhancements
* As number of entries are not fixed in the NAT file, dynamically creating entries in a linked list made sense. But in my opinion variation of hash table would have been a better choice. As searching in a linked list can take O(n) time. 
* A script/testfile for functional testing. Also, *gtest* suite can be used to do a unit testing.
