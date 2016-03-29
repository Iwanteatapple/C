#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <sys/socket.h>
#include <bluetooth/rfcomm.h>

int port = 11;

sdp_session_t* register_service( )
{
	uint32_t svc_uuid_int[] = { 0 , 0 , 0 , 0xABCD } ;
	uint8_t rfcomm_channel = port;
	const char *service_name = "Roto-Rooter Data Router" ;
	const char *svc_dsc = "An experimental plumbing router" ;
	const char *service_prov = "Roto-Rooter" ;
	
	uuid_t root_uuid , l2cap_uuid , rfcomm_uuid , svc_uuid , svc_class_uuid ;
	sdp_list_t *l2cap_list = 0,*rfcomm_list = 0,*root_list = 0,
				*proto_list = 0,*access_proto_list = 0,
				*svc_class_list = 0,*profile_list = 0;
	sdp_data_t *channel = 0;
	sdp_profile_desc_t profile ;
	sdp_record_t record = { 0 } ;
	sdp_session_t *session = 0;
	
	// set the general service ID	 
	sdp_uuid128_create( &svc_uuid, &svc_uuid_int ) ;
	sdp_set_service_id( &record , svc_uuid ) ;
	
	// set the service class
	sdp_uuid16_create(&svc_class_uuid , SERIAL_PORT_SVCLASS_ID ) ;
	svc_class_list = sdp_list_append(0 , &svc_class_uuid ) ;
	sdp_set_service_classes(&record , svc_class_list ) ;
	
	// set the Bluetooth profile information
	sdp_uuid16_create(&profile.uuid , SERIAL_PORT_PROFILE_ID ) ;
	profile.version = 0x0100 ;
	profile_list = sdp_list_append(0 , &profile ) ;
	sdp_set_profile_descs (&record , profile_list ) ;
	
	// make the service record publicly browsable
	sdp_uuid16_create(&root_uuid , PUBLIC_BROWSE_GROUP ) ;
	root_list = sdp_list_append(0 , &root_uuid ) ;
	sdp_set_browse_groups( &record , root_list ) ;
	
	// set l2cap information
	sdp_uuid16_create(&l2cap_uuid , L2CAP_UUID ) ;
	l2cap_list = sdp_list_append( 0, &l2cap_uuid ) ;
	proto_list = sdp_list_append( 0, l2cap_list ) ;
	
	// register the RFCOMM channel for RFCOMM sockets
	sdp_uuid16_create(&rfcomm_uuid , RFCOMM_UUID ) ;
	channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel ) ;
	rfcomm_list = sdp_list_append( 0, &rfcomm_uuid ) ;
	sdp_list_append( rfcomm_list , channel ) ;
	sdp_list_append( proto_list , rfcomm_list ) ;
	
	access_proto_list = sdp_list_append( 0, proto_list ) ;
	sdp_set_access_protos( &record , access_proto_list ) ;
	
	// set the name, provider, and description
	sdp_set_info_attr(&record , service_name , service_prov , svc_dsc ) ;
	
	// connect to the local SDP server, register the service record,
	// and disconnect
	session = sdp_connect (BDADDR_ANY , BDADDR_LOCAL , SDP_RETRY_IF_BUSY ) ;
	sdp_record_register(session, &record, 0);
	
	// cleanup
	sdp_data_free( channel ) ;
	sdp_list_free( l2cap_list, 0 );
	sdp_list_free( rfcomm_list, 0 );
	sdp_list_free( root_list, 0 );
	sdp_list_free( access_proto_list, 0 );
	sdp_list_free( svc_class_list, 0 );
	sdp_list_free( profile_list, 0 );
	
	return session ;
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

void link_rfc()
{
	//printf("222222222222");
	struct sockaddr_rc loc_addr = { 0 } , rem_addr = { 0 } ;
	char buf[1024] = { 0 } ;
	int s , client , bytes_read ;
	unsigned int opt = sizeof(rem_addr ) ;
	// allocate socket
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM) ;
	
	printf("socket = %d \n",s);
	

	// bind socket to port 1 of the first available bluetooth adapter
	loc_addr.rc_family = AF_BLUETOOTH;
	loc_addr.rc_bdaddr = *BDADDR_ANY;
	loc_addr.rc_channel = port;
	bind (s, (struct sockaddr *)&loc_addr , sizeof(loc_addr ) ) ;

	// put socket into listening mode
	listen(s, 1);

	// accept one connection
	client = accept (s, (struct sockaddr *)&rem_addr, &opt ) ;

	ba2str( &rem_addr.rc_bdaddr,buf ) ;
	fprintf(stderr,"accepted connection from %s\n",buf ) ;
	memset(buf, 0, sizeof(buf ) ) ;

	// read data from the client

	//bytes_read = recv (client,buf,sizeof(buf) , 0);
	//if( bytes_read > 0 ) {
	//printf( "received [%s]\n" , buf ) ;
	//}

	pthread_t send , recv;
	int send_ret , recv_ret;
	void *ret;

	send_ret=pthread_create(&send,NULL,(void *) send_message,(int *)client);
	recv_ret=pthread_create(&recv,NULL,(void *) recv_message,(int *)client);
	pthread_join( send, NULL);
	
	// close connection
	close (client ) ;
	close (s ) ;
}

void thread(void)
{
	sdp_session_t* session = register_service ( ) ;
}

int main( )
{
	pthread_t id;
	int ret;
	ret=pthread_create(&id,NULL,(void *) thread,NULL);
	
//	sdp_session_t* session = register_service ( ) ;
//	sleep( 25 );
//	sdp_close( session ) ;

	printf("run Server\n");
	link_rfc();

	return 0;
}
