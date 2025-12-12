#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#define MAX_ITER_BCO 100
#define MAX_ITER_LS 5
#define INF 1e9
#define B 100
#define NC 30
#define EPS 1e-10
#define P1 35
#define P2 25

// funkcija koja vraca random broj iz intervala [0,1]

#define drand() rand() / (double)RAND_MAX

/* struktura koja se pravi da bi se upamtila vrednost funkcije cilja u svakoj 
   iteraciji kao i vreme koje je proteklo od pocetka izvrsavanja programa do izracunavanja resenja*/

struct resenje{
  double value;
  double vreme;
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
  struct cena *a1 = (struct cena *)a;
  struct cena *a2 = (struct cena *)b;
  if ((*a1).value < (*a2).value){
    return -1;
  }
  else if ((*a1).value > (*a2).value){
    return 1;
  }
  else{
    return 0;
  }
}

//funkcija za racunanje vrednosti funkcije cilja

double upgradedSolutionValue(bool *solution, double *fixedCost, int m, int level1, int level2, struct cena **c){
  register double value = 0.0;
  register double minValue;
  int i, j, jUsed, kUsed;
  int level = level1 + level2;
  int n = level1 * level2;
  bool *used = (bool *)malloc((unsigned long)level * sizeof(bool));
  assert(used != NULL);
  for (j = 0; j < level; j++){
    used[j] = false;
  }
  for (i = 0; i < m; i++){
    jUsed = -1;
    kUsed = level1 - 1;
    minValue = INF;
    for (j = 0; j < n; j++){
      if (!(solution[c[i][j].j1] && solution[level1 + c[i][j].j2])){
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

  for (j = 0; j < level; j++){
    solution[j] = used[j];
    if (!used[j]){
      continue;
    }
    value += fixedCost[j];
  }
  return value;
}

//funkcija koja proverava da li je konstruisano resenje ispravno

bool ispravno(bool *solution, int level1, int level2){
  bool a = false, b = false;
  for (int i = 0; i < level1; i++){
    if (solution[i]){
      a = true;
      break;
    }
  }
  for (int j = 0; j < level2; j++){
    if (solution[level1 + j]){
      b = true;
      break;
    }
  }
  return a && b;
}

//funkcija koja nalazi minimalni element niza

double minimum(double *value){
  double min = value[0];
  for (int i = 1; i < B; i++){
    if (min > value[i]){
      min = value[i];
    }
  }
  return min;
}

//funkcija koja nalzi minimalno resenje zajedno sa vremenom kada je ono prvi put dobijeno

struct resenje minimumsol(struct resenje *niz){
  struct resenje min;
  min.value = niz[0].value;
  min.vreme = niz[0].vreme;
  for (int i = 1; i < B; i++){
    if (min.value > niz[i].value){
      min.value = niz[i].value;
      min.vreme = niz[i].vreme;
    }
  }
  return min;
}

//funkcija koja nalazi maksimalni element niza

double maksimum(double *value){
  double max = value[0];
  for (int i = 1; i < B; i++){
    if (max < value[i]){
      max = value[i];
    }
  }
  return max;
}

//funkcija koja racuna sumu normalizovanih vrednosti funkcije cilja za pcele regrutere

double suma(int r, double *l){
  double sum = 0.0;
  for (int i = 0; i < r; i++){
    sum += l[i];
  }
  return sum;
}

//funkcija koja kopira elemente jednog niza u drugi

void arraycopy(bool *a, bool *b, int n){
  for (int i = 0; i < n; i++){
    a[i] = b[i];
  }
}

//funkcija koja racuna normalizovanu vrednost resenja

double loyality(double max, double min, double value){
  return (max - min < EPS) ? 1.0 : ((max - value) / (max - min));
}

//funkcija koja racuna lojalnost svake pcele

double decision(int u, double l){
  return exp(-(1 - l) / (double)(u + 1.0));
}

//funkcija koja inicijalizuje resenje

void initialize(bool *solution, int level1, int level2){
  int level = level1 + level2;
  for (int i = 0; i < level1; i++){
    solution[i] = rand() % 100 < P1;
  }
  for (int i = level1; i < level; i++){
    solution[i] = rand() % 100 < P2;
  }
  if (!ispravno(solution, level1, level2)){
    int i = rand() % level1;
    int k = rand() % level2;
    solution[i] = true;
    solution[level1 + k] = true;
  }
}

//funkcija koja vraca suseda trenutnog resenja

int *invert(bool *solution, int level1, int level2){
  int *a = (int *)malloc(2UL * sizeof(int));
  assert(a != NULL);
  int j = rand() % level1;
  int k = rand() % level2;
  solution[j] = !solution[j];
  solution[level1 + k] = !solution[level1 + k];
  if (ispravno(solution, level1, level2)){
    a[0] = j;
    a[1] = k;
    return a;
  }
  else{
    solution[j] = !solution[j];
    solution[level1 + k] = !solution[level1 + k];
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

//LS za hibridizaciju

void localSearch(bool *solution, double *fixedCost, int m, int level1, int level2, struct cena **c){
  register double currentValue;
  int iteration = 0;
  double newValue;
  int *a;
  currentValue = upgradedSolutionValue(solution, fixedCost, m, level1, level2, c);
  //int level = level1 + level2;
  while (iteration++ < MAX_ITER_LS){
    a = invert(solution, level1, level2);
    if ((a[0] == -1 && a[1] == -1) || a[0] == -1 || a[1] == -1){
      iteration--;
      continue;
    }
    newValue = upgradedSolutionValue(solution, fixedCost, m, level1, level2, c);
    if (newValue < currentValue){
      currentValue = newValue;
    }
    else{
      restore(solution, a[0], a[1], level1);
    }
  }
}

//algoritam hibridizacije LS i BCO

struct resenje hybrid(double *fixedCost, int m, int level1, int level2, struct cena **c){
  double startTime = clock();
  int b;
  int br;
  int u;
  int R;
  int K;
  int iteration = 0;
  int level = level1 + level2;
  double min;
  double max;
  double suma;
  struct resenje minimalno;
  bool **solution = (bool **)malloc((size_t)B * sizeof(bool *));
  assert(solution != NULL);
  for (b = 0; b < B; b++){
    solution[b] = malloc((size_t)level * sizeof(bool));
    assert(solution[b] != NULL);
  }
  struct resenje *solValue = (struct resenje *)malloc((unsigned long int)B * sizeof(struct resenje));
  assert(solValue != NULL);
  double *dec = (double *)malloc((unsigned long int)B * sizeof(double));
  bool *uncomitted = (bool *)malloc(B * sizeof(bool));
  bool *recruiter = (bool *)malloc(B * sizeof(bool));
  int *a = (int *)malloc(2UL * sizeof(int));
  assert(dec != NULL);
  assert(uncomitted != NULL);
  assert(recruiter != NULL);
  assert(a != NULL);
  for (b = 0; b < B; b++){
    uncomitted[b] = false;
    recruiter[b] = false;
  }
  struct resenje bestValue;
  bestValue.value = INF;
  bestValue.vreme = startTime;
  do{
    for (b = 0; b < B; b++){
      initialize(solution[b], level1, level2);
    }
    for (u = 0; u < NC; u++){
      //forward pass
      for (b = 0; b < B; b++){
        if (recruiter[b]){
          localSearch(solution[b], fixedCost, m, level1, level2, c);
        }
      }
      //bacward pass
      for (b = 0; b < B; b++){
        solValue[b].value = upgradedSolutionValue(solution[b], fixedCost, m, level1, level2, c);
      }
      K = 0;
      min = minimum(&solValue->value);
      max = maksimum(&solValue->value);
      for (int b = 0; b < B; b++){
        uncomitted[b] = false;
        recruiter[b] = false;
      }
      //loyality decision
      for (b = 0; b < B; b++){
        dec[b] = decision(u, loyality(max, min, solValue[b].value));
        if (!(dec[b] > drand())){
          uncomitted[b] = true;
          K++;
        }
        else{
          recruiter[b] = true;
        }
      }
      R = B - K;
      bool **rsolution = (bool **)malloc((unsigned long int)R * sizeof(bool *));
      assert(rsolution != NULL);
      double *rsolValue = (double *)malloc((unsigned long int)R * sizeof(double));
      assert(rsolValue != NULL);
      double *loy = (double *)malloc((unsigned long int)R * sizeof(double));
      assert(loy != NULL);
      for (int b = 0; b < R; b++){
        rsolution[b] = (bool *)malloc((unsigned long int)level * sizeof(bool));
        assert(rsolution[b] != NULL);
      }
      //recruiting procces
      br = 0;
      suma = 0.0;
      for (b = 0; b < B; b++){
        if (recruiter[b]){
          arraycopy(rsolution[br], solution[b], level);
          rsolValue[br] = solValue[b].value;
          suma += loyality(max, min, solValue[b].value);
          br++;
        }
      }
      for (br = 0; br < R; br++){
        loy[br] = loyality(max, min, rsolValue[br]);
      }
      for (int b = 0; b < B; b++){
        if (uncomitted[b]){
          for (br = 0; br < R; br++){
            if ((loy[br] / suma) > drand()){
              arraycopy(solution[b], rsolution[br], level);
              break;
            }
          }
        }
      }
      free(loy);
      free(rsolValue);
      for (b = 0; b < R; b++){
        free(rsolution[b]);
      }
      free(rsolution);
    }
    for (b = 0; b < B; b++){
      solValue[b].value = upgradedSolutionValue(solution[b], fixedCost, m, level1, level2, c);
      solValue[b].vreme = (clock() - startTime) / CLOCKS_PER_SEC;
    }
    minimalno = minimumsol(solValue);
    if (minimalno.value < bestValue.value){
      bestValue.value = minimalno.value;
      bestValue.vreme = minimalno.vreme;
    }
  } while (iteration++ < MAX_ITER_BCO);
  return bestValue;
}

//main funkcija gde se vrsi ucitavanje podataka iz fajla i poziv hibridnog algoritma kao i ispis resenja

int main(int argc, char **argv){
  srand((unsigned int)time(NULL));
  struct resenje solutionValue;
  double startTime = clock();
  FILE *in = fopen(argv[1], "r");
  assert(in != NULL);
  if (ferror(in)){
    fprintf(stderr, "Error reading file %s!\n", argv[1]);
    exit(EXIT_FAILURE);
  }
  //Ucitavanje ulaznih podataka

  int nbCustomers, level1, level2;
  fscanf(in, "%d%d%d", &nbCustomers, &level1, &level2);

  //Broj promenljivih

  //Broj Yjr za r = 1,2

  int level = level1 + level2;
  int n = level1 * level2;

  //Broj Xij1j2

  //Ucitavanje zahteva korisnika

  double *d = (double *)malloc((unsigned long)nbCustomers * sizeof(double));
  assert(d != NULL);
  for (int i = 0; i < nbCustomers; i++){
    fscanf(in, "%lf", &d[i]);
  }

  //Ucitavanje fiksnih troskova

  double *fixedCost = (double *)malloc((unsigned long)level * sizeof(double));
  assert(fixedCost != NULL);
  for (int j = 0; j < level; j++){
    fscanf(in, "%lf", &fixedCost[j]);
  }

  //Ucitavanje matrice cena izmedju korisnika i facility-ja prvog nivoa

  double **a = (double **)malloc((unsigned long)nbCustomers * sizeof(double *));
  assert(a != NULL);
  for (int i = 0; i < nbCustomers; i++){
    a[i] = (double *)malloc((unsigned long)level1 * sizeof(double));
    assert(a[i] != NULL);
    for (int j1 = 0; j1 < level1; j1++){
      fscanf(in, "%lf", &a[i][j1]);
    }
  }

  //Ucitavanje matrice cena izmedju facility-ja prvog i drugog nivoa

  double **b = (double **)malloc((unsigned long)level1 * sizeof(double *));
  assert(b != NULL);
  for (int j1 = 0; j1 < level1; j1++){
    b[j1] = (double *)malloc((unsigned long)level2 * sizeof(double));
    assert(b[j1] != NULL);
    for (int j2 = 0; j2 < level2; j2++){
      fscanf(in, "%lf", &b[j1][j2]);
    }
  }

  fclose(in);

  /* za svakog potrosaca pravi se niz struktura sa cenama za svaki par j1 j2
     i takav se niz sortira u rastucem poretku da bi se smanjilo vreme 
     pretrage pri racunanju vrednosti funkcije cilja*/

  struct cena **c = (struct cena **)malloc((unsigned long)nbCustomers * sizeof(struct cena *));
  assert(c != NULL);
  for (int i = 0; i < nbCustomers; i++){
    c[i] = (struct cena *)malloc((unsigned long)n * sizeof(struct cena));
    assert(c[i] != NULL);
    for (int j1 = 0; j1 < level1; j1++){
      for (int j2 = 0; j2 < level2; j2++){
        c[i][j1 * level2 + j2].j1 = j1;
        c[i][j1 * level2 + j2].j2 = j2;
        c[i][j1 * level2 + j2].value = d[i] * (a[i][j1] + b[j1][j2]);
      }
    }
  }

  for (int i = 0; i < nbCustomers; i++){
    free(a[i]);
  }
  free(a);
  free(d);
  for (int j = 0; j < level1; j++){
    free(b[j]);
  }
  free(b);

  //sortiranje pre offline tj. pre same primene algoritma

  for (int i = 0; i < nbCustomers; i++){
    qsort(c[i], (size_t)n, sizeof(c[0][0]), cmp);
  }

  //resavanje problema

  solutionValue = hybrid(fixedCost, nbCustomers, level1, level2, c);
  double endTime = clock();
  double vreme = (endTime - startTime) / CLOCKS_PER_SEC;

  //ispis resenja

  printf("Resenje: %.4lf\n", solutionValue.value);
  printf("Najbolje vreme: %.4lf sekundi.\n", solutionValue.vreme);
  printf("Vreme izvrsavanja: %.4lf sekundi.\n", vreme);

  //oslobadjanje alocirane memorije

  free(fixedCost);
  for (int i = 0; i < nbCustomers; i++){
    free(c[i]);
  }
  free(c);
  exit(EXIT_SUCCESS);
}
