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
int comm_size;
int comm_rank;
#else
extern int comm_rank;
extern int comm_size;
#endif

void real_one_to_one(px_my_time_type *results,
                     int mes_length,
                     int num_repeats,
                     int source_proc,
                     int dest_proc);

int one_to_one(px_my_time_type **results,
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
    one_to_one(results, ms, nrep);
}

void
print_test_description()
{
    printf("\t\t\tone_to_one - is a test that lock process when translate data\n"
           "\t\t\t\tfrom one processor to other.\n");
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

int one_to_one(px_my_time_type **results,
               int mes_length,
               int num_repeats)
{
    int i;
    int pair[2];

    int confirmation_flag;

    int send_proc, recv_proc;

    MPI_Status status;

    px_my_time_type *tmp_results = NULL;
    tmp_results = (px_my_time_type *) malloc(num_repeats * sizeof(*tmp_results));


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
                real_one_to_one(results[send_proc],
                                mes_length,
                                num_repeats,
                                send_proc,
                                recv_proc);
            }
            if(send_proc==0)
            {
                real_one_to_one(tmp_results,
                                mes_length,
                                num_repeats,
                                send_proc,
                                recv_proc);
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
            if(send_proc==comm_rank)
                real_one_to_one(tmp_results,
                                mes_length,
                                num_repeats,
                                send_proc,
                                recv_proc);
            if(recv_proc==comm_rank)
                real_one_to_one(results[send_proc],
                                mes_length,
                                num_repeats,
                                send_proc,
                                recv_proc);

            confirmation_flag=1;
            MPI_Send(&confirmation_flag,1,MPI_INT,0,1,MPI_COMM_WORLD);
        }
    } /* end else comm_rank==0 */
    int j;

    free(tmp_results);
    return 0;
} /* end one_to_one */


void
real_one_to_one(px_my_time_type *results,
                int mes_length,
                int num_repeats,
                int source_proc,
                int dest_proc)
{
    px_my_time_type time_beg,time_end;
    char *data=NULL;
    MPI_Status status;
    int i;
    px_my_time_type sum;

    px_my_time_type st_deviation;
    int tmp;

    if(source_proc==dest_proc)
    {
        for (i=0; i < num_repeats; ++i)
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

        if(comm_rank==source_proc)
        {
            time_beg=px_my_cpu_time();

            MPI_Send(	data,
                        mes_length,
                        MPI_BYTE,
                        dest_proc,
                        0,
                        MPI_COMM_WORLD
                    );

            time_end=px_my_cpu_time();

            MPI_Recv(&tmp,1,MPI_INT,dest_proc,100,MPI_COMM_WORLD,&status);

            results[i]=(time_end-time_beg);
        }
        if(comm_rank==dest_proc)
        {

            time_beg=px_my_cpu_time();

            MPI_Recv(	data,
                        mes_length,
                        MPI_BYTE,
                        source_proc,
                        0,
                        MPI_COMM_WORLD,
                        &status
                    );


            time_end=px_my_cpu_time();
            results[i]=(time_end-time_beg);

            MPI_Send(&comm_rank,1,MPI_INT,source_proc,100,MPI_COMM_WORLD);
        }
    }


    free(data);
}

