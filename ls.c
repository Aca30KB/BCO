#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_ITER 10000
#define INF 1e9
#define P1 35
#define P2 25
#define EPS 1e-4

/* struktura koja se pravi da bi se upamtila vrednost funkcije cilja u svakoj 
   iteraciji kao i vreme koje je proteklo od pocetka izvrsavanja programa do izracunavanja resenja*/

struct resenje{
  double vreme;
  double value;
};

/* struktura koja se pravi da bi kada se elementi matrice za i-tog
   korisnika sortiraju moglo da se pristupi njihovim originalnim indeksima*/

struct cena{
  int j1;
  int j2;
  double value;
};

//funkcija poredjenja za sortiranje

int cmp(const void *a, const void *b){
  struct cena *a1 = (struct cena *) a;
  struct cena *a2 = (struct cena *) b;
  if((*a1).value < (*a2).value){
    return -1;
  }
  else if((*a1).value > (*a2).value){
    return 1;
  }
  else{
    return 0;
  }
}

//funkcija za racunanje vrednosti funkcije cilja

double upgradedSolutionValue(bool *solution, double *fixedCost, int m, int level1, int level2, struct cena **c){
  register double value = 0.0;
  double minValue;
  int i, j, jUsed, kUsed;
  int level = level1 + level2;
  int n = level1 * level2;
  bool* used = (bool*) malloc((unsigned long) level * sizeof(bool));
  assert(used != NULL);
  for(j = 0; j < level; j++){
    used[j] = false;
  }
  for(i = 0; i < m; i++){
    for(j = 0; j < n; j++){
      if(!(solution[c[i][j].j1] && solution[level1 + c[i][j].j2])){
        continue;
      }
      else{
        jUsed = c[i][j].j1;
        kUsed = level1 + c[i][j].j2;
        minValue = c[i][j].value;
        break;
      }
    }
    used[jUsed] = true;
    used[kUsed] = true;
    value += minValue;
  }
  for(j = 0; j < level; j++){
    solution[j] = used[j];
    if(!used[j]){
      continue;
    }
    value += fixedCost[j];
  }
  return value;
}

//funkcija koja proverava da li je konstruisano resenje ispravno

bool ispravno(bool *solution, int level1, int level2){
  bool a = false, b = false;
  for(int i = 0; i < level1; i++){
    if(solution[i]){
       a = true;
       break;
    }
  }
  for(int j = 0; j < level2; j++){
    if(solution[level1 + j]){
      b = true;
      break;
    }
  }
  return a && b;
}

//funkcija koja inicijalizuje resenje

void initialize(bool *solution, int level1, int level2){
  int level = level1 + level2;
  for(int i = 0; i < level1; i++){
    solution[i] = rand() % 100 < P1;
  }
  for(int i = level1; i < level; i++){
    solution[i] = rand() % 100 < P2;
  }
  if(!ispravno(solution, level1, level2)){
    int i = rand() % level1;
    int k = rand() % level2;
    solution[i] = true;
    solution[level1 + k] = true;
  }
}

//funkcija koja vraca suseda trenutnog resenja

int *invert(bool *solution, int level1, int level2){
  int *a = (int*) malloc(2UL * sizeof(int));
  assert(a != NULL);
  int j = rand() % level1;
  int k = rand() % level2;
  solution[j] = !solution[j];
  solution[level1 + k] = !solution[level1 + k];
  if(ispravno(solution, level1, level2)){
    a[0] = j;
    a[1] = k;
    return a;
  }
  else{
    solution[j] = !solution[j];
    solution [level1 + k] = !solution[level1 + k];
    a[0] = -1;
    a[1] = -1;
    return a;
  }
}

//funkcija koja se od suseda vraca na pocetno resenje

void restore(bool *solution, int j, int k, int level1){
  solution[j] = !solution[j];
  solution[level1 + k] = !solution[level1 + k];
}

//algoritam lokalne pretrage

struct resenje localSearch(bool *solution, double *fixedCost, int m, int level1, int level2, struct cena **c){
  struct resenje currentValue;
  struct resenje bestValue;
  int iteration = 0;
  int *a;
  double t;
  struct resenje newValue;
  newValue.value = 0.0;
  double startTime = clock();
  currentValue.value = upgradedSolutionValue(solution, fixedCost, m, level1, level2, c);
  currentValue.vreme = startTime;
  bestValue.value = currentValue.value;

  while(iteration++ < MAX_ITER){
    a = invert(solution, level1, level2);
    assert(a != NULL);
    if((a[0] == -1 && a[1] == -1) || a[0] == -1 || a[1] == -1){
      continue;
    }
    newValue.value = upgradedSolutionValue(solution, fixedCost, m, level1, level2, c);
    t = clock();
    newValue.vreme = (t - startTime) / CLOCKS_PER_SEC;
    if(newValue.value < currentValue.value){
      currentValue.value = newValue.value;
      currentValue.vreme = newValue.vreme;
    }
    else{
      restore(solution, a[0], a[1], level1);
    }
    if(newValue.value < bestValue.value){
      bestValue.value = newValue.value;
      bestValue.vreme = newValue.vreme;
    }
  }
  return bestValue;
}

//main funkcija gde se vrsi ucitavanje podataka iz fajla i poziv algoritma LS kao i ispis resenja

int main(int argc, char** argv){
  srand((unsigned int) time(NULL));
  //srand(0);
  double startTime = clock();
  FILE *in = fopen(argv[1], "r");
  assert(in != NULL);
  if(ferror(in)){
    fprintf(stderr,"Error reading file %s!\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  //Ucitavanje ulaznih podataka

  int nbCustomers, level1, level2;
  fscanf(in, "%d%d%d", &nbCustomers, &level1, &level2);

  //Broj Yjr za r = 1,2

  int level = level1 + level2;
  //int nbVariables = nbCustomers * level1 * level2;
  int n = level1 * level2;

  //Ucitavanje zahteva korisnika

  double *d = (double*) malloc((unsigned long) nbCustomers * sizeof(double));
  assert(d != NULL);
  for(int i = 0; i < nbCustomers; i++){
    fscanf(in, "%lf", &d[i]);
  }

  //Ucitavanje fiksnih troskova

  double *fixedCost = (double*) malloc((unsigned long) level * sizeof(double));
  assert(fixedCost != NULL);
  for(int j = 0; j < level; j++){
    fscanf(in, "%lf", &fixedCost[j]);
  }

  //Ucitavanje matrice cena izmedju korisnika i facility-ja prvog nivoa

  double **a = (double**) malloc((unsigned long) nbCustomers * sizeof(double*));
  assert(a != NULL);
  for(int i = 0; i < nbCustomers; i++){
    a[i] = (double*) malloc((unsigned long) level1 * sizeof(double));
    assert(a[i] != NULL);
    for(int j1 = 0; j1 < level1; j1++){
      fscanf(in, "%lf", &a[i][j1]);
    }
  }

  //Ucitavanje matrice cena izmedju facility-ja prvog i drugog nivoa

  double **b = (double**) malloc((unsigned long) level1 * sizeof(double*));
  assert(b != NULL);
  for(int j1 = 0; j1 < level1; j1++){
    b[j1] = (double*) malloc((unsigned long) level2 * sizeof(double));
    assert(b[j1] != NULL);
    for(int j2 = 0; j2 < level2; j2++){
      fscanf(in, "%lf", &b[j1][j2]);
    }
  }

  fclose(in);

  /* za svakog potrosaca pravi se niz struktura sa cenama za svaki par j1 j2
     i takav se niz sortira u rastucem poretku da bi se smanjilo vreme 
     pretrage pri racunanju vrednosti funkcije cilja*/

  struct cena **c = (struct cena**) malloc((unsigned long) nbCustomers * sizeof(struct cena*));
  assert(c != NULL);
  for(int i = 0; i < nbCustomers; i++){
    c[i] = (struct cena *) malloc((unsigned long) n * sizeof(struct cena));
    assert(c[i] != NULL);
    for(int j1 = 0; j1 < level1; j1++){
      for(int j2 = 0; j2 < level2; j2++){
        c[i][j1 * level2 + j2].j1 = j1;
        c[i][j1 * level2 + j2].j2 = j2;
        c[i][j1 * level2 + j2].value = d[i] * (a[i][j1] + b[j1][j2]);
      }
    }
  }

  for(int i = 0; i < nbCustomers; i++){
    free(a[i]);
  }
  free(a);
  free(d);
  for(int j = 0; j < level1; j++){
    free(b[j]);
  }
  free(b);

  //sortiranje pre (offline) tj. pre same primene algoritma

  for(int i = 0; i < nbCustomers; i++){
    qsort(c[i], (size_t) n, sizeof(c[0][0]), cmp);
  }

  //inicijalizacija resenja

  bool *solution = (bool*) malloc((unsigned long) level * sizeof(bool));
  assert(solution != NULL);
  initialize(solution, level1, level2);

  //resavanje problema

  struct resenje solutionValue = localSearch(solution, fixedCost, nbCustomers, level1, level2, c);
  double endTime = clock();
  double vreme = (endTime - startTime) / CLOCKS_PER_SEC;

  //ispis resenja

  printf("Resenje: %.4lf\n", solutionValue.value);
  printf("Vreme najboljeg resenja: %.4lf\n", solutionValue.vreme);
  printf("Vreme izvrsavanja: %.4lf sekundi.\n", vreme);

  //oslobadjanje alocirane memorije

  free(fixedCost);
  for(int i = 0; i < nbCustomers; i++){
    free(c[i]);
  }
  free(c);
  free(solution);
  exit(EXIT_SUCCESS);
}
