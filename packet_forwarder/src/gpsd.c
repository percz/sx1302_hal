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
#include <unistd.h>         /* getopt, access */
#include <stdlib.h>         /* atoi, exit */
#include <errno.h>          /* error messages */
#include <math.h>           /* modf */

#include <pthread.h>

#include "../libloragw/inc/loragw_gps.h"
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

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

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

    return LGW_GPS_SUCCESS;
}

int gpsd_disable(struct gps_data_t *gps_tcp_dev) {
	// When you are done...
    if (0 != gps_stream(gps_tcp_dev, WATCH_DISABLE, NULL)) {
    	printf("DEBUG: 125 \n");
        DEBUG_MSG("ERROR: TCP FAIL TO CLOSE\n");
        return LGW_GPS_ERROR;
    }

    (void)gps_close(gps_tcp_dev);

	return LGW_GPS_SUCCESS;
}

int gspd_update() {
	#define GPSD_DATA_TIMEOUT 5000
	#define MODE_STR_NUM 4

	/* gps_tcp_thread set during gpsd_enable */
	CHECK_NULL(gps_tcp_thread);

    while (gps_waiting(gps_tcp_thread, GPSD_DATA_TIMEOUT)) {

    	if (-1 == gps_read(gps_tcp_thread, NULL, 0)) {
            return LGW_GPS_ERROR;
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

    /* Check location is valid */
    if (!isfinite(gps_tcp_thread->fix.latitude) || !isfinite( gps_tcp_thread->fix.longitude) || gps_tcp_thread->fix.mode != MODE_3D ) {
    	return LGW_GPS_ERROR;
    }

    return LGW_GPS_SUCCESS;
}
