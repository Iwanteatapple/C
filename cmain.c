#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>

char trgaddr[19];

void ret_addr()
{
    inquiry_info *devices = NULL;
    int max_rsp , num_rsp;
    int adapter_id , sock , len , flags;
    int i,j;
    char addr[19] = { 0 };
    char name[248] = { 0 };
    
    char numaddr[100][19] = {0};
    int num ;
    
    adapter_id = hci_get_route(NULL);       //int hci_get_route ( bdaddr_t * addr )
    sock = hci_open_dev(adapter_id );       //int hci_open_dev ( int adapter_id )
    if (adapter_id < 0 || sock < 0){
        perror( "opening socket" );
        exit ( 1 ) ;
    }
    
    len = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    devices = (inquiry_info*)malloc(max_rsp*sizeof(inquiry_info));
    
    num_rsp = hci_inquiry(adapter_id , len , max_rsp , NULL, &devices , flags) ;        //int hci_inquiry(int adapter_id, int len, int max_rsp,const uint8_t * lap , inquiry_info * * devs , long flags )
    if( num_rsp < 0 ) perror( "hci_inquiry" ) ;
    
    for (i = 0; i < num_rsp ; i++){
        ba2str (&(devices+i)->bdaddr , addr );      //int str2ba( const char *str, bdaddr_t *ba )   string轉byte
        //int ba2str( const bdaddr_t *ba, char *str )   byte轉string
        memset(name, 0, sizeof(name));
        if (0 != hci_read_remote_name (sock, &(devices+i)->bdaddr ,sizeof(name) , name, 0)){
            //int hci_read remote name(int hci_sock, const bdaddr_t *addr,int len , char *name , int timeout )
            strcpy(name , "[unknown]" );
        }
        printf( "%d : %s %s\n" , i , addr , name ) ;
        
	for(j=0 ; j<19 ; j++){
		numaddr[i][j] = addr[j];	
	}
    }
    free(devices) ;
    close(sock) ;
    

    printf("選擇目標：\n");
    scanf("%d",&num);
    for(j=0 ; j<19 ; j++){
    	trgaddr[j] = numaddr[num][j];
    }
}


int get_portnum()
{
	uint32_t svc_uuid_int[] = { 0 , 0 , 0 , 0xABCD } ;
		
	int status ;
	bdaddr_t target ;
	uuid_t svc_uuid ;
	sdp_list_t *response_list , *search_list , *attrid_list ;
	sdp_session_t *session = 0;
	uint32_t range = 0x0000ffff ;
	uint8_t port = 0;
	
	
	str2ba ( trgaddr , &target ) ;
	
	// connect to the SDP server running on the remote machine
	session = sdp_connect( BDADDR_ANY, &target, 0 );
	
	sdp_uuid128_create( &svc_uuid, &svc_uuid_int ) ;
	search_list = sdp_list_append( 0, &svc_uuid ) ;
	attrid_list = sdp_list_append( 0, &range ) ;
	
	// get a list of service records that have UUID 0xabcd
	response_list = NULL ;
	status = sdp_service_search_attr_req( session , search_list ,
			SDP_ATTR_REQ_RANGE , attrid_list, &response_list ) ;
			
	if( status == 0 ) {
		sdp_list_t *proto_list = NULL ;
		sdp_list_t *r = response_list ;



		int len = 50 ;
		char buf[50] ;
		uuid_t uuid  ;
		sdp_list_t *service_class_list = NULL;
		sdp_list_t *profile_descriptor_list = NULL;
		
		// go through each of the service records
		for ( ; r ; r = r->next ) {
			sdp_record_t *rec = (sdp_record_t * ) r->data ;
						
			printf("---------------------------------------\n");

			// get a list of the protocol sequences
			if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {
				
				// get the RFCOMM port number
				port = sdp_get_proto_port( proto_list , RFCOMM_UUID ) ;
				sdp_list_free( proto_list, 0 );
			}

			if( sdp_get_service_name(rec, buf , len) == 0 ) {
				printf("name = %s\n",buf);
			}

			if( sdp_get_service_desc(rec, buf , len) == 0 ) {
				printf("desc = %s\n",buf);
			}	

			if( sdp_get_provider_name(rec, buf , len) == 0 ) {
				printf("provider = %s\n",buf);
			}
			
			if( sdp_get_service_id(rec, &uuid) == 0 ){
				printf("get uuid\n");
			}

			if( sdp_get_service_classes(rec, &service_class_list) == 0 ){
				printf("get service class\n");
				sdp_list_free( service_class_list, 0 );
			}
			
			if( sdp_get_profile_descs(rec, &profile_descriptor_list) == 0 ){
				printf("get profile descriptor\n");
				sdp_list_free( profile_descriptor_list, 0 );
			}


			sdp_record_free( rec ) ;
		}
	}
	sdp_list_free( response_list, 0 );
	sdp_list_free( search_list, 0 );
	sdp_list_free( attrid_list, 0 );
	sdp_close( session ) ;
	
	if( port != 0 ) {
		printf( "found service running on RFCOMM port %d\n" , port ) ;
		return port ;
	}

	return 0 ;
}

void send_message(int *ptr)
{
	printf("open_send\n");	

	int *s ;
	s = (int *)ptr;
	char buffer[1024];
	
	while(1)
	{
		memset(buffer, 0, sizeof(buffer ) ) ;
		scanf("%s",&buffer);
		send (s , buffer , sizeof(buffer)-1, 0);
		printf("送出:%s\n",buffer);
	}	
}

void recv_message(int *ptr)
{
	printf("open_recv\n");	

	int *s ;
	s = (int *)ptr;
	int read;
	
	while(1)
	{
		char buffer[1024];
		read = recv (s , buffer,sizeof(buffer), 0);
		if(read > 0)
		{
		printf("接收:%s\n",buffer);
		}
	}	
}

void link_rfc(int port)
{
	//printf("111111111");
	struct sockaddr_rc addr = { 0 };
	int s , status;
	
	// allocate a socket
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM) ;
	// set the connection parameters (who to connect to)
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = port;
	str2ba(trgaddr,&addr.rc_bdaddr ) ;
	// connect to server
	status = connect(s, (struct sockaddr *)&addr,sizeof(addr)) ;
	// send a message
	if( 0 == status ) {

	pthread_t send , recv;
	int send_ret , recv_ret;

	send_ret=pthread_create(&send,NULL,(void *) send_message,(int *)s);
	recv_ret=pthread_create(&recv,NULL,(void *) recv_message,(int *)s);
	pthread_join( send, NULL);
	
	//status = send (s , "hollow" , 6, 0);
	}
	if( status < 0 ) perror( "uh oh" ) ;
	close (s) ;
}

int main()
{
    int port ;

    ret_addr();
    printf("%s\n" , trgaddr);

    port = get_portnum();
    printf("%d\n" , port);

    link_rfc(port);
    
    return 0;
}
