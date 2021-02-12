/*
 * build: 
 * gcc -o wserver wserver.c -lbluetooth -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>

#define HC6ADDRESS  "00:20:10:08:FB:8A"
#define MAIN_CYCLE_PERIOD_SEC   (300)   

int main(int argc, char **argv)
{
    static int run_server = 1;
    long    time_millis;
    long    time_millis_prev;
    time_t  time_seconds;
    time_t  time_seconds_prev;
    struct  timespec timestamp;
    long    currentTimeMs;
    long    previousTimeMs;
    
    // Scan devices init
    inquiry_info* devices = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };

    // for RFCOMM client
    struct sockaddr_rc address = {0};
    int s, status;
    char dest[18] = HC6ADDRESS;
    int bytes_read;
    char readBuffer[200] = {0};

    FILE* pDataLogFile;
    pDataLogFile = fopen("datalog.txt", "a+");
    
    clock_gettime(CLOCK_MONOTONIC, &timestamp);       // CLOCK_REALTIME
    time_seconds_prev = timestamp.tv_sec;
    time_millis_prev = round(timestamp.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (time_millis_prev > 999) {
        time_seconds_prev ++;
        time_millis_prev = 0;
    }
    printf("TS: %ld.%ld s\n", time_seconds_prev, time_millis_prev);
    printf("size of long = %ld\n", sizeof(long));

    // dev_id = hci_get_route(NULL);
    // sock = hci_open_dev( dev_id );
    // if (dev_id < 0 || sock < 0) {
    //     perror("opening socket");
    //     exit(1);
    // }

    // len = 8;
    // max_rsp = 255;
    // flags = IREQ_CACHE_FLUSH;
    // devices = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
    
    // num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &devices, flags);
    // if( num_rsp < 0 ) perror("hci_inquiry");

    // for (i = 0; i < num_rsp; i++) {
    //     ba2str(&(devices+i)->bdaddr, addr);
    //     memset(name, 0, sizeof(name));
    //     if (hci_read_remote_name(sock, &(devices+i)->bdaddr, sizeof(name), 
    //         name, 0) < 0)
    //     strcpy(name, "[unknown]");
    //     printf("%s  %s\n", addr, name);
    // }

    // free(devices);
    // close(sock);

    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    previousTimeMs = timestamp.tv_sec; // * 1000 + timestamp.tv_nsec / 1000;
    printf("---> previousTimeMs (sec) = %ld\n", previousTimeMs);

    while (run_server)
    {
        clock_gettime(CLOCK_MONOTONIC, &timestamp);
        currentTimeMs = timestamp.tv_sec; // * 1000 + timestamp.tv_nsec / 1000;

        if (currentTimeMs - previousTimeMs > MAIN_CYCLE_PERIOD_SEC)
        {
            printf("   ===> currentTimeMs (sec) = %ld\n", currentTimeMs);
            
            // RFCOMM client
            // allocate socket
            s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

            // Set the connection parameters
            address.rc_family = AF_BLUETOOTH;
            address.rc_channel = 1;
            str2ba(dest, &address.rc_bdaddr);

            // connect to server
            printf("Connecting to HC-06...\n");
            status = connect(s, (struct sockaddr*)&address, sizeof(address));
            
            clock_gettime(CLOCK_MONOTONIC, &timestamp);       // CLOCK_REALTIME
            time_seconds_prev = timestamp.tv_sec;
            time_millis_prev = round(timestamp.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
            if (time_millis_prev > 999) {
                time_seconds_prev ++;
                time_millis_prev = 0;
            }
            printf("TS: %ld.%ld s\n", time_seconds_prev, time_millis_prev);

            // send message
            if (0 == status)
            {
                printf("Successful.\n");
                printf("Requesting data from HC-06...\n");
                status = send(s, "---> wserver data request", 43, 0);
            }

            clock_gettime(CLOCK_MONOTONIC, &timestamp);       // CLOCK_REALTIME
            time_seconds_prev = timestamp.tv_sec;
            time_millis_prev = round(timestamp.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
            if (time_millis_prev > 999) {
                time_seconds_prev ++;
                time_millis_prev = 0;
            }
            printf("TS: %ld.%ld s\n", time_seconds_prev, time_millis_prev);

            if (status >= 0)
            {
                // receive data
                printf("... waiting ...\n");
                bytes_read = recv(s, readBuffer, sizeof(readBuffer), 0);
                printf("received packet\n");
                if (bytes_read > 0)
                {
                    printf("Received data: %s\n", readBuffer);
                    fprintf(pDataLogFile, "ARD1:: %s", readBuffer);     // CR is already in the received buffer
                    fflush(pDataLogFile);
                }
            }
            else
            {
                perror("Can't connect to server");
            }

            clock_gettime(CLOCK_MONOTONIC, &timestamp);       // CLOCK_REALTIME
            time_seconds_prev = timestamp.tv_sec;
            time_millis_prev = round(timestamp.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
            if (time_millis_prev > 999) {
                time_seconds_prev ++;
                time_millis_prev = 0;
            }
            printf("TS: %ld.%ld s\n", time_seconds_prev, time_millis_prev);

            printf("Close connection.\n");
            close(s);

            clock_gettime(CLOCK_MONOTONIC, &timestamp);
            previousTimeMs = timestamp.tv_sec;
            //previousTimeMs = currentTimeMs;
        }
    }

    fclose(pDataLogFile);

    return 0;
}
