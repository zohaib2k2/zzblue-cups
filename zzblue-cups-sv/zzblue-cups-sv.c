#include <stdio.h>

#include <fcntl.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include <cups/cups.h>

#include <sys/stat.h>
#include "../utils.h"



int main(int argc, char *argv[])
{
    //uint32_t svc_uuid_int[] = { 0, 0, 0, 0xABCD };
   
   
    printf("setting up SDP server\n");
    sdp_session_t* session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
	if(!session) {
		printf("fail to SDP connect\n");
		exit(1);
	}
	if(register_service(session,rfcomm_channel,service_name,service_dsc,
                                service_prov) < 0) {
		printf("fail to register service\n");
		exit(1);
	}

    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buf[MAX_FILE_SIZE] = { 0 };
    int s, client, bytes_read;
    socklen_t opt = sizeof(rem_addr);


    char fileName[30];
    int tmpOutFd;

    char numCopies[3];
    int foundDefaultDestination = 0;
    int job_id = 0;
    int num_options = 0;
    cups_option_t *options = NULL;

    cups_dest_t *dest_list;
    cups_dest_t default_dest ;

    //char *printer_status;


    strncpy(numCopies,"1",1);
    
    int numDestination = cupsGetDests2(CUPS_HTTP_DEFAULT, &dest_list);

   
    if(numDestination > 0 )
    {
        for(int loopIndex = 0; loopIndex < numDestination; loopIndex++){
            if (dest_list[loopIndex].is_default) {
                foundDefaultDestination = TRUE;
                default_dest = dest_list[loopIndex];
            }
        }
        // printer_status = "The default printer is ";
        // strcat(printer_status, default_dest.name);
        // write(client,printer_status,sizeof(printer_status));
    }
    else {
        printf("no printer found\n");
        // strcpy(printer_status,"[!!ERROR!!] no printer found");
        // write(client,printer_status,sizeof(printer_status));
        close(client);
        close(s);
        exit(EXIT_FAILURE);
    }

    if(foundDefaultDestination == FALSE){
        printf("[!!] No default detination found exiting...\n");
        sdp_close(session);
        exit(EXIT_FAILURE);
    }
    cups_dinfo_t *info = cupsCopyDestInfo(CUPS_HTTP_DEFAULT, &default_dest);
    
    num_options = cupsAddOption(CUPS_COPIES, numCopies,num_options, &options);
    num_options = cupsAddOption(CUPS_MEDIA, CUPS_MEDIA_A4,num_options, &options);
	num_options = cupsAddOption(CUPS_SIDES,CUPS_SIDES_ONE_SIDED,num_options, &options);

    
    ipp_status_t stat;
	stat = cupsCreateDestJob(CUPS_HTTP_DEFAULT, &default_dest, info,&job_id, "Document", num_options, options); 
	if (stat == IPP_STATUS_OK) 
	{
		printf("\ncreated a job to submit a print\n");
	}
	else{
		//printf("error num %d\n",stat);
        printf("[!!ERROR!!]: \n");
		//printf("Unable to create job: %s\n",
        printf("Unavle to create job: \n");
       // ec_write(client,cupsLastErrorString());
        return 0;
	}

   // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) rfcomm_channel;
    if(bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr))){
        printf("Succefully created the reciving end points...\n");
    }

    // put socket into listening mode
    listen(s, 1);

    // accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);

    char str_address[sizeof(struct sockaddr_rc)];
    ba2str( &rem_addr.rc_bdaddr, str_address);
    
    size_t nbytes;

    if(client > 0){
        tmpOutFd = open(TMP_FILE,
                        O_CREAT | O_RDWR | O_TRUNC, 
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
        
        if(tmpOutFd == -1){
            printf("Failed to open a temporary file \n");
            close(s);
            sdp_close(session);
            close(client);
            
            exit(EXIT_FAILURE);
            
        }

        fprintf(stdout, "accepted connection from %s\n", str_address);
        *buf = '\0';
        printf("Reciving printing data from client...");
        while((nbytes = recv(client,buf,sizeof(buf),0)) != -1){
            printf(".. %ld\n",nbytes);
            if (write(tmpOutFd,buf,nbytes) != nbytes) {
                printf("failed to write complete buffer\n");
                
            } else if( nbytes <= 0){
                break;
            }
        }

        close(client);
        close(s);

    } else {
        fprintf(stderr, "error while accepting connection\n");
        close(s);
        exit(0);
    }
    //give information about default printer
    // read data from the client
    
    printf("Reading data... ");

    
    if(lseek(tmpOutFd, 0 , SEEK_SET) == -1){
        printf("failed to read data\n");
        exit(EXIT_FAILURE);
    }
    nbytes = 0;
	if (cupsStartDestDocument(CUPS_HTTP_DEFAULT, dest_list, info,job_id, fileName,CUPS_FORMAT_TEXT, 0, NULL,1) == HTTP_STATUS_CONTINUE)
	{
       while ((nbytes = read(tmpOutFd,buf,sizeof(buf) )) > 0){
			if (cupsWriteRequestData(CUPS_HTTP_DEFAULT, buf,nbytes) != HTTP_STATUS_CONTINUE)
				break;
		}
			

		if (cupsFinishDestDocument(CUPS_HTTP_DEFAULT, &default_dest,info) == IPP_STATUS_OK){
			printf("Document buffer send succeeded.\n");
		}
		else{
            printf("Document send failed:\n ");
			//ec_write(client,cupsLastErrorString());

		}
	}
    close(tmpOutFd);
    // close connection
    close(client);
    close(s);
    // The rest of the program here
    //sdp_close( session );
    return 0;
}
