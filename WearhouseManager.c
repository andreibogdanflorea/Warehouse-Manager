#include <stdio.h>
#include "WearhouseManager.h"


Package *create_package(long priority, const char* destination){
	Package *package = (Package*)malloc(sizeof(Package));
	package->priority = priority;
	// strdup are undefined behaviour pentru NULL ca parametru
	if (destination == NULL)
		package->destination = NULL;
	else
		package->destination = strdup(destination);
	return package;
}

void destroy_package(Package* package){
	if (package != NULL)
		free(package->destination);
	free(package);
}

Manifest* create_manifest_node(void){
	Manifest *manifest_node = (Manifest*)calloc(1, sizeof(Manifest));
	return manifest_node;
}

void destroy_manifest_node(Manifest* manifest_node){
	if (manifest_node != NULL) {
		destroy_package(manifest_node->package);
	}
	free(manifest_node);
}

Wearhouse* create_wearhouse(long capacity){
	if (capacity == 0) {
		return NULL;
	}
	Wearhouse *wearhouse = (Wearhouse*)malloc(sizeof(Wearhouse));
	wearhouse->capacity = capacity;
	wearhouse->size = 0;
	// capacity pointeri catre pachete
	wearhouse->packages = (Package**)malloc(capacity * sizeof(Package*));
	return wearhouse;
}

Wearhouse *open_wearhouse(const char* file_path){
	ssize_t read_size;
	char* line = NULL;
	size_t len = 0;
	char* token = NULL;
	Wearhouse *w = NULL;


	FILE *fp = fopen(file_path, "r");
	if(fp == NULL)
		goto file_open_exception;

	if((read_size = getline(&line, &len, fp)) != -1){
		token = strtok(line, ",\n ");
		w = create_wearhouse(atol(token));

		free(line);
		line = NULL;
		len = 0;
	}

	while((read_size = getline(&line, &len, fp)) != -1){
		token = strtok(line, ",\n ");
		long priority = atol(token);
		token = strtok(NULL, ",\n ");
		Package *p = create_package(priority, token);
		w->packages[w->size++] = p;

		free(line);
		line = NULL;
		len = 0;
	}

	free(line);


	fclose(fp);
	return w;

	file_open_exception:
	return NULL;
}

long wearhouse_is_empty(Wearhouse *w){
	return w->size == 0;
}

long wearhouse_is_full(Wearhouse *w){
	return w->size == w->capacity;
}

long wearhouse_max_package_priority(Wearhouse *w){
	long max_prio = w->packages[0]->priority;
	for (int i = 1; i < w->size; i++) {
		Package *p = w->packages[i];
		if (p->priority > max_prio) {
			max_prio = p->priority;
		} 
	}
	return max_prio;
}

long wearhouse_min_package_priority(Wearhouse *w){
	long min_prio = w->packages[0]->priority;
	for (int i = 1; i < w->size; i++) {
		Package *p = w->packages[i];
		if (p->priority < min_prio) {
			min_prio = p->priority;
		} 
	}
	return min_prio;
}


void wearhouse_print_packages_info(Wearhouse *w){
	for(long i = 0; i < w->size; i++){
		printf("P: %ld %s\n",
				w->packages[i]->priority,
				w->packages[i]->destination);
	}
	printf("\n");
}

void destroy_wearhouse(Wearhouse* wearhouse){
	for (int i = 0; i < wearhouse->size; i++) {
		destroy_package(wearhouse->packages[i]);
	}
	free(wearhouse->packages);
	free(wearhouse);
}


Robot* create_robot(long capacity){
	Robot* robot = (Robot*) malloc(sizeof(Robot));
	robot->size = 0;
	robot->capacity = capacity;
	robot->manifest = NULL;
	return robot;
}

int robot_is_full(Robot* robot){
	return robot->size == robot->capacity;
}

int robot_is_empty(Robot* robot){
	return robot->size == 0;
}

Package* robot_get_wearhouse_priority_package(Wearhouse *w, long priority){
	// NULL pentru ca la remove wearhouse package asa marcam initial stergerea
	// am uitat sa mai schimb dupa
	for (int i = 0; i < w->size; i++) {
		if (w->packages[i] != NULL && w->packages[i]->priority == priority)
			return w->packages[i];
	}
	return NULL;
}

void robot_remove_wearhouse_package(Wearhouse *w, Package* package){
	// daca dau de pachetul respectiv, mut celelalte elemente "peste el"
	for (int i = 0; i < w->size; i++) {
		if (w->packages[i] == package) {
			for (int j = i + 1; j < w->size; j++)
				w->packages[j - 1] = w->packages[j]; 
			w->size--;
			return;
		}
	}
}

void robot_load_one_package(Robot* robot, Package* package){
	if (robot_is_full(robot))
		return;
	// creez nodul de manifest
	Manifest *new_man = (Manifest*)malloc(sizeof(Manifest));
	new_man->package = package;
	new_man->next = new_man->prev = NULL;
	// daca lista este goala, adaug in fata
	if (robot->manifest == NULL) {
		robot->manifest = new_man;
		robot->size++;
		return;
	}
	// altfel ii caut locul
	Manifest *man = robot->manifest;
	Manifest *prev = NULL;
	while (man != NULL && ((man->package->priority > package->priority) ||
		  (man->package->priority == package->priority && 
		  strcmp(man->package->destination, package->destination) < 0))) {
		prev = man;
		man = man->next;
	}
	// daca lista nu e goala, dar trebuie adaugat fix la inceput
	if (prev == NULL) {
		new_man->next = robot->manifest;
		robot->manifest = new_man;
		robot->size++;
		return;
	}
	// la mijloc sau la final
	new_man->prev = prev;
	new_man->next = man;
	prev->next = new_man;
	// daca e la final, nu trebuie facuta aceasta instructiune
	if (man != NULL)
		man->prev = new_man;
	robot->size++;
}

long robot_load_packages(Wearhouse* wearhouse, Robot* robot){
	long no_packages = 0;
	
	while (!wearhouse_is_empty(wearhouse) && robot->size < robot->capacity) {
		no_packages++;
		long priority = wearhouse_max_package_priority(wearhouse);
		Package *package = robot_get_wearhouse_priority_package(wearhouse, priority);
		robot_load_one_package(robot, package);
		robot_remove_wearhouse_package(wearhouse, package);
	}
	
	return no_packages;
}

Package* robot_get_destination_highest_priority_package(Robot* robot, const char* destination){
	Package *package = NULL;
	long max_prio = -1;
	// caut pachetul de prioritate maxima asociat unei destinatii
	Manifest *man = robot->manifest;
	while (man != NULL) {
		if (strcmp(man->package->destination, destination) == 0) {
			if (man->package->priority > max_prio) {
				max_prio = man->package->priority;
				package = man->package;
			}
		}
		man = man->next;
	}
	return package;
}

void destroy_robot(Robot* robot){
	Manifest *manifest = robot->manifest, *prev = NULL;
	while (manifest != NULL) {
		prev = manifest;
		manifest = manifest->next;
		destroy_package(prev->package);
		free(prev);
	}
	free(robot);
}

void robot_unload_packages(Truck* truck, Robot* robot){
	// adaug in manifestul tirului de la inceputul listei din robot, daca destinatia este buna
	Manifest *man = robot->manifest;
	while (man != NULL && (strcmp(man->package->destination, truck->destination) == 0) && (truck->size < truck->capacity)) {
		robot->manifest = robot->manifest->next;
		Manifest *new_node = create_manifest_node();
		new_node->next = truck->manifest;
		new_node->package = create_package(man->package->priority, man->package->destination);
		truck->manifest = new_node;
		destroy_manifest_node(man);
		robot->size--;
		truck->size++;
		man = robot->manifest;
	}
	man = robot->manifest;
	Manifest *prev = NULL, *tmp;
	// parcurg restul listei din robot, sar elementul daca nu are destinatia corespunzatoare
	while (man != NULL && (truck->size < truck->capacity)) {
		if (strcmp(man->package->destination, truck->destination) == 0) {
			tmp = man;
			prev->next = man->next;
			if (man->next != NULL)
				man->next->prev = prev;
			man = man->next;
			Manifest *new_node = create_manifest_node();
			new_node->next = truck->manifest;
			new_node->package = create_package(tmp->package->priority, tmp->package->destination);
			truck->manifest = new_node;
			destroy_manifest_node(tmp);
			robot->size--;
			truck->size++;
		} else {
			prev = man;
			man = man->next;
		}
	}
}



// Attach to specific truck
int robot_attach_find_truck(Robot* robot, Parkinglot *parkinglot){
	int found_truck = 0;
	long size = 0;
	Truck *arrived_iterator = parkinglot->arrived_trucks->next;
	Manifest* m_iterator = robot->manifest;


	while(m_iterator != NULL){
		while(arrived_iterator != parkinglot->arrived_trucks){
			size  = truck_destination_robots_unloading_size(arrived_iterator);
			if(strncmp(m_iterator->package->destination, arrived_iterator->destination, MAX_DESTINATION_NAME_LEN) == 0 &&
					size < (arrived_iterator->capacity-arrived_iterator->size)){
				found_truck = 1;
				break;
			}

			arrived_iterator = arrived_iterator->next;
		}

		if(found_truck)
			break;
		m_iterator = m_iterator->next;
	}

	if(found_truck == 0)
		return 0;


	Robot* prevr_iterator = NULL;
	Robot* r_iterator = arrived_iterator->unloading_robots;
	while(r_iterator != NULL){
		Package *pkg = robot_get_destination_highest_priority_package(r_iterator, m_iterator->package->destination);
		if(m_iterator->package->priority >= pkg->priority)
			break;
		prevr_iterator = r_iterator;
		r_iterator = r_iterator->next;
	}

	robot->next = r_iterator;
	if(prevr_iterator == NULL)
		arrived_iterator->unloading_robots = robot;
	else
		prevr_iterator->next = robot;

	return 1;
}

void robot_print_manifest_info(Robot* robot){
	Manifest *iterator = robot->manifest;
	while(iterator != NULL){
		printf(" R->P: %s %ld\n", iterator->package->destination, iterator->package->priority);
		iterator = iterator->next;
	}

	printf("\n");
}



Truck* create_truck(const char* destination, long capacity, long transit_time, long departure_time){
	Truck *truck = malloc(sizeof(Truck));
	truck->manifest = NULL;
	truck->unloading_robots = NULL;
	truck->capacity = capacity;
	truck->size = 0;
	truck->next = NULL;
	truck->in_transit_time = 0;
	truck->departure_time = departure_time;
	truck->transit_end_time = transit_time;
	if (destination == NULL) {
		truck->destination = NULL;
	} else {
		truck->destination = strdup(destination);
	}

	return truck;
}

int truck_is_full(Truck *truck){
	return truck->size == truck->capacity;
}

int truck_is_empty(Truck *truck){
	return truck->size == 0;
}

long truck_destination_robots_unloading_size(Truck* truck){
	Robot *robot = truck->unloading_robots;
	long size = 0;
	while (robot != NULL) {
		Manifest *man = robot->manifest;
		while (man != NULL) {
			if (strcmp(man->package->destination, truck->destination) == 0) {
				size += robot->size;
				break;
			}
			man = man->next;
		}
		robot = robot->next;
	}
	return size;
}


void truck_print_info(Truck* truck){
	printf("T: %s %ld %ld %ld %ld %ld\n", truck->destination, truck->size, truck->capacity,
			truck->in_transit_time, truck->transit_end_time, truck->departure_time);

	Manifest* m_iterator = truck->manifest;
	while(m_iterator != NULL){
		printf(" T->P: %s %ld\n", m_iterator->package->destination, m_iterator->package->priority);
		m_iterator = m_iterator->next;
	}

	Robot* r_iterator = truck->unloading_robots;
	while(r_iterator != NULL){
		printf(" T->R: %ld %ld\n", r_iterator->size, r_iterator->capacity);
		robot_print_manifest_info(r_iterator);
		r_iterator = r_iterator->next;
	}
}


void destroy_truck(Truck* truck){
	Manifest *man = truck->manifest, *prev_man = NULL;
	while (man != NULL) {
		prev_man = man;
		man = man->next;
		destroy_manifest_node(prev_man);
	}
	Robot *robot = truck->unloading_robots, *prev_rob = NULL;
	while (robot != NULL) {
		prev_rob = robot;
		robot = robot->next;
		destroy_robot(prev_rob);
	}
	free(truck->destination);
	free(truck);
}


Parkinglot* create_parkinglot(void){
	Parkinglot *parkinglot = malloc(sizeof(Parkinglot));
	// santinela
	parkinglot->arrived_trucks = create_truck(NULL, 0, 0, 0);
	parkinglot->arrived_trucks->next = parkinglot->arrived_trucks;
	parkinglot->departed_trucks = create_truck(NULL, 0, 0, 0);
	parkinglot->departed_trucks->next = parkinglot->departed_trucks;
	parkinglot->pending_robots = create_robot(0);
	parkinglot->pending_robots->next = parkinglot->pending_robots;
	parkinglot->standby_robots = create_robot(0);
	parkinglot->standby_robots->next = parkinglot->standby_robots;
	return parkinglot;
}

Parkinglot* open_parckinglot(const char* file_path){
	ssize_t read_size;
	char* line = NULL;
	size_t len = 0;
	char* token = NULL;
	Parkinglot *parkinglot = create_parkinglot();

	FILE *fp = fopen(file_path, "r");
	if(fp == NULL)
		goto file_open_exception;

	while((read_size = getline(&line, &len, fp)) != -1){
		token = strtok(line, ",\n ");
		// destination, capacitym transit_time, departure_time, arrived
		if(token[0] == 'T'){
			token = strtok(NULL, ",\n ");
			char *destination = token;

			token = strtok(NULL, ",\n ");
			long capacity = atol(token);

			token = strtok(NULL, ",\n ");
			long transit_time = atol(token);

			token = strtok(NULL, ",\n ");
			long departure_time = atol(token);

			token = strtok(NULL, ",\n ");
			int arrived = atoi(token);

			Truck *truck = create_truck(destination, capacity, transit_time, departure_time);

			if(arrived)
				truck_arrived(parkinglot, truck);
			else
				truck_departed(parkinglot, truck);

		}else if(token[0] == 'R'){
			token = strtok(NULL, ",\n ");
			long capacity = atol(token);

			Robot *robot = create_robot(capacity);
			parkinglot_add_robot(parkinglot, robot);

		}

		free(line);
		line = NULL;
		len = 0;
	}
	free(line);

	fclose(fp);
	return parkinglot;

	file_open_exception:
	return NULL;
}

void parkinglot_add_robot(Parkinglot* parkinglot, Robot *robot){
	// daca e gol, adaug in lista de standby, daca nu, adaug in lsita de pending
	if (robot_is_empty(robot)) {
		Robot *r = parkinglot->standby_robots->next, *prev = parkinglot->standby_robots;
		while (r != parkinglot->standby_robots && (r->capacity > robot->capacity)) {
			prev = r;
			r = r->next;
		}
		robot->next = r;
		prev->next = robot;
	} else {
		Robot *r = parkinglot->pending_robots->next, *prev = parkinglot->pending_robots;
		while (r != parkinglot->pending_robots && (r->size > robot->size)) {
			prev = r;
			r = r->next;
		}
		robot->next = r;
		prev->next = robot;
	}
}

void parkinglot_remove_robot(Parkinglot *parkinglot, Robot* robot){
	Robot *r, *prev;
	// il caut in lista corespnzatoare
	if (robot_is_empty(robot)) {
		r = parkinglot->standby_robots->next;
		prev = parkinglot->standby_robots;
		while (r != parkinglot->standby_robots && r != robot) {
			prev = r;
			r = r->next;
		}
	} else {
		r = parkinglot->pending_robots->next;
		prev = parkinglot->pending_robots;
		while (r != parkinglot->pending_robots && r != robot) {
			prev = r;
			r = r->next;
		}
	}
	// indiferent unde era, operatiile sunt la fel
	Robot *temp = robot->next;
	prev->next = temp;
}

int parckinglot_are_robots_peding(Parkinglot* parkinglot){
	return parkinglot->pending_robots->next != parkinglot->pending_robots;
}

int parkinglot_are_arrived_trucks_empty(Parkinglot* parkinglot){
	Truck *truck = parkinglot->arrived_trucks->next;
	while (truck != parkinglot->arrived_trucks) {
		if (!truck_is_empty(truck))
			return 0;
		truck = truck->next;
	}
	return 1;
}


int parkinglot_are_trucks_in_transit(Parkinglot* parkinglot){
	return parkinglot->departed_trucks != parkinglot->departed_trucks->next;
}


void destroy_parkinglot(Parkinglot* parkinglot){
	Truck *truck = parkinglot->arrived_trucks->next, *prev = parkinglot->arrived_trucks;
	while (truck != parkinglot->arrived_trucks) {
		prev = truck;
		truck = truck->next;
		destroy_truck(prev);
	}
	free(parkinglot->arrived_trucks);
	truck = parkinglot->departed_trucks->next, prev = parkinglot->departed_trucks;
	while (truck != parkinglot->departed_trucks) {
		prev = truck;
		truck = truck->next;
		destroy_truck(prev);
	}
	free(parkinglot->departed_trucks);
	Robot *robot = parkinglot->pending_robots->next, *prev_r = parkinglot->pending_robots;
	while (robot != parkinglot->pending_robots) {
		prev_r = robot;
		robot = robot->next;
		destroy_robot(prev_r);
	}
	free(parkinglot->pending_robots);
	robot = parkinglot->standby_robots->next, prev_r = parkinglot->standby_robots;
	while (robot != parkinglot->standby_robots) {
		prev_r = robot;
		robot = robot->next;
		destroy_robot(prev_r);
	}
	free(parkinglot->standby_robots);
	free(parkinglot);
}

void parkinglot_print_arrived_trucks(Parkinglot* parkinglot){
	Truck *iterator = parkinglot->arrived_trucks->next;
	while(iterator != parkinglot->arrived_trucks){

		truck_print_info(iterator);
		iterator = iterator->next;
	}

	printf("\n");

}

void parkinglot_print_departed_trucks(Parkinglot* parkinglot){
	Truck *iterator = parkinglot->departed_trucks->next;
	while(iterator != parkinglot->departed_trucks){
		truck_print_info(iterator);
		iterator = iterator->next;
	}
	printf("\n");

}

void parkinglot_print_pending_robots(Parkinglot* parkinglot){
	Robot *iterator = parkinglot->pending_robots->next;
	while(iterator != parkinglot->pending_robots){
		printf("R: %ld %ld\n", iterator->size, iterator->capacity);
		robot_print_manifest_info(iterator);
		iterator = iterator->next;
	}
	printf("\n");

}

void parkinglot_print_standby_robots(Parkinglot* parkinglot){
	Robot *iterator = parkinglot->standby_robots->next;
	while(iterator != parkinglot->standby_robots){
		printf("R: %ld %ld\n", iterator->size, iterator->capacity);
		robot_print_manifest_info(iterator);
		iterator = iterator->next;
	}
	printf("\n");

}


void truck_departed(Parkinglot *parkinglot, Truck* truck){
	// daca e in lista de arrived_trucks, il sterg
	Truck *truck_it = parkinglot->arrived_trucks->next, *prev = parkinglot->arrived_trucks;
	while (truck_it != parkinglot->arrived_trucks) {
		if (truck_it == truck) {
			prev->next = truck_it->next;
			break;
		}
		prev = truck_it;
		truck_it = truck_it->next;
	}
	// parcurg lista de departed trucks ca sa ii gasesc locul potrivit, in functie de departure time
	truck_it = parkinglot->departed_trucks->next, prev = parkinglot->departed_trucks;
	while (truck_it != parkinglot->departed_trucks && (truck_it->departure_time < truck->departure_time)) {
		prev = truck_it;
		truck_it = truck_it->next;
	}
	prev->next = truck;
	truck->next = truck_it;
}


void truck_arrived(Parkinglot *parkinglot, Truck* truck){
	if(parkinglot == NULL || truck == NULL) return;

	// il sterg din lista de departed trucks, daca il gasesc
	Truck *truck_it = parkinglot->departed_trucks->next, *prev = parkinglot->departed_trucks;
	while (truck_it != parkinglot->departed_trucks) {
		if (truck_it == truck) {
			prev->next = truck_it->next;
			break;
		}
		prev = truck_it;
		truck_it = truck_it->next;
	}
	// il adaug in pozitia buna in arrived_trucks
	truck_it = parkinglot->arrived_trucks->next, prev = parkinglot->arrived_trucks;
	while (truck_it != parkinglot->arrived_trucks && 
		  (strcmp(truck_it->destination, truck->destination) < 0 ||
		  (strcmp(truck_it->destination, truck->destination) == 0 && truck_it->departure_time < truck->departure_time))) {
		prev = truck_it;
	truck_it = truck_it->next;
	}
	prev->next = truck;
	truck->next = truck_it;
	// il "descarc" si actualizez faptul ca nu mai e in tranzit
	truck->size = 0;
	truck->in_transit_time = 0;
	// ii sterg pachetele (ca si cum le-ar fi descarcat)
	Manifest *man = truck->manifest, *prev_m = NULL;
	while (man != NULL) {
		prev_m = man;
		man = man->next;
		destroy_manifest_node(prev_m);
	}
	truck->manifest = NULL;
}

void truck_transfer_unloading_robots(Parkinglot* parkinglot, Truck* truck){
	Robot *robot = truck->unloading_robots;
	while (robot != NULL) {
		Robot *tmp = robot->next;
		parkinglot_add_robot(parkinglot, robot);
		robot = tmp;
	}
	truck->unloading_robots = NULL;
}


// Depends on parking_turck_departed
void truck_update_depatures(Parkinglot* parkinglot, long day_hour){
	// pentru tirurile care pleaca la ora data, le actualizez starea 
	// adica le bag in lista de departed_trucks
	Truck *truck = parkinglot->arrived_trucks->next;
	while (truck != parkinglot->arrived_trucks) {
		if (truck->departure_time == day_hour) {
			Truck *tmp = truck->next;
			truck_departed(parkinglot, truck);
			truck = tmp;
		} else {
			truck = truck->next;
		}
	}
}

// Depends on parking_turck_arrived
void truck_update_transit_times(Parkinglot* parkinglot){
	// pentru tirurile in tranzit, actualizez timpul
	Truck *truck = parkinglot->departed_trucks->next;
	while (truck != parkinglot->departed_trucks) {
		truck->in_transit_time++;
		if (truck->in_transit_time >= truck->transit_end_time) {
			Truck *tmp = truck->next;
			truck_arrived(parkinglot, truck);
			truck = tmp;
		} else {
			truck = truck->next;
		}
	}
}

void robot_swarm_collect(Wearhouse *wearhouse, Parkinglot *parkinglot){
	Robot *head_robot = parkinglot->standby_robots;
	Robot *current_robot = parkinglot->standby_robots->next;
	while(current_robot != parkinglot->standby_robots){

		// Load packages from wearhouse if possible
		if(!robot_load_packages(wearhouse, current_robot)){
			break;
		}

		// Remove robot from standby list
		Robot *aux = current_robot;
		head_robot->next = current_robot->next;
		current_robot = current_robot->next;

		// Add robot to the
		parkinglot_add_robot(parkinglot, aux);
	}
}


void robot_swarm_assign_to_trucks(Parkinglot *parkinglot){

	Robot *current_robot = parkinglot->pending_robots->next;

	while(current_robot != parkinglot->pending_robots){
		Robot* aux = current_robot;
		current_robot = current_robot->next;
		parkinglot_remove_robot(parkinglot, aux);
		int attach_succeded = robot_attach_find_truck(aux, parkinglot);
		if(!attach_succeded)
			parkinglot_add_robot(parkinglot, aux);
	}
}

void robot_swarm_deposit(Parkinglot* parkinglot){
	Truck *arrived_iterator = parkinglot->arrived_trucks->next;
	while(arrived_iterator != parkinglot->arrived_trucks){
		Robot *current_robot = arrived_iterator->unloading_robots;
		while(current_robot != NULL){
			robot_unload_packages(arrived_iterator, current_robot);
			Robot *aux = current_robot;
			current_robot = current_robot->next;
			arrived_iterator->unloading_robots = current_robot;
			parkinglot_add_robot(parkinglot, aux);
		}
		arrived_iterator = arrived_iterator->next;
	}
}

