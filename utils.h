//#include <bits/stdint-uintn.h>
//#include <bluetooth/bluetooth.h>
//#include <bluetooth/hci.h>
//#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

int ec_write(int fd,const char *buffer);


int register_service(sdp_session_t *session,uint8_t rfcomm_channel,const char *service_name,
                     const char    *service_dsc   ,const char *service_prov);

int get_sdp_service_port(char *target,uint32_t svc_uuid_int[]);


int register_service(sdp_session_t *session,uint8_t     rfcomm_channel, const char *service_name,
                    const char     *svc_dsc,const char *service_prov)
{
	// uint8_t rfcomm_channel = 11;
	// const char *service_name = "Atmel Simple Test Service";
	// const char *svc_dsc = "Simple Test for SDP Register";
	// const char *service_prov = "Tony";
	int ret = 0;

	uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid, 
	   svc_class_uuid;
	sdp_list_t *l2cap_list = 0, 
	       *rfcomm_list = 0,
	       *root_list = 0,
	       *proto_list = 0, 
	       *access_proto_list = 0,
	       *svc_class_list = 0,
	       *profile_list = 0;
	sdp_data_t *channel = 0;
	sdp_profile_desc_t profile;
	sdp_record_t record = { 0 };

	/* set the general service ID */
	sdp_uuid128_create( &svc_uuid, &uuid128 );
	sdp_set_service_id( &record, svc_uuid );

	/* set the service class */
	sdp_uuid16_create(&svc_class_uuid, SERIAL_PORT_SVCLASS_ID);
	svc_class_list = sdp_list_append(0, &svc_class_uuid);
	sdp_set_service_classes(&record, svc_class_list);

	/* set the Bluetooth profile information */
	sdp_uuid16_create(&profile.uuid, SERIAL_PORT_PROFILE_ID);
	profile.version = 0x0100;
	profile_list = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, profile_list);

	/* make the service record publicly browsable */
	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root_list = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups( &record, root_list );

	/* set l2cap information */
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	l2cap_list = sdp_list_append( 0, &l2cap_uuid );
	proto_list = sdp_list_append( 0, l2cap_list );

	/* register the RFCOMM channel for RFCOMM sockets */
	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
	rfcomm_list = sdp_list_append( 0, &rfcomm_uuid );
	sdp_list_append( rfcomm_list, channel );
	sdp_list_append( proto_list, rfcomm_list );

	access_proto_list = sdp_list_append( 0, proto_list );
	sdp_set_access_protos( &record, access_proto_list );

	/* set the name, provider, and description */
	sdp_set_info_attr(&record, service_name, service_prov, svc_dsc);

	/* register the service record, */
	if(sdp_record_register(session, &record, SDP_RECORD_PERSIST) < 0) {
		printf("Service Record registration failed\n");
		ret = -1;
		goto end;
	}

	end:
	// cleanup
	sdp_data_free(channel);
	sdp_list_free(l2cap_list, 0);
	sdp_list_free(rfcomm_list, 0);
	sdp_list_free(root_list, 0);
	sdp_list_free(access_proto_list, 0);
	sdp_list_free(svc_class_list, 0);
	sdp_list_free(profile_list, 0);

	return ret;
}



int get_sdp_service_port(char *str_target,uint32_t svc_uuid_int[]){
    // uint32_t svc_uuid_int[] = { 0, 0, 0, 0xABCD };
    int status;
    bdaddr_t target;
    uuid_t svc_uuid;
    sdp_list_t *response_list, *search_list, *attrid_list;
    sdp_session_t *session = 0;
    uint32_t range = 0x0000ffff;
    uint8_t port = 0;
    

    str2ba( str_target, &target );
    // connect to the SDP server running on the remote machine
    session = sdp_connect( BDADDR_ANY, &target, 0 );
    sdp_uuid128_create( &svc_uuid, &svc_uuid_int );
    search_list = sdp_list_append( 0, &svc_uuid );
    attrid_list = sdp_list_append( 0, &range );
    // get a list of service records that have UUID 0xabcd
    response_list = NULL;
    status = sdp_service_search_attr_req( session, search_list, \
    SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
    if( status == 0 ) {
        sdp_list_t *proto_list;
        sdp_list_t *r = response_list;
        // go through each of the service records
        for (; r; r = r->next ) {
            sdp_record_t *rec = (sdp_record_t*) r->data;
            // get a list of the protocol sequences
            if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {
                // get the RFCOMM port number
                port = sdp_get_proto_port( proto_list, RFCOMM_UUID );
                sdp_list_free( proto_list, 0 );
            }
            sdp_record_free( rec );
        }
    }
    sdp_list_free( response_list, 0 );
    sdp_list_free( search_list, 0 );
    sdp_list_free( attrid_list, 0 );
    sdp_close( session );
    if( port != 0 ) {
        return port;
    }
    return 0;
}

int ec_write(int fd,const char *buffer){
    int len = strlen(buffer);
    int nwriten = 0;
    nwriten = write(fd, buffer,len);
    return nwriten;
}
