#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define CHAIRS 3                /* número de cadeiras para os clientes à espera */


sem_t cust_d, cust_c, cust_s;   /* número de cliente à espera de atendimento */
sem_t barb_d, barb_c, barb_s;   /* número de barbeiros à espera de clientes */
sem_t mutex;                    /* para exclusão mútua */
int waiting = 0;                /* clientes que estão esperando (não estão cortando) */
int b_id_d = 1, b_id_c = 2, b_id_s = 3;

/* protótipos */
void* barber(int *type);
void* customer(int *type);
void dye_hair();
void cut_hair();
void shave_beard();
void customer_has_arrived();
void get_hair_colored();
void get_haircut();
void get_beard_shaven();
void customer_has_given_up();

int main(){
	sem_init(&cust_d, 0, 0);
	sem_init(&cust_c, 0, 0);
	sem_init(&cust_s, 0, 0);
	sem_init(&barb_d, 0, 0);
	sem_init(&barb_c, 0, 0);
	sem_init(&barb_s, 0, 0);
	sem_init(&mutex, 0, 1);
	srand(time(NULL));

	pthread_t dye, cut, shave, cust;


	/* criando barbeiros */
	printf("Criando barbeiros.\n");
	pthread_create(&dye, NULL, (void *) barber, &b_id_d);
	printf("Primeiro barbeiro criado.\n");
	sleep(1);
	pthread_create(&cut, NULL, (void *) barber, &b_id_c);
	printf("Segundo barbeiro criado.\n");
	sleep(1);
	pthread_create(&shave, NULL, (void *) barber, &b_id_s);
	printf("Terceiro barbeiro criado.\n");
	sleep(1);

	/* criação indefinida de clientes */
	printf("\n\nCriando clientes:\n\n");
	while(1){
		int r = (rand() % 3) + 1;   /* retorna um número entre 1 e 3*/
		//printf("\nTipo de cliente = %d.\n", r);
		pthread_create(&cust, NULL, (void *) customer, &r);
		sleep(1);
	}

	return 0;
}

void* barber(int *type){
	while(1){
		/* se nao tiver cliente, o barbeiro em questao dorme */
		if(*type == 1) sem_wait(&cust_d);
		else if(*type == 2) sem_wait(&cust_c);
		else if(*type == 3) sem_wait(&cust_s);

		/* entra e sai da regiao crítica e decresce o waiting */
		sem_wait(&mutex);
		waiting--;
		sem_post(&mutex);

		/* efetua um servico e libera o barbeiro */
		if(*type == 1){
			dye_hair();
			sem_post(&barb_d);
		} else if(*type == 2){
			cut_hair();
			sem_post(&barb_c);
		} else if(*type == 3){
			shave_beard();
			sem_post(&barb_s);
		}
	}

	pthread_exit(NULL);
}

void* customer(int *type){
	sem_wait(&mutex);							/* entra na regiao crítica */

	if(waiting < CHAIRS){      		/* se não houver cadeiras vazias, saia */
		customer_has_arrived();
		waiting++;  								/* incrementa o contador de clientes à espera */
		
		/* acorda o barbeiro se necessário */
		if(*type == 1) sem_post(&cust_d);
		else if(*type == 2) sem_post(&cust_c);
		else if(*type == 3) sem_post(&cust_s);
		sem_post(&mutex); 					/* sai da regiao crítica */

		/* vai dormir se o número de barbeiros livres for 0 */
		if(*type == 1){ 
			sem_wait(&barb_d);
			get_hair_colored();
		} else if(*type == 2){
			sem_wait(&barb_c);
			get_haircut();
		} else if(*type == 3){
			sem_wait(&barb_s);
			get_beard_shaven();
  	}			

	} else{
		sem_post(&mutex); 					/* sai da regiao crítica */
		customer_has_given_up();
	}

	pthread_exit(NULL);
}

void dye_hair(){
	printf("O barbeiro 1 está pintando o cabelo de alguém...\n");
	sleep(3);
}

void cut_hair(){
	printf("O barbeiro 2 está cortando o cabelo de alguém...\n");
	sleep(3);
}

void shave_beard(){
	printf("O barbeiro 3 está fazendo a barba de alguém...\n");
	sleep(3);
}

void customer_has_arrived(){
	printf("Um cliente chegou ao salão.\n");
}

void get_hair_colored(){
	printf("Um cliente está tendo seu cabelo pintado.\n");
}

void get_haircut(){
	printf("Um cliente está recebendo um corte de cabelo.\n");
}

void get_beard_shaven(){
	printf("Um cliente está tendo sua barba feita.\n");
}

void customer_has_given_up(){
	printf("Um cliente desistiu... O salão está muito cheio.\n");
}