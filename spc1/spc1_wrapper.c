/*
 *  spc1_wrapper.c
 *
 *  Purpose: Integrate  'fio' (Jens Axboe) with 'open source SPC-1' (Stephen Daniel)
 *
 *  Main Author: Michael O'Sullivan (michael.osullivan@auckland.ac.nz)
 *  Sub Author: DongJin Lee (dongjin.lee@auckland.ac.nz)
 *
 *  Department of Engineering Science
 *  The University of Auckland
 *
 *  Created: 23/12/2009
 *  Updated: 30/05/2010
 *
 */

#ifdef _USE_SPC1

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef _SPC1_H
#define _SPC1_H
#include "spc1.h"
#endif

#include "../fio.h"
#include "../flist.h"
#include "../flist_sort.h"

#include "spc1_wrapper.h"

unsigned numReads;
unsigned numWrites;

struct spc1_options global_spc1_opts;

size_t spc1meter(const void *el) {
	return sizeof(struct spc1_io_s);
}

int spc1compare(const void *a, const void *b) {
	const struct spc1_io_s *A = (struct spc1_io_s *)a;
	const struct spc1_io_s *B = (struct spc1_io_s *)b;
	return (A->when < B->when);
}

int spc1_entry_compare(void *priv,
		struct flist_head *a,
		struct flist_head *b) {
	struct spc1_io_entry * aIO, * bIO;
	aIO = flist_entry(a, struct spc1_io_entry, list);
	bIO = flist_entry(b, struct spc1_io_entry, list);

	return (aIO->io.when > bIO->io.when);
}

int flist_count(struct flist_head *head) {
	int count = 0;
	struct flist_head *pos;

//	printf("head = %ld\n", head);
	flist_for_each(pos, head) {
		count++;
//		printf("pos = %ld\n", pos);
//		printf("pos->next = %ld\n", pos->next);
	}

	return count;
}

struct flist_head ** iostore;

void set_spc1_options(char *inputFile) {
	int i = 0, j = 0;
	int counter = 0;

	printf("Accepting input file named..: %s\n", inputFile);
	char c[200];
	char input[26][200];

	// Initialize string array
	for (i=0; i<26; i++) {
		for (j=0; j<200; j++) {
			input[i][j]='\0';
		}
	}
	i = 0, j = 0;
	FILE *file;
	file = fopen(inputFile, "r");
	assert(file != NULL);
	counter = 0;
	/* Open a text file for reading */
	printf("File opened successfully. Contents:\n");
	while (fgets(c, 200, file) != NULL) {
		if (strlen(c) > 2 && c[0] != '#') { //skip the empty and first #
			char *pos = strchr(c, '#'); // finds the position of # (for commenting)
			strncpy(input[i], c, (pos-c-1));
			// printf("checking input vars: %d, %s\n", i, input[i]);
			i = i+1;
		}

	}
	printf("\nNow closing file...\n");
	fclose(file);
	// return 0;

	/*
	Assigns inputs to variables Input range [0] to [24] (total 25 variables)
	*/

	printf("Getting SPC-1 options...\n");
	i = 0;
	for (i=0; i<26; i++) {
		//printf("Last check: %d, I: %s\n", i, input[i]);
	}

	global_spc1_opts.use_spc1 = atoi(input[3]);
	global_spc1_opts.run_split = atoi(input[4]);
	global_spc1_opts.BSU_number = atoi(input[5]);
	global_spc1_opts.BSU_first_context = atoi(input[6]); /* First BSU context generated by fio */
	global_spc1_opts.BSU_last_context = atoi(input[7]);  /* Last BSU context generated by fio */

	global_spc1_opts.min_iops_generated = atoi(input[8]); /* The minimum number of IOPs generated */

	global_spc1_opts.runtime_hrs  = atoi(input[9]);  /* The hours of the SPC-1 runtime */
	global_spc1_opts.runtime_mins = atoi(input[10]);  /* The minutes of the SPC-1 runtime */
	global_spc1_opts.runtime_secs = atoi(input[11]); /* The seconds of the SPC-1 runtime */
	global_spc1_opts.runtime_millis = atoi(input[12]); /* The milliseconds of the SPC-1 runtime */

	global_spc1_opts.fio_hrs = atoi(input[13]); /* The hours of the fio runtime */
	global_spc1_opts.fio_mins = atoi(input[14]); /* The minutes of the fio runtime */
	global_spc1_opts.fio_secs = atoi(input[15]); /* The seconds of the fio runtime */
	global_spc1_opts.fio_millis = atoi(input[16]); /* The milliseconds of the fio runtime */

	global_spc1_opts.use_asu[0] = atoi(input[17]); /* Are IOPs for ASU1 being generated? */
	global_spc1_opts.use_asu[1] = atoi(input[18]); /* Are IOPs for ASU2 being generated? */
	global_spc1_opts.use_asu[2] = atoi(input[19]); /* Are IOPs for ASU3 being generated? */

	global_spc1_opts.asu_size[0] = atoi(input[20]); /* The size of the ASU1 device */
	global_spc1_opts.asu_size[1] = atoi(input[21]); /* The size of the ASU2 device */
	global_spc1_opts.asu_size[2] = atoi(input[22]); /* The size of the ASU3 device */

	global_spc1_opts.iops_info_every = atoi(input[23]); /* How often to generate IOPs info */
	global_spc1_opts.iops_all_after  = atoi(input[24]);  /* When to start generating all IOPs info */

	strcpy(global_spc1_opts.asu_name[0], input[0]); /* The full path name for the ASU1 device */
	strcpy(global_spc1_opts.asu_name[1], input[1]); /* The full path name for the ASU2 device */
	strcpy(global_spc1_opts.asu_name[2], input[2]); /* The full path name for the ASU3 device */

}

void copy_spc1_options(struct spc1_options * destination, struct spc1_options * source) {
	int i;

	destination->use_spc1 = source->use_spc1;

	destination->run_split = source->run_split;

	destination->BSU_number = source->BSU_number;
	destination->BSU_first_context = source->BSU_first_context;
	destination->BSU_last_context = source->BSU_last_context;

	destination->min_iops_generated = source->min_iops_generated;

	destination->runtime_hrs  = source->runtime_hrs;
	destination->runtime_mins = source->runtime_mins;
	destination->runtime_secs = source->runtime_secs;
	destination->runtime_millis = source->runtime_millis;

	destination->fio_hrs  = source->fio_hrs;
	destination->fio_mins = source->fio_mins;
	destination->fio_secs = source->fio_secs;
	destination->fio_millis = source->fio_millis;

	memcpy(destination->use_asu, source->use_asu, ASU * sizeof(short));

	for (i=0; i<ASU; i++)
		strcpy(destination->asu_name[i], source->asu_name[i]);

	memcpy(destination->asu_size, source->asu_size, ASU * sizeof(unsigned));

	destination->iops_info_every = source->iops_info_every;
	destination->iops_all_after  = source->iops_all_after;

}

int filter_spc1_ios(struct thread_data *td, const char *jobname) {
	unsigned asu, bsu, str;
	unsigned i, j, nread, start, stop;

#ifdef _SPC1_DEBUG
	printf("Filtering SPC-1 IOs for %s...\n", jobname);
#endif

	INIT_FLIST_HEAD(&td->iostore);


	switch (td->spc1_opts.run_split) {
	case RUN_NO_SPLIT:

		for (i=0; i<td->spc1_opts.BSU_number; i++)
			if ( (spc1_context_from_bsu(i) >= global_spc1_opts.BSU_first_context) &&
			     (spc1_context_from_bsu(i) <= global_spc1_opts.BSU_last_context) )
				for (j=0; j<STREAMS; j++) {
				//	printf("DJ %d %d: \n", i, j);
				//	printf("Count=%d\n", flist_count(&iostore[i][j]));
					flist_splice(&iostore[i][j], &td->iostore);
				//	printf("Count=%d\n", flist_count(&td->iostore));
				}
		break;
	case RUN_ALL_SPLIT:
		nread = sscanf(jobname, "bsu%d_str%d", &bsu, &str);
		if (nread != 2)
			return 1;

		assert( (bsu >= 0) && (bsu < td->spc1_opts.BSU_number) && (str >= 0) && (str <= STREAMS) );
		td->bsu = bsu;
		td->str = str;

		flist_splice(&iostore[bsu][str], &td->iostore);
		break;
	case RUN_ASU_SPLIT:

		nread = sscanf(jobname, "asu%d", &asu);
		if (nread != 1)
			return 1;

		assert( (asu >= 1) && (asu <= ASU) );
		td->asu = asu;

		switch (asu) {
			case 1:
				start = ASU1FIRST;
				stop  = ASU2FIRST;
				break;
			case 2:
				start = ASU2FIRST;
				stop  = ASU3FIRST;
				break;
			case 3:
				start = ASU3FIRST;
				stop  = STREAMS;
				break;
		}
		for (i=0; i<td->spc1_opts.BSU_number; i++)
			if ( (spc1_context_from_bsu(i) >= global_spc1_opts.BSU_first_context) &&
				 (spc1_context_from_bsu(i) <= global_spc1_opts.BSU_last_context) )
				for (j=start; j<stop; j++)
					flist_splice(&iostore[i][j], &td->iostore);
		break;
	case RUN_BSU_SPLIT:


		nread = sscanf(jobname, "bsu%d", &bsu);
		if (nread != 1)
			return 1;

		assert( (bsu >= 0) && (bsu < td->spc1_opts.BSU_number) );
		td->bsu = bsu;


		for (j=0; j<STREAMS; j++)
			flist_splice(&iostore[bsu][j], &td->iostore);

		break;
	}
#ifdef _SPC1_DEBUG
	printf("Lists merged, total list size = %d!\n", flist_count(&td->iostore));
#endif

    flist_sort(NULL, &td->iostore, &spc1_entry_compare);
    td->first_io = 1;

#ifdef _SPC1_DEBUG
	printf("List sorted, total list size = %d!\n", flist_count(&td->iostore));
#endif

#ifdef _SPC1_DEBUG
	printf("Filtering complete!\n");
#endif

	return 0;
}

void spc1_io_debug_info(const char *pre, struct spc1_io_s *spc1_io_s_addr) {
	printf("%s", pre);
	printf(": address = %d, ASU = %d, R/W = %d, length = %d, bsu stream = %d, bsu = %d, pos = %d, time = %d\n",
		spc1_io_s_addr,
		spc1_io_s_addr->asu,
		spc1_io_s_addr->dir,
		spc1_io_s_addr->len,
		spc1_io_s_addr->stream,
		spc1_io_s_addr->bsu,
		spc1_io_s_addr->pos,
		spc1_io_s_addr->when); // .when in 0.1 milliseconds
	printf("pid = %d\n", getpid());
	fflush(stdout);
}
void fio_io_debug_info(const char *pre, struct io_u *io_u_addr) {
	printf("%s", pre);
	printf(": address = %d, R/W = %d, offset = %lld, length = %ld, file %d, ",
		io_u_addr,
		io_u_addr->ddir,
		io_u_addr->offset,
		io_u_addr->buflen,
		io_u_addr->file);
	printf("pid = %d\n", getpid());
	fflush(stdout);
}

unsigned spc1_context_from_bsu(unsigned bsu) {
	int retcode;

	retcode = (bsu + 1) / 100;

	return retcode;
}

int gen_spc1_ios() {
	int numContexts;
	char vbuf[BUFLEN];
	char pname[] = PNAME;

	struct spc1_io_entry * next;
	struct spc1_io_s nextio;
	int retcode;
	unsigned int i, j, asu, bsu, stream, totalIO, iops;
    unsigned long long whenTenthMillis, finalTenthMillis;
    unsigned long totalSeconds, totalReads, totalWrites, genReads, genWrites;
	char message[BUFLEN];
	unsigned numStarted, numFinished;
	int ** status;

	// Set a random seed based on the current time
	time_t t1;

	(void) time(&t1);
	srand48((long) t1);

	numContexts = (global_spc1_opts.BSU_number + 1) / 100;
    finalTenthMillis = 10 * (IN_MILLIS( TOTAL_TIME_SECS( global_spc1_opts.runtime_hrs,
											  global_spc1_opts.runtime_mins,
											  global_spc1_opts.runtime_secs ) ) +
											  global_spc1_opts.runtime_millis);
    totalSeconds = TOTAL_TIME_SECS( global_spc1_opts.runtime_hrs,
									global_spc1_opts.runtime_mins,
									global_spc1_opts.runtime_secs );

	printf("Generating SPC-1 workload, num contexts = %d...\n", numContexts);

	printf("Initialising data structures...\n");

    assert( (global_spc1_opts.BSU_first_context >= 0) &&
    		(global_spc1_opts.BSU_first_context <= global_spc1_opts.BSU_last_context) &&
    		(global_spc1_opts.BSU_last_context <= numContexts) );

	iostore = (struct flist_head **)malloc(global_spc1_opts.BSU_number * sizeof(struct flist_head *));
	status = (int **)malloc(global_spc1_opts.BSU_number * sizeof(int *));
	for (i=0; i<global_spc1_opts.BSU_number; i++) {
		if ( (spc1_context_from_bsu(i) >= global_spc1_opts.BSU_first_context) &&
		     (spc1_context_from_bsu(i) <= global_spc1_opts.BSU_last_context) ) {
			iostore[i] = (struct flist_head *)malloc(STREAMS * sizeof(struct flist_head));
			status[i] = (int *)malloc(STREAMS * sizeof(int));
			for (j=0; j<STREAMS; j++) {
				INIT_FLIST_HEAD(&iostore[i][j]);
				status[i][j] = 0;
			}
		}
	}

#ifdef _SPC1_DEBUG
	printf("SPC-1 checking iostore, pid = %d\n", getpid());
	for (i=0; i<global_spc1_opts.BSU_number; i++)
                if ( (spc1_context_from_bsu(i) >= global_spc1_opts.BSU_first_context) &&
                     (spc1_context_from_bsu(i) <= global_spc1_opts.BSU_last_context) )
			for (j=0; j<STREAMS; j++)
				printf("size of iostore[%d][%d] = %d\n", i, j, flist_count(&iostore[i][j]));
	fflush(stdout);
#endif

	printf("Initialising SPC-1 benchmark generator...\n");
#ifdef _SPC1_DEBUG
	printf("spc1_init inputs = %s, %d, %d, %d, %d, %d, %s, %d\n",
			pname,										// char *m			Name of the program.  Used for error messages.
  		    global_spc1_opts.BSU_number,				// int b			Number of BSUs.
  		    GB_TO_BLOCK(global_spc1_opts.asu_size[0]),	// unsigned int a1	Size of ASU 1 in 4K blocks.
	        GB_TO_BLOCK(global_spc1_opts.asu_size[1]),	// unsigned int a2	Size of ASU 2 in 4K blocks.
	        GB_TO_BLOCK(global_spc1_opts.asu_size[2]),	// unsigned int a3	Size of ASU 3 in 4K blocks.
	        numContexts,								// int n_contexts	The number of context blocks to allocate
	        vbuf,										// version	An output buffer where a version string may be written.  If NULL, no version is written.
	        BUFLEN);									// len	The length of the output buffer.
#endif


    retcode = spc1_init(pname,								// char *m			Name of the program.  Used for error messages.
    		  global_spc1_opts.BSU_number,					// int b			Number of BSUs.
    		  GB_TO_BLOCK(global_spc1_opts.asu_size[0]),	// unsigned int a1	Size of ASU 1 in 4K blocks.
	          GB_TO_BLOCK(global_spc1_opts.asu_size[1]),	// unsigned int a2	Size of ASU 2 in 4K blocks.
	          GB_TO_BLOCK(global_spc1_opts.asu_size[2]),	// unsigned int a3	Size of ASU 3 in 4K blocks.
	          numContexts,									// int n_contexts	The number of context blocks to allocate
	          vbuf,											// version	An output buffer where a version string may be written.  If NULL, no version is written.
	          BUFLEN);										// len	The length of the output buffer.

	if (retcode != SPC1_ENOERR) {
		printf("Error initialising SPC-1 workload, errcode = %d\n", retcode);
		return 1;
	}

	totalReads = totalWrites = genReads = genWrites = 0;

	numStarted = numFinished = 0; /* No stream from any BSU has started/finished generating load */

	printf("Initialisation complete!\n");

	/* Generate SPC-1 IOs until the set time has elapsed */
	printf("Creating SPC-1 workload...\n");

	iops = 0;
	do {
		retcode = spc1_next_op_any(&nextio);
		iops++;

		if (retcode == SPC1_ENOERR) {
#ifdef _SPC1_DEBUG
			if ( (iops % global_spc1_opts.iops_info_every == 0) || (iops >= global_spc1_opts.iops_all_after) ) {
				sprintf(message, "IOP #%d, context = %d", iops, spc1_context_from_bsu(nextio.bsu));
                        	spc1_io_debug_info(message, &nextio);
			}
#endif
			if ( (spc1_context_from_bsu(nextio.bsu) < global_spc1_opts.BSU_first_context) ||
				 (spc1_context_from_bsu(nextio.bsu) > global_spc1_opts.BSU_last_context) )
				continue; /* This bsu is outside the contexts considered */

			asu = nextio.asu;
			bsu = nextio.bsu;
			stream = nextio.stream;
			whenTenthMillis = nextio.when;

			if (status[bsu][stream] == -1)
				continue; /* Nothing to do for this (bsu, stream), go to next IO */
			else {
				if (status[bsu][stream] == 0) { /* (bsu, stream) not started yet */
					status[bsu][stream] = 1; /* Start (bsu, stream) */
					numStarted++; /* One more (bsu, steam) started */
				}

#ifdef _SPC1_DEBUG
				if ( (iops % global_spc1_opts.iops_info_every == 0) || (iops >= global_spc1_opts.iops_all_after) ) {
					printf("current = %lld (0.1 ms), max = %lld (0.1ms)\n", whenTenthMillis, finalTenthMillis);
					fflush(stdout);
				}
#endif
				if (whenTenthMillis > finalTenthMillis) { /* IO happens after time limit */
					status[bsu][stream] = -1; /* Stop generating for this (bsu, stream) */
					numFinished++; /* One more (bsu, steam) finished */
				} else if (global_spc1_opts.use_asu[asu-1]) {
#ifdef _SPC1_DEBUG
					if ( (iops % global_spc1_opts.iops_info_every == 0) || (iops >= global_spc1_opts.iops_all_after) ) {
						printf("Adding SPC-1 IO\n");
						fflush(stdout);
					}
#endif

					switch (nextio.dir) {
					case 0:
						totalReads += BLOCK_TO_B(nextio.len);
						genReads++;
						break;
					case 1:
						totalWrites += BLOCK_TO_B(nextio.len);
						genWrites++;
						break;
					}

				//	printf("IOP added!\n");
					next = calloc(1, sizeof(struct spc1_io_entry));
					memcpy(&next->io, &nextio, sizeof(struct spc1_io_s));
					flist_add(&next->list, &iostore[bsu][stream]); /* Store IO for (bsu, stream) */
				}
			}

#ifdef _SPC1_DEBUG
			if ( (iops % global_spc1_opts.iops_info_every == 0) || (iops >= global_spc1_opts.iops_all_after) ) {
				printf("started = %d, finished = %d\n", numStarted, numFinished);
				fflush(stdout);
			}
#endif
		} else {
			printf("Problem creating workload.");
			return 1;
		}

	} while ( (numStarted > numFinished) || (iops <= global_spc1_opts.min_iops_generated) );

	totalIO = 0;
	for (i=0; i<global_spc1_opts.BSU_number; i++)
	        if ( (spc1_context_from_bsu(i) >= global_spc1_opts.BSU_first_context) &&
                 (spc1_context_from_bsu(i) <= global_spc1_opts.BSU_last_context) )
			for (j=0; j<STREAMS;j++) {
				totalIO += flist_count(&iostore[i][j]);
#ifdef _SPC1_DEBUG
				printf("size of iostore[%d][%d] = %d\n", i, j, flist_count(&iostore[i][j]));
				fflush(stdout);
#endif
			}
	printf("totalIO = %d, totalSeconds = %ld, numBSU = %d, average = %g\n",
			totalIO, totalSeconds, global_spc1_opts.BSU_number, (double)totalIO / totalSeconds / global_spc1_opts.BSU_number);
	printf("totalReads = %g MB, numReads = %ld, read bandwidth = %g MB/s, read IOPS = %g\n",
			(double) totalReads / MEGABYTE, genReads, (double)totalReads / MEGABYTE / totalSeconds, (double)genReads / totalSeconds);
	printf("totalWrites = %g MB, numWrites = %ld, write bandwidth = %g MB/s, write IOPS = %g\n",
			(double) totalWrites / MEGABYTE, genWrites, (double)totalWrites / MEGABYTE / totalSeconds, (double)genWrites / totalSeconds);

    printf("Workload created!\n");

    return 0;
}

int add_asu(struct thread_data *td, unsigned index) {
	int fileno;

#ifdef _SPC1_DEBUG
	printf("Adding ASU%d...\n", index + 1);
	fflush(stdout);
#endif

	fileno = get_fileno(td, td->spc1_opts.asu_name[index]);
#ifdef _SPC1_DEBUG
	printf("Adding: ASU%d file = %s, fileno = %d...\n", index + 1, td->spc1_opts.asu_name[index], fileno);
	fflush(stdout);
	#endif
	if (fileno == -1) {
		td->o.nr_files++;
		fileno = add_file(td, td->spc1_opts.asu_name[index]);
	}
	if (fileno == -1) {
#ifdef _SPC1_DEBUG
		printf("ASU%d file = %s could not be added...\n", index + 1, td->spc1_opts.asu_name[index]);
		fflush(stdout);
#endif
		return 1;
	}
	if (td_io_open_file(td, td->files[fileno])) {
#ifdef _SPC1_DEBUG
		printf("ASU%d file = %s could not be opened...\n", index + 1, td->spc1_opts.asu_name[index]);
		fflush(stdout);
		#endif
		return 1;
	}

#ifdef _SPC1_DEBUG
		printf("ASU%d file = %s added, fileno = %d...\n", index + 1, td->spc1_opts.asu_name[index], fileno);
#endif

	return 0;
}

int remove_asu(struct thread_data *td, unsigned index) {
	int fileno;

	fileno = get_fileno(td, td->spc1_opts.asu_name[index]);
#ifdef _SPC1_DEBUG
	printf("Removing: ASU%d file = %s, fileno = %d...\n", index + 1, td->spc1_opts.asu_name[index], fileno);
#endif
	if (fileno == -1) {
#ifdef _SPC1_DEBUG
		printf("ASU%d file not open when closing...\n", index + 1);
#endif
		return 1;
	}
	td_io_close_file(td, td->files[fileno]);

	return 0;
}

char* gen_spc1_file(char *string, unsigned int *global_addr,
		unsigned int *line_addr, int *bsu_addr, int *str_addr) {

	if (*global_addr) {
#ifdef _SPC1_DEBUG
		printf("SPC-1, global section, line = %d\n", *line_addr);
#endif
		switch (*line_addr) {
		case 0:
			sprintf(string, "[global]");
			(*line_addr)++;
			break;
		case 1:
			sprintf(string, "rw=randrw");
			(*line_addr)++;
			break;
		case 2:
			sprintf(string, "ioengine=sync");
			(*line_addr)++;
			break;
		case 3:
//			sprintf(string, "refill_buffers=1");
//			sprintf(string, "direct=1");
			sprintf(string, "iodepth=1");
//			sprintf(string, "iodepth=25000");
			(*line_addr)++;
			*global_addr = 0;
			*line_addr = 0;
			*bsu_addr = 0;
			*str_addr = 0;
			break;
		}

	} else {

		if (*bsu_addr == global_spc1_opts.BSU_number) {
#ifdef _SPC1_DEBUG
			printf("SPC-1 generate null string\n");
#endif
			return NULL;
		}

		switch (global_spc1_opts.run_split) {
		case RUN_NO_SPLIT:

			switch (*line_addr) {
			case 0:
				sprintf(string, "[spc_all]");
				(*line_addr)++;
				break;
			case 1:
				sprintf(string, "write_bw_log=spc_all");
				(*line_addr)++;
				break;
			case 2:
				sprintf(string, "write_lat_log=spc_all");
				*str_addr = STREAMS;
				*bsu_addr = global_spc1_opts.BSU_number;
				*line_addr = 0;
				break;
			}

			break;
		case RUN_ALL_SPLIT:
			if ( (spc1_context_from_bsu(*bsu_addr) >= global_spc1_opts.BSU_first_context) &&
				 (spc1_context_from_bsu(*bsu_addr) <= global_spc1_opts.BSU_last_context) &&
                 (flist_count(&iostore[*bsu_addr][*str_addr]) > 0) ) {
#ifdef _SPC1_DEBUG
				printf("SPC-1, bsu%d_str%d section, line = %d\n", *bsu_addr, *str_addr, *line_addr);
#endif
				switch (*line_addr) {
				case 0:
					sprintf(string, "[bsu%d_str%d]", *bsu_addr, *str_addr);
					(*line_addr)++;
					break;
				case 1:
					sprintf(string, "write_bw_log=spc_bsu%d_str%d", *bsu_addr, *str_addr);
					(*line_addr)++;
					break;
				case 2:
					sprintf(string, "write_lat_log=spc_bsu%d_str%d", *bsu_addr, *str_addr);
					(*str_addr)++;
					if (*str_addr == STREAMS) {
						(*bsu_addr)++;
						*str_addr = 0;
					}
					*line_addr = 0;
					break;
				}

			} else { // No IO for this (bsu, str), so no job needed
				(*str_addr)++;
				if (*str_addr == STREAMS) {
					(*bsu_addr)++;
					*str_addr = 0;
				}
				*line_addr = 0;
			}
			break;
		case RUN_ASU_SPLIT:
			if (*bsu_addr == 0) {

				if (*str_addr == ASU1FIRST) {

					switch (*line_addr) {
					case 0:
						sprintf(string, "[asu1]");
						(*line_addr)++;
						break;
					case 1:
						sprintf(string, "write_bw_log=spc_asu1");
						(*line_addr)++;
						break;
					case 2:
						sprintf(string, "write_lat_log=spc_asu1");
						(*str_addr)++;
						*line_addr = 0;
						break;
					}

				} else if (*str_addr == ASU2FIRST) {

					switch (*line_addr) {
					case 0:
						sprintf(string, "[asu2]");
						(*line_addr)++;
						break;
					case 1:
						sprintf(string, "write_bw_log=spc_asu2");
						(*line_addr)++;
						break;
					case 2:
						sprintf(string, "write_lat_log=spc_asu2");
						(*str_addr)++;
						*line_addr = 0;
						break;
					}

				} else if (*str_addr == ASU3FIRST) {

					switch (*line_addr) {
					case 0:
						sprintf(string, "[asu3]");
						(*line_addr)++;
						break;
					case 1:
						sprintf(string, "write_bw_log=spc_asu3");
						(*line_addr)++;
						break;
					case 2:
						sprintf(string, "write_lat_log=spc_asu3");
						(*str_addr)++;
						*line_addr = 0;
						break;
					}
				} else
					(*str_addr)++;

				if (*str_addr == STREAMS)
					*bsu_addr = global_spc1_opts.BSU_number;
			}
			break;
		case RUN_BSU_SPLIT:
			if ( (spc1_context_from_bsu(*bsu_addr) >= global_spc1_opts.BSU_first_context) &&
				 (spc1_context_from_bsu(*bsu_addr) <= global_spc1_opts.BSU_last_context) &&
                 (*str_addr == 0) ) {

				switch (*line_addr) {
				case 0:
					sprintf(string, "[bsu%d]", *bsu_addr);
					(*line_addr)++;
					break;
				case 1:
					sprintf(string, "write_bw_log=spc_bsu%d", *bsu_addr);
					(*line_addr)++;
					break;
				case 2:
					sprintf(string, "write_lat_log=spc_bsu%d", *bsu_addr);
					(*bsu_addr)++;
					*str_addr = 0;
					*line_addr = 0;
					break;
				}

			}
			break;
		}
	}

#ifdef _SPC1_DEBUG
	printf("SPC-1 generate string %s\n", string);
#endif
	return string;
}

int init_spc1_io(struct thread_data *td) {
	int i, rw;
	unsigned long bytes;
	struct spc1_io_entry * entry_addr;

	// Adjust the iostore list to account for data copying during thread forking
//	printf("td->iostore.prev = %ld\n", td->iostore.prev);
//	printf("td->iostore.prev->next = %ld\n", td->iostore.prev->next);
//	printf("&td->iostore = %ld\n", &td->iostore);
	td->iostore.prev->next = &td->iostore;

#ifdef _SPC1_DEBUG
	printf("Initialising max_bs..., iostore has %d entries\n", flist_count(&td->iostore));
	fflush(stdout);
#endif

	struct flist_head * pos;
	flist_for_each(pos, &td->iostore) {
		entry_addr = flist_entry(pos, struct spc1_io_entry, list);
		rw = entry_addr->io.dir;
		bytes = BLOCK_TO_B((unsigned long)entry_addr->io.len);
		if (bytes > td->o.max_bs[rw]) td->o.max_bs[rw] = bytes;
	}

#ifdef _SPC1_DEBUG
	printf("Adding ASUs...\n");
	fflush(stdout);
#endif

	switch (td->spc1_opts.run_split) {
	case RUN_NO_SPLIT: case RUN_BSU_SPLIT:
		for (i=0; i<ASU; i++)
			if (td->spc1_opts.use_asu[i])
				if (add_asu(td, i)) return 1;
		break;
	case RUN_ASU_SPLIT:
		if (td->spc1_opts.use_asu[td->asu - 1])
			if (add_asu(td, td->asu - 1)) return 1;
		break;
	case RUN_ALL_SPLIT:
		if ( (td->str >= ASU1FIRST) && (td->str < ASU2FIRST) ) {
			if (add_asu(td, 0)) return 1;
		} else if ( (td->str >= ASU2FIRST) && (td->str < ASU3FIRST) ) {
			if (add_asu(td, 1)) return 1;
		} else if ( (td->str >= ASU3FIRST) && (td->str < STREAMS) ) {
			if (add_asu(td, 2)) return 1;
		}
		break;
	}

	td->spc1_get_starts = mtime_since_genesis();
	printf("SPC-1 IOPS being generated, time = %lld ms\n", td->spc1_get_starts);
#ifdef _SPC1_DEBUG
	printf("Initialisation finished for bsu = %d, str = %d, pid = %d\n", td->bsu, td->str, getpid());
	fflush(stdout);
#endif

	return 0;
}



int fin_spc1_io(struct thread_data *td) {
	int i;

	printf("SPC-1 IOPS finished being generated, time taken = %lld ms\n", mtime_since_genesis() - td->spc1_get_starts);

	#ifdef _SPC1_DEBUG
	printf("Removing ASUs...\n");
	fflush(stdout);
#endif

	switch (td->spc1_opts.run_split) {
	case RUN_NO_SPLIT: case RUN_BSU_SPLIT:
		for (i=0; i<ASU; i++)
			if (td->spc1_opts.use_asu[i])
				if (remove_asu(td, i)) return 1;
		break;
	case RUN_ASU_SPLIT:
		if (td->spc1_opts.use_asu[td->asu - 1])
			if (remove_asu(td, td->asu - 1)) return 1;
		break;
	case RUN_ALL_SPLIT:
		if ( (td->str >= ASU1FIRST) && (td->str < ASU2FIRST) ) {
			if (remove_asu(td, 0)) return 1;
		} else if ( (td->str >= ASU2FIRST) && (td->str < ASU3FIRST) ) {
			if (remove_asu(td, 1)) return 1;
		} else if ( (td->str >= ASU3FIRST) && (td->str < STREAMS) ) {
			if (remove_asu(td, 2)) return 1;
		}
		break;
	}

#ifdef _SPC1_DEBUG
	printf("Finalisation finished for bsu = %d, str = %d, pid = %d\n", td->bsu, td->str, getpid());
	fflush(stdout);
#endif
	return 0;
}

int get_spc1_io(struct thread_data *td, struct io_u *io_u_addr) {

	struct spc1_io_entry * entry_addr;
	int i, fileno;
	unsigned long elapsed, whenTenthMillis;

#ifdef _SPC1_DEBUG
	printf("Reading from SPC-1 IOs, pid = %d\n", getpid());
	fflush(stdout);
#endif

#ifdef _SPC1_DEBUG
	for (i=0; i<ASU; i++)
		printf("ASU%d = %s\n", i + 1, td->spc1_opts.asu_name[i]);
#endif

	if (flist_empty(&td->iostore)) {
		td->done = 1;
		return 1;
	}

	entry_addr = flist_entry(td->iostore.next, struct spc1_io_entry, list);
//	printf("&td->iostore = %ld, entry_addr->list = %ld\n",
//			&td->iostore, &entry_addr->list);
	flist_del(&entry_addr->list);

	fflush(stdout);

#ifdef _SPC1_DEBUG
	spc1_io_debug_info("SPC-1 IOP", &(entry_addr->io));
	fflush(stdout);
#endif

	elapsed = 10 * (mtime_since_genesis() - td->spc1_get_starts); //real (system) time since fio started..
	whenTenthMillis = entry_addr->io.when; //SPC1 library When (in Tenth Millisecond)
	//printf("DJ CHECK: whenTenthMillis= %ld", mtime_since_genesis());

#ifdef _SPC1_DEBUG
	printf("SPC-1 IO: elapsed = %ld, when = %ld, pid = %d\n", elapsed, whenTenthMillis, getpid());
	fflush(stdout);
#endif

	long int fioStopTime = 10 * (IN_MILLIS( TOTAL_TIME_SECS( td->spc1_opts.fio_hrs,
			td->spc1_opts.fio_mins,
			td->spc1_opts.fio_secs)) + td->spc1_opts.fio_millis);

	//if (whenTenthMillis > 10 * (IN_MILLIS( TOTAL_TIME_SECS( td->spc1_opts.fio_hrs,
	//		td->spc1_opts.fio_mins,
	//		td->spc1_opts.fio_secs ) ) +
	//		td->spc1_opts.fio_millis) ) {
	if ( (fioStopTime > 0) && ( elapsed > fioStopTime ) ) {
		td->done = 1;
		printf("DJ Check Finished: elapsed = %ld, when = %ld, currentTime = %ld\n", elapsed, whenTenthMillis, fioStopTime);
		return 1;
	}
//	printf("DJ Check 1..!!!!!!! = %ld, when = %ld, currentTime = %ld\n", elapsed, whenTenthMillis, fioStopTime );
//	spc1_io_debug_info("SPC-1 IOP", &(entry_addr->io));
	if (whenTenthMillis > elapsed) { //Disk is waiting (disk workload is doing fast enough to process and wait..)
		printf("1 DJ when>elapsed..  elapsed = %ld, when = %ld, diff = %lld\n", elapsed, whenTenthMillis, whenTenthMillis-elapsed);
	//	spc1_io_debug_info("SPC-1 IOP", &(entry_addr->io));
		usec_sleep(td, (whenTenthMillis - elapsed) * 100);
	}
	else
		printf("2 DJ elapsed>=when.  elapsed = %ld, when = %ld, diff = %lld\n", elapsed, whenTenthMillis, elapsed-whenTenthMillis);

	//spc1_io_debug_info("DJ 3 SPC-1 IOP", &(entry_addr->io));

#ifdef _SPC1_DEBUG
	printf("SPC-1 IO: wait over\n");
	fflush(stdout);
#endif

	io_u_addr->ddir = entry_addr->io.dir;
	io_u_addr->offset = BLOCK_TO_B((unsigned long long)entry_addr->io.pos);
	io_u_addr->buflen = BLOCK_TO_B((unsigned long)entry_addr->io.len);

#ifdef _SPC1_DEBUG
	for (i=0; i<ASU; i++)
		printf("ASU%d = %s\n", i + 1, td->spc1_opts.asu_name[i]);
#endif
	/* Get file to read from/write to */
	if ( (entry_addr->io.stream >= ASU1FIRST) &&
		 (entry_addr->io.stream < ASU2FIRST) ) {

		fileno = get_fileno(td, td->spc1_opts.asu_name[0]);
#ifdef _SPC1_DEBUG
		printf("IO file %s identified, fileno = %d... bsu = %d, str = %d, pid = %d\n", td->spc1_opts.asu_name[0], fileno, td->bsu, td->str, getpid());
#endif
	} else if ( (entry_addr->io.stream >= ASU2FIRST) &&
			    (entry_addr->io.stream < ASU3FIRST) ) {

		fileno = get_fileno(td, td->spc1_opts.asu_name[1]);
#ifdef _SPC1_DEBUG
		printf("IO file %s identified, fileno = %d... bsu = %d, str = %d, pid = %d\n", td->spc1_opts.asu_name[1], fileno, td->bsu, td->str, getpid());
#endif
	} else if ( (entry_addr->io.stream >= ASU3FIRST) &&
			    (entry_addr->io.stream < STREAMS) ) {

		fileno = get_fileno(td, td->spc1_opts.asu_name[2]);
#ifdef _SPC1_DEBUG
		printf("IO file %s identified, fileno = %d... bsu = %d, str = %d, pid = %d\n", td->spc1_opts.asu_name[2], fileno, td->bsu, td->str, getpid());
#endif
	}

	if (fileno == -1) {
		return 1;
	}

	io_u_addr->file = td->files[fileno];

	get_file(io_u_addr->file);

	td->first_io = 0;

#ifdef _SPC1_DEBUG
	fio_io_debug_info("SPC-1 io_u", io_u_addr);
	fflush(stdout);
#endif

	free((void *)entry_addr);
#ifdef _SPC1_DEBUG
	fio_io_debug_info("Leaving get_spc1_io", io_u_addr);
	fflush(stdout);
#endif

	return 0;
}

#endif