/***********************************************************************
	Copyright (c) 2017, The OpenBRAS project authors. All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:

	  * Redistributions of source code must retain the above copyright
		notice, this list of conditions and the following disclaimer.

	  * Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in
		the documentation and/or other materials provided with the
		distribution.

	  * Neither the name of OpenBRAS nor the names of its contributors may
		be used to endorse or promote products derived from this software
		without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************/


#ifndef PPPDEF_H
#define PPPDEF_H
#include "aceinclude.h"
 #include <arpa/inet.h>
#include <list>

#define DEFMRU	1500		/* Try for this */
#define MINMRU	128		/* No MRUs below this */
#define MAXMRU	16384		/* Normally limit MRU to this */


/*
 * The basic PPP frame.
 */
#define PPP_HDRLEN	4	/* octets for standard ppp header */
#define PPP_FCSLEN	2	/* octets for FCS */
#define PPP_MRU		1500	/* default MRU = max length of info field */

#define PPP_ADDRESS(p)	(((__u8 *)(p))[0])
#define PPP_CONTROL(p)	(((__u8 *)(p))[1])
#define PPP_PROTOCOL(p)	((((__u8 *)(p))[2] << 8) + ((__u8 *)(p))[3])

/*
 * Significant octet values.
 */
#define	PPP_ALLSTATIONS	0xff	/* All-Stations broadcast address */
#define	PPP_UI		0x03	/* Unnumbered Information */
#define	PPP_FLAG	0x7e	/* Flag Sequence */
#define	PPP_ESCAPE	0x7d	/* Asynchronous Control Escape */
#define	PPP_TRANS	0x20	/* Asynchronous transparency modifier */

/*
 * Protocol field values.
 */
#define PPP_IP		0x21	/* Internet Protocol */
#define PPP_AT		0x29	/* AppleTalk Protocol */
#define PPP_IPX		0x2b	/* IPX protocol */
#define	PPP_VJC_COMP	0x2d	/* VJ compressed TCP */
#define	PPP_VJC_UNCOMP	0x2f	/* VJ uncompressed TCP */
#define PPP_MP		0x3d	/* Multilink protocol */
#define PPP_IPV6	0x57	/* Internet Protocol Version 6 */
#define PPP_COMPFRAG	0xfb	/* fragment compressed below bundle */
#define PPP_COMP	0xfd	/* compressed packet */
#define PPP_IPCP	0x8021	/* IP Control Protocol */
#define PPP_ATCP	0x8029	/* AppleTalk Control Protocol */
#define PPP_IPXCP	0x802b	/* IPX Control Protocol */
#define PPP_IPV6CP	0x8057	/* IPv6 Control Protocol */
#define PPP_CCPFRAG	0x80fb	/* CCP at link level (below MP bundle) */
#define PPP_CCP		0x80fd	/* Compression Control Protocol */
#define PPP_ECPFRAG	0x8055	/* ECP at link level (below MP bundle) */
#define PPP_ECP		0x8053	/* Encryption Control Protocol */
#define PPP_LCP		0xc021	/* Link Control Protocol */
#define PPP_PAP		0xc023	/* Password Authentication Protocol */
#define PPP_LQR		0xc025	/* Link Quality Report protocol */
#define PPP_CHAP	0xc223	/* Cryptographic Handshake Auth. Protocol */
#define PPP_CBCP	0xc029	/* Callback Control Protocol */


#define HEADERLEN	4


/*
 *  CP (LCP, IPCP, etc.) codes.
 */
#define CONFREQ		1	/* Configuration Request */
#define CONFACK		2	/* Configuration Ack */
#define CONFNAK		3	/* Configuration Nak */
#define CONFREJ		4	/* Configuration Reject */
#define TERMREQ		5	/* Termination Request */
#define TERMACK		6	/* Termination Ack */
#define CODEREJ		7	/* Code Reject */

/*
 * Link states.
 */
#define INITIAL		0	/* Down, hasn't been opened */
#define STARTING	1	/* Down, been opened */
#define CLOSED		2	/* Up, hasn't been opened */
#define STOPPED		3	/* Open, waiting for down event */
#define CLOSING		4	/* Terminating the connection, not open */
#define STOPPING	5	/* Terminating, but open */
#define REQSENT		6	/* We've sent a Config Request */
#define ACKRCVD		7	/* We've received a Config Ack */
#define ACKSENT		8	/* We've sent a Config Ack */
#define OPENED		9	/* Connection available */


/*
 * Flags - indicate options controlling FSM operation
 */
#define OPT_PASSIVE	1	/* Don't die if we don't get a response */
#define OPT_RESTART	2	/* Treat 2nd OPEN as DOWN, UP */
#define OPT_SILENT	4	/* Wait for peer to speak first */


/*
 * Timeouts.
 */
#define DEFTIMEOUT	3	/* Timeout time in seconds */
#define DEFMAXTERMREQS	2	/* Maximum Terminate-Request transmissions */
#define DEFMAXCONFREQS	10	/* Maximum Configure-Request transmissions */
#define DEFMAXNAKLOOPS	5	/* Maximum number of nak loops */


#define NUM_PPP		1	/* One PPP interface supported (per process) */
#define MAXWORDLEN	1024	/* max length of word in file (incl null) */
#define MAXARGS		1	/* max # args to a command */
//#define MAXNAMELEN	256	/* max length of hostname or name for auth */
#define MAXSECRETLEN	256	/* max length of password or secret */


/*
 * LCP-specific packet types (code numbers).
 */
#define PROTREJ		8	/* Protocol Reject */
#define ECHOREQ		9	/* Echo Request */
#define ECHOREP		10	/* Echo Reply */
#define DISCREQ		11	/* Discard Request */
#define IDENTIF		12	/* Identification */
#define TIMEREM		13	/* Time Remaining */

/* Value used as data for CI_CALLBACK option */
#define CBCP_OPT	6	/* Use callback control protocol */

#define CI_VENDOR	0	/* Vendor Specific */
#define CI_MRU		1	/* Maximum Receive Unit */
#define CI_ASYNCMAP	2	/* Async Control Character Map */
#define CI_AUTHTYPE	3	/* Authentication Type */
#define CI_QUALITY	4	/* Quality Protocol */
#define CI_MAGICNUMBER	5	/* Magic Number */
#define CI_PCOMPRESSION	7	/* Protocol Field Compression */
#define CI_ACCOMPRESSION 8	/* Address/Control Field Compression */
#define CI_FCSALTERN	9	/* FCS-Alternatives */
#define CI_SDP		10	/* Self-Describing-Pad */
#define CI_NUMBERED	11	/* Numbered-Mode */
#define CI_CALLBACK	13	/* callback */
#define CI_MRRU		17	/* max reconstructed receive unit; multilink */
#define CI_SSNHF	18	/* short sequence numbers for multilink */
#define CI_EPDISC	19	/* endpoint discriminator */
#define CI_MPPLUS	22	/* Multi-Link-Plus-Procedure */
#define CI_LDISC	23	/* Link-Discriminator */
#define CI_LCPAUTH	24	/* LCP Authentication */
#define CI_COBS		25	/* Consistent Overhead Byte Stuffing */
#define CI_PREFELIS	26	/* Prefix Elision */
#define CI_MPHDRFMT	27	/* MP Header Format */
#define CI_I18N		28	/* Internationalization */
#define CI_SDL		29	/* Simple Data Link */

/*
 * LCP-specific packet types (code numbers).
 */
#define PROTREJ		8	/* Protocol Reject */
#define ECHOREQ		9	/* Echo Request */
#define ECHOREP		10	/* Echo Reply */
#define DISCREQ		11	/* Discard Request */
#define IDENTIF		12	/* Identification */
#define TIMEREM		13	/* Time Remaining */

/* Value used as data for CI_CALLBACK option */
#define CBCP_OPT	6	/* Use callback control protocol */


/*
 * The basic PPP frame.
 */
#define PPP_HDRLEN	      4	/* octets for standard ppp header */
#define PPP_FCSLEN	      2	/* octets for FCS */
#define PPP_ADDR_CTRL_LEN 2 /* octets for Address and Control fields */
/*
 * Packet sizes
 *
 * Note - lcp shouldn't be allowed to negotiate stuff outside these
 *	  limits.  See lcp.h in the pppd directory.
 * (XXX - these constants should simply be shared by lcp.c instead
 *	  of living in lcp.h)
 */
#define	PPP_MTU		1500	/* Default MTU (size of Info field) */
#define PPP_MAXMTU	65535 - (PPP_HDRLEN + PPP_FCSLEN)
#define PPP_MINMTU	64
#define PPP_MRU		1500	/* default MRU = max length of info field */
#define PPP_MAXMRU	65000	/* Largest MRU we allow */
#define PPP_MINMRU	128

/*
 * Significant octet values.
 */
#define	PPP_ALLSTATIONS	0xff	/* All-Stations broadcast address */
#define	PPP_UI		0x03	/* Unnumbered Information */
#define	PPP_FLAG	0x7e	/* Flag Sequence */
#define	PPP_ESCAPE	0x7d	/* Asynchronous Control Escape */
#define	PPP_TRANS	0x20	/* Asynchronous transparency modifier */

/*
 * Protocol field values.
 */
#define PPP_IP		0x21	/* Internet Protocol */
#define PPP_AT		0x29	/* AppleTalk Protocol */
#define PPP_IPX		0x2b	/* IPX protocol */
#define	PPP_VJC_COMP	0x2d	/* VJ compressed TCP */
#define	PPP_VJC_UNCOMP	0x2f	/* VJ uncompressed TCP */
#define PPP_IPV6	0x57	/* Internet Protocol Version 6 */
#define PPP_COMP	0xfd	/* compressed packet */
#define PPP_IPCP	0x8021	/* IP Control Protocol */
#define PPP_ATCP	0x8029	/* AppleTalk Control Protocol */
#define PPP_IPXCP	0x802b	/* IPX Control Protocol */
#define PPP_IPV6CP	0x8057	/* IPv6 Control Protocol */
#define PPP_CCP		0x80fd	/* Compression Control Protocol */
#define PPP_ECP		0x8053	/* Encryption Control Protocol */
#define PPP_LCP		0xc021	/* Link Control Protocol */
#define PPP_PAP		0xc023	/* Password Authentication Protocol */
#define PPP_LQR		0xc025	/* Link Quality Report protocol */
#define PPP_CHAP	0xc223	/* Cryptographic Handshake Auth. Protocol */
#define PPP_CBCP	0xc029	/* Callback Control Protocol */
#define PPP_EAP		0xc227	/* Extensible Authentication Protocol */

/*
 * Values for FCS calculations.
 */
#define PPP_INITFCS	0xffff	/* Initial FCS value */
#define PPP_GOODFCS	0xf0b8	/* Good final FCS value */
#define PPP_FCS(fcs, c)	(((fcs) >> 8) ^ fcstab[((fcs) ^ (c)) & 0xff])



/*
 * Inline versions of get/put char/short/long.
 * Pointer is advanced; we assume that both arguments
 * are lvalues and will already be in registers.
 * cp MUST be u_char *.
 */
#define GETCHAR(c, cp) { \
	(c) = *(cp)++; \
}
#define PUTCHAR(c, cp) { \
	*(cp)++ = (u_char) (c); \
}


#define GETSHORT(s, cp) { \
	(s) = *(cp)++ << 8; \
	(s) |= *(cp)++; \
}
#define PUTSHORT(s, cp) { \
	*(cp)++ = (u_char) ((s) >> 8); \
	*(cp)++ = (u_char) (s); \
}

#define GETLONG(l, cp) { \
	(l) = *(cp)++ << 8; \
	(l) |= *(cp)++; (l) <<= 8; \
	(l) |= *(cp)++; (l) <<= 8; \
	(l) |= *(cp)++; \
}
#define PUTLONG(l, cp) { \
	*(cp)++ = (u_char) ((l) >> 24); \
	*(cp)++ = (u_char) ((l) >> 16); \
	*(cp)++ = (u_char) ((l) >> 8); \
	*(cp)++ = (u_char) (l); \
}

#define INCPTR(n, cp)	((cp) += (n))
#define DECPTR(n, cp)	((cp) -= (n))

/*
 * System dependent definitions for user-level 4.3BSD UNIX implementation.
 */

#define TIMEOUT(r, f, t)	timeout((r), (f), (t), 0)
#define UNTIMEOUT(r, f)		untimeout((r), (f))

#define BCOPY(s, d, l)		memcpy(d, s, l)
#define BZERO(s, n)		memset(s, 0, n)
#define	BCMP(s1, s2, l)		memcmp(s1, s2, l)

#define PRINTMSG(m, l)		{ info("Remote message: %0.*v", l, m); }

/*
 * MAKEHEADER - Add Header fields to a packet.
 */
#define MAKEHEADER(p, t) { \
    PUTCHAR(PPP_ALLSTATIONS, p); \
    PUTCHAR(PPP_UI, p); \
    PUTSHORT(t, p); }

/*
 * Exit status values.
 */
#define EXIT_OK			0
#define EXIT_FATAL_ERROR	1
#define EXIT_OPTION_ERROR	2
#define EXIT_NOT_ROOT		3
#define EXIT_NO_KERNEL_SUPPORT	4
#define EXIT_USER_REQUEST	5
#define EXIT_LOCK_FAILED	6
#define EXIT_OPEN_FAILED	7
#define EXIT_CONNECT_FAILED	8
#define EXIT_PTYCMD_FAILED	9
#define EXIT_NEGOTIATION_FAILED	10
#define EXIT_PEER_AUTH_FAILED	11
#define EXIT_IDLE_TIMEOUT	12
#define EXIT_CONNECT_TIME	13
#define EXIT_CALLBACK		14
#define EXIT_PEER_DEAD		15
#define EXIT_HANGUP		16
#define EXIT_LOOPBACK		17
#define EXIT_INIT_FAILED	18
#define EXIT_AUTH_TOPEER_FAILED	19
#ifdef MAXOCTETS
#define EXIT_TRAFFIC_LIMIT	20
#endif
#define EXIT_CNID_AUTH_FAILED	21

/*
 * Debug macros.  Slightly useful for finding bugs in pppd, not particularly
 * useful for finding out why your connection isn't being established.
 */
#ifdef DEBUGALL
#define DEBUGMAIN	1
#define DEBUGFSM	1
#define DEBUGLCP	1
#define DEBUGIPCP	1
#define DEBUGIPV6CP	1
#define DEBUGUPAP	1
#define DEBUGCHAP	1
#endif

#ifndef LOG_PPP			/* we use LOG_LOCAL2 for syslog by default */
#if defined(DEBUGMAIN) || defined(DEBUGFSM) || defined(DEBUGSYS) \
  || defined(DEBUGLCP) || defined(DEBUGIPCP) || defined(DEBUGUPAP) \
  || defined(DEBUGCHAP) || defined(DEBUG) || defined(DEBUGIPV6CP)
#define LOG_PPP LOG_LOCAL2
#else
#define LOG_PPP LOG_DAEMON
#endif
#endif /* LOG_PPP */

#ifdef DEBUGMAIN
#define MAINDEBUG(x)	if (debug) dbglog x
#else
#define MAINDEBUG(x)
#endif

#ifdef DEBUGSYS
#define SYSDEBUG(x)	if (debug) dbglog x
#else
#define SYSDEBUG(x)
#endif

#ifdef DEBUGFSM
#define FSMDEBUG(x)	if (debug) dbglog x
#else
#define FSMDEBUG(x)
#endif

#ifdef DEBUGLCP
#define LCPDEBUG(x)	if (debug) dbglog x
#else
#define LCPDEBUG(x)
#endif

#ifdef DEBUGIPCP
#define IPCPDEBUG(x)	if (debug) dbglog x
#else
#define IPCPDEBUG(x)
#endif

#ifdef DEBUGIPV6CP
#define IPV6CPDEBUG(x)  if (debug) dbglog x
#else
#define IPV6CPDEBUG(x)
#endif

#ifdef DEBUGUPAP
#define UPAPDEBUG(x)	if (debug) dbglog x
#else
#define UPAPDEBUG(x)
#endif

#ifdef DEBUGCHAP
#define CHAPDEBUG(x)	if (debug) dbglog x
#else
#define CHAPDEBUG(x)
#endif

#ifdef DEBUGIPXCP
#define IPXCPDEBUG(x)	if (debug) dbglog x
#else
#define IPXCPDEBUG(x)
#endif

#ifndef SIGTYPE
#if defined(sun) || defined(SYSV) || defined(POSIX_SOURCE)
#define SIGTYPE void
#else
#define SIGTYPE int
#endif /* defined(sun) || defined(SYSV) || defined(POSIX_SOURCE) */
#endif /* SIGTYPE */

#ifndef MIN
#define MIN(a, b)	((a) < (b)? (a): (b))
#endif
#ifndef MAX
#define MAX(a, b)	((a) > (b)? (a): (b))
#endif

#ifndef offsetof
#define offsetof(type, member) ((size_t) &((type *)0)->member)
#endif


typedef struct lcp_options {
    bool passive;		/* Don't die if we don't get a response */
    bool silent;		/* Wait for the other end to start first */
    bool restart;		/* Restart vs. exit after close */
    bool neg_mru;		/* Negotiate the MRU? */
    bool neg_asyncmap;		/* Negotiate the async map? */
    bool neg_upap;		/* Ask for UPAP authentication? */
    bool neg_chap;		/* Ask for CHAP authentication? */
    bool neg_eap;		/* Ask for EAP authentication? */
    bool neg_magicnumber;	/* Ask for magic number? */
    bool neg_pcompression;	/* HDLC Protocol Field Compression? */
    bool neg_accompression;	/* HDLC Address/Control Field Compression? */
    bool neg_lqr;		/* Negotiate use of Link Quality Reports */
    bool neg_cbcp;		/* Negotiate use of CBCP */
    bool neg_mrru;		/* negotiate multilink MRRU */
    bool neg_ssnhf;		/* negotiate short sequence numbers */
    bool neg_endpoint;		/* negotiate endpoint discriminator */
    int  mru;			/* Value of MRU */
    int	 mrru;			/* Value of MRRU, and multilink enable */
    u_char chap_mdtype;		/* which MD types (hashing algorithm) */
    uint32_t asyncmap;		/* Value of async map */
    uint32_t magicnumber;
    int  numloops;		/* Number of loops during magic number neg. */
    uint32_t lqr_period;	/* Reporting period for LQR 1/100ths second */
} lcp_options;

#define LCPDEBUG(x)

/*
 * Length of each type of configuration option (in octets)
 */
#define CILEN_VOID	2
#define CILEN_CHAR	3
#define CILEN_SHORT	4	/* CILEN_VOID + 2 */
#define CILEN_CHAP	5	/* CILEN_VOID + 2 + 1 */
#define CILEN_LONG	6	/* CILEN_VOID + 4 */
#define CILEN_LQR	8	/* CILEN_VOID + 2 + 4 */
#define CILEN_CBCP	3

#define CODENAME(x)	((x) == CONFACK ? "ACK" : \
    (x) == CONFNAK ? "NAK" : "REJ") 


/*
 * CHAP packets begin with a standard header with code, id, len (2 bytes).
 */
#define CHAP_HDRLEN	4

/*
 * Values for the code field.
 */
#define CHAP_CHALLENGE	1
#define CHAP_RESPONSE	2
#define CHAP_SUCCESS	3
#define CHAP_FAILURE	4

/*
 * CHAP digest codes.
 */
#define CHAP_MD5		5
#define CHAP_MICROSOFT		0x80
#define CHAP_MICROSOFT_V2	0x81

/*
 * Semi-arbitrary limits on challenge and response fields.
 */
#define MAX_CHALLENGE_LEN	64
#define MAX_RESPONSE_LEN	64

/* bitmask of supported algorithms */
#define MDTYPE_MICROSOFT_V2	0x1
#define MDTYPE_MICROSOFT	0x2
#define MDTYPE_MD5		0x4
#define MDTYPE_NONE		0

#define MD4_SIGNATURE_SIZE	16	/* 16 bytes in a MD4 message digest */
#define MAX_NT_PASSWORD		256	/* Max (Unicode) chars in an NT pass */

#define MS_CHAP_RESPONSE_LEN	49	/* Response length for MS-CHAP */
#define MS_CHAP2_RESPONSE_LEN	49	/* Response length for MS-CHAPv2 */
#define MS_AUTH_RESPONSE_LENGTH	40	/* MS-CHAPv2 authenticator response, */
					/* as ASCII */

/* E=eeeeeeeeee error codes for MS-CHAP failure messages. */
#define MS_CHAP_ERROR_RESTRICTED_LOGON_HOURS	646
#define MS_CHAP_ERROR_ACCT_DISABLED		647
#define MS_CHAP_ERROR_PASSWD_EXPIRED		648
#define MS_CHAP_ERROR_NO_DIALIN_PERMISSION	649
#define MS_CHAP_ERROR_AUTHENTICATION_FAILURE	691
#define MS_CHAP_ERROR_CHANGING_PASSWORD		709

/*
 * Offsets within the response field for MS-CHAP
 */
#define MS_CHAP_LANMANRESP	0
#define MS_CHAP_LANMANRESP_LEN	24
#define MS_CHAP_NTRESP		24
#define MS_CHAP_NTRESP_LEN	24
#define MS_CHAP_USENT		48

/*
 * Offsets within the response field for MS-CHAP2
 */
#define MS_CHAP2_PEER_CHALLENGE	0
#define MS_CHAP2_PEER_CHAL_LEN	16
#define MS_CHAP2_RESERVED_LEN	8
#define MS_CHAP2_NTRESP		24
#define MS_CHAP2_NTRESP_LEN	24
#define MS_CHAP2_FLAGS		48

/* Are we the authenticator or authenticatee?  For MS-CHAPv2 key derivation. */
#define MS_CHAP2_AUTHENTICATEE 0
#define MS_CHAP2_AUTHENTICATOR 1

#ifdef CHAPMS
#include "chap_ms.h"
#define MDTYPE_ALL (MDTYPE_MICROSOFT_V2 | MDTYPE_MICROSOFT | MDTYPE_MD5)
#else
#define MDTYPE_ALL (MDTYPE_MD5)
#endif


/* Return the digest alg. ID for the most preferred digest type. */
#define CHAP_DIGEST(mdtype) \
    ((mdtype) & MDTYPE_MD5)? CHAP_MD5: \
    ((mdtype) & MDTYPE_MICROSOFT_V2)? CHAP_MICROSOFT_V2: \
    ((mdtype) & MDTYPE_MICROSOFT)? CHAP_MICROSOFT: \
    0

/* Return the bit flag (lsb set) for our most preferred digest type. */
#define CHAP_MDTYPE(mdtype) ((mdtype) ^ ((mdtype) - 1)) & (mdtype)

/* Return the bit flag for a given digest algorithm ID. */
#define CHAP_MDTYPE_D(digest) \
    ((digest) == CHAP_MICROSOFT_V2)? MDTYPE_MICROSOFT_V2: \
    ((digest) == CHAP_MICROSOFT)? MDTYPE_MICROSOFT: \
    ((digest) == CHAP_MD5)? MDTYPE_MD5: \
    0

/* Can we do the requested digest? */
#define CHAP_CANDIGEST(mdtype, digest) \
    ((digest) == CHAP_MICROSOFT_V2)? (mdtype) & MDTYPE_MICROSOFT_V2: \
    ((digest) == CHAP_MICROSOFT)? (mdtype) & MDTYPE_MICROSOFT: \
    ((digest) == CHAP_MD5)? (mdtype) & MDTYPE_MD5: \
    0
	
/*ppp ipcp */

#define CI_ADDRS	1       /* IP Addresses */
#define CI_COMPRESSTYPE	2   /* Compression Type */
#define	CI_ADDR         3

#define CI_MS_DNS1  129	/* Primary DNS value */
#define CI_MS_WINS1 130	/* Primary WINS value */
#define CI_MS_DNS2  131	/* Secondary DNS value */
#define CI_MS_WINS2 132	/* Secondary WINS value */

#define MAX_STATES 16       /* from slcompress.h */

#define IPCP_VJMODE_OLD 1       /* "old" mode (option # = 0x0037) */
#define IPCP_VJMODE_RFC1172 2   /* "old-rfc"mode (option # = 0x002d) */
#define IPCP_VJMODE_RFC1332 3   /* "new-rfc"mode (option # = 0x002d, */
                                /*  maxslot and slot number compression) */

#define IPCP_VJ_COMP 0x002d     /* current value for VJ compression option*/
#define IPCP_VJ_COMP_OLD 0x0037     /* "old" (i.e, broken) value for VJ */
/* compression option*/ 

/*
 * Lengths of configuration options.
 */
#define CILEN_VOID	2
#define CILEN_COMPRESS	4	/* min length for compression protocol opt. */
#define CILEN_VJ	6	/* length for RFC1332 Van-Jacobson opt. */
#define CILEN_ADDR	6	/* new-style single address option */
#define CILEN_ADDRS	10	/* old-style dual address option */


#define CODENAME(x)	((x) == CONFACK ? "ACK" : \
			 (x) == CONFNAK ? "NAK" : "REJ")


#define IPCPDEBUG(x)

typedef struct ipcp_options {
    bool neg_addr;		/* Negotiate IP Address? */
    bool old_addrs;		/* Use old (IP-Addresses) option? */
    bool req_addr;		/* Ask peer to send IP address? */
    bool default_route;		/* Assign default route through interface? */
    bool proxy_arp;		/* Make proxy ARP entry for peer? */
    bool neg_vj;		/* Van Jacobson Compression? */
    bool old_vj;		/* use old (short) form of VJ option? */
    bool accept_local;		/* accept peer's value for ouraddr */
    bool accept_remote;		/* accept peer's value for hisaddr */
    bool req_dns1;		/* Ask peer to send primary DNS address? */
    bool req_dns2;		/* Ask peer to send secondary DNS address? */
    int  vj_protocol;		/* protocol value to use in VJ option */
    int  maxslotindex;		/* values for RFC1332 VJ compression neg. */
    bool cflag;
    uint32_t ouraddr, hisaddr;	/* Addresses in NETWORK BYTE ORDER */
    uint32_t dnsaddr[2];	/* Primary and secondary MS DNS entries. Addresses in NETWORK BYTE ORDER */
    uint32_t winsaddr[2];	/* Primary and secondary MS WINS entries. Addresses in NETWORK BYTE ORDER */
} ipcp_options;

typedef uint32_t u_int32_t;


/* Values for flags in chap_client_state and chap_server_state */
#define LOWERUP			1
#define AUTH_STARTED		2
#define AUTH_DONE		4
#define AUTH_FAILED		8
#define TIMEOUT_PENDING		0x10
#define CHALLENGE_VALID		0x20


/*
 * Option descriptor structure.
 */

enum opt_type {
    o_special_noarg = 0,
    o_special = 1,
    o_bool,
    o_int,
    o_uint32,
    o_string,
    o_wild
};

typedef struct {
    char	*name;		/* name of the option */
    enum opt_type type;
    void	*addr;
    char	*description;
    unsigned int flags;
    void	*addr2;
    int	upper_limit;
    int	lower_limit;
    const char *source;
    short int priority;
    short int winner;
} option_t;

/* Values for flags */
#define OPT_VALUE	0xff	/* mask for presupplied value */
#define OPT_HEX		0x100	/* int option is in hex */
#define OPT_NOARG	0x200	/* option doesn't take argument */
#define OPT_OR		0x400	/* for u32, OR in argument to value */
#define OPT_INC		0x400	/* for o_int, increment value */
#define OPT_A2OR	0x800	/* for o_bool, OR arg to *(u_char *)addr2 */
#define OPT_PRIV	0x1000	/* privileged option */
#define OPT_STATIC	0x2000	/* string option goes into static array */
#define OPT_NOINCR	0x2000	/* for o_int, value mustn't be increased */
#define OPT_LLIMIT	0x4000	/* check value against lower limit */
#define OPT_ULIMIT	0x8000	/* check value against upper limit */
#define OPT_LIMITS	(OPT_LLIMIT|OPT_ULIMIT)
#define OPT_ZEROOK	0x10000	/* 0 value is OK even if not within limits */
#define OPT_HIDE	0x10000	/* for o_string, print value as ?????? */
#define OPT_A2LIST	0x20000 /* for o_special, keep list of values */
#define OPT_A2CLRB	0x20000 /* o_bool, clr val bits in *(u_char *)addr2 */
#define OPT_ZEROINF	0x40000	/* with OPT_NOINCR, 0 == infinity */
#define OPT_PRIO	0x80000	/* process option priorities for this option */
#define OPT_PRIOSUB	0x100000 /* subsidiary member of priority group */
#define OPT_ALIAS	0x200000 /* option is alias for previous option */
#define OPT_A2COPY	0x400000 /* addr2 -> second location to rcv value */
#define OPT_ENABLE	0x800000 /* use *addr2 as enable for option */
#define OPT_A2CLR	0x1000000 /* clear *(bool *)addr2 */
#define OPT_PRIVFIX	0x2000000 /* user can't override if set by root */
#define OPT_INITONLY	0x4000000 /* option can only be set in init phase */
#define OPT_DEVEQUIV	0x8000000 /* equiv to device name */
#define OPT_DEVNAM	(OPT_INITONLY | OPT_DEVEQUIV)
#define OPT_A2PRINTER	0x10000000 /* *addr2 printer_func to print option */
#define OPT_A2STRVAL	0x20000000 /* *addr2 points to current string value */
#define OPT_NOPRINT	0x40000000 /* don't print this option at all */

#define OPT_VAL(x)	((x) & OPT_VALUE)

/* Values for priority */
#define OPRIO_DEFAULT	0	/* a default value */
#define OPRIO_CFGFILE	1	/* value from a configuration file */
#define OPRIO_CMDLINE	2	/* value from the command line */
#define OPRIO_SECFILE	3	/* value from options in a secrets file */
#define OPRIO_ROOT	100	/* added to priority if OPT_PRIVFIX && root */


#define VPN_PUBLIC


#endif//PPPDEF_H


