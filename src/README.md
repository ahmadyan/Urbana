TreeSAT
A randomized SAT solver
(C) 2013 Adel Ahmadyan & Biplab Deka
12345
11100
101

1 v 1 v 1
0 v 0 v 1 
# Data structure examples

## CNF File
3 clause, 5 variable
1 -5 4 0
-1 5 3 4 0
-3 -4 0

##clause[i][j]
### WalkSAT
1 -5 4 
3 5 -1 4 
-3 -4 

### TreeSAT





Important data structures (Our core representation for SAT is very similar to WalkSAT implementation v50.)


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


