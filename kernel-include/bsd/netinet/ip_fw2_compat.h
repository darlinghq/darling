/* IPFW backward compatibility */

#ifndef _IP_FW_COMPAT_H_
#define _IP_FW_COMPAT_H_

/* prototypes */
void    ipfw_convert_from_latest(struct ip_fw *curr_rule, void *old_rule, u_int32_t api_version, int is64user);
int             ipfw_convert_to_latest(struct sockopt *sopt, struct ip_fw *rule, int api_version, int is64user);
int     ipfw_get_command_and_version(struct sockopt *sopt, int *command, u_int32_t *api_version);


/*
 * ******************************
 * ****** IPFW version one ******
 * ******************************
 */

/*
 * This union structure identifies an interface, either explicitly
 * by name or implicitly by IP address. The flags IP_FW_F_IIFNAME
 * and IP_FW_F_OIFNAME say how to interpret this structure. An
 * interface unit number of -1 matches any unit number, while an
 * IP address of 0.0.0.0 indicates matches any interface.
 *
 * The receive and transmit interfaces are only compared against the
 * the packet if the corresponding bit (IP_FW_F_IIFACE or IP_FW_F_OIFACE)
 * is set. Note some packets lack a receive or transmit interface
 * (in which case the missing "interface" never matches).
 */

union ip_fw_if_compat {
	struct in_addr fu_via_ip; /* Specified by IP address */
	struct {                /* Specified by interface name */
#define FW_IFNLEN_COMPAT     10 /* need room ! was IFNAMSIZ */
		char  name[FW_IFNLEN_COMPAT];
		short unit;     /* -1 means match any unit */
	} fu_via_if_compat;
};

/*
 * Format of an IP firewall descriptor
 *
 * fw_src, fw_dst, fw_smsk, fw_dmsk are always stored in network byte order.
 * fw_flg and fw_n*p are stored in host byte order (of course).
 * Port numbers are stored in HOST byte order.
 */


struct ip_fw_compat {
	u_int32_t version;              /* Version of this structure.  Should always be */
	/* set to IP_FW_CURRENT_API_VERSION by clients. */
	void *context;                  /* Context that is usable by user processes to */
	/* identify this rule. */
	u_int64_t fw_pcnt, fw_bcnt;     /* Packet and byte counters */
	struct in_addr fw_src, fw_dst;  /* Source and destination IP addr */
	struct in_addr fw_smsk, fw_dmsk; /* Mask for src and dest IP addr */
	u_short fw_number;              /* Rule number */
	u_int fw_flg;                   /* Flags word */
#define IP_FW_MAX_PORTS_COMPAT  10              /* A reasonable maximum */
	union {
		u_short fw_pts[IP_FW_MAX_PORTS_COMPAT]; /* Array of port numbers to match */
#define IP_FW_ICMPTYPES_MAX_COMPAT      128
#define IP_FW_ICMPTYPES_DIM_COMPAT      (IP_FW_ICMPTYPES_MAX_COMPAT / (sizeof(unsigned) * 8))
		unsigned fw_icmptypes[IP_FW_ICMPTYPES_DIM_COMPAT]; /* ICMP types bitmap */
	} fw_uar_compat;
	u_int fw_ipflg;                 /* IP flags word */
	u_char fw_ipopt, fw_ipnopt;     /* IP options set/unset */
	u_char fw_tcpopt, fw_tcpnopt;   /* TCP options set/unset */
	u_char fw_tcpf, fw_tcpnf;       /* TCP flags set/unset */
	long timestamp;                 /* timestamp (tv_sec) of last match */
	union ip_fw_if_compat fw_in_if, fw_out_if; /* Incoming and outgoing interfaces */
	union {
		u_short fu_divert_port;         /* Divert/tee port (options IPDIVERT) */
		u_short fu_pipe_nr;             /* queue number (option DUMMYNET) */
		u_short fu_skipto_rule;         /* SKIPTO command rule number */
		u_short fu_reject_code;         /* REJECT response code */
		struct sockaddr_in fu_fwd_ip;
	} fw_un_compat;
	u_char fw_prot;                 /* IP protocol */
	/*
	 * N'of src ports and # of dst ports in ports array (dst ports
	 * follow src ports; max of 10 ports in all; count of 0 means
	 * match all ports)
	 */
	u_char fw_nports;
	void *pipe_ptr;                /* flow_set ptr for dummynet pipe */
	void *next_rule_ptr;           /* next rule in case of match */
	uid_t fw_uid;                   /* uid to match */
	int fw_logamount;               /* amount to log */
	u_int64_t fw_loghighest;        /* highest number packet to log */
};

/*
 * extended ipfw structure... some fields in the original struct
 * can be used to pass parameters up/down, namely pointers
 *     void *pipe_ptr
 *     void *next_rule_ptr
 * some others can be used to pass parameters down, namely counters etc.
 *     u_int64_t fw_pcnt,fw_bcnt;
 *     long timestamp;
 */

struct ip_fw_ext_compat {             /* extended structure */
	struct ip_fw rule;  /* must be at offset 0 */
	long    dont_match_prob;    /* 0x7fffffff means 1.0, always fail */
	u_int   dyn_type;/* type for dynamic rule */
};

struct ip_fw_chain_compat {
	LIST_ENTRY(ip_fw_chain_compat) next;
	struct ip_fw_compat *rule;
};

/*
 * dynamic ipfw rule
 */

struct ipfw_dyn_rule_compat {
	struct ipfw_dyn_rule *next;

	struct ipfw_flow_id id;
	struct ipfw_flow_id mask;
	struct ip_fw_chain_compat *chain;       /* pointer to parent rule	*/
	u_int32_t type;                 /* rule type			*/
	u_int32_t expire;               /* expire time			*/
	u_int64_t pcnt, bcnt;           /* match counters		*/
	u_int32_t bucket;               /* which bucket in hash table	*/
	u_int32_t state;                /* state of this rule (typ. a   */
	                                /* combination of TCP flags)	*/
};

#ifdef BSD_KERNEL_PRIVATE
#pragma pack(4)

struct ip_fw_compat_32 {
	u_int32_t version;              /* Version of this structure.  Should always be */
	/* set to IP_FW_CURRENT_API_VERSION by clients. */
	user32_addr_t context;  /* Context that is usable by user processes to */
	/* identify this rule. */
	u_int64_t fw_pcnt, fw_bcnt;     /* Packet and byte counters */
	struct in_addr fw_src, fw_dst;  /* Source and destination IP addr */
	struct in_addr fw_smsk, fw_dmsk;/* Mask for src and dest IP addr */
	u_short fw_number;      /* Rule number */
	u_int fw_flg;                   /* Flags word */
#define IP_FW_MAX_PORTS_COMPAT  10              /* A reasonable maximum */
	union {
		u_short fw_pts[IP_FW_MAX_PORTS_COMPAT]; /* Array of port numbers to match */
#define IP_FW_ICMPTYPES_MAX_COMPAT      128
#define IP_FW_ICMPTYPES_DIM_COMPAT      (IP_FW_ICMPTYPES_MAX_COMPAT / (sizeof(unsigned) * 8))
		unsigned fw_icmptypes[IP_FW_ICMPTYPES_DIM_COMPAT]; /* ICMP types bitmap */
	} fw_uar_compat;
	u_int fw_ipflg;                 /* IP flags word */
	u_char fw_ipopt, fw_ipnopt;     /* IP options set/unset */
	u_char fw_tcpopt, fw_tcpnopt;   /* TCP options set/unset */
	u_char fw_tcpf, fw_tcpnf;       /* TCP flags set/unset */
	u_int32_t timestamp;                    /* timestamp (tv_sec) of last match */
	union ip_fw_if_compat fw_in_if, fw_out_if; /* Incoming and outgoing interfaces */
	union {
		u_short fu_divert_port;         /* Divert/tee port (options IPDIVERT) */
		u_short fu_pipe_nr;             /* queue number (option DUMMYNET) */
		u_short fu_skipto_rule;         /* SKIPTO command rule number */
		u_short fu_reject_code;         /* REJECT response code */
		struct sockaddr_in fu_fwd_ip;
	} fw_un_compat;
	u_char fw_prot;                 /* IP protocol */
	/*
	 * N'of src ports and # of dst ports in ports array (dst ports
	 * follow src ports; max of 10 ports in all; count of 0 means
	 * match all ports)
	 */
	u_char fw_nports;
	user32_addr_t pipe_ptr;                /* flow_set ptr for dummynet pipe */
	user32_addr_t next_rule_ptr;           /* next rule in case of match */
	uid_t fw_uid;                   /* uid to match */
	int fw_logamount;               /* amount to log */
	u_int64_t fw_loghighest;        /* highest number packet to log */
};
#pragma pack()

struct ip_fw_compat_64 {
	u_int32_t version;              /* Version of this structure.  Should always be */
	/* set to IP_FW_CURRENT_API_VERSION by clients. */
	user64_addr_t context;                  /* Context that is usable by user processes to */
	/* identify this rule. */
	u_int64_t fw_pcnt, fw_bcnt;     /* Packet and byte counters */
	struct in_addr fw_src, fw_dst;  /* Source and destination IP addr */
	struct in_addr fw_smsk, fw_dmsk;/* Mask for src and dest IP addr */
	u_short fw_number;                      /* Rule number */
	u_int fw_flg;                                   /* Flags word */
#define IP_FW_MAX_PORTS_COMPAT  10              /* A reasonable maximum */
	union {
		u_short fw_pts[IP_FW_MAX_PORTS_COMPAT]; /* Array of port numbers to match */
#define IP_FW_ICMPTYPES_MAX_COMPAT      128
#define IP_FW_ICMPTYPES_DIM_COMPAT      (IP_FW_ICMPTYPES_MAX_COMPAT / (sizeof(unsigned) * 8))
		unsigned fw_icmptypes[IP_FW_ICMPTYPES_DIM_COMPAT]; /* ICMP types bitmap */
	} fw_uar_compat;
	u_int fw_ipflg;                                 /* IP flags word */
	u_char fw_ipopt, fw_ipnopt;     /* IP options set/unset */
	u_char fw_tcpopt, fw_tcpnopt;   /* TCP options set/unset */
	u_char fw_tcpf, fw_tcpnf;       /* TCP flags set/unset */
	u_int64_t timestamp;                    /* timestamp (tv_sec) of last match */
	union ip_fw_if_compat fw_in_if, fw_out_if; /* Incoming and outgoing interfaces */
	union {
		u_short fu_divert_port;         /* Divert/tee port (options IPDIVERT) */
		u_short fu_pipe_nr;                     /* queue number (option DUMMYNET) */
		u_short fu_skipto_rule;         /* SKIPTO command rule number */
		u_short fu_reject_code;         /* REJECT response code */
		struct sockaddr_in fu_fwd_ip;
	} fw_un_compat;
	u_char fw_prot;                 /* IP protocol */
	/*
	 * N'of src ports and # of dst ports in ports array (dst ports
	 * follow src ports; max of 10 ports in all; count of 0 means
	 * match all ports)
	 */
	u_char fw_nports;
	user64_addr_t pipe_ptr;                /* flow_set ptr for dummynet pipe */
	user64_addr_t next_rule_ptr;           /* next rule in case of match */
	uid_t fw_uid;                                                           /* uid to match */
	int fw_logamount;                                               /* amount to log */
	u_int64_t fw_loghighest;                                /* highest number packet to log */
};

struct ipfw_dyn_rule_compat_32 {
	user32_addr_t next;

	struct ipfw_flow_id id;
	struct ipfw_flow_id mask;
	user32_addr_t chain;            /* pointer to parent rule	*/
	u_int32_t type;                 /* rule type			*/
	u_int32_t expire;               /* expire time			*/
	u_int64_t pcnt, bcnt;           /* match counters		*/
	u_int32_t bucket;               /* which bucket in hash table	*/
	u_int32_t state;                /* state of this rule (typ. a   */
	                                /* combination of TCP flags)	*/
};

struct ipfw_dyn_rule_compat_64 {
	user64_addr_t next;

	struct ipfw_flow_id id;
	struct ipfw_flow_id mask;
	user64_addr_t chain;            /* pointer to parent rule	*/
	u_int32_t type;                 /* rule type			*/
	u_int32_t expire;               /* expire time			*/
	u_int64_t pcnt, bcnt;           /* match counters		*/
	u_int32_t bucket;               /* which bucket in hash table	*/
	u_int32_t state;                /* state of this rule (typ. a   */
	                                /* combination of TCP flags)	*/
};
#endif /* BSD_KERNEL_PRIVATE */


#define IP_FW_GETNSRCP_COMPAT(rule)             ((rule)->fw_nports & 0x0f)
#define IP_FW_SETNSRCP_COMPAT(rule, n)          do {                            \
	                                  (rule)->fw_nports &= ~0x0f;   \
	                                  (rule)->fw_nports |= (n);     \
	                                } while (0)
#define IP_FW_GETNDSTP_COMPAT(rule)             ((rule)->fw_nports >> 4)
#define IP_FW_SETNDSTP_COMPAT(rule, n)          do {                            \
	                                  (rule)->fw_nports &= ~0xf0;   \
	                                  (rule)->fw_nports |= (n) << 4;\
	                                } while (0)

#define fw_divert_port_compat   fw_un_compat.fu_divert_port
#define fw_skipto_rule_compat   fw_un_compat.fu_skipto_rule
#define fw_reject_code_compat   fw_un_compat.fu_reject_code
#define fw_pipe_nr_compat       fw_un_compat.fu_pipe_nr
#define fw_fwd_ip_compat        fw_un_compat.fu_fwd_ip

/*
 * Values for "flags" field .
 */
#define IP_FW_F_COMMAND_COMPAT 0x000000ff       /* Mask for type of chain entry:	*/
#define IP_FW_F_DENY_COMPAT     0x00000000      /* This is a deny rule			*/
#define IP_FW_F_REJECT_COMPAT   0x00000001      /* Deny and send a response packet	*/
#define IP_FW_F_ACCEPT_COMPAT   0x00000002      /* This is an accept rule		*/
#define IP_FW_F_COUNT_COMPAT    0x00000003      /* This is a count rule			*/
#define IP_FW_F_DIVERT_COMPAT   0x00000004      /* This is a divert rule		*/
#define IP_FW_F_TEE_COMPAT      0x00000005      /* This is a tee rule			*/
#define IP_FW_F_SKIPTO_COMPAT   0x00000006      /* This is a skipto rule		*/
#define IP_FW_F_FWD_COMPAT      0x00000007      /* This is a "change forwarding address" rule */
#define IP_FW_F_PIPE_COMPAT     0x00000008      /* This is a dummynet rule */
#define IP_FW_F_QUEUE_COMPAT    0x00000009      /* This is a dummynet queue */

#define IP_FW_F_IN_COMPAT       0x00000100      /* Check inbound packets		*/
#define IP_FW_F_OUT_COMPAT      0x00000200      /* Check outbound packets		*/
#define IP_FW_F_IIFACE_COMPAT   0x00000400      /* Apply inbound interface test		*/
#define IP_FW_F_OIFACE_COMPAT   0x00000800      /* Apply outbound interface test	*/

#define IP_FW_F_PRN_COMPAT      0x00001000      /* Print if this rule matches		*/

#define IP_FW_F_SRNG_COMPAT     0x00002000      /* The first two src ports are a min	*
	                                         * and max range (stored in host byte	*
	                                         * order).				*/

#define IP_FW_F_DRNG_COMPAT     0x00004000      /* The first two dst ports are a min	*
	                                         * and max range (stored in host byte	*
	                                         * order).				*/

#define IP_FW_F_FRAG_COMPAT     0x00008000      /* Fragment				*/

#define IP_FW_F_IIFNAME_COMPAT  0x00010000      /* In interface by name/unit (not IP)	*/
#define IP_FW_F_OIFNAME_COMPAT  0x00020000      /* Out interface by name/unit (not IP)	*/

#define IP_FW_F_INVSRC_COMPAT   0x00040000      /* Invert sense of src check		*/
#define IP_FW_F_INVDST_COMPAT   0x00080000      /* Invert sense of dst check		*/

#define IP_FW_F_ICMPBIT_COMPAT 0x00100000       /* ICMP type bitmap is valid		*/

#define IP_FW_F_UID_COMPAT      0x00200000      /* filter by uid			*/

#define IP_FW_F_RND_MATCH_COMPAT 0x00800000     /* probabilistic rule match		*/
#define IP_FW_F_SMSK_COMPAT     0x01000000      /* src-port + mask                      */
#define IP_FW_F_DMSK_COMPAT     0x02000000      /* dst-port + mask                      */
#define IP_FW_BRIDGED_COMPAT    0x04000000      /* only match bridged packets		*/
#define IP_FW_F_KEEP_S_COMPAT   0x08000000      /* keep state	                        */
#define IP_FW_F_CHECK_S_COMPAT  0x10000000      /* check state	                        */

#define IP_FW_F_SME_COMPAT      0x20000000      /* source = me				*/
#define IP_FW_F_DME_COMPAT      0x40000000      /* destination = me			*/

#define IP_FW_F_MASK_COMPAT     0x7FFFFFFF      /* All possible flag bits mask		*/

/*
 * Flags for the 'fw_ipflg' field, for comparing values of ip and its protocols.
 */
#define IP_FW_IF_TCPEST_COMPAT  0x00000020      /* established TCP connection */
#define IP_FW_IF_TCPMSK_COMPAT  0x00000020      /* mask of all TCP values */

/*
 * Definitions for TCP flags.
 */
#define IP_FW_TCPF_FIN_COMPAT           TH_FIN
#define IP_FW_TCPF_SYN_COMPAT           TH_SYN
#define IP_FW_TCPF_RST_COMPAT           TH_RST
#define IP_FW_TCPF_PSH_COMPAT           TH_PUSH
#define IP_FW_TCPF_ACK_COMPAT           TH_ACK
#define IP_FW_TCPF_URG_COMPAT           TH_URG

/*
 * For backwards compatibility with rules specifying "via iface" but
 * not restricted to only "in" or "out" packets, we define this combination
 * of bits to represent this configuration.
 */

#define IF_FW_F_VIAHACK_COMPAT  (IP_FW_F_IN_COMPAT|IP_FW_F_OUT_COMPAT|IP_FW_F_IIFACE_COMPAT|IP_FW_F_OIFACE_COMPAT)

/*
 * Definitions for REJECT response codes.
 * Values less than 256 correspond to ICMP unreachable codes.
 */
#define IP_FW_REJECT_RST_COMPAT 0x0100          /* TCP packets: send RST */


/*
 * ******************************
 * ****** IPFW version zero *****
 * ******************************
 */

/*
 * This union structure identifies an interface, either explicitly
 * by name or implicitly by IP address. The flags IP_FW_F_IIFNAME
 * and IP_FW_F_OIFNAME say how to interpret this structure. An
 * interface unit number of -1 matches any unit number, while an
 * IP address of 0.0.0.0 indicates matches any interface.
 *
 * The receive and transmit interfaces are only compared against the
 * the packet if the corresponding bit (IP_FW_F_IIFACE or IP_FW_F_OIFACE)
 * is set. Note some packets lack a receive or transmit interface
 * (in which case the missing "interface" never matches).
 */

union ip_old_fw_if {
	struct in_addr fu_via_ip; /* Specified by IP address */
	struct {                /* Specified by interface name */
#define OLD_FW_IFNLEN     10 /* need room ! was IFNAMSIZ */
		char  name[OLD_FW_IFNLEN];
		short unit;     /* -1 means match any unit */
	} fu_via_if;
};

/*
 * Format of an IP firewall descriptor
 *
 * fw_src, fw_dst, fw_smsk, fw_dmsk are always stored in network byte order.
 * fw_flg and fw_n*p are stored in host byte order (of course).
 * Port numbers are stored in HOST byte order.
 * Warning: setsockopt() will fail if sizeof(struct ip_fw) > MLEN (108)
 */

struct ip_old_fw {
	u_int64_t fw_pcnt, fw_bcnt;     /* Packet and byte counters */
	struct in_addr fw_src, fw_dst;  /* Source and destination IP addr */
	struct in_addr fw_smsk, fw_dmsk; /* Mask for src and dest IP addr */
	u_short fw_number;              /* Rule number */
	u_int fw_flg;                   /* Flags word */
#define IP_OLD_FW_MAX_PORTS     10              /* A reasonable maximum */
	union {
		u_short fw_pts[IP_OLD_FW_MAX_PORTS]; /* Array of port numbers to match */
#define IP_OLD_FW_ICMPTYPES_MAX 128
#define IP_OLD_FW_ICMPTYPES_DIM (IP_OLD_FW_ICMPTYPES_MAX / (sizeof(unsigned) * 8))
		unsigned fw_icmptypes[IP_OLD_FW_ICMPTYPES_DIM]; /* ICMP types bitmap */
	} fw_uar;
	u_char fw_ipopt, fw_ipnopt;     /* IP options set/unset */
	u_char fw_tcpf, fw_tcpnf;       /* TCP flags set/unset */
	long timestamp;                 /* timestamp (tv_sec) of last match */
	union ip_old_fw_if fw_in_if, fw_out_if; /* Incoming and outgoing interfaces */
	union {
		u_short fu_divert_port; /* Divert/tee port (options IPDIVERT) */
		u_short fu_pipe_nr;     /* pipe number (option DUMMYNET) */
		u_short fu_skipto_rule; /* SKIPTO command rule number */
		u_short fu_reject_code; /* REJECT response code */
		struct sockaddr_in fu_fwd_ip;
	} fw_un;
	u_char fw_prot;                 /* IP protocol */
	u_char fw_nports;               /* N'of src ports and # of dst ports */
	                                /* in ports array (dst ports follow */
	                                /* src ports; max of 10 ports in all; */
	                                /* count of 0 means match all ports) */
	void *pipe_ptr;                /* Pipe ptr in case of dummynet pipe */
	void *next_rule_ptr;           /* next rule in case of match */
};

#define IP_OLD_FW_GETNSRCP(rule)                ((rule)->fw_nports & 0x0f)
#define IP_OLD_FW_SETNSRCP(rule, n)             do {                            \
	                                  (rule)->fw_nports &= ~0x0f;   \
	                                  (rule)->fw_nports |= (n);     \
	                                } while (0)
#define IP_OLD_FW_GETNDSTP(rule)                ((rule)->fw_nports >> 4)
#define IP_OLD_FW_SETNDSTP(rule, n)             do {                            \
	                                  (rule)->fw_nports &= ~0xf0;   \
	                                  (rule)->fw_nports |= (n) << 4;\
	                                } while (0)

#define old_fw_divert_port      fw_un.fu_divert_port
#define old_fw_skipto_rule      fw_un.fu_skipto_rule
#define old_fw_reject_code      fw_un.fu_reject_code
#define old_fw_pipe_nr  fw_un.fu_pipe_nr
#define old_fw_fwd_ip   fw_un.fu_fwd_ip

/*
 * Values for "flags" field .
 */
#define IP_OLD_FW_F_COMMAND 0x000000ff  /* Mask for type of chain entry:	*/
#define IP_OLD_FW_F_DENY        0x00000000      /* This is a deny rule			*/
#define IP_OLD_FW_F_REJECT      0x00000001      /* Deny and send a response packet	*/
#define IP_OLD_FW_F_ACCEPT      0x00000002      /* This is an accept rule		*/
#define IP_OLD_FW_F_COUNT       0x00000003      /* This is a count rule			*/
#define IP_OLD_FW_F_DIVERT      0x00000004      /* This is a divert rule		*/
#define IP_OLD_FW_F_TEE 0x00000005      /* This is a tee rule			*/
#define IP_OLD_FW_F_SKIPTO      0x00000006      /* This is a skipto rule		*/
#define IP_OLD_FW_F_FWD 0x00000007      /* This is a "change forwarding address" rule */
#define IP_OLD_FW_F_PIPE        0x00000008      /* This is a dummynet rule */

#define IP_OLD_FW_F_IN  0x00000100      /* Check inbound packets		*/
#define IP_OLD_FW_F_OUT 0x00000200      /* Check outbound packets		*/
#define IP_OLD_FW_F_IIFACE      0x00000400      /* Apply inbound interface test		*/
#define IP_OLD_FW_F_OIFACE      0x00000800      /* Apply outbound interface test	*/

#define IP_OLD_FW_F_PRN 0x00001000      /* Print if this rule matches		*/

#define IP_OLD_FW_F_SRNG        0x00002000      /* The first two src ports are a min	*
	                                         * and max range (stored in host byte	*
	                                         * order).				*/

#define IP_OLD_FW_F_DRNG        0x00004000      /* The first two dst ports are a min	*
	                                         * and max range (stored in host byte	*
	                                         * order).				*/

#define IP_OLD_FW_F_FRAG        0x00008000      /* Fragment				*/

#define IP_OLD_FW_F_IIFNAME     0x00010000      /* In interface by name/unit (not IP)	*/
#define IP_OLD_FW_F_OIFNAME     0x00020000      /* Out interface by name/unit (not IP)	*/

#define IP_OLD_FW_F_INVSRC      0x00040000      /* Invert sense of src check		*/
#define IP_OLD_FW_F_INVDST      0x00080000      /* Invert sense of dst check		*/

#define IP_OLD_FW_F_ICMPBIT 0x00100000  /* ICMP type bitmap is valid		*/

#define IP_OLD_FW_F_MASK        0x001FFFFF      /* All possible flag bits mask		*/

/*
 * For backwards compatibility with rules specifying "via iface" but
 * not restricted to only "in" or "out" packets, we define this combination
 * of bits to represent this configuration.
 */

#define IF_OLD_FW_F_VIAHACK     (IP_OLD_FW_F_IN|IP_OLD_FW_F_OUT|IP_OLD_FW_F_IIFACE|IP_OLD_FW_F_OIFACE)

/*
 * Definitions for TCP flags - abridged
 */
#define IP_OLD_FW_TCPF_ESTAB    0x40

#endif /* _IP_FW_COMPAT_H_ */
