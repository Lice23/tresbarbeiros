#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define CHAIRS 6                /* número de cadeiras para os clientes à espera */


sem_t customers;                /* número de cliente à espera de atendimento */
sem_t barbers;                  /* número de barbeiros à espera de clientes */
sem_t mutex1;                    /* para exclusão mútua */
sem_t mutex2;                    /* para exclusão mútua */
sem_t mutex3;                    /* para exclusão mútua */
int waiting = 0;                /* clientes que estão esperando (não estão cortando) */

/* protótipos */
void* barber_d(void *arg);
void* barber_c(void *arg);
void* barber_s(void *arg);
void* customer(int type);
void dye_hair();
void cut_hair();
void shave_beard();
void customer_has_arrived();
void get_hair_colored();
void get_haircut();
void get_beard_shaven();
void customer_has_given_up();

int main(){
	sem_init(&customers, 0, 0);
	sem_init(&barbers, 0, 0);
	sem_init(&mutex1, 0, 1);
	sem_init(&mutex2, 0, 1);
	sem_init(&mutex3, 0, 1);
	srand(time(NULL));

	pthread_t dye, cut, shave, cust;

	/* criando barbeiros */
	printf("Criando barbeiros.\n");

	pthread_create(&dye, NULL, (void *) barber_d, NULL);
	sleep(1);
	printf("Primeiro barbeiro criado.\n");
	pthread_create(&cut, NULL, (void *) barber_c, NULL);
	sleep(1);
	printf("Segundo barbeiro criado.\n");
	pthread_create(&shave, NULL, (void *) barber_s, NULL);
	sleep(1);
	printf("Terceiro barbeiro criado.\n");

	/* criação indefinida de clientes */
	printf("\n\nCriando clientes:\n\n");
	while(1){
		int r = rand() % 3;      /* retorna um número entre 0 e 2*/
		printf("r = %d.\n", r);
		pthread_create(&cust, NULL, (void *) customer(r), NULL);
		sleep(1);
	}

	return 0;
}

void* barber_d(void *arg){
	printf("Barbeiro criado. Entrando no while...\n");
	while(1){
		printf("No while do barbeiro.\n");
		sem_wait(&customers);   			/* vai dormir se o número de clientes for 0 */
		printf("Passou o espera clientes.\n");
		sem_wait(&mutex1);       			/* obtém acesso a 'waiting' */
		printf("Obteve acesso ao waiting.\n");
		waiting = waiting - 1;  			/*descresce de um o contador de clientes à espera */
		printf("Decresceu waiting em 1.\n");
		sem_post(&barbers);     			/* um barbeiro está agora pronto para cortar cabelo */
		printf("Um barbeiro agora está pronto para fazer algo.\n");
		sem_post(&mutex1);       			/* libera 'waiting' */
		printf("Liberou o waiting.\n");
		dye_hair(); 				/* pinta o cabelo (fora da região crítica) */
		printf("Fez um servico.\n");
	}

	pthread_exit(NULL);
}

void* barber_c(void *arg) {
	while(1) {
		sem_wait(&customers);   /* vai dormir se o número de clientes for 0 */
		sem_wait(&mutex2);       /* obtém acesso a 'waiting' */
		waiting = waiting - 1;  /*descresce de um o contador de clientes à espera */
		sem_post(&barbers);     /* um barbeiro está agora pronto para cortar cabelo */
		sem_post(&mutex2);       /* libera 'waiting' */
		cut_hair();             /* corta o cabelo (fora da região crítica) */
	}

	pthread_exit(NULL);
}

void* barber_s(void *arg) {
	while(1) {
		sem_wait(&customers);   /* vai dormir se o número de clientes for 0 */
		sem_wait(&mutex3);       /* obtém acesso a 'waiting' */
		waiting = waiting - 1;  /*descresce de um o contador de clientes à espera */
		sem_post(&barbers);     /* um barbeiro está agora pronto para cortar cabelo */
		sem_post(&mutex3);       /* libera 'waiting' */
		shave_beard();             /* corta o cabelo (fora da região crítica) */
	}

	pthread_exit(NULL);
}

void* customer(int type){
	printf("Customer type is %d.\n", type);
	if(!type) sem_wait(&mutex1);           				/* entra na região crítica */
	else if(type == 1) sem_wait(&mutex2);
	else sem_wait(&mutex3);
	if(waiting < CHAIRS){      					/* se não houver cadeiras vazias, saia */
		customer_has_arrived();
		waiting = waiting + 1;  					/* incrementa o contador de clientes à espera */
		sem_post(&customers);   					/* acorda o barbeiro se necessário */
		if(!type) sem_post(&mutex1);           				/* entra na região crítica */
		else if(type == 1) sem_post(&mutex2);
		else sem_post(&mutex3);      					/* libera o acesso a 'waiting' */
		sem_wait(&barbers);     					/* vai dormir se o número de barbeiros livres for 0 */
		if(!type) get_hair_colored();			/* sentado e sendo servido */
		else if(type == 1) get_haircut();			/* sentado e sendo servido */
		else get_beard_shaven();	/* sentado e sendo servido */
	} else{
		if(!type) sem_post(&mutex1);           				/* entra na região crítica */
		else if(type == 1) sem_post(&mutex2);
		else sem_post(&mutex3);      					/* a barbearia está cheia; não espera */
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