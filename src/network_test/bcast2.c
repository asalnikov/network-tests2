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

void bcast(px_my_time_type **results, 
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
    bcast(results, ms, nrep);
}

void
print_test_description()
{
    printf("bcast2 - is a test with broadcast messages.");
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

void bcast(px_my_time_type **results,
            int mes_length, 
            int num_repeats)
{
    px_my_time_type time_beg,time_end;
    char *data=NULL;
    px_my_time_type st_deviation;
    int i,j;
    int flag=0;
    double sum;


    data=(char *)malloc(mes_length*sizeof(char));
    if(data == NULL)
    {
        return; 
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for(j=0; j<comm_size; j++)
    {
        for(i=0; i<num_repeats; i++)
        {
            time_beg=px_my_cpu_time();
            MPI_Bcast(data,
                      mes_length,
                      MPI_BYTE,
                      j,
                      MPI_COMM_WORLD
                     );
            time_end=px_my_cpu_time();
            results[j][i]=time_end-time_beg;
            MPI_Barrier(MPI_COMM_WORLD);
        }

        /*
         for(j=0;j<comm_size;j++)
         {
          MPI_Waitany(comm_size,recv_request,&finished,&status);
          time_end=px_my_cpu_time();
          results[j][i]=time_end-time_beg;

          // printf("process %d from %d:\n  Finished recive message length=%d from %d throug the time %ld\n",
           //comm_rank,comm_size,mes_length,finished,times[finished]);

         }*/
    }
    free(data);
}
