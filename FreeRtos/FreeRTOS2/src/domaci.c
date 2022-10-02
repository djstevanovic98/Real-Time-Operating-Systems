#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include<stdio.h>
#include<stdlib.h>
#include <time.h>


static int count_id = 1;
static int flag_rek = 0;
static xSemaphoreHandle xAddMutex;
static xSemaphoreHandle xConnectMutex;

static int random = 0;
static int randomKon = 5;

static int broj_poslova = 0;

struct{

	int id;
	char title[16];
	char type[1];
	int predecesors_id[5];
	int succesors_id[5];

}Jobs[64];

static int add_job(char name[], char type[]){

	broj_poslova++;

	if(strlen(name) > 16){

		printf("Greska, preveliko ime!\n");
		fflush(stdout);
		return 0;

	}else if((strcmp(type, "A") && strcmp(type, "B") && strcmp(type, "C") && strcmp(type, "D")) == 1){

		printf("Greska, pogresan tip!\n");
		fflush(stdout);
		return 0;

	}else if(name == NULL || type == NULL){

		printf("Greska, niste naveli oba karaktera!\n");
		fflush(stdout);
		return 0;

	}
	if(xSemaphoreTake(xAddMutex, (TickType_t) portMAX_DELAY) == pdTRUE){

		if(broj_poslova>=64){	// moze i preko semafora sa 64 i da dajem give u remove_job

			printf("Greska, sistem je pun!\n");
			fflush(stdout);
			return 0;

		}else{



			strcpy( Jobs[count_id].title, name);
			strcpy( Jobs[count_id].type, type);

			Jobs[count_id].id = count_id;
			count_id +=1;

			//bio ovde take

	}

	xSemaphoreGive(xAddMutex);
				}
	return 1;
}

int rek_ciklicni_preth(int p_id, int s_id){
	int i;

	for(i = 0; i<5; i++){

		if(Jobs[p_id].predecesors_id[i] == Jobs[s_id].id){
			flag_rek = 1;
			break;
		}
		if(Jobs[p_id].predecesors_id[i] != 0 && flag_rek!=1){
		rek_ciklicni_preth(Jobs[p_id].predecesors_id[i], s_id);
		}

	}

	if(flag_rek == 1){
		return 1;
	}
	return 0;
}

static void connect_jobs  (int p_id, int s_id){

	if(((p_id && s_id) == 0) || (p_id == s_id)){
		printf("Greska, niste validno uneli parametre! \n");
		return;
	}

	int flag = 1;
	for(int i =0; i<count_id;i++){
		if(Jobs[i].id == p_id){
			flag = 0;
		}
	}

	if(flag == 1){
		printf("Greska, ne postoji p_id! \n");
		return;
	}


	flag = 1;
	for(int i =0; i<count_id;i++){
		if(Jobs[i].id == s_id){
			flag = 0;
		}
	}
	if(flag == 1){
		printf("Greska, ne postoji s_id! \n");
		return;
	}
	if(xSemaphoreTake(xConnectMutex, (TickType_t)portMAX_DELAY) == pdTRUE){

		int pos_sl;	// pozicija u nizu sledbenika
		flag = 1;
		for(int i=0;i<5;i++){
			if(Jobs[p_id].succesors_id[i] == 0){
				flag = 0;
				pos_sl = i;
				break;				//ova dva reda
			}
		}
		if(flag == 1){
			printf("Greska, p_id vec ima 5 sledbenika! \n");
			fflush(stdout);
			return;
		}
		//*

		int pos_pr;				// pozicija u nizu prethodnika
		flag = 1;
		for(int i=0;i<5;i++){
			if(Jobs[s_id].predecesors_id[i] == 0){
						flag = 0;
						pos_pr = i;
						break;
			}
		}
		if(flag == 1){
			printf("Greska, s_id vec ima 5 sledbenika! \n");
			fflush(stdout);
			return;
		}
		//**


		for(int i = 0; i<5;i++){
			if(Jobs[p_id].succesors_id[i] == s_id){
				printf("Veza vec postoji! \n");
				fflush(stdout);
				return;
			}
		}
		//*** semafor

			if(rek_ciklicni_preth(p_id, s_id)){
				printf("Pojavljuje se ciklus!");
				fflush(stdout);
				return;
				}
				flag_rek = 0;


				Jobs[p_id].succesors_id[pos_sl] = s_id;
				Jobs[s_id].predecesors_id[pos_pr] = p_id;

			xSemaphoreGive(xConnectMutex);
		}
		printf("Uspesno konektovanje!\n");
		fflush(stdout);

}

static void disconnect_jobs (int p_id, int s_id){

	printf("%d %d", p_id, s_id);

	if(((p_id && s_id) == 0) || (p_id == s_id)){ // da nisu 0;
		printf("Greska, niste validno uneli parametre! \n");
		return;
	}

	if(Jobs[p_id].id != p_id){
		printf("Ne postoji p_id! \n");
		return;
	}
	if(Jobs[s_id].id != s_id){
		printf("Greska, ne postoji s_id! \n");
		return;
	}

	int flag = 1;
	int pos_sl;
	for(int i = 0; i < 5; i++){
		if(Jobs[p_id].succesors_id[i] == s_id){
			flag = 0;
			pos_sl = i;
		}
	}
	if(flag == 1){
		printf("Ne postoji sledbenik!");
		return;
	}
	flag = 1;
	int pos_pr;
	for(int i = 0; i < 5; i++){
		if(Jobs[s_id].predecesors_id[i] == p_id){ // da li su povezani
			flag = 0;
			pos_pr = i;
		}
	}
	if(flag == 1){
		printf("Ne postoji prethodnik!");
		return;
	}

	Jobs[p_id].succesors_id[pos_sl] = 0;
	Jobs[s_id].predecesors_id[pos_pr] = 0;

	printf("Uspesan diskonekt!\n");
	fflush(stdout);

}

static void remove_job(int job_id){

	if(job_id == 0 || job_id>count_id){
		printf("Niste validno uneli parametre!\n");
		fflush(stdout);
		return;
	}

	if(Jobs[job_id].id != job_id){
		printf("Posao ne postoji!\n");
		fflush(stdout);
		return;
	}

	for(int i = 0; i<5;i++){
		if(Jobs[job_id].predecesors_id[i] != 0){
			disconnect_jobs(Jobs[job_id].predecesors_id[i], job_id);
		}
	}
	for(int i = 0; i<5;i++){
		if(Jobs[job_id].succesors_id[i] != 0){
			disconnect_jobs(job_id, Jobs[job_id].succesors_id[i]);
		}
	}


	Jobs[job_id].id = 0;

	printf("Uspesno brisanje!\n");
	fflush(stdout);


}

static void list_jobs(){


	for(int i = 1; i<count_id; i++){

		if(Jobs[i].id == 0){
			continue;
		}

		printf("%d | %s | %s | ", Jobs[i].id, Jobs[i].title, Jobs[i].type);
		fflush(stdout);

		for(int j = 0; j<5; j++){
			if(Jobs[i].predecesors_id[j] == 0){
				printf("- ");
				continue;
			}

			printf("%d ", Jobs[i].predecesors_id[j]);
		}

		printf("| ");
		fflush(stdout);

		for(int k = 0; k<5; k++){
			if(Jobs[i].succesors_id[k] == 0){
				printf("- ");
				continue;
			}

			printf("%d ", Jobs[i].succesors_id[k]);
		}

		printf("|\n");
		fflush(stdout);
	}
}

static void xInterakcijaSaSistemom(void* pvParametri){

	for(;;){

		int choice;
		char name[100];
		char type[100];

		printf("Your options: \n1: add_job <name> <type> \n2: connect_jobs <predecesor_id> <successor_id> \n"
				"3: disconnect_jobs <predecesor_id> <successor_id>\n4: remove_job <job_id> \n5: list_jobs\n");
		fflush(stdout);

		scanf("%d" , &choice);

		if(choice == 1){

			printf("Unesite ime i tip: \n");
			fflush(stdout);

			scanf("%s%s", name, type);

			int dodaj_posao = add_job(name, type);

			if(dodaj_posao){
				printf("Uspesno dodavanje!\n");
				fflush(stdout);
			}
		}else if(choice == 2){
			printf("Povezi dva posla preko id: ");
			fflush(stdout);

			int p_id, s_id;
			scanf("%d%d", &p_id,&s_id);

			connect_jobs(p_id, s_id);


		}else if(choice == 3){
			printf("Unesite dva id-a za diskonekt: \n");
			fflush(stdout);

			int p_id, s_id;
			scanf("%d%d", &p_id, &s_id);
			disconnect_jobs(p_id, s_id);

		}else if(choice == 4){
			printf("Unesite id za brisanje: \n");
			fflush(stdout);
			int job_id;

			scanf("%d", &job_id);
			remove_job(job_id);

		}
		else if(choice == 5){
			list_jobs();
		}else{
			printf("Pogresna komanda!\n");
			fflush(stdout);
		}

		vTaskDelay(500);

	}

	vTaskDelete(0);
}

static void xRandomAddTask(void* PvParametri){

	int* task = (int*)PvParametri;
	int taski = *task;

	srand(time(NULL));

	for(int i = 0; i < 5; i++){ //svaki task kreira 5 taska.
		char name[] = "Job";
		char name_id[2] = "";
		itoa(taski, name_id, 10);


		strcat(name, name_id);

		char type[2] = "";

		int r = ((rand() % 3) + randomKon) % 3;

		if(r == 0){
			strcpy(type, "A");
		}else if(r == 1){
			strcpy(type, "B");
		}else if(r == 2){
			strcpy(type, "C");
		}else{
			strcpy(type, "D");
		}

		add_job(name, type);

	}
	list_jobs();
	vTaskDelete(0);
}

static void xRandomConnectTask(void* PvParametri){
	int* task = (int*)PvParametri;
	int taski = *task;


	for(int i  = 0; i < 5; i++){
		if(randomKon >= 10){
			randomKon = 3;
		}
		if(random >=10){
			random = 0;
		}
		int p_id = ((rand() % count_id) + random++ + randomKon++) % count_id;
		int s_id = ((rand() % count_id) + random++ + randomKon++) % count_id;
		printf("\n Pokusaj povezivanja %d i %d od strane: %d\n", p_id, s_id, taski);
		fflush(stdout);
		connect_jobs(p_id, s_id);
	}


	list_jobs();
	vTaskDelete(0);
}

static void xRandomDeleteTask(void* PvParametri){

	int* task = (int*)PvParametri;
	int taski = *task;

	for(int i = 0; i<2; i++){
		if(random >= 10){
			random = 0;
		}
		if(randomKon >= 10){
			randomKon = 3;
		}
		int r = ((rand() % count_id) + random++ + randomKon++) % count_id;
		remove_job(r);
		printf("Task %d brise posao: %d\n", taski, r);
	}
	fflush(stdout);

	list_jobs();

	vTaskDelete(0);
}

int domaci( void )
{

	int choice;

	printf("Birajte rezim rada: \n1: Interaktivno\n2: Nasumicno\n3: Kombinovano\n");
	fflush(stdout);
	scanf("%d", &choice);


	xAddMutex = xSemaphoreCreateMutex();
	xConnectMutex = xSemaphoreCreateMutex();

	if(choice == 1){
		xTaskCreate(xInterakcijaSaSistemom, "", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	}else if(choice == 2){
		srand(time(NULL));

		int task1 = 1;int task2 = 2;int task3 = 3;int task4 = 4;int task5 = 5;

		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task1, 1, NULL);
		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task2, 1, NULL);
		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task3, 1, NULL);
		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task4, 1, NULL);
		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task5, 1, NULL);

		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task1, 1, NULL);
		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task2, 1, NULL);
		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task3, 1, NULL);
		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task4, 1, NULL);
		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task5, 1, NULL);

		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task1, 1, NULL);
		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task2, 1, NULL);
		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task3, 1, NULL);
		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task4, 1, NULL);
		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task5, 1, NULL);

	}else if(choice == 3){
		xTaskCreate(xInterakcijaSaSistemom, "", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

		srand(time(NULL));

		int task1 = 1;int task2 = 2;int task3 = 3;int task4 = 4;int task5 = 5;

		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task1, 1, NULL);
		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task2, 1, NULL);
		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task3, 1, NULL);
		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task4, 1, NULL);
		xTaskCreate(xRandomAddTask, "", configMINIMAL_STACK_SIZE, &task5, 1, NULL);

		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task1, 1, NULL);
		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task2, 1, NULL);
		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task3, 1, NULL);
		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task4, 1, NULL);
		xTaskCreate(xRandomConnectTask, "", configMINIMAL_STACK_SIZE, &task5, 1, NULL);

		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task1, 1, NULL);
		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task2, 1, NULL);
		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task3, 1, NULL);
		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task4, 1, NULL);
		xTaskCreate(xRandomDeleteTask, "", configMINIMAL_STACK_SIZE, &task5, 1, NULL);
	}else{
		printf("Nevalidan unos!");
		fflush(stdout);
	}
	vTaskStartScheduler();



	return 0;

}

