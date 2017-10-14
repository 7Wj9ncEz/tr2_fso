#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define SEM_KEY		0x1234
#define PENSANDO 1
#define ESPERANDO 2
#define ATRAVESSANDO 3
#define CANSADO 4

int		g_sem_id, count;

struct sembuf	g_lock_sembuf[1];
struct sembuf	g_unlock_sembuf[1];

void retira_crianca(){
	count -= 1;
}

void liga_crianca(int lado, int id, int num_travessias) {
	int estado = PENSANDO;
	sleep(2);
	while(1) {
		if(estado == PENSANDO) {

			printf("Crianca %d: PENSANDO do lado %d\n", id,lado);
			fflush(stdout);

			srand( (unsigned)time(NULL) + id );
			int random_brincando = rand() % 30; 
			sleep(random_brincando);

			estado = ESPERANDO;
			
		}
		if(estado == ESPERANDO) {
			printf("Crianca %d: ESPERANDO\n", id);
			fflush(stdout);
			semop( g_sem_id, g_lock_sembuf, 1 );
			estado = ATRAVESSANDO;
		}
		if(estado == ATRAVESSANDO) {
			printf("Crianca %d: ATRAVESSANDO\n", id);
			fflush(stdout);
			usleep(70);
			num_travessias-=1;
			printf("Crianca %d: Atravessei\n", id);
			semop (g_sem_id, g_unlock_sembuf, 1);
			fflush(stdout);
			if(num_travessias>0){
				estado = PENSANDO;
			}else{
				estado = CANSADO;
			}
		}
		if(estado == CANSADO) {
			kill(getppid(), SIGUSR1);
			printf("Crianca %d: Cansei, vou pra casa\n", id);
			exit(1);
		}
	}
}

int main() {
	int num_criancas_dir = 0;
	int num_criancas_esq = 0;
	int num_travessias = 0;
	printf("Digite a quantidade de criancas: ESQUERDA DIREITA\n");
	scanf("%d %d", &num_criancas_esq, &num_criancas_dir);

	printf("Digite o numero de travesias:");
	scanf("%d", &num_travessias);


	int rtn;
	int * pid = malloc(sizeof(int)*(num_criancas_esq+num_criancas_dir));

	/* Construindo a estrutura de controle do semaforo */
	g_lock_sembuf[0].sem_num   = 0; g_lock_sembuf[0].sem_op   = -1;g_lock_sembuf[0].sem_flg   = 0;
	g_unlock_sembuf[0].sem_num = 0; g_unlock_sembuf[0].sem_op = 1; g_unlock_sembuf[0].sem_flg = 0;
  
	/* Criando o semaforo */	
	g_sem_id = semget( SEM_KEY, 1, IPC_CREAT | 0666 );
	semop (g_sem_id, g_unlock_sembuf, 1);

  	/* Criando criancas */
  	rtn = 1;
	for (count = 0; count < num_criancas_esq + num_criancas_dir; count++ ) {
	    if( rtn != 0 ) {
	      pid[count] = rtn = fork();
	    } 
	    else {
	      break;
	    }
	}


	/* Verificando o valor retornado para determinar se o processo e pai ou filho  */
  	if( rtn == 0 ) {
		/* Estou no processo filho... */
		printf("Filho %i comecou ...\n", count);
		fflush(stdout);
		liga_crianca(count%2, count, num_travessias);
  	} 
  	else {
		  signal(SIGUSR1,retira_crianca);
		/* Estou no processo pai ... */
		//esperando ate todas as crianças se cansarem
		while(count>0){
			sleep(1);
		}
		printf("TODAS AS CRIANÇAS SE CANSARAM E FORAM EMBORA.\n");
		/* Removendo o semaforo */
		semctl(g_sem_id, 0, IPC_RMID, 0);
  	} /* fim-else */
	return 0;
}