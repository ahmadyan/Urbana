

/* walksat */
/* Maintained by Henry Kautz <kautz@cs.rochester.edu> */
#define VERSION "walksat-v50"

/************************************/
/* Compilation flags                */
/************************************/

/* If the constant DYNAMIC is set, then dynamic arrays are used instead of static arrays.
 This allows very large or very small problems to be handled, without
 having to adjust the constants MAXATOM and/or MAXCLAUSE.  However, on some
 architectures the compiled program is about 15% slower, because not
 as many optimizations can be performed. */

#define DYNAMIC 1

/********************************************************************/
/* Please select only one of following flags: UNIX, OSX, ANSI or NT */
/* Description:                                                     */
/*    UNIX:  uses some non-POSIX unix time routines, compiles       */
/*           (at least) under Linux and Mac OS X                    */
/*    ANSI:  use POSIX time routines, can compile under all         */
/*           unix and NT, time accuracy is 1sec                     */
/*    NT:    use NT/Win32 multimedia routines, compile under        */
/*           NT only, time accuracy is 1ms                          */
/*           Uses NT "timeGetTime" function:                        */
/*             Header: Decl. in Mmsystem.h; include Windows.h.      */
/*             Library: Use Winmm.lib.                              */
/********************************************************************/

#define UNIX 1
#define ANSI 0
#define NT 0


/* Define BIGINT to be the type used for the "cutoff" variable.
 Under gcc "long long int" gives a 64 bit integer.
 Program will still function using a 32-bit value, but it
 limit size of cutoffs that can be specified. */

#if UNIX
#define BIGINT long long int
#define BIGFORMAT "lli"
#endif
#if NT
#define BIGINT __int64
#define BIGFORMAT "I64d"
#endif
#if ANSI
#define BIGINT long int
#define BIGFORMAT "li"
#endif


/************************************/
/* Standard includes                */
/************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>

#if UNIX
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#if NT
#include <time.h>
#include <windows.h>
#include <mmsystem.h>
#endif
#if ANSI
#include <sys/time.h>
#endif

#if ANSI || NT
#define random() rand()
#define srandom(seed) srand(seed)
#endif

/* No longer using constant CLK_TCK - it is deprecated! */
/* Instead: */
long ticks_per_second;


/************************************/
/* Constant parameters              */
/************************************/

#define MAXFILENAME 256

#define MAXATOM 150000		/* maximum possible number of atoms */

#ifdef DYNAMIC
#define STOREBLOCK 2000000	/* size of block to malloc each time */
#else
#define STOREBLOCK 2000000	/* size of block to malloc each time */
#define MAXCLAUSE 500000	/* maximum possible number of clauses */
#endif

#define TRUE 1
#define FALSE 0

#define MAXLENGTH 500           /* maximum number of literals which can be in any clause */

/************************************/
/* Internal constants               */
/************************************/

enum heuristics { RANDOM, BEST, TABU, NOVELTY, RNOVELTY, NOVELTY_PLUS,
	RNOVELTY_PLUS};

#define NOVALUE -1
#define INIT_PARTIAL 1
#define HISTMAX 101		/* length of histogram of tail */

#define Var(CLAUSE, POSITION) (ABS(clause[CLAUSE][POSITION]))

static int scratch;
#define ABS(x) ((scratch=(x))>0?(scratch):(-scratch))

#define BIG 100000000

/************************************/
/* Main data structures             */
/************************************/

/* Atoms start at 1 */
/* Not a is recorded as -1 * a */
/* One dimensional arrays are statically allocated. */
/* Two dimensional arrays are dynamically allocated in */
/* the second dimension only.  */

int numatom;
int numclause;
int numliterals;

#ifdef DYNAMIC
int ** clause;			/* clauses to be satisfied */
/* indexed as clause[clause_num][literal_num] */
int * size;			/* length of each clause */
int * false;			/* clauses which are false */
int * lowfalse;
int * wherefalse;		/* where each clause is listed in false */
int * numtruelit;		/* number of true literals in each clause */
#else
int * clause[MAXCLAUSE];	/* clauses to be satisfied */
/* indexed as clause[clause_num][literal_num] */
int size[MAXCLAUSE];		/* length of each clause */
int false[MAXCLAUSE];		/* clauses which are false */
int lowfalse[MAXCLAUSE];
int wherefalse[MAXCLAUSE];	/* where each clause is listed in false */
int numtruelit[MAXCLAUSE];	/* number of true literals in each clause */
#endif

int *occurrence[2*MAXATOM+1];	/* where each literal occurs */
/* indexed as occurrence[literal+MAXATOM][occurrence_num] */

int numoccurrence[2*MAXATOM+1];	/* number of times each literal occurs */


int atom[MAXATOM+1];		/* value of each atom */
int lowatom[MAXATOM+1];
int solution[MAXATOM+1];

int changed[MAXATOM+1];		/* step at which atom was last flipped */

int breakcount[MAXATOM+1];	/* number of clauses that become unsat if var if flipped */
int makecount[MAXATOM+1];	/* number of clauses that become sat if var if flipped */

int numfalse;			/* number of false clauses */

/************************************/
/* Global flags and parameters      */
/************************************/

FILE * cnfStream;

int status_flag = 0;		/* value returned from main procedure */
int abort_flag;

int heuristic = BEST;		/* heuristic to be used */
int numerator = NOVALUE;	/* make random flip with numerator/denominator frequency */
int denominator = 100;
#define RANDOM_MASK 0xFF
int adjusted_numerator;

int tabu_length;		/* length of tabu list */

int wp_numerator = NOVALUE;	/* walk probability numerator/denominator */
int wp_denominator = 100;

BIGINT numflip;		/* number of changes so far */
BIGINT numnullflip;		/*  number of times a clause was picked, but no  */
/*  variable from it was flipped  */
int numrun = 10;
BIGINT cutoff = 100000;
BIGINT base_cutoff = 100000;
int target = 0;
int numtry = 0;			/* total attempts at solutions */
int numsol = NOVALUE;		/* stop after this many tries succeeds */
int superlinear = FALSE;
int rrtflag = FALSE;
int makeflag = FALSE;		/* set to true by heuristics that require the make values to be calculated */

/* Histogram of tail */

long int tailhist[HISTMAX];	/* histogram of num unsat in tail of run */
long histtotal;
int tail = 3;
int tail_start_flip;

/* Printing options */

int printonlysol = FALSE;
int printsolcnf = FALSE;
int printfalse = FALSE;
int printlow = FALSE;
int printhist = FALSE;
int printtrace = FALSE;
int trace_assign = FALSE;

char outfile[MAXFILENAME] = { 0 };

/* Initialization options */

char initfile[MAXFILENAME] = { 0 };
int initoptions = FALSE;

/* Randomization */

unsigned int seed;  /* Sometimes defined as an unsigned long int */

#if UNIX
struct timeval tv;
struct timezone tzp;
#endif
#if NT
DWORD win_time;     /* elapsed time in ms, since windows boot up */
#endif

/* Statistics */

double expertime;
BIGINT flips_this_solution;
int lowbad;		/* lowest number of bad clauses during try */
BIGINT totalflip = 0;		/* total number of flips in all tries so far */
BIGINT totalsuccessflip = 0;	/* total number of flips in all tries which succeeded so far */
int numsuccesstry = 0;		/* total found solutions */

BIGINT x;
BIGINT integer_sum_x = 0;
double sum_x = 0.0;
double sum_x_squared = 0.0;
double mean_x;
double second_moment_x;
double variance_x;
double std_dev_x;
double std_error_mean_x;
double seconds_per_flip;
int r;
int sum_r = 0;
double sum_r_squared = 0.0;
double mean_r;
double variance_r;
double std_dev_r;
double std_error_mean_r;

double avgfalse;
double sumfalse;
double sumfalse_squared;
double second_moment_avgfalse, variance_avgfalse, std_dev_avgfalse, ratio_avgfalse;
double std_dev_avgfalse;
double f;
double sample_size;

double sum_avgfalse = 0.0;
double sum_std_dev_avgfalse = 0.0;
double mean_avgfalse;
double mean_std_dev_avgfalse;
int number_sampled_runs = 0;
double ratio_mean_avgfalse;

double suc_sum_avgfalse = 0.0;
double suc_sum_std_dev_avgfalse = 0.0;
double suc_mean_avgfalse;
double suc_mean_std_dev_avgfalse;
int suc_number_sampled_runs = 0;
double suc_ratio_mean_avgfalse;

double nonsuc_sum_avgfalse = 0.0;
double nonsuc_sum_std_dev_avgfalse = 0.0;
double nonsuc_mean_avgfalse;
double nonsuc_mean_std_dev_avgfalse;
int nonsuc_number_sampled_runs = 0;
double nonsuc_ratio_mean_avgfalse;

/* Hamming calculations */

char hamming_target_file[MAXFILENAME] = { 0 };
char hamming_data_file[MAXFILENAME] = { 0 };
int hamming_sample_freq;
int hamming_flag = FALSE;
int hamming_distance;
int hamming_target[MAXATOM+1];
FILE * hamming_fp;

/* How frequently to sample the tail for calculating statistics */
int samplefreq = 1;

/************************************/
/* Forward declarations             */
/************************************/

void parse_parameters(int argc,char *argv[]);
void print_parameters(int argc, char * argv[]);

int pickrandom(void);
int pickbest(void);
int picktabu(void);
int picknovelty(void);
int pickrnovelty(void);
int picknoveltyplus(void);
int pickrnoveltyplus(void);

char * heuristic_names[] = { "random", "best", "tabu", "novelty", "rnovelty", "novelty+", "rnovelty+"};
static int (*pickcode[])(void) = {pickrandom, pickbest, picktabu, picknovelty, pickrnovelty, picknoveltyplus, pickrnoveltyplus};

double elapsed_seconds(void);
int countunsat(void);
void scanone(int argc, char *argv[], int i, int *varptr);
void scanonell(int argc, char *argv[], int i, BIGINT *varptr);
void scanoneu(int argc, char *argv[], int i, unsigned int *varptr);
void init(char initfile[], int initoptions);
void initprob(void);
void flipatom(int toflip);
void print_false_clauses(int lowbad);
void save_false_clauses(int lowbad);
void print_low_assign(int lowbad);
void save_low_assign(void);
void save_solution(void);
void print_current_assign(void);
void handle_interrupt(int sig);
long super(int i);
void print_sol_file(char * filename);
void print_statistics_header(void);
void initialize_statistics(void);
void update_statistics_start_try(void);
void print_statistics_start_flip(void);
void update_and_print_statistics_end_try(void);
void update_statistics_end_flip(void);
void print_statistics_final(void);
void print_sol_cnf(void);
void read_hamming_file(char initfile[]);
void open_hamming_data(char initfile[]);
int calc_hamming_dist(int atom[], int hamming_target[], int numatom);

struct node{
	int id;
	int parent; 
	int * false;			/* clauses which are false */
	int * lowfalse;
	int * wherefalse;		/* where each clause is listed in false */
	int * numtruelit;		/* number of true literals in each clause */
	int *occurrence[2*MAXATOM+1];	/* where each literal occurs */ /* indexed as occurrence[literal+MAXATOM][occurrence_num] */
	int numoccurrence[2*MAXATOM+1];	/* number of times each literal occurs */
	int atom[MAXATOM+1];		/* value of each atom */
	int lowatom[MAXATOM+1];
	int solution[MAXATOM+1];
	int changed[MAXATOM+1];		/* step at which atom was last flipped */
	int breakcount[MAXATOM+1];	/* number of clauses that become unsat if var if flipped */
	int makecount[MAXATOM+1];	/* number of clauses that become sat if var if flipped */
	int numfalse;			/* number of false clauses */
	
	int hamming_distance;
	int numnullflip;
	int numflip;

};
int numnodes;
struct node * rrt; 
/************************************/
/* Main                             */
/************************************/

int* rrt_pick_state();
int rrt_find_nearest_node(int* sample);
int rrt_find_nearest_node(int* sample);
void rrt_load_state(int id);
void rrt_save_state(int parentid);
void rrt_init();


void rrt_init(){
	rrt_save_state(-1);//save the root state
}

// generate a random sample state (q_sample)
int* rrt_pick_state(){
	int i;
	int* state = (int *) malloc(sizeof(int)*(numatom+1));
	for(i = 1;i <= numatom;i++)
		state[i] = random()%2;
	return state;
}

// todo: multiple node with the same lowest distance, randomize this part. 
int rrt_find_nearest_node(int* sample){
	int i;
	int mindist = BIG;
	int nearestnodeid = 0;
	for(i=0;i<numnodes;i++){
		int distance = calc_hamming_dist(sample, rrt[i].atom, numatom);
		if(distance<mindist){
			mindist=distance;
			nearestnodeid=i;
		}
	}
	return nearestnodeid;
}

void rrt_load_state(int id){
	int i;
	
	//initializing/saving
	numfalse=rrt[id].numfalse;
	hamming_distance=rrt[id].hamming_distance;
	numnullflip=rrt[id].numnullflip;
	numflip = rrt[id].numflip;	
	
	for(i = 1;i <= numatom;i++){
		atom[i] = rrt[id].atom[i];
		lowatom[i]=rrt[id].lowatom[i];
		changed[i]=rrt[id].changed[i];
		breakcount[i]=rrt[id].breakcount[i];
		makecount[i]=rrt[id].makecount[i];	
	}
		
	for(i=1;i<=numclause;i++){
		false[i]=rrt[id].false[i];
		lowfalse[i]=rrt[id].lowfalse[i];
		wherefalse[i]=rrt[id].wherefalse[i];
		numtruelit[i]=rrt[id].numtruelit[i];
	}
	
}

void rrt_save_state(int parentid){
	int i;
	if(parentid=-1) numnodes=0;
	
	//allocating memory
	//rrt[numnodes].atom = (int *) malloc(sizeof(int)*(numatom+1));
	rrt[numnodes].false = (int *) malloc(sizeof(int)*(numclause+1));
	rrt[numnodes].lowfalse = (int *) malloc(sizeof(int)*(numclause+1));
	rrt[numnodes].wherefalse = (int *) malloc(sizeof(int)*(numclause+1));
	rrt[numnodes].numtruelit = (int *) malloc(sizeof(int)*(numclause+1));
	
	//initializing/saving
	rrt[numnodes].id = numnodes;
	rrt[numnodes].parent = parentid;
	rrt[numnodes].numfalse=numfalse;
	rrt[numnodes].hamming_distance=hamming_distance;
	rrt[numnodes].numnullflip=numnullflip;
	rrt[numnodes].numflip = numflip;	
	
	for(i = 1;i <= numatom;i++){
		rrt[numnodes].atom[i] = atom[i];
		rrt[numnodes].lowatom[i]=lowatom[i];
		rrt[numnodes].changed[i]=changed[i];
		rrt[numnodes].breakcount[i]=breakcount[i];
		rrt[numnodes].makecount[i]=makecount[i];	
	}
		
	for(i=1;i<=numclause;i++){
		rrt[numnodes].false[i]=false[i];
		rrt[numnodes].lowfalse[i]=lowfalse[i];
		rrt[numnodes].wherefalse[i]=wherefalse[i];
		rrt[numnodes].numtruelit[i]=numtruelit[i];
	}
	
	numnodes++;
}

int main(int argc,char *argv[]){
	int i=0;
	int count=0;
#if UNIX
	ticks_per_second = sysconf(_SC_CLK_TCK);
	gettimeofday(&tv,&tzp);
	seed = (unsigned int)((( tv.tv_sec & 0177 ) * 1000000) + tv.tv_usec);
#endif
#if NT
	seed = (unsigned int)(timeGetTime());
#endif
#if ANSI
	seed = (unsigned int)(time());
#endif
	parse_parameters(argc, argv);
	srandom(seed);
	print_parameters(argc, argv);
	initprob();
	initialize_statistics();
	print_statistics_header();
	signal(SIGINT, handle_interrupt);
	abort_flag = FALSE;
	(void) elapsed_seconds();

	printf("[adel-dbg] %d %d\n", numsol, numrun); //default=10 10
	while (! abort_flag && numsuccesstry < numsol && numtry < numrun) {
		numtry++;
		init(initfile, initoptions);
		update_statistics_start_try();
		numflip = 0;

		if (superlinear) cutoff = base_cutoff * super(numtry);


		if(rrtflag){
			printf("rrtflag is enabled.\n");
			while((numfalse > target) && (count < cutoff)) {
				printf("%d\n",count);
				count++;
				//printf("------------------------------\n");
				//printf("numflip = %d\n", numflip);
				//print_statistics_start_flip();
				numflip++;
				//step1: sample a state q_sample
				int* sample = rrt_pick_state();
				
				//printf("q(sample)=");
				//for(i=1;i<=numatom;i++){
				//	printf("%d", sample[i]);
				//}
				//printf("\n");
				
				//step2: find nearest node to q_sample
				int nearestnode = rrt_find_nearest_node(sample);
				//printf("nearest node = %d\n", nearestnode);
				//load the nearest node
				rrt_load_state(nearestnode);
				
				//printf("q(nearestnode)=");
				//for(i=1;i<=numatom;i++){
				//	printf("%d", rrt[nearestnode].atom[i]);
				//}
				//printf("\n");
				
				//printf("Atoms after rrt loading = ");
				//for(i=1;i<=numatom;i++){
				//	printf("%d", atom[i]);
				//}
				//printf("\n");
				
				
				//step3: find optimum trajectory
				int code = (pickcode[heuristic])();
				
				//printf("code= %d\n", code);
				//step4: extend the tree
				
				
				
				flipatom(code);
				//printf("Atoms after flipping = ");
				//for(i=1;i<=numatom;i++){
				//	printf("%d", atom[i]);
				//}
				//printf("\n");
				
				
				rrt_save_state(nearestnode);
				
				//printf("numfalse = %d\n", numfalse);
				//dbg & cleanup
				//printf("current state is ");
				
				//for(i=1;i<=numatom;i++){
				//	printf("%d", atom[i]);
				//}
				//printf("\n");
				//printf("count=%d\n", count);
				free(sample);
				
				update_statistics_end_flip();
			}
			printf("*** total iterations=%d\n", count);
		}else{
			while((numfalse > target) && (numflip < cutoff)) {
				//printf("[adel-dbg] numtry=%d, numflip=%d, cutoff=%d, numfalse=%d, target=%d\n", numtry, numflip, cutoff, numfalse, target );
				print_statistics_start_flip();
				numflip++;
				flipatom((pickcode[heuristic])());
				
				//printf("current state is ");
				//int i=0;
				//for(i=1;i<=numatom;i++){
				//	printf("%d", atom[i]);
				//}
				//printf("\n");
				
				update_statistics_end_flip();
				
			}	
			printf("*** total iterations=%d\n", numflip);
		}
		
		update_and_print_statistics_end_try();
	}
	
	
	//printf("solution state is ");
	//for(i=1;i<=numatom;i++){
	//	printf("%d", atom[i]);
	//} printf("\n");
	
	expertime = elapsed_seconds();
	print_statistics_final();
	return status_flag;
}



void parse_parameters(int argc,char *argv[])
{
	int i;
	int temp;

	cnfStream = stdin;
	for (i=1;i < argc;i++)
	{
		if (argv[i][0] != '-' && cnfStream == stdin){
			cnfStream = fopen(argv[i],"r");
			if (cnfStream == NULL){
				fprintf(stderr, "Cannot open file named %s\n", argv[i]);
				exit(-1);
			}
			++i;
		}
		else if (strcmp(argv[i],"-rrt")==0){
			rrtflag = TRUE;
		}
		else if (strcmp(argv[i],"-seed") == 0){
			scanone(argc,argv,++i,&temp);
			seed = (unsigned int)temp;
		}
		else if (strcmp(argv[i],"-out") == 0 && i<argc-1)
			strcpy(outfile, argv[++i]);
		else if (strcmp(argv[i],"-hist") == 0)
			printhist = TRUE;
		else if (strcmp(argv[i],"-status") == 0)
			status_flag = 1;
		else if (strcmp(argv[i],"-cutoff") == 0)
			scanonell(argc,argv,++i,&cutoff);
		else if (strcmp(argv[i],"-random") == 0)
			heuristic = RANDOM;
	   	else if (strcmp(argv[i],"-novelty") == 0){
			heuristic = NOVELTY;
			makeflag = TRUE;
		}
		else if (strcmp(argv[i],"-rnovelty") == 0){
			heuristic = RNOVELTY;
			makeflag = TRUE;
		}
		else if (strcmp(argv[i],"-novelty+") == 0){
			heuristic = NOVELTY_PLUS;
			makeflag = TRUE;
		}
		else if (strcmp(argv[i],"-rnovelty+") == 0){
			heuristic = RNOVELTY_PLUS;
			makeflag = TRUE;
		}
		else if (strcmp(argv[i],"-best") == 0)
			heuristic = BEST;
		else if (strcmp(argv[i],"-noise") == 0){
			scanone(argc,argv,++i,&numerator);
			if (i < argc-1 && sscanf(argv[i+1],"%i",&temp)==1){
				denominator = temp;
				i++;
			}
		}
		else if (strcmp(argv[i],"-wp") == 0){
			scanone(argc,argv,++i,&wp_numerator);
			if (i < argc-1 && sscanf(argv[i+1],"%i",&temp)==1){
				wp_denominator = temp;
				i++;
			}
		}
		else if (strcmp(argv[i],"-init") == 0  && i < argc-1)
			sscanf(argv[++i], " %s", initfile);
		else if (strcmp(argv[i],"-hamming") == 0  && i < argc-3){
			sscanf(argv[++i], " %s", hamming_target_file);
			sscanf(argv[++i], " %s", hamming_data_file);
			sscanf(argv[++i], " %i", &hamming_sample_freq);
			hamming_flag = TRUE;
			numrun = 1;
		}
		else if (strcmp(argv[i],"-partial") == 0)
			initoptions = INIT_PARTIAL;
		else if (strcmp(argv[i],"-super") == 0){
			superlinear = TRUE;
		}
		else if (strcmp(argv[i],"-tries") == 0 || strcmp(argv[i],"-restart") == 0)
			scanone(argc,argv,++i,&numrun);
		else if (strcmp(argv[i],"-target") == 0)
			scanone(argc,argv,++i,&target);
		else if (strcmp(argv[i],"-tail") == 0)
			scanone(argc,argv,++i,&tail);
		else if (strcmp(argv[i],"-sample") == 0)
			scanone(argc,argv,++i,&samplefreq);
		else if (strcmp(argv[i],"-tabu") == 0)
		{
			scanone(argc,argv,++i,&tabu_length);
			heuristic = TABU;
		}
		else if (strcmp(argv[i],"-low") == 0)
			printlow = TRUE;
		else if (strcmp(argv[i],"-sol") == 0)
		{
			printonlysol = TRUE;
			printlow = TRUE;
		}
		else if (strcmp(argv[i],"-solcnf") == 0)
		{
			printsolcnf = TRUE;
			if (numsol == NOVALUE) numsol = 1;
		}
		else if (strcmp(argv[i],"-bad") == 0)
			printfalse = TRUE;
		else if (strcmp(argv[i],"-numsol") == 0)
			scanone(argc,argv,++i,&numsol);
		else if (strcmp(argv[i],"-trace") == 0)
			scanone(argc,argv,++i,&printtrace);
		else if (strcmp(argv[i],"-assign") == 0){
			scanone(argc,argv,++i,&printtrace);
			trace_assign = TRUE;
		}
		else
		{
			fprintf(stderr, "General parameters:\n");
			fprintf(stderr, "  filename = CNF file, if not specified read stdin\n");
			fprintf(stderr, "  -rrt, enable RRT flag\n");
			fprintf(stderr, "  -seed N = use N to initialize random()\n");
			fprintf(stderr, "  -cutoff N = bound on the number of flips per trial\n");
			fprintf(stderr, "  -restart N = bound on the number of trials\n");
			fprintf(stderr, "  -numsol N = stop after finding N solutions\n");
			fprintf(stderr, "  -super N = use the series 1,1,2,1,1,2,4,... times cutoff for bound on flips per trial\n");
			fprintf(stderr, "  -init FILE = set vars not included in FILE to false\n");
			fprintf(stderr, "  -partial FILE = set vars not included in FILE randomly\n");
			fprintf(stderr, "  -status = return fail status if solution not found\n");
			fprintf(stderr, "  -target N = succeed if N or fewer clauses unsatisfied\n");
			fprintf(stderr, "Heuristics:\n");
			fprintf(stderr, "  -random -best -tabu N -novelty -rnovelty\n");
			fprintf(stderr, "  -noise N or -noise N M (default M = 100)\n");
			fprintf(stderr, "  -novelty+ -rnovelty+\n");
			fprintf(stderr, "  -wp N or -wp N M (default M = 100) = cycle breaking for (r)novelty+\n");
			fprintf(stderr, "Printing:\n");
			fprintf(stderr, "  -out FILE = print solution as a list of literals to FILE\n");
			fprintf(stderr, "  -trace N = print statistics every N flips\n");
			fprintf(stderr, "  -assign N = print assignments at flip N, 2N, ...\n");
			fprintf(stderr, "  -sol = print satisfying assignments to stdout\n");
			fprintf(stderr, "  -solcnf = print sat assign to stdout in DIMACS format, and exit\n");
			fprintf(stderr, "  -low = print lowest assignment each try\n");
			fprintf(stderr, "  -bad = print unsat clauses each try\n");
			fprintf(stderr, "  -hist = print histogram of tail\n");
			fprintf(stderr, "  -tail N = assume tail begins at nvars*N\n");
			fprintf(stderr, "  -sample N = sample noise level every N flips\n");
			fprintf(stderr, "  -hamming TARGET_FILE DATA_FILE SAMPLE_FREQUENCY\n");
			exit(-1);
		}
	}
	base_cutoff = cutoff;
	if (numsol==NOVALUE || numsol>numrun) numsol = numrun;
	if (numerator==NOVALUE){
		switch(heuristic) {
			case BEST:
			case NOVELTY:
			case RNOVELTY:
			case NOVELTY_PLUS:
			case RNOVELTY_PLUS:
				numerator = 50;
				break;
			default:
				numerator = 0;
				break;
		}
	}
	if (wp_numerator==NOVALUE){
		switch(heuristic) {
			case NOVELTY_PLUS:
			case RNOVELTY_PLUS:
				wp_numerator = 1;
				break;
			default:
				wp_numerator = 0;
				break;
		}
	}

	adjusted_numerator = numerator * (RANDOM_MASK + 1) / denominator;

}


void print_parameters(int argc, char * argv[]){
	int i;
	printf("%s\n", VERSION);
	printf("command line =");
	for (i=0;i < argc;i++){
		printf(" %s", argv[i]);
	}
	printf("\n");
	printf("seed = %u\n",seed);
	printf("cutoff = %" BIGFORMAT "\n",cutoff);
	printf("tries = %i\n",numrun);
	printf("heuristic = ");
	switch(heuristic){
		case TABU:
			printf("tabu %i", tabu_length);
			break;
		default:
			printf("%s", heuristic_names[heuristic]);
			break;
	}
	if (numerator>0){
		printf(", noise %i / %i", numerator, denominator);
	}
	if (wp_numerator>0){
		printf(", wp %i / %i", wp_numerator, wp_denominator);
	}
	printf("\n");
}

void print_statistics_header(void){
	printf("numatom = %i, numclause = %i, numliterals = %i\n",numatom,numclause,numliterals);
	printf("wff read in\n\n");
	printf("    lowest     final       avg     dist      dist      total                 avg        mean        mean\n");
	printf("    #unsat    #unsat      dist   std dev     ratio     flips              length       flips       flips\n");
	printf("      this      this      this      this      this      this   success   success       until         std\n");
	printf("       try       try       try       try       try       try      rate     tries      assign         dev\n\n");
	fflush(stdout);
}

void initialize_statistics(void){
	x = 0; r = 0;
	if (hamming_flag){
		read_hamming_file(hamming_target_file);
		open_hamming_data(hamming_data_file);
	}
	tail_start_flip = tail * numatom;
	printf("tail starts after flip = %i\n", tail_start_flip);
	numnullflip = 0;
}

void update_statistics_start_try(void){
	int i;
	lowbad = numfalse;
	sample_size = 0;
	sumfalse = 0.0;
	sumfalse_squared = 0.0;
	for (i=0; i<HISTMAX; i++)
		tailhist[i] = 0;
	if (tail_start_flip == 0){
		tailhist[numfalse < HISTMAX ? numfalse : HISTMAX - 1] ++;
	}
	if (printfalse) save_false_clauses(lowbad);
	if (printlow) save_low_assign();
}

void print_statistics_start_flip(void){
	if (printtrace && (numflip % printtrace == 0)){
		printf(" %9i %9i                     %9" BIGFORMAT "\n", lowbad,numfalse,numflip);
		if (trace_assign)
			print_current_assign();
		fflush(stdout);
	}
}


void update_and_print_statistics_end_try(void){
	int i;
	int j;
	
	totalflip += numflip;
	x += numflip;
	r ++;

	if (sample_size > 0){
		avgfalse = sumfalse/sample_size;
		second_moment_avgfalse = sumfalse_squared / sample_size;
		variance_avgfalse = second_moment_avgfalse - (avgfalse * avgfalse);
		if (sample_size > 1) { variance_avgfalse = (variance_avgfalse * sample_size)/(sample_size - 1); }
		std_dev_avgfalse = sqrt(variance_avgfalse);

		ratio_avgfalse = avgfalse / std_dev_avgfalse;

		sum_avgfalse += avgfalse;
		sum_std_dev_avgfalse += std_dev_avgfalse;
		number_sampled_runs += 1;

		if (numfalse <= target){
			suc_number_sampled_runs += 1;
			suc_sum_avgfalse += avgfalse;
			suc_sum_std_dev_avgfalse += std_dev_avgfalse;
		}
		else {
			nonsuc_number_sampled_runs += 1;
			nonsuc_sum_avgfalse += avgfalse;
			nonsuc_sum_std_dev_avgfalse += std_dev_avgfalse;
		}
	}
	else{
		avgfalse = 0;
		variance_avgfalse = 0;
		std_dev_avgfalse = 0;
		ratio_avgfalse = 0;
	}

	if(numfalse <= target){
		status_flag = 0;
		save_solution();
		numsuccesstry++;
		totalsuccessflip += numflip;
		integer_sum_x += x;
		sum_x = (double) integer_sum_x;
		sum_x_squared += ((double)x)*((double)x);
		mean_x = sum_x / numsuccesstry;
		if (numsuccesstry > 1){
			second_moment_x = sum_x_squared / numsuccesstry;
			variance_x = second_moment_x - (mean_x * mean_x);
			/* Adjustment for small small sample size */
			variance_x = (variance_x * numsuccesstry)/(numsuccesstry - 1);
			std_dev_x = sqrt(variance_x);
			std_error_mean_x = std_dev_x / sqrt((double)numsuccesstry);
		}
		sum_r += r;
		mean_r = ((double)sum_r)/numsuccesstry;
		sum_r_squared += ((double)r)*((double)r);

		x = 0;
		r = 0;
	}

	printf(" %9i %9i %9.2f %9.2f %9.2f %9" BIGFORMAT " %9i",
		   lowbad,numfalse,avgfalse, std_dev_avgfalse,ratio_avgfalse,numflip, (numsuccesstry*100)/numtry);
	if (numsuccesstry > 0){
		printf(" %9" BIGFORMAT, totalsuccessflip/numsuccesstry);
		printf(" %11.2f", mean_x);
		if (numsuccesstry > 1){
			printf(" %11.2f", std_dev_x);
		}
	}
	printf("\n");

	if (printhist){
		printf("histogram: ");
		for (j=HISTMAX-1; tailhist[j] == 0; j--);
		for (i=0; i<=j; i++){
			printf(" %li(%i)", tailhist[i], i);
			if ((i+1) % 10 == 0) printf("\n           ");
		}
		if (j==HISTMAX-1) printf(" +++");
		printf("\n");
	}

	if (numfalse>0 && printfalse)
		print_false_clauses(lowbad);
	if (printlow && (!printonlysol || numfalse >= target))
		print_low_assign(lowbad);

	if(numfalse == 0 && countunsat() != 0){
		fprintf(stderr, "Program error, verification of solution fails!\n");
		exit(-1);
	}

	fflush(stdout);
}

void update_statistics_end_flip(void)
{
	if (numfalse < lowbad){
		lowbad = numfalse;
		if (printfalse) save_false_clauses(lowbad);
		if (printlow) save_low_assign();
	}
	if (numflip >= tail_start_flip){
		tailhist[(numfalse < HISTMAX) ? numfalse : (HISTMAX - 1)] ++;
		if ((numflip % samplefreq) == 0){
	  		sumfalse += numfalse;
	  		sumfalse_squared += numfalse * numfalse;
	  		sample_size ++;
		}
	}
}

void print_statistics_final(void)
{
	seconds_per_flip = expertime / totalflip;
	printf("\ntotal elapsed seconds = %f\n", expertime);
	printf("average flips per second = %li\n", (long)(totalflip/expertime));
	if (heuristic == TABU)
		printf("proportion null flips = %f\n", ((double)numnullflip)/totalflip);
	printf("number solutions found = %i\n", numsuccesstry);
	printf("final success rate = %f\n", ((double)numsuccesstry * 100.0)/numtry);
	printf("average length successful tries = %" BIGFORMAT "\n", numsuccesstry ? (totalsuccessflip/numsuccesstry) : 0);
	if (numsuccesstry > 0)
	{
		printf("average flips per assign (over all runs) = %f\n", ((double)totalflip)/numsuccesstry);
		printf("average seconds per assign (over all runs) = %f\n", (((double)totalflip)/numsuccesstry)*seconds_per_flip);
		printf("mean flips until assign = %f\n", mean_x);
		if (numsuccesstry>1){
			printf("  variance = %f\n", variance_x);
			printf("  standard deviation = %f\n", std_dev_x);
			printf("  standard error of mean = %f\n", std_error_mean_x);
		}
		printf("mean seconds until assign = %f\n", mean_x * seconds_per_flip);
		if (numsuccesstry>1){
			printf("  variance = %f\n", variance_x * seconds_per_flip * seconds_per_flip);
			printf("  standard deviation = %f\n", std_dev_x * seconds_per_flip);
			printf("  standard error of mean = %f\n", std_error_mean_x * seconds_per_flip);
		}
		printf("mean restarts until assign = %f\n", mean_r);
		if (numsuccesstry>1){
			variance_r = (sum_r_squared / numsuccesstry) - (mean_r * mean_r);
			if (numsuccesstry > 1) variance_r = (variance_r * numsuccesstry)/(numsuccesstry - 1);
			std_dev_r = sqrt(variance_r);
			std_error_mean_r = std_dev_r / sqrt((double)numsuccesstry);
			printf("  variance = %f\n", variance_r);
			printf("  standard deviation = %f\n", std_dev_r);
			printf("  standard error of mean = %f\n", std_error_mean_r);
		}
	}

	if (number_sampled_runs){
		mean_avgfalse = sum_avgfalse / number_sampled_runs;
		mean_std_dev_avgfalse = sum_std_dev_avgfalse / number_sampled_runs;
		ratio_mean_avgfalse = mean_avgfalse / mean_std_dev_avgfalse;

		if (suc_number_sampled_runs){
			suc_mean_avgfalse = suc_sum_avgfalse / suc_number_sampled_runs;
			suc_mean_std_dev_avgfalse = suc_sum_std_dev_avgfalse / suc_number_sampled_runs;
			suc_ratio_mean_avgfalse = suc_mean_avgfalse / suc_mean_std_dev_avgfalse;
		}
		else {
			suc_mean_avgfalse = 0;
			suc_mean_std_dev_avgfalse = 0;
			suc_ratio_mean_avgfalse = 0;
		}

		if (nonsuc_number_sampled_runs){
			nonsuc_mean_avgfalse = nonsuc_sum_avgfalse / nonsuc_number_sampled_runs;
			nonsuc_mean_std_dev_avgfalse = nonsuc_sum_std_dev_avgfalse / nonsuc_number_sampled_runs;
			nonsuc_ratio_mean_avgfalse = nonsuc_mean_avgfalse / nonsuc_mean_std_dev_avgfalse;
		}
		else {
			nonsuc_mean_avgfalse = 0;
			nonsuc_mean_std_dev_avgfalse = 0;
			nonsuc_ratio_mean_avgfalse = 0;
		}

		printf("final numbad level statistics\n");
		printf("    statistics over all runs:\n");
		printf("      overall mean average numbad = %f\n", mean_avgfalse);
		printf("      overall mean meanbad std deviation = %f\n", mean_std_dev_avgfalse);
		printf("      overall ratio mean numbad to mean std dev = %f\n", ratio_mean_avgfalse);
		printf("    statistics on successful runs:\n");
		printf("      successful mean average numbad = %f\n", suc_mean_avgfalse);
		printf("      successful mean numbad std deviation = %f\n", suc_mean_std_dev_avgfalse);
		printf("      successful ratio mean numbad to mean std dev = %f\n", suc_ratio_mean_avgfalse);
		printf("    statistics on nonsuccessful runs:\n");
		printf("      nonsuccessful mean average numbad level = %f\n", nonsuc_mean_avgfalse);
		printf("      nonsuccessful mean numbad std deviation = %f\n", nonsuc_mean_std_dev_avgfalse);
		printf("      nonsuccessful ratio mean numbad to mean std dev = %f\n", nonsuc_ratio_mean_avgfalse);
	}

	if (hamming_flag){
		fclose(hamming_fp);
		printf("Final distance to hamming target = %i\n", calc_hamming_dist(atom, hamming_target, numatom));
		printf("Hamming distance data stored in %s\n", hamming_data_file);
	}

	if (numsuccesstry > 0){
		printf("ASSIGNMENT FOUND\n");
		if (printsolcnf == TRUE) print_sol_cnf();
		if (outfile[0]) print_sol_file(outfile);
	}
	else
		printf("ASSIGNMENT NOT FOUND\n");

}


long super(int i)
{
	long power;
	int k;

	if (i<=0){
		fprintf(stderr, "bad argument super(%i)\n", i);
		exit(1);
	}
	/* let 2^k be the least power of 2 >= (i+1) */
	k = 1;
	power = 2;
	while (power < (i+1)){
		k += 1;
		power *= 2;
	}
	if (power == (i+1)) return (power/2);
	return (super(i - (power/2) + 1));
}

void handle_interrupt(int sig)
{
	if (abort_flag) exit(-1);
	abort_flag = TRUE;
}

void scanone(int argc, char *argv[], int i, int *varptr)
{
	if (i>=argc || sscanf(argv[i],"%i",varptr)!=1){
		fprintf(stderr, "Bad argument %s\n", i<argc ? argv[i] : argv[argc-1]);
		exit(-1);
	}
}

void scanoneu(int argc, char *argv[], int i, unsigned int *varptr)
{
	if (i>=argc || sscanf(argv[i],"%u",varptr)!=1){
		fprintf(stderr, "Bad argument %s\n", i<argc ? argv[i] : argv[argc-1]);
		exit(-1);
	}
}

void scanonell(int argc, char *argv[], int i, BIGINT *varptr)
{
	if (i>=argc || sscanf(argv[i],"%Li",varptr)!=1){
		fprintf(stderr, "Bad argument %s\n", i<argc ? argv[i] : argv[argc-1]);
		exit(-1);
	}
}


int calc_hamming_dist(int atom[], int hamming_target[], int numatom){
	int i;
	int dist = 0;

	for (i=1; i<=numatom; i++){
		if (atom[i] != hamming_target[i]) dist++;
	}
	return dist;
}

void open_hamming_data(char initfile[])
{
	if ((hamming_fp = fopen(initfile, "w")) == NULL){
		fprintf(stderr, "Cannot open %s for output\n", initfile);
		exit(1);
	}
}


void read_hamming_file(char initfile[])
{
	int i;			/* loop counter */
	FILE * infile;
	int lit;

	printf("loading hamming target file %s ...", initfile);

	if ((infile = fopen(initfile, "r")) == NULL){
		fprintf(stderr, "Cannot open %s\n", initfile);
		exit(1);
	}
	i=0;
	for(i = 1;i < numatom+1;i++)
		hamming_target[i] = 0;

	while (fscanf(infile, " %i", &lit)==1){
		if (ABS(lit)>numatom){
			fprintf(stderr, "Bad hamming file %s\n", initfile);
			exit(1);
		}
		if (lit>0) hamming_target[lit]=1;
	}
	printf("done\n");
}


void init(char initfile[], int initoptions){
	printf("[adel-dbg] Init: numclauses:%d numatoms%d\n", numclause, numatom);
	int i;
	int j;
	int thetruelit;
	FILE * infile;
	int lit;

	for(i = 0;i < numclause;i++)
		numtruelit[i] = 0;
	numfalse = 0;

	for(i = 1;i < numatom+1;i++){
		changed[i] = -BIG;
		breakcount[i] = 0;
		makecount[i] = 0;
	}

	if (initfile[0] && initoptions!=INIT_PARTIAL){
		for(i = 1;i < numatom+1;i++)
			atom[i] = 0;
	}else{
		for(i = 1;i < numatom+1;i++)
			atom[i] = random()%2;
	}

	if (initfile[0]){
		if ((infile = fopen(initfile, "r")) == NULL){
			fprintf(stderr, "Cannot open %s\n", initfile);
			exit(1);
		}
		i=0;
		while (fscanf(infile, " %i", &lit)==1){
			i++;
			if (ABS(lit)>numatom){
				fprintf(stderr, "Bad init file %s\n", initfile);
				exit(1);
			}
			if (lit<0) atom[-lit]=0;
			else atom[lit]=1;
		}
		if (i==0){
			fprintf(stderr, "Bad init file %s\n", initfile);
			exit(1);
		}
		fclose(infile);
	}

	/* Initialize breakcount and makecount in the following: */
	for(i = 0;i < numclause;i++)
	{
		for(j = 0;j < size[i];j++)
		{
			if((clause[i][j] > 0) == atom[ABS(clause[i][j])])
			{
				numtruelit[i]++;
				thetruelit = clause[i][j];
			}
		}
		if(numtruelit[i] == 0)
		{
			wherefalse[i] = numfalse;
			false[numfalse] = i;
			numfalse++;
			for(j = 0;j < size[i];j++){
				makecount[ABS(clause[i][j])]++;
			}
		}
		else if (numtruelit[i] == 1)
		{
			breakcount[ABS(thetruelit)]++;
		}
	}

	if (hamming_flag){
		hamming_distance = calc_hamming_dist(atom, hamming_target, numatom);
		fprintf(hamming_fp, "0 %i\n", hamming_distance);
	}
	
	if(rrtflag) rrt_init();
}

void
print_false_clauses(int lowbad)
{
	int i, j;
	int cl;

	printf("Unsatisfied clauses:\n");
	for (i=0; i<lowbad; i++){
		cl = lowfalse[i];
		for (j=0; j<size[cl]; j++){
			printf("%i ", clause[cl][j]);
		}
		printf("0\n");
	}
	printf("End unsatisfied clauses\n");
}

void
save_false_clauses(int lowbad)
{
	int i;

	for (i=0; i<lowbad; i++)
		lowfalse[i] = false[i];
}


void initprob(void){
	int i;
	int j;
	int lastc;
	int nextc;
	int *storeptr;
	int freestore;
	int lit;
	int number_blocks_needed;

	while ((lastc = getc(cnfStream)) == 'c'){
		while ((nextc = getc(cnfStream)) != EOF && nextc != '\n');
	}
	ungetc(lastc,cnfStream);
	if (fscanf(cnfStream,"p cnf %i %i",&numatom,&numclause) != 2){
		fprintf(stderr,"Bad input file\n");
		exit(-1);
	}
	if(numatom > MAXATOM){
		fprintf(stderr,"ERROR - too many atoms\n");
		exit(-1);
	}

#ifdef DYNAMIC
	clause = (int **) malloc(sizeof(int *)*(numclause+1));
	size = (int *) malloc(sizeof(int)*(numclause+1));
	false = (int *) malloc(sizeof(int)*(numclause+1));
	lowfalse = (int *) malloc(sizeof(int)*(numclause+1));
	wherefalse = (int *) malloc(sizeof(int)*(numclause+1));
	numtruelit = (int *) malloc(sizeof(int)*(numclause+1));
	if(rrtflag) rrt = (struct node *) malloc(sizeof(struct node)*(cutoff+1));
#else
	if(numclause >= MAXCLAUSE){
		fprintf(stderr,"ERROR - too many clauses\n");
		exit(-1);
	}
#endif
	freestore = 0;
	numliterals = 0;
	for(i = 0;i < 2*MAXATOM+1;i++)
		numoccurrence[i] = 0;
	for(i = 0;i < numclause;i++){
		size[i] = -1;
		if (freestore < MAXLENGTH){
			storeptr = (int *) malloc( sizeof(int) * STOREBLOCK );
			freestore = STOREBLOCK;
			fprintf(stdout,"allocating memory...\n");
		}
		clause[i] = storeptr;
		do{
			size[i]++;
			if(size[i] > MAXLENGTH){
				printf("ERROR - clause too long\n");
				exit(-1);
			}
			if (fscanf(cnfStream,"%i ",&lit) != 1){
				fprintf(stderr, "Bad input file\n");
				exit(-1);
			}
			if(lit != 0){
				*(storeptr++) = lit; /* clause[i][size[i]] = j; */
				freestore--;
				numliterals++;
				numoccurrence[lit+MAXATOM]++;
			}
		}while(lit != 0);
	}
	
	if(size[0] == 0){
		fprintf(stderr,"ERROR - incorrect problem format or extraneous characters\n");
		exit(-1);
	}

	for(i = 0;i < 2*MAXATOM+1;i++){
		if (freestore < numoccurrence[i]){
			number_blocks_needed = (numoccurrence[i] / STOREBLOCK) + 1;
			storeptr = (int *) malloc( sizeof(int) * STOREBLOCK * number_blocks_needed);
			freestore = STOREBLOCK * number_blocks_needed;
			fprintf(stdout,"allocating memory...\n");
		}
		occurrence[i] = storeptr;
		freestore -= numoccurrence[i];
		storeptr += numoccurrence[i];
		numoccurrence[i] = 0;
	}

	for(i = 0;i < numclause;i++){
		for(j = 0;j < size[i];j++){
			occurrence[clause[i][j]+MAXATOM]
			[numoccurrence[clause[i][j]+MAXATOM]] = i;
			numoccurrence[clause[i][j]+MAXATOM]++;
		}
	}
}

void flipatom(int toflip){
	int i, j;
	int toenforce;
	register int cli;
	register int lit;
	int numocc;
	register int sz;
	register int * litptr;
	int * occptr;
	int temp;

	if (toflip == NOVALUE){
		numnullflip++;
		return;
	}

	changed[toflip] = numflip;
	if(atom[toflip] > 0)
		toenforce = -toflip;
	else
		toenforce = toflip;

	atom[toflip] = 1-atom[toflip];

	if (hamming_flag){
		if (atom[toflip] == hamming_target[toflip])
			hamming_distance--;
		else
			hamming_distance++;
		if ((numflip % hamming_sample_freq) == 0)
			fprintf(hamming_fp, "%" BIGFORMAT " %i\n", numflip, hamming_distance);
	}

	numocc = numoccurrence[MAXATOM-toenforce];
	occptr = occurrence[MAXATOM-toenforce];
	for(i = 0; i < numocc ;i++){
		/* cli = occurrence[MAXATOM-toenforce][i]; */
		cli = *(occptr++);

		if (--numtruelit[cli] == 0){
			false[numfalse] = cli;
			wherefalse[cli] = numfalse;
			numfalse++;
			/* Decrement toflip's breakcount */
			breakcount[toflip]--;

			if (makeflag){
				/* Increment the makecount of all vars in the clause */
				sz = size[cli];
				litptr = clause[cli];
				for (j=0; j<sz; j++){
					/* lit = clause[cli][j]; */
					lit = *(litptr++);
					makecount[ABS(lit)]++;
				}
			}
		}
		else if (numtruelit[cli] == 1){
			/* Find the lit in this clause that makes it true, and inc its breakcount */
			litptr = clause[cli];
			while (1){
				/* lit = clause[cli][j]; */
				lit = *(litptr++);
				if((lit > 0) == atom[ABS(lit)]){
					breakcount[ABS(lit)]++;

					/* Swap lit into first position in clause */
					if ((--litptr) != clause[cli]) {
						temp = clause[cli][0];
						clause[cli][0] = *(litptr);
						*(litptr) = temp;
					}
					break;
				}
			}
		}
	}

	numocc = numoccurrence[MAXATOM+toenforce];
	occptr = occurrence[MAXATOM+toenforce];
	for(i = 0; i < numocc; i++){
		/* cli = occurrence[MAXATOM+toenforce][i]; */
		cli = *(occptr++);

		if (++numtruelit[cli] == 1){
			numfalse--;
			false[wherefalse[cli]] =
			false[numfalse];
			wherefalse[false[numfalse]] =
			wherefalse[cli];
			/* Increment toflip's breakcount */
			breakcount[toflip]++;

			if (makeflag){
				/* Decrement the makecount of all vars in the clause */
				sz = size[cli];
				litptr = clause[cli];
				for (j=0; j<sz; j++){
					/* lit = clause[cli][j]; */
					lit = *(litptr++);
					makecount[ABS(lit)]--;
				}
			}
		}
		else if (numtruelit[cli] == 2){
			/* Find the lit in this clause other than toflip that makes it true,
			 and decrement its breakcount */
			litptr = clause[cli];
			while (1){
				/* lit = clause[cli][j]; */
				lit = *(litptr++);
				if( ((lit > 0) == atom[ABS(lit)]) &&
				   (toflip != ABS(lit)) ){
					breakcount[ABS(lit)]--;
					break;
				}
			}
		}
	}
}

// pick a random variable in one of the unsatisfied clauses.
int pickrandom(void){
	int tofix;
	tofix = false[random()%numfalse];
	int v = Var(tofix, random()%size[tofix]); 
	return v;
}

// pick a random unsatisfied clauses, then
// pick a variable that causes the least break in other satisfied clauses.
int pickbest(void){
	int numbreak;
	int tofix;
	int clausesize;
	int i;
	int best[MAXLENGTH];
	register int numbest;
	register int bestvalue;
	register int var;

	tofix = false[random()%numfalse];
	clausesize = size[tofix];
	numbest = 0;
	bestvalue = BIG;

	for (i=0; i< clausesize; i++){
		var = ABS(clause[tofix][i]);
		numbreak = breakcount[var];
		if (numbreak<=bestvalue){
			if (numbreak<bestvalue) numbest=0;
			bestvalue = numbreak;
			best[numbest++] = var;					
			//in case there are multiple best values, this data structure allows for one of them to be returned randomly.
		}
	}

	if (bestvalue>0 && ((random() & RANDOM_MASK) < adjusted_numerator))
		return ABS(clause[tofix][random()%clausesize]);

	if (numbest == 1) return best[0];
	return best[random()%numbest];
}

int picknovelty(void){
	int var, diff, birthdate;
	int youngest, youngest_birthdate, best, second_best, best_diff, second_best_diff;
	int tofix, clausesize, i;

	tofix = false[random()%numfalse];
	clausesize = size[tofix];

	if (clausesize == 1) return ABS(clause[tofix][0]);

	youngest_birthdate = -1;
	best_diff = -BIG;
	second_best_diff = -BIG;

	for(i = 0; i < clausesize; i++){
		var = ABS(clause[tofix][i]);
		diff = makecount[var] - breakcount[var];
		birthdate = changed[var];
		if (birthdate > youngest_birthdate){
			youngest_birthdate = birthdate;
			youngest = var;
		}
		if (diff > best_diff || (diff == best_diff && changed[var] < changed[best])) {
			/* found new best, demote best to 2nd best */
			second_best = best;
			second_best_diff = best_diff;
			best = var;
			best_diff = diff;
		}
		else if (diff > second_best_diff || (diff == second_best_diff && changed[var] < changed[second_best])){
			/* found new second best */
			second_best = var;
			second_best_diff = diff;
		}
	}
	if (best != youngest) return best;
	if ((random()%denominator < numerator)) return second_best;
	return best;
}

int pickrnovelty(void)
{
	int var, diff, birthdate;
	int diffdiff;
	int youngest, youngest_birthdate, best, second_best, best_diff, second_best_diff;
	int tofix, clausesize, i;

	tofix = false[random()%numfalse];
	clausesize = size[tofix];

	if (clausesize == 1) return ABS(clause[tofix][0]);
	if ((numflip % 100) == 0) return ABS(clause[tofix][random()%clausesize]);

	youngest_birthdate = -1;
	best_diff = -BIG;
	second_best_diff = -BIG;

	for(i = 0; i < clausesize; i++){
		var = ABS(clause[tofix][i]);
		diff = makecount[var] - breakcount[var];
		birthdate = changed[var];
		if (birthdate > youngest_birthdate){
			youngest_birthdate = birthdate;
			youngest = var;
		}
		if (diff > best_diff || (diff == best_diff && changed[var] < changed[best])) {
			/* found new best, demote best to 2nd best */
			second_best = best;
			second_best_diff = best_diff;
			best = var;
			best_diff = diff;
		}
		else if (diff > second_best_diff || (diff == second_best_diff && changed[var] < changed[second_best])){
			/* found new second best */
			second_best = var;
			second_best_diff = diff;
		}
	}
	if (best != youngest) return best;

	diffdiff = best_diff - second_best_diff;

	/*
	 if (numerator < 50 && diffdiff > 1) return best;
	 if (numerator < 50 && diffdiff == 1){
	 */

	if (numerator*2 < denominator && diffdiff > 1) return best;
	if (numerator*2 < denominator && diffdiff == 1){
		if ((random()%denominator) < 2*numerator) return second_best;
		return best;
	}
	if (diffdiff == 1) return second_best;

	/*
	 if ((random()%denominator) < 2*(numerator-50)) return second_best;
	 */

	if ((random()%denominator) < 2*(numerator-(denominator/2))) return second_best;

	return best;
}


/* hh: the following are variants of novelty and rnovelty that are typically
 more robust than the original heuristics.

 For details, see:

 Holger H. Hoos and Thomas Stuetzle:
 Local Search Algorithms for SAT: An Empirical Evaluation.
 Journal of Automated Reasoning, Vol.24, pp.421-481, 2000
 A modified version appeared as a book chapter in:
 I.P.Gent, H.v.Maaren, T.Walsh, editors, SAT 2000, pp.43-86, IOS Press, 2000.

 Holger H. Hoos:
 On the Run-time Behaviour of Stochastic Local Search Algorithms for SAT.
 Proc. of AAAI-99, pp.661-666, MIT Press, 1999

 Holger H. Hoos:
 Stochastic Local Search - Methods, Models, Applications.
 infix-Verlag, Sankt Augustin, Germany, 1999; ISBN 3-89601-215-0.

 Kautz modified slightly, to fix bug when denominator is not 100

 */

int picknoveltyplus(void)
{
	int var, diff, birthdate;
	int youngest, youngest_birthdate, best, second_best, best_diff, second_best_diff;
	int tofix, clausesize, i;

	tofix = false[random()%numfalse];
	clausesize = size[tofix];

	if (clausesize == 1) return ABS(clause[tofix][0]);

	/* hh: inserted modified loop breaker: */
	if ((random()%wp_denominator < wp_numerator)) return ABS(clause[tofix][random()%clausesize]);

	youngest_birthdate = -1;
	best_diff = -BIG;
	second_best_diff = -BIG;

	for(i = 0; i < clausesize; i++){
		var = ABS(clause[tofix][i]);
		diff = makecount[var] - breakcount[var];
		birthdate = changed[var];
		if (birthdate > youngest_birthdate){
			youngest_birthdate = birthdate;
			youngest = var;
		}
		if (diff > best_diff || (diff == best_diff && changed[var] < changed[best])) {
			/* found new best, demote best to 2nd best */
			second_best = best;
			second_best_diff = best_diff;
			best = var;
			best_diff = diff;
		}
		else if (diff > second_best_diff || (diff == second_best_diff && changed[var] < changed[second_best])){
			/* found new second best */
			second_best = var;
			second_best_diff = diff;
		}
	}
	if (best != youngest) return best;
	if ((random()%denominator < numerator)) return second_best;
	return best;
}

int pickrnoveltyplus(void)
{
	int var, diff, birthdate;
	int diffdiff;
	int youngest, youngest_birthdate, best, second_best, best_diff, second_best_diff;
	int tofix, clausesize, i;

	tofix = false[random()%numfalse];
	clausesize = size[tofix];

	if (clausesize == 1) return ABS(clause[tofix][0]);

	/* hh: modified loop breaker: */
	if ((random()%wp_denominator < wp_numerator)) return ABS(clause[tofix][random()%clausesize]);

	/*
	 if ((numflip % 100) == 0) return ABS(clause[tofix][random()%clausesize]);
	 */

	youngest_birthdate = -1;
	best_diff = -BIG;
	second_best_diff = -BIG;

	for(i = 0; i < clausesize; i++){
		var = ABS(clause[tofix][i]);
		diff = makecount[var] - breakcount[var];
		birthdate = changed[var];
		if (birthdate > youngest_birthdate){
			youngest_birthdate = birthdate;
			youngest = var;
		}
		if (diff > best_diff || (diff == best_diff && changed[var] < changed[best])) {
			/* found new best, demote best to 2nd best */
			second_best = best;
			second_best_diff = best_diff;
			best = var;
			best_diff = diff;
		}
		else if (diff > second_best_diff || (diff == second_best_diff && changed[var] < changed[second_best])){
			/* found new second best */
			second_best = var;
			second_best_diff = diff;
		}
	}
	if (best != youngest) return best;

	diffdiff = best_diff - second_best_diff;

	/*
	 if (numerator < 50 && diffdiff > 1) return best;
	 if (numerator < 50 && diffdiff == 1){
	 */

	if (numerator*2 < denominator && diffdiff > 1) return best;
	if (numerator*2 < denominator && diffdiff == 1){
		if ((random()%denominator) < 2*numerator) return second_best;
		return best;
	}
	if (diffdiff == 1) return second_best;

	/*
	 if ((random()%denominator) < 2*(numerator-50)) return second_best;
	 */

	if ((random()%denominator) < 2*(numerator-(denominator/2))) return second_best;

	return best;

}

int picktabu(void){
	int numbreak[MAXLENGTH];
	int tofix;
	int clausesize;
	int i;			/* a loop counter */
	int best[MAXLENGTH];	/* best possibility so far */
	int numbest;		/* how many are tied for best */
	int bestvalue;		/* best value so far */
	int noisypick;

	tofix = false[random()%numfalse];
	clausesize = size[tofix];
	for(i = 0;i < clausesize;i++)
		numbreak[i] = breakcount[ABS(clause[tofix][i])];

	numbest = 0;
	bestvalue = BIG;

	noisypick = (numerator > 0 && random()%denominator < numerator);
	for (i=0; i < clausesize; i++) {
		if (numbreak[i] == 0) {
			if (bestvalue > 0) {
				bestvalue = 0;
				numbest = 0;
			}
			best[numbest++] = i;
		}
		else if (tabu_length < numflip - changed[ABS(clause[tofix][i])]) {
			if (noisypick && bestvalue > 0) {
				best[numbest++] = i;
			}
			else {
				if (numbreak[i] < bestvalue) {
					bestvalue = numbreak[i];
					numbest = 0;
				}
				if (numbreak[i] == bestvalue) {
					best[numbest++] = i;
				}
			}
		}
	}
	if (numbest == 0) return NOVALUE;
	if (numbest == 1) return Var(tofix, best[0]);
	return (Var(tofix, best[random()%numbest]));
}

int countunsat(void){
	int i, j, unsat, bad, lit, sign;

	unsat = 0;
	for (i=0;i < numclause;i++)
	{
		bad = TRUE;
		for (j=0; j < size[i]; j++)
		{
			lit = clause[i][j];
			sign = lit > 0 ? 1 : 0;
			if ( atom[ABS(lit)] == sign )
			{
				bad = FALSE;
				break;
			}
		}
		if (bad)
			unsat++;
	}
	return unsat;
}

double elapsed_seconds(void){
	double answer;

#if ANSI
	static long prev_time = 0;
	time( &long_time );
	/* Note:  time(&t) returns t in seconds, so do not need to /CLK_TCK */
	answer = long_time - prev_time;
	prev_time = long_time;
#endif
#if NT
	static DWORD prev_time = 0;
	win_time = timeGetTime();
	/* Note:  return value of timeGetTime() is ms, so divide by 1000*/
	answer = (double)(win_time - prev_time) / (double)1000;
	prev_time = win_time;
#endif
#if UNIX
	static struct tms prog_tms;
	static long prev_times = 0;
	(void) times(&prog_tms);
	answer = ((double)(((long)prog_tms.tms_utime)-prev_times))/((double) ticks_per_second);
	prev_times = (long) prog_tms.tms_utime;
#endif
	return answer;
}

void print_sol_cnf(void){
	int i;
	for(i = 1;i < numatom+1;i++)
		printf("v %i\n", solution[i] == 1 ? i : -i);
}

void print_sol_file(char * filename){
	FILE * fp;
	int i;

	if ((fp = fopen(filename, "w"))==NULL){
		fprintf(stderr, "Cannot open output file\n");
		exit(-1);
	}
	for(i = 1;i < numatom+1;i++){
		fprintf(fp, " %i", solution[i] == 1 ? i : -i);
		if (i % 10 == 0) fprintf(fp, "\n");
	}
	if ((i-1) % 10 != 0) fprintf(fp, "\n");
	fclose(fp);
}

void print_low_assign(int lowbad){
	int i;
	printf("Begin assign with lowest # bad = %d\n", lowbad);
	for (i=1; i<=numatom; i++){
		printf(" %i", lowatom[i]==0 ? -i : i);
		if (i % 10 == 0) printf("\n");
	}
	if ((i-1) % 10 != 0) printf("\n");
	printf("End assign\n");
}

void print_current_assign(void){
	int i;
	printf("Begin assign at flip = %" BIGFORMAT "\n", numflip);
	for (i=1; i<=numatom; i++){
		printf(" %i", atom[i]==0 ? -i : i);
		if (i % 10 == 0) printf("\n");
	}
	if ((i-1) % 10 != 0) printf("\n");
	printf("End assign\n");
}

void save_low_assign(void){
	int i;
	for (i=1; i<=numatom; i++)
		lowatom[i] = atom[i];
}

void save_solution(void){
	int i;
	for (i=1; i<=numatom; i++)
		solution[i] = atom[i];
}
