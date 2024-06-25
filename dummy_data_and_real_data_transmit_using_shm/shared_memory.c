/*
 * shared_memory.c
 *
 *  Created on: May 14, 2024
 *      Author: Thusitha Samarasekara
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define GETEKYDIR ("/tmp")
#define PROJECTID  (2345)
#define SHMSIZE (124928)	//122k for both ACC and ECG data buffers


#define ERROR -1

static int shmid;
static char *addr;

int shared_memory_init(void)
{
    key_t key = ftok(GETEKYDIR, PROJECTID);

    if ( key < 0 )
    {
    	printf("Error: ftok error\n");
        return ERROR;
    }
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if ( shmid == -1 ) {
        if ( errno == EEXIST ) {
            printf("Error: shared memeory already exist\n");
            shmid = shmget(key ,0, 0);
            printf("reference shmid = %d\n", shmid);
        } else {
            perror("errno");
            printf("Error: shmget error\n");
            return ERROR;
        }
    }


    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ( (addr = shmat(shmid, 0, 0) ) == (void*)-1) {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
        {
        	printf("Error: shmctl error\n");
        	return ERROR;
        }
        else
        {
            printf("Error: Attach shared memory failed\n");
            printf("remove shared memory identifier successful\n");
        }

        printf("Error: shmat error\n");
        return ERROR;
    }
    return 0;
}

int shared_memory_release(void)
{
    if ( shmdt(addr) < 0)
    {
    	printf("shmdt error\n");
    	return ERROR;
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
    	printf("shmctl error\n");
    	return ERROR;
    }
    else
    {
        printf("remove shared memory identifier successful\n");
    }

    return 0;
}

//strcpy( addr, "Shared memory test\n" );

char *shared_memory_get_addr(void)
{
	return addr;
}
