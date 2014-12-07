#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tests_common.h"
#include "my_time.h"
#include "types.h"


int my_time_cmp(const void *a, const void *b)
{
    px_my_time_type val_a=*(px_my_time_type *)a;
    px_my_time_type val_b=*(px_my_time_type *)b;

    if((val_a - val_b)>0) return 1;
    else if((val_a - val_b)<0) return -1;
    else return 0;
}

int create_test_hosts_file
(
	const struct network_test_parameters_struct *parameters,
	char **hosts_names
)
{
	FILE *f;
	char *file_name;
	int i;

	file_name=(char *)malloc((strlen(parameters->file_name_prefix)+strlen("_hosts.txt")+1)*sizeof(char));
	if(file_name==NULL)
	{
		printf("Memory allocation error\n");
		return -1;
	}

	sprintf(file_name,"%s_hosts.txt",parameters->file_name_prefix);
	
	f=fopen(file_name,"w");
	if(f==NULL)
	{
		printf("File open error\n");
		return -1;
	}

	for(i=0;i<parameters->num_procs;i++)
	{
		fprintf(f,"%s\n",hosts_names[i]);
	}

	fclose(f);
	free(file_name);

	return 0;
}

void 
calculate_statistics(px_my_time_type **results, 
                     Test_time_result_type *times, 
                     int comm_size, int num_repeats)
{
    int send_proc;
    int itr;
    px_my_time_type sum;
    px_my_time_type st_dev;
    px_my_time_type *buf = malloc(num_repeats * sizeof(*buf));

    for (send_proc = 0; send_proc < comm_size; ++send_proc) {
        sum = 0;
        for (itr = 0; itr < num_repeats; ++itr) {
            sum += results[send_proc][itr];
        }
        times[send_proc].average = sum / (double)num_repeats;
        st_dev = 0;
        for (itr = 0; itr < num_repeats; ++itr) {
            st_dev += (results[send_proc][itr] - times[send_proc].average) *
                      (results[send_proc][itr] - times[send_proc].average); 
        }
        st_dev /= (double)num_repeats;
        times[send_proc].deviation = sqrt(st_dev);

        memcpy(buf, results[send_proc], sizeof(*buf) * num_repeats);
        qsort(buf, num_repeats, sizeof(*buf), my_time_cmp);
        times[send_proc].median = buf[num_repeats / 2];
        times[send_proc].min = buf[0];
    }

    free(buf);
}
