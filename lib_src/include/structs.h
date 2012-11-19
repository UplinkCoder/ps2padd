/* my OWN UDP/IP STACK */
/* Structurs for Packet-Layout */
#ifndef __STRUCTS_H
#define __STRUCTS_H
#include <stdint.h>
// Magic Numbers & Network stuff
#define ETH_PROTO_TYPE_ARP 	0x0608
#define ETH_PROTO_TYPE_IP  	0x0008
#define IP_PROTO_UDP		0x11
#define IP_PROTO_TCP 		0x09
#define IP_PROTO_ICMP 		0x01
#define MAX_ARP_TABLE_SIZE 	0x10 
// error numbers definitions
#define ERR_ARP_NOT_IN_TABLE 	-1
#define ERR_ARP_TABLE_FULL 		-2
#define ERR_UNKOWN 				-127



// again Network Stuff 





union mac {
	uint8_t mac8[6];
	uint64_t mac64:48;
} __attribute__((packed)); 

 struct eth_hdr { //ethernet header (14 bytes)  
	union mac	EthDst;		//0xFFFFFFFFFFFF=any (broadcast)
	union mac 	EthSrc;
	uint16_t 	ProtocolType;
	} __attribute__((packed));
	
 struct arp_pkg {//arp header (2+2+1+1+2+6+4+6+4=28 bytes)
	struct 		eth_hdr eth_hdr;
	uint16_t 	Hardware; 		//0x0100 (0x00 then 0x01)=Ethernet (10Mbps)
	uint16_t 	Protocol;		//0x0008 (0x08 then 0x00)=IP
	uint8_t 	HardwareAddrLen;	//6 (bytes)
	uint8_t 	ProtocolAddrLen;	//4 (bytes)
	uint16_t 	Operation;		//0x0100 (0x00 then 0x01)=Request 0x0200=Answer
	union mac	SenderHardwareAddr; 
	uint32_t 	SenderProtocolAddr;	//Sender IP address
	union mac	TargetHardwareAddr;	//0 if not known yet
	uint32_t	TargetProtocolAddr;	//Target IP address
	} __attribute__((packed));
	
	
struct arp_entry { // arp table entry (10 bytes) // you should consider adding time
	union mac mac;
	uint32_t	ip;
	} __attribute__((packed));

	
 struct ip_hdr { //ip header (20 bytes)
	//	unsigned 		Version:4;		//4 bits, value 4	
	//	unsigned 		HeaderLength:4;		//4 bits, value 5 (means 20 bytes)
	uint8_t		Version; // IP Version and HeaderLength = 0x45
	uint8_t 	DifferentiatedServices;	//0
	uint16_t	TotalLength;		//mostly 44 bytes (in case of PS2PADd) 
	uint16_t	Identifier;		//Variant
	uint16_t	FragmentationFlagOffset;//0
	uint8_t 	TimeToLive;		//128
	uint8_t 	Protocol;		//17 = UDP (User Datagram Protocol)
	uint16_t	HeaderChecksum;		//Variant
	uint32_t 	SrcIPAddr;		//Sender IP address
	uint32_t	DstIPAddr;		//Target IP address
	} __attribute__((packed));

	struct icmp_hdr {
	uint8_t 	ICMPType;		//8 = Echo request
	uint8_t		ICMPCode;		//0
	uint16_t	Checksum;		//Variant
	uint16_t 	ID;			//Variant
	uint16_t	SequenceNumber;		//0 then increments with retries
	uint8_t  	ICMPDataArea[18];
	} __attribute__((packed));
	
struct icmp_pkg{
	struct eth_hdr eth_hdr;
	struct ip_hdr ip_hdr;
	struct icmp_hdr icmp_hdr;
	}  __attribute__((packed));

	
struct udp_hdr { //udp header (8 bytes)
	uint16_t	UDPSrcPort;
	uint16_t	UDPDstPort;
	uint16_t	Length;
	uint16_t	Checksum;
	} __attribute__((packed));
	
struct udp_pkg { //udp packet 
	struct eth_hdr eth_hdr;
	struct ip_hdr ip_hdr;
	struct udp_hdr udp_hdr;
	} __attribute__((packed));

#endif
