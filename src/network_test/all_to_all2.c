/*
 *  This file is a part of the PARUS project.
 *  Copyright (C) 2006  Alexey N. Salnikov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alexey N. Salnikov (salnikov@cmc.msu.ru)
 *
 */

#include "my_time.h"
#include "my_malloc.h"
#include "tests_common.h"
#include "../logger/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#ifdef MODULES_SUPPORT
int comm_rank;
int comm_size;
#else
extern int comm_rank;
extern int comm_size;
#endif

int all_to_all(px_my_time_type **results, int mes_length, int num_repeats);

#ifdef MODULES_SUPPORT
void *parse_args(int argc, char **argv);
void run(px_my_time_type **results, int ms, int nrep, void *add_params);
void print_test_description();
void print_params_description();
void params_free(void *add_params);

void *
parse_args(int argc, char **argv)
{
    return (void *)NULL;
}

void 
run(px_my_time_type **results, int ms, int nrep, void *add_params)
{
    MPI_Comm_size(MPI_COMM_WORLD,&comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&comm_rank);
    all_to_all(results, ms, nrep);
}

void
print_test_description()
{
    printf("all_to_all2 - is a test that translate data simulteniously to "
           "all other processes.");
}

void
print_params_description()
{
}

void
params_free(void *add_params)
{
}
#endif

int all_to_all(px_my_time_type **results, int mes_length, int num_repeats)
{
    px_my_time_type time_beg,time_end;
    char **send_data=NULL;
    char **recv_data=NULL;
    MPI_Status status;
    MPI_Request *send_request=NULL;
    MPI_Request *recv_request=NULL;
    int finished;
    px_my_time_type st_deviation;
    int i,j;
    int flag=0;
    double sum;


    send_request=(MPI_Request *)malloc(comm_size*sizeof(MPI_Request));
    if(send_request == NULL)
    {
        return -1;
    }

    recv_request=(MPI_Request *)malloc(comm_size*sizeof(MPI_Request));
    if(recv_request == NULL)
    {
        free(send_request);
        return -1;
    }
    send_data=(char **)malloc(sizeof(char *)*comm_size);
    if(send_data == NULL)
    {
        free(send_request);
        free(recv_request);
        return -1;
    }
    recv_data=(char **)malloc(sizeof(char *)*comm_size);
    if(recv_data == NULL)
    {
        free(send_request);
        free(recv_request);
        free(send_data);
        return -1;
    }


    for(i=0; i<comm_size; i++)
    {
        send_data[i]=NULL;
        recv_data[i]=NULL;

        send_data[i]=(char *)malloc(mes_length*sizeof(char));
        if(send_data[i]==NULL)
        {
            flag=1;
        }

        recv_data[i]=(char *)malloc(mes_length*sizeof(char));
        if(recv_data[i] == NULL)
        {
            flag=1;
        }
    }

    if(flag == 1)
    {
        free(send_request);
        free(recv_request);
        for(i=0; i<comm_size; i++)
        {
            if(send_data[i]!=NULL)   free(send_data[i]);
            if(recv_data[i]!=NULL)   free(recv_data[i]);
        }
        free(send_data);
        free(recv_data);
        return -1;
    }

    for(i=0; i<num_repeats; i++)
    {

        time_beg=px_my_cpu_time();

        for(j=0; j<comm_size; j++)
        {
            MPI_Isend(send_data[j],
                      mes_length,
                      MPI_BYTE,
                      j,
                      0,
                      MPI_COMM_WORLD,
                      &send_request[j]
                     );


            MPI_Irecv(recv_data[j],
                      mes_length,
                      MPI_BYTE,
                      j,
                      0,
                      MPI_COMM_WORLD,
                      &recv_request[j]
                     );
        }



        for(j=0; j<comm_size; j++)
        {
            MPI_Waitany(comm_size,recv_request,&finished,&status);
            time_end=px_my_cpu_time();
            results[finished][i]=time_end-time_beg;
        }
    }

    free(send_request);
    free(recv_request);
    for(i=0; i<comm_size; i++)
    {
        if(send_data[i]!=NULL)  free(send_data[i]);
        if(recv_data[i]!=NULL)  free(recv_data[i]);
    }
    free(send_data);
    free(recv_data);

    return 0;
}
