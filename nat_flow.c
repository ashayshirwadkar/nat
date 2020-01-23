#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <error.h>

#define MAXIPLEN   16
#define MAXPORTLEN 6

#define NAT_FILE    "nat.txt"
#define FLOW_FILE   "flow.txt"
#define OUTPUT_FILE "output.txt"

typedef struct ip_addr {
    char ip[MAXIPLEN];
    char port[MAXPORTLEN];
} ip_addr_t;

typedef struct nat_pair {
    ip_addr_t input_ip;
    ip_addr_t output_ip;
    struct nat_pair *next;
} nat_pair_t;


/** HELPER FUNCTIONS **/

int validate_ip_str(char *str, char **port_ptr)
{
    int ret;

    /* Separate port and ip from the string */
    char *port = strchr(str, ':');
    if (port == NULL)
        return -1;

    *port = '\0';

    /* Check IP */
    struct sockaddr_in sa;
    ret = inet_pton(AF_INET, str, &(sa.sin_addr));
    if (!ret && (strcmp(str, "*") != 0))
        return -1;

    port++;
    *port_ptr = port;
    return 0;
}

int parse_ip(char *str, ip_addr_t *ip_addr)
{
    int i = 0;
    int ret = 0;
    char *port = NULL;

    ret = validate_ip_str(str, &port);
    if (ret) {
        printf ("Invalid IP: %s\n", str);
        return -1;
    }

    strcpy(ip_addr->ip, str);

    /* We have \n at the end of the port. Ignore that */
    int len = strlen(port);
    if (len > 0 && port[len-1] == '\n') port[len-1] = '\0';

    strcpy(ip_addr->port, port);

    return ret;
}

ip_addr_t *check_flow(ip_addr_t *ip_addr, nat_pair_t *head)
{

    nat_pair_t *it = head;
    while (it != NULL) {
        if ((strcmp(it->input_ip.ip, "*") == 0 ||
             strcmp(it->input_ip.ip, ip_addr->ip) == 0) &&
            (strcmp(it->input_ip.port, "*") == 0 ||
             strcmp(it->input_ip.port, ip_addr->port) == 0)) {
            return &it->output_ip;
        }
        it = it->next;
    }
    return NULL;
}


/** PRIMAY FUNCTIONS **/

int parse_nat_file(const char *filename, nat_pair_t **head)
{
    FILE *fd;
    int ret = 0;
    char line[44 /* IP1:PORT1,IP2:PORT2 */];
    nat_pair_t *tail = NULL;

    /* open the input file */
    fd = fopen(filename, "r");
    if (!fd) {
        printf ("Error opening file: %s\n", filename);
        return -1;
    }

    /* Get lines from the input file and append to the
     * nat_pair list */
    while(fgets(line, 42, fd)) {
        char *ip_addr;
        ip_addr = strtok(line, ",");

        nat_pair_t *npair = (nat_pair_t *)malloc(sizeof(nat_pair_t));

        /* Parse input IP from NAT pair */
        ret = parse_ip(ip_addr, &(npair->input_ip));
        if (ret)
            goto out;

        ip_addr = strtok(NULL, ",");

        /* Parse output IP from NAT pair */
        ret = parse_ip(ip_addr, &(npair->output_ip));
        if (ret)
            goto out;

        if (tail != NULL) {
            tail->next = npair;
        }
        else {
            *head = npair;
        }

        npair->next = NULL;
        tail = npair;
    }

 out:
    fclose(fd);
    return ret;
}

int process_flow_file(const char *flow_filename, const char *output_filename,
                      nat_pair_t *head)
{
    FILE *out_fd, *flow_fd;
    int ret;
    char line[22 /* IP1:PORT1 */];
    ip_addr_t *node = NULL;

    flow_fd = fopen(flow_filename, "r");
    if (!flow_fd) {
        printf ("Error opening file: %s\n", flow_filename);
        return -1;
    }

    out_fd = fopen(output_filename, "w");
    if (!out_fd) {
        printf ("Error opening file: %s\n", output_filename);
        return -1;
    }

    /* Get lines from the input file and append to the
     * nat_pair list */
    while(fgets(line, 22, flow_fd)) {

        ip_addr_t ip_addr;

        /* Parse the input IP from flow file */
        ret = parse_ip(line, &ip_addr);
        if (ret)
            goto out;

        node = check_flow(&ip_addr, head);
        if (!node) {
            fprintf(out_fd, "No NAT match for %s:%s\n", ip_addr.ip, ip_addr.port);
        }
        else {
            fprintf(out_fd, "%s:%s -> %s:%s\n", ip_addr.ip, ip_addr.port,
                    node->ip, node->port);
        }
    }

out:
    fclose(flow_fd);
    fclose(out_fd);
    return ret;
}

int print_ll(nat_pair_t *head)
{
    nat_pair_t *temp = head;

    while (temp != NULL) {
        printf("%s:%s %s:%s\n", temp->input_ip.ip, temp->input_ip.port,
               temp->output_ip.ip, temp->output_ip.port);
        temp = temp->next;
    }
    return 0;
}

int main()
{
    int ret = 0;
    nat_pair_t *head = NULL;

    ret = parse_nat_file(NAT_FILE, &head);
    if (ret)
        goto out;

    ret = process_flow_file(FLOW_FILE, OUTPUT_FILE, head);
    if (ret)
        goto out;

    /* Free up the resources */
    while (head != NULL) {
        nat_pair_t *temp = head->next;
        free(head);
        head = temp;
    }

    
    //print_ll(head);
 out:
    if (!ret)
        printf("Generated output successfully!\n");
    return ret;
}
