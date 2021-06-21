/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:


License: Revised BSD License, see LICENSE.TXT file include in the project
*/

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */


//TODO: Check which of these we really need, as they have just been copy and pasted here
#include <stdint.h>         /* C99 types */
#include <stdbool.h>        /* bool type */
#include <stdio.h>          /* printf, fprintf, snprintf, fopen, fputs */

#include <string.h>         /* memset */
#include <signal.h>         /* sigaction */
#include <time.h>           /* time, clock_gettime, strftime, gmtime */
#include <sys/time.h>       /* timeval */
#include <unistd.h>         /* for sleep() */
#include <stdlib.h>         /* atoi, exit */
#include <errno.h>          /* error messages */
#include <math.h>           /* for isfinite() */

#include <pthread.h>

#include "../libloragw/inc/loragw_gps.h"
#include "../libloragw/inc/loragw_aux.h"
#include "gpsd_client.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_GPS == 1
    #define DEBUG_MSG(args...)  fprintf(stderr, args)
    #define DEBUG_PRINTF(fmt, args...)    fprintf(stderr,"%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define DEBUG_ARRAY(a,b,c)  for(a=0;a<b;++a) fprintf(stderr,"%x.",c[a]);fprintf(stderr,"end\n")
    #define CHECK_NULL(a)       if(a==NULL){fprintf(stderr,"%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_GPS_ERROR;}
#else
    #define DEBUG_MSG(args...)
    #define DEBUG_PRINTF(fmt, args...)
    #define DEBUG_ARRAY(a,b,c)  for(a=0;a!=0;){}
    #define CHECK_NULL(a)       if(a==NULL){return LGW_GPS_ERROR;}
#endif
#define TRACE()         fprintf(stderr, "@ %s %d\n", __FUNCTION__, __LINE__);


/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS & TYPES -------------------------------------------- */


/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES (GLOBAL) ------------------------------------------- */

struct gps_data_t *gps_tcp_thread;
pthread_t thrid_gpsd;

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

void thread_gpsd(void) {
	#define GPSD_DATA_TIMEOUT 1500000 //Try and keep between (what should be) one and two updates from GPSd daemon
	#define MODE_STR_NUM 4

	while (1) {
		while (gps_waiting(gps_tcp_thread, GPSD_DATA_TIMEOUT)) {

			if (-1 == gps_read(gps_tcp_thread, NULL, 0)) {
				printf("WARNING: [GPSd] could not get a valid message from GPS\n");
				break;
			}

			if (MODE_SET != (MODE_SET & gps_tcp_thread->set)) {
				// did not even get mode, nothing to see here
				continue;
			}

			/* Check returned mode is defined */
			if (0 > gps_tcp_thread->fix.mode ||
				MODE_STR_NUM <= gps_tcp_thread->fix.mode) {
				gps_tcp_thread->fix.mode = MODE_NOT_SEEN;
			}
		}
		printf("WARNING: [GPSd] Timeout getting a valid message from GPS\n");
		wait_ms(100);
	}
}

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ----------------------------------------- */

int gpsd_enable(char *tcp_path, char *tcp_port, struct gps_data_t *gps_tcp_dev ) {
	/* check input parameters */
    CHECK_NULL(tcp_path);
    CHECK_NULL(gps_tcp_dev);

	//Do stuff
    if (0 != gps_open(tcp_path, tcp_port, gps_tcp_dev)) {
        DEBUG_MSG("ERROR: TCP FAIL TO OPEN, CHECK ADDRESS AND PORT\n");
        return LGW_GPS_ERROR;
    }
    if (0 != gps_stream(gps_tcp_dev, WATCH_ENABLE | WATCH_JSON, NULL)) {
        DEBUG_MSG("ERROR: TCP FAIL TO OPEN, CHECK ADDRESS AND PORT\n");
        return LGW_GPS_ERROR;
    }

    gps_tcp_thread  = gps_tcp_dev;

    printf("INFO: [main] Waiting for GPSd\n");

	/* gps_tcp_thread set by gpsd_enable */
	CHECK_NULL(gps_tcp_thread);

    int i = pthread_create(&thrid_gpsd, NULL, (void * (*)(void *))thread_gpsd, NULL);
    if (i != 0) {
        printf("ERROR: [main] impossible to create GPS thread\n");
        exit(EXIT_FAILURE);
    }

    return LGW_GPS_SUCCESS;
}

int gpsd_disable(struct gps_data_t *gps_tcp_dev) {

	pthread_cancel(thrid_gpsd); //TODO: Check if thread is running before trying to cancel it

	// When you are done...
    if (0 != gps_stream(gps_tcp_dev, WATCH_DISABLE, NULL)) {
        DEBUG_MSG("ERROR: TCP FAIL TO CLOSE\n");
        return LGW_GPS_ERROR;
    }

    (void)gps_close(gps_tcp_dev);

	return LGW_GPS_SUCCESS;
}

int gspd_update() {

    /* Check if we have a valid fix yet */
    if (!isfinite(gps_tcp_thread->fix.latitude) || !isfinite( gps_tcp_thread->fix.longitude) || gps_tcp_thread->fix.mode != MODE_3D ) {
    	return LGW_GPS_ERROR;
    }

    return LGW_GPS_SUCCESS;
}
