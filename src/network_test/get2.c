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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "my_time.h"
#include "my_malloc.h"
#include "tests_common.h"

#ifdef MODULES_SUPPORT
int comm_rank;
int comm_size;
#else
extern int comm_rank;
extern int comm_size;
#endif

void real_get_one_to_one(px_my_time_type *results,
                         int mes_length,
                         int num_repeats,
                         int source_proc,
                         int dest_proc, 
                         MPI_Win *win);
int get_one_to_one(px_my_time_type **results, 
                   int mes_length, 
                   int num_repeats);


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
    get_one_to_one(results, ms, nrep);
}

void
print_test_description()
{
    printf("get2 - MPI_Get function testing");
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


int get_one_to_one(px_my_time_type **results, 
                   int mes_length, 
                   int num_repeats)
{
    int i;
    int pair[2];

    int confirmation_flag;

    int send_proc,recv_proc;

    char *data_window=NULL;

    MPI_Win win;
    MPI_Status status;

    data_window=(char *)malloc(mes_length*sizeof(char));
    if(data_window==NULL)
    {
        printf("proc %d from %d: Can not allocate memory %d*sizeof(char)\n",comm_rank,comm_size,mes_length);
        MPI_Abort(MPI_COMM_WORLD,-1);
        return -1;
    }

    /*
     * MPI2 Window creation then all processes will
     * use this window in memory.
     */
    MPI_Win_create(data_window, mes_length*sizeof(char), sizeof(char), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    if(comm_rank==0)
    {
        for(i=0; i<comm_size*comm_size; i++)
        {
            send_proc=get_send_processor(i,comm_size);
            recv_proc=get_recv_processor(i,comm_size);

            pair[0]=send_proc;
            pair[1]=recv_proc;

            if(send_proc)
                MPI_Send(pair,2,MPI_INT,send_proc,1,MPI_COMM_WORLD);
            if(recv_proc)
                MPI_Send(pair,2,MPI_INT,recv_proc,1,MPI_COMM_WORLD);

            if(recv_proc==0)
            {
                real_get_one_to_one(results[send_proc],
                                    mes_length,
                                    num_repeats,
                                    send_proc,
                                    recv_proc,
                                    &win);
            }
            if(send_proc)
            {
                MPI_Recv(&confirmation_flag,1,MPI_INT,send_proc,1,MPI_COMM_WORLD,&status);
            }

            if(recv_proc)
            {
                MPI_Recv(&confirmation_flag,1,MPI_INT,recv_proc,1,MPI_COMM_WORLD,&status);
            }

        } /* End for */
        pair[0]=-1;
        for(i=1; i<comm_size; i++)
            MPI_Send(pair,2,MPI_INT,i,1,MPI_COMM_WORLD);
    } /* end if comm_rank==0 */
    else
    {
        for( ; ; )
        {
            MPI_Recv(pair,2,MPI_INT,0,1,MPI_COMM_WORLD,&status);
            send_proc=pair[0];
            recv_proc=pair[1];

            if(send_proc==-1)
                break;
            if(recv_proc==comm_rank)
                real_get_one_to_one(results[send_proc], 
                                    mes_length,
                                    num_repeats,
                                    send_proc,
                                    recv_proc,
                                    &win);

            confirmation_flag=1;
            MPI_Send(&confirmation_flag,1,MPI_INT,0,1,MPI_COMM_WORLD);
        }
    } /* end else comm_rank==0 */

    MPI_Win_free(&win);
    free(data_window);

    return 0;
}

void real_get_one_to_one(px_my_time_type *results,
                         int mes_length,
                         int num_repeats,
                         int source_proc,
                         int dest_proc, 
                         MPI_Win *win)
{
    px_my_time_type time_beg,time_end;
    char *data=NULL;
    int i;
    px_my_time_type sum;

    px_my_time_type st_deviation;

    if(source_proc==dest_proc)
    {
        for (i = 0; i < num_repeats; ++i) 
            results[i] = 0;
        return;
    }

    data=(char *)malloc(mes_length*sizeof(char));

    if(data==NULL)
    {
        printf("proc %d from %d: Can not allocate memory\n",comm_rank,comm_size);
        return;
    }


    for(i=0; i<num_repeats; i++)
    {

        if(comm_rank==dest_proc)
        {
            MPI_Win_lock(MPI_LOCK_EXCLUSIVE, source_proc, 0, *win);
            time_beg=px_my_cpu_time();

            MPI_Get(data, mes_length, MPI_BYTE, source_proc , 0, mes_length, MPI_BYTE, *win);

            MPI_Win_unlock(source_proc, *win);
            time_end=px_my_cpu_time();

            results[i]=(time_end-time_beg);

        }
    }

    free(data);

    /*
    if((comm_rank==source_proc)||(comm_rank==dest_proc)) return times;
    else
    {
        times.average=-1;
        times.min=0;
        return times;
    }
    */
}

