/*
 * Author: Nuri Melih Sensoy
 * github.com/nmelihsensoy
 * File: "device_client.c"
 * 
 * This client sends real device events
*/
#include <stdio.h> 
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h> 

#include <linux/input.h>

#define PORT 8080

int device_count = 0;
char **device;
int sock = 0;

void error_handling(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void scan_devices(){
    struct dirent *de;
    int i=0;

    DIR *dr = opendir("/dev/input"); 
  
    if (dr == NULL){ 
        error_handling("Could not open directory");
    } 

    device = malloc(50 * sizeof(char*));

    while((de = readdir(dr)) != NULL){
        if(strncmp("event", de->d_name, 5) == 0){
            char filename[64];
            int fd = -1;
            char name[256] = "???";

            sprintf(filename, "%s%s", "/dev/input/", de->d_name);

            fd = open(filename, O_RDONLY);
            if(fd < 0)
                continue;
            
            ioctl(fd, EVIOCGNAME(sizeof(name)), name);            
            printf("%i - (%s) %s\n", i, de->d_name, name);
            device[i] = malloc(100);
            strncpy(device[i], filename, sizeof(filename));
            i++;
        }    
    }
    device_count = i;
    device = realloc(device,  (i)* sizeof(char*));
    closedir(dr); 
}

void socket_connect(char* server_address){
    struct sockaddr_in serv_addr; 

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
        error_handling("Socket creation error");
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 

	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, server_address, &serv_addr.sin_addr)<=0){ 
		error_handling("Invalid Address"); 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
		error_handling("Connection Failed");
	}
}

int main(int argc, char* argv[]){ 

    char payload[6];
    socket_connect(argv[1]);

    int choice;
    char* selected_dev;

    scan_devices();
    printf("Select Device: ");
    scanf("%d", &choice);
    selected_dev = *(device+choice);
    
    int fd = -1;
    int rd;
    struct input_event ev[64];
    int size = sizeof(struct input_event);
    fd_set set;

    if((fd = open(selected_dev, O_RDONLY)) == 0){
        error_handling("cannot open device");
    }

    //Exclusive access
    ioctl(fd, EVIOCGRAB, 1);

    FD_ZERO(&set);
    FD_SET(fd, &set);

    while(1){
        select(fd+1, &set, NULL, NULL, NULL);
        rd = read(fd, ev, sizeof(ev));

        if(rd < size){
            error_handling("read error");
        }

        for(int i = 0; i < rd / size; i++){
			printf("%s: Type[%d] Code[%d] Value[%d]\n", selected_dev, ev[i].type, ev[i].code, ev[i].value);

            if(ev[i].type != 0){
                if(ev[i].type == 2)
                    sprintf(payload, "%d%d%03d", ev[i].type, ev[i].code, ev[i].value);
                else if(ev[i].type == 1)
                    sprintf(payload, "%d%d%d", ev[i].type, ev[i].value, ev[i].code);

                send(sock , payload, 6, 0);
                printf("%s", payload);
            } 
            sleep(0);
		}
        usleep(10);
    }

    ioctl(fd, EVIOCGRAB, 0);
    close(sock);
    close(fd);
    return 0; 
} 