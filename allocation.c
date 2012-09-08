/**
 * Problem statement: Assign a set of persons to a set of groups.
 * Each person provides a certain preference to which group it wants
 * to belong. Maximize the global satisfaction of the participants!
 *
 * Input format:
 * persons(per row) groups(per column)
 * preference preference ...
 * preference preference ...
 * ...        ...        ...
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

// --------
// Parameters of the genetic algorithm
#define POPSIZE 1000 // population size
#define GOOD_RATIO 0.3 // ratio of good solutions to keep
#define RANDOM_RATIO 0.2 // ratio of random solutions to keep
#define MUTATE_RATIO 0.2 // ratio of mutations per iteration
// --------

int persons;
int groups;

int** pref;
int max_fitness;
typedef int* CONFIG;

CONFIG new_config() {
	CONFIG config = (CONFIG)malloc(persons*sizeof(int));
	int p;
	for (p=0; p<persons; p++)
		config[p] = -1;
	return config;
}

void read_input(char* filename) {
	FILE* f = fopen(filename, "r");
	fscanf(f, "%i %i", &persons, &groups);
	pref = (int**)malloc(persons*sizeof(int*));
	int g,p;
	max_fitness = 0;
	for (p=0; p<persons; p++) {
		pref[p] = (int*)malloc(groups*sizeof(int));
		int max = 0;
		for (g=0; g<groups; g++) {
			fscanf(f, "%i", &pref[p][g]);
			if (pref[p][g] > max)
				max = pref[p][g];
		}
		max_fitness += max;
	}
	fclose(f);
}

int fitness(CONFIG config) {
	int fitness = 0;
	int g,p;
	for (p=0; p<persons; p++)
		fitness += pref[p][config[p]];
	return fitness;
}

void print(CONFIG config) {
	int g,p;
	for (g=0; g<groups; g++) {
		printf("GROUP%2i: [ ",g);
		for (p=0; p<persons; p++)
			printf("%c ", config[p] == g ? 'x' : ' ');
		printf("]\n");
	}
	printf("Fitness of solution: %i/%i\n", fitness(config), max_fitness);
}

void printarray(int array[], int size) {
	int i;
	printf("[ ");
	for (i=0; i<size; i++)
		printf("%d ", array[i]);
	printf("]\n");
}

CONFIG max(CONFIG config[]) {
	int max_fitness = -1;
	int c, max_c;
	for (c=0; c<POPSIZE; c++)
		if (fitness(config[c]) > max_fitness) {
			max_fitness = fitness(config[c]);
			max_c = c;
		}
	return config[max_c];
}

int* groupsize(CONFIG config) {
	int* group_size = (int*)malloc(groups*sizeof(int));
	int g,p;
	for (g=0; g<groups; g++)
		group_size[g] = 0;
	for (p=0; p<persons; p++)
		if (config[p]>=0)
			group_size[config[p]]++;
	return group_size;
}

/**
 * greedily assign persons to groups starting with the smallest groups
 */
void greedy(CONFIG config) {
	int p,g;
	int* group_size = groupsize(config);
	int total_group_size = 0;
	for (g=0; g<groups; g++)
		total_group_size += group_size[g];

	for (; total_group_size < persons; total_group_size++) {
		int min_g = 0;
		for (g=0; g<groups; g++)
			if (group_size[g] < group_size[min_g])
				min_g = g;
#ifdef DEBUG
		printf("GREEDY: Choosing group %i\n", min_g);
#endif
		
		// get highest preference per group for unassigned persons
		int max_pref = 0;
		for (p=0; p<persons; p++) {
			if (config[p]>=0)
				continue;
			if (max_pref >= pref[p][min_g])
				continue;
			max_pref = pref[p][min_g];
		}
		if (max_pref == 0)
			warn("Greedy algorithm could not fully assign persons to groups");
		// select randomly a person with the max_pref for this group
		for (p=rand()%persons;;p=(p+1)%persons)
			if (config[p]==-1 && pref[p][min_g]==max_pref)
				break;
		config[p] = min_g;
#ifdef DEBUG
		printf("GREEDY: assign person %i to group %i\n", p, min_g);
#endif
		group_size[min_g]++;
	}
	free(group_size);
}

/**
 * remove participants from groups which are too large
 * then assign them to small groups with a greedy strategy
 */
void repair(CONFIG config) {
	int* size = groupsize(config);
	int p,g;

	int groupavg = persons / groups + (persons%groups == 0 ? 0 : 1);
	// randomly remove participants from groups which are too large
	for (g=0; g<groups; g++) {
		if (size[g] <= groupavg)
			continue;
		p = rand() % persons;
		while (config[p]!=g)
			p = (p+1)%persons;
		config[p] = -1; // remove the person from the group
		size[g]--; // now the group has gotten smaller
		g--; // repeat the check for this group
	}
	free(size);

	// assign the participants which were kicked out greedily
	greedy(config);
}

CONFIG mutate(CONFIG config1, CONFIG config2) {
	CONFIG config = new_config();
	int p;
	// merge first part of first configuration with second part of second configuration
	for (p=0; p<persons; p++)
		config[p] = p<persons/2 ? config1[p] : config2[p];
	
	repair(config);

#ifdef DEBUG
	printf("MUTATE: new config ");
	printarray(config, persons);
	getchar();
#endif

	return config;
}

void iteration(CONFIG configs[]) {
	int num_good = (int)(GOOD_RATIO*POPSIZE);
	int num_random = (int)(RANDOM_RATIO*POPSIZE);
	int num_mutate = (int)(MUTATE_RATIO*POPSIZE);
	CONFIG* configs_good = (CONFIG*)malloc(num_good*sizeof(CONFIG));
	CONFIG* configs_random = (CONFIG*)malloc(num_random*sizeof(CONFIG));

	int c;
	// select the best solutions
	for (c=0; c<num_good; c++) {
		int max_fitness = -1;
		int c2, max_c;
		for (c2=0; c2<POPSIZE; c2++) {
			if (configs[c2] == NULL)
				continue;
			if (fitness(configs[c2]) > max_fitness) {
				max_fitness = fitness(configs[c2]);
				max_c = c2;
			}
		}
		configs_good[c] = configs[max_c];
		configs[max_c] = NULL;
	}

	// select random solutions
	for (c=0; c<num_random; c++) {
		int c2 = rand()%POPSIZE;
		while (configs[c2] == NULL)
			c2 = (c2+1)%POPSIZE;
		configs_random[c] = configs[c2];
		configs[c2] = NULL;
	}

	// drop the remaining solutions
	for (c=0; c<POPSIZE; c++) {
		if (configs[c] == NULL)
			continue;
		free(configs[c]);
		configs[c] = NULL;
	}

	// put the selected configurations back
	for (c=0; c<num_good; c++)
		configs[c] = configs_good[c];
	free(configs_good);
	for (c=0; c<num_random; c++)
		configs[num_good+c] = configs_random[c];
	free(configs_random);

	// fill some gaps with mutations
	for (c=num_good+num_random; c<POPSIZE; c++) {
		// take randomly two solutions
		int c1 = rand()%(num_random+num_good);
		while (configs[c1] == NULL)
			c1 = (c1+1)%(num_random+num_good);
		int c2 = rand()%(num_random+num_good);
		while (configs[c2] == NULL || c1 == c2)
			c2 = (c2+1)%(num_random+num_good);
#ifdef DEBUG
		printf("MUTATE: creating config %6i from %6i and %6i\n", c, c1, c2);
#endif
		configs[c] = mutate(configs[c1], configs[c2]);
		if (--num_mutate <= 0)
			break;
	}

	// fill the remaining gaps with random new configurations
	int* used = (int*)malloc(groups*sizeof(int));
	for (;c<POPSIZE; c++) {
		CONFIG config = new_config();
		int p;
		for (p=0; p<persons; p++) {
			config[p] = rand()%groups;
		}
		repair(config);
		configs[c] = config;
	}
}

void solve() {
	CONFIG configs[POPSIZE];
	int c,i;
	for (c=0; c<POPSIZE; c++) {
		configs[c] = new_config();
		greedy(configs[c]);
	}
	int cur_fitness = 0;
	while (1) {
		iteration(configs);

		CONFIG maxconfig = max(configs);
		if (cur_fitness >= fitness(maxconfig))
			continue;
		cur_fitness = fitness(maxconfig);
		print(maxconfig);
		int* size = groupsize(maxconfig);
		printarray(size, groups);
		free(size);
	}
}

void create_random_example() {
	int* help = (int*)malloc(groups*sizeof(int));

	printf("%i %i\n", persons, groups);
	int g,p;
	for (p=0; p<persons; p++) {
		for (g=0; g<groups; g++)
			help[g] = 0;
		for (g=0; g<groups; g++) {
			int g2 = rand()%groups;
			while (help[g2])
				g2 = (g2+1)%groups;
			printf("%i ", g2+1);
			help[g2] = 1;
		}
		printf("\n");
	}
	free(help);
}

void create_random_example_arbitrary_weights() {
	printf("%i %i\n", persons, groups);
	int g,p;
	for (p=0; p<persons; p++) {
		for (g=0; g<groups; g++)
			printf("%i ", 1+rand()%100000);
		printf("\n");
	}
}

int main(int argc, char** args) {
	srand(time(NULL));
	if (argc == 2) {
		read_input(args[1]);
		solve();
	}
	else if (argc == 3) {
		persons = atoi(args[1]);
		groups = atoi(args[2]);
//		create_random_example();
		create_random_example_arbitrary_weights();
	}
	else
		printf("Please provide a valid data file as the only argument!\n");
	return 0;
}
