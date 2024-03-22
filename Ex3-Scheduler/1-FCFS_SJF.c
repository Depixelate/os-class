#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <math.h>

typedef struct PCB {
	int proc_num;
	char pid[128];
	int priority;
	double burst_left;
} PCB;

PCB *init_pcb(int proc_num, const char *pid, int priority, double burst_time) {
	PCB *pcb = (PCB *)malloc(sizeof(PCB));
	pcb->proc_num = proc_num;
	strcpy(pcb->pid, pid);
	pcb->priority = priority;
	pcb->burst_left = burst_time;
	return pcb;
}

PCB *copy_pcb(PCB *other) {
	return init_pcb(other->proc_num, other->pid, other->priority, other->burst_left);
}

#define LEFT(X) (2 * (X) + 1)
#define RIGHT(X) (2 * (X) + 2)
#define PAR(X) (((X)-1)/2)

typedef struct PEntry {
	double priority;
	int add_num;
	PCB *pcb;
} PEntry;

PEntry *init_pentry(double priority, PCB *pcb) {
	PEntry *pentry = (PEntry *)malloc(sizeof(PEntry));
	pentry->priority = priority;
	pentry->pcb = pcb;
	pentry->add_num = 0;
	return pentry;
}

int pentry_comp(PEntry *e1, PEntry *e2) {
	if(e1->priority < e2->priority) return -1;
	if(e1->priority > e2->priority) return 1;
	if(e1->add_num < e2->add_num) return -1;
	if(e1->add_num > e2->add_num) return 1;
	return 0;
}

typedef struct PQueue {
	PEntry **entries;
	int size;
	int max_size;
	int add_num;
} PQueue;

void free_pqueue(PQueue *pqueue) {
	free(pqueue->entries);
	free(pqueue);
}

PQueue *init_pqueue(int max_size) {
	PQueue *queue = (PQueue *)malloc(sizeof(PQueue));
	queue->entries = (PEntry **)calloc(max_size, sizeof(PEntry *));
	queue->size = 0;
	queue->max_size = max_size;
	queue->add_num = 0;
	return queue;
}

void pqueue_swap(PQueue *queue, int i1, int i2) {
	PEntry *temp = queue->entries[i1];
	queue->entries[i1] = queue->entries[i2];
	queue->entries[i2] = temp;
}

void pqueue_bubble_up(PQueue *queue, int index) {
	if(pentry_comp(queue->entries[index], queue->entries[PAR(index)]) == -1) {
		pqueue_swap(queue, index, PAR(index));
		pqueue_bubble_up(queue, PAR(index));
	}
}

void pqueue_heapify(PQueue *queue, int index) {
	PEntry *cur = queue->entries[index];
	if(LEFT(index) >= queue->size) return;
	PEntry *left = queue->entries[LEFT(index)];
	int swap_ind = index;
	
	if(pentry_comp(left, cur) == -1) {
		swap_ind = LEFT(index);
	}
	
	if(RIGHT(index) < queue->size) {
		PEntry *right = queue->entries[RIGHT(index)];
		if(pentry_comp(right, cur) == -1 && pentry_comp(right, left) == -1) {
			swap_ind = RIGHT(index);
		}
	}
	
	if(swap_ind != index) {
		pqueue_swap(queue, index, swap_ind);
		pqueue_heapify(queue, swap_ind);
	}
}


void pqueue_add(PQueue *queue, PEntry *entry, bool keep_add_num) {
	if(!keep_add_num) {
		entry->add_num = queue->add_num++;
	}
	queue->entries[queue->size++] = entry;
	pqueue_bubble_up(queue, queue->size - 1);
}

PEntry *pqueue_extract_min(PQueue *queue) {
	if(queue->size == 0) return NULL;
	PEntry *ret = queue->entries[0];
	pqueue_swap(queue, 0, queue->size - 1);
	queue->size--;
	pqueue_heapify(queue, 0);
	return ret;
}

PEntry *pqueue_remove(PQueue *queue, int proc_num) {
	PEntry *ret = NULL;
	for(int i = 0; i < queue->size; i++) {
		if(queue->entries[i]->pcb->proc_num == proc_num) {
			ret = queue->entries[i];
			pqueue_swap(queue, i, queue->size - 1);
			queue->size-=1;
			pqueue_heapify(queue, i);
			pqueue_bubble_up(queue, i);
		}
	}
	
	return ret;
}

typedef enum EventType {
	Arrived,
	QuantaOver,
	Done
} EventType;

typedef struct Event {
	int add_num;
	double time;
	PCB *pcb;
	EventType type;
} Event;

Event *init_event(double time, PCB *pcb, EventType type) {
	Event *event = (Event *)malloc(sizeof(Event));
	event->time = time;
	event->pcb = pcb;
	event->type = type;
	event->add_num = 0;
	return event;
}

int event_comp(Event *e1, Event *e2) {
	if(e1->time < e2->time) return -1;
	if(e1->time > e2->time) return 1;
	if(e1->add_num < e2->add_num) return -1;
	if(e1->add_num > e2->add_num) return 1;
	return 0;
}

typedef struct EventQueue {
	Event **events;
	int size;
	int max_size;
	int add_num;
} EventQueue;

EventQueue *init_event_queue(int max_size) {
	EventQueue *queue = (EventQueue *)malloc(sizeof(EventQueue));
	queue->events = (Event **)calloc(max_size, sizeof(Event *));
	queue->size = 0;
	queue->max_size = max_size;
	queue->add_num = 0;
	return queue;
}

void event_queue_swap(EventQueue *queue, int i1, int i2) {
	Event *temp = queue->events[i1];
	queue->events[i1] = queue->events[i2];
	queue->events[i2] = temp;
}

void event_queue_bubble_up(EventQueue *queue, int index) {
	if(event_comp(queue->events[index], queue->events[PAR(index)]) == -1) {
		event_queue_swap(queue, index, PAR(index));
		event_queue_bubble_up(queue, PAR(index));
	}
}

void event_queue_heapify(EventQueue *queue, int index) {
	Event *cur = queue->events[index];
	if(LEFT(index) >= queue->size) return;
	Event *left = queue->events[LEFT(index)];
	int swap_ind = index;
	
	if(event_comp(left, cur) == -1) {
		swap_ind = LEFT(index);
	}
	
	if(RIGHT(index) < queue->size) {
		Event *right = queue->events[RIGHT(index)];
		if(event_comp(right, cur) == -1 && event_comp(right, left) == -1) {
			swap_ind = RIGHT(index);
		}
	}
	
	if(swap_ind != index) {
		event_queue_swap(queue, index, swap_ind);
		event_queue_heapify(queue, swap_ind);
	}
}


void event_queue_add(EventQueue *queue, Event *event) {
	event->add_num = queue->add_num++;
	queue->events[queue->size++] = event;
	event_queue_bubble_up(queue, queue->size - 1);
}

Event *event_queue_peek(EventQueue *queue) {
	if(queue->size == 0) return NULL;
	Event *ret = queue->events[0];
	return ret;
}

Event *event_queue_extract_min(EventQueue *queue) {
	Event *ret = event_queue_peek(queue);
	if(ret == NULL) return NULL;
	event_queue_swap(queue, 0, queue->size - 1);
	queue->size--;
	event_queue_heapify(queue, 0);
	return ret;
}

void event_queue_remove(EventQueue *queue, int proc_num) {
	Event *ret = NULL;
	for(int i = 0; i < queue->size; i++) {
		if((queue->events[i]->type == Done || queue->events[i]->type == QuantaOver) && queue->events[i]->pcb->proc_num == proc_num) {
			ret = queue->events[i];
			event_queue_swap(queue, i, queue->size - 1);
			queue->size-=1;
			event_queue_heapify(queue, i);
			event_queue_bubble_up(queue, i);
			break;
		}
	}
	
	free(ret);
}


typedef struct GanttEntry {
	char pid[128];
	double start;
} GanttEntry;

typedef struct ProcInfo {
	char pid[128];
	double arrival_time;
	double burst_time;
	double finish_time;
} ProcInfo;

PQueue *pqueue = NULL;
EventQueue *event_queue = NULL;
PCB *cur_pcb = NULL;
ProcInfo *proc_infos = NULL;


typedef double (*PriorityFunc)(PCB *pcb);

// Used for fcfs and Round Robin
double no_priority(PCB *pcb) {
	return 0;
}

double sjfs_priority(PCB *pcb) {
	return pcb->burst_left;
}

double priority_priority(PCB *pcb) {
	return pcb->priority;
}

void update_time(double timedelta) {
	if(cur_pcb != NULL) cur_pcb->burst_left -= timedelta;
}

void handle_event(PriorityFunc pf, Event *event) {
	switch(event->type) {
		case Arrived:
			pqueue_add(pqueue, init_pentry(pf(event->pcb), event->pcb), false);
			break;
		case Done:
			proc_infos[cur_pcb->proc_num].finish_time = event->time;
			free(cur_pcb);
			cur_pcb = NULL; //Can't set old_pcb null, as check at end if cur_pcb == old_pcb
			break;
		case QuantaOver: 
			pqueue_add(pqueue, init_pentry(pf(cur_pcb), cur_pcb), false); // Different than premptive, actually puts at back of queue, not front.
			cur_pcb = NULL;
			break;
		default:
			printf("Error! Not Supposed to be here at line %d\n", __LINE__);
			break;			
	}
}

//Returns if current process changes
void try_switch_proc(PriorityFunc pf, double cur_time, bool preemptive, double quanta) {	
	bool interrupted = false;
	PCB *before_interrupt = cur_pcb;

	if(preemptive && cur_pcb != NULL) { 
		pqueue_add(pqueue, init_pentry(pf(cur_pcb), cur_pcb), true);
		cur_pcb = NULL; // NULL means must get from ready queue
		interrupted = true;
	}	
		
	if(cur_pcb != NULL) {
		return; 
	}

	PEntry *entry = pqueue_extract_min(pqueue);
	if(entry == NULL) {
		return;
	}

	cur_pcb = entry->pcb;
	free(entry);

	//Only return if not a process over event like QuantaOver or Done
	if(interrupted) {
		if(cur_pcb == before_interrupt) return;
		if(before_interrupt != NULL) event_queue_remove(event_queue, before_interrupt->proc_num);
	}

	if(cur_pcb->burst_left <= quanta) {
		event_queue_add(event_queue, init_event(cur_time + cur_pcb->burst_left, cur_pcb, Done));
	} else {
		event_queue_add(event_queue, init_event(cur_time + quanta, cur_pcb, QuantaOver));
	} 
}

bool get_preemptive_pref() {
	char buf[4096];
	printf("Premptive?(Y/N): ");
	scanf("%s", buf);
	return tolower(buf[0]) == 'y';
}

void get_quanta_pref(double *quanta) {
	printf("Enter Quanta amount: ");
	scanf("%lf", quanta);
}

void get_priority_prefs(PCB **procs, int num_procs) {
	for(int i = 0; i < num_procs; i++) {
		printf("%d. Enter the priority for Process %s: ", i + 1, procs[i]->pid);
		scanf("%d", &(procs[i]->priority));
	}
}

void print_gantt_entries(GanttEntry *entries, int num_entries) {
	printf("\nGantt Chart\n\n");
	printf("PID\tTime Span\n\n");
	for(int i = 0; i < num_entries - 1; i++) {
		if(entries[i].start == entries[i+1].start) continue;
		printf("%s\t%0.2lf - %0.2lf\n", entries[i].pid, entries[i].start, entries[i+1].start);
	}

	printf("\n");	
}

void print_proc_info(int num_procs) {
	printf("PID\tArriv\tBurst\tWait\tTurn\n");
	printf("\n");
	double tot_arrival = 0, tot_burst = 0, tot_waiting = 0, tot_turnaround = 0;
	for(int i = 0; i < num_procs; i++) {
		const char *pid = proc_infos[i].pid;
		double arrival = proc_infos[i].arrival_time;
		double burst = proc_infos[i].burst_time;
		double turnaround = proc_infos[i].finish_time - arrival;
		double waiting = turnaround - burst;

		tot_arrival += arrival;
		tot_burst += burst;
		tot_waiting += waiting;
		tot_turnaround += turnaround;
		
		printf("%s\t%0.2lf\t%0.2lf\t%0.2lf\t%0.2lf\n", pid, arrival, burst, waiting, turnaround);
	}
	printf("Avg\t%0.2lf\t%0.2lf\t%0.2lf\t%0.2lf\n", tot_arrival/num_procs, tot_burst/num_procs, tot_waiting/num_procs, tot_turnaround/num_procs);
	printf("\n");
}


int main() {
	printf("Enter The number of processes: ");
	int num_procs;
	scanf("%d", &num_procs);
	event_queue = init_event_queue(num_procs);
	pqueue = init_pqueue(num_procs);
	proc_infos = (ProcInfo *)calloc(num_procs, sizeof(ProcInfo));
	GanttEntry gantt_entries[4096] = {0};
	strcpy(gantt_entries[0].pid,"Idle");
	gantt_entries[0].start = 0;
	int num_gantt_entries = 1;

	PCB **procs = (PCB **)calloc(num_procs, sizeof(PCB *));
	bool preemptive = false;
	for(int i = 0; i < num_procs; i++) {
		printf("\n===========Process %d=============\n", i + 1);
		printf("Enter the process pid(No spaces!): ");
		char pid[128];
		scanf("%s", pid);

		printf("Enter the process arrival time: ");
		double arrival;		
		scanf("%lf", &arrival);
		
		printf("Enter the process burst time: ");
		double burst_time;
		scanf("%lf", &burst_time);
		procs[i] = init_pcb(i, pid, 0, burst_time);
		strcpy(proc_infos[i].pid,procs[i]->pid);
		proc_infos[i].arrival_time = arrival;
		proc_infos[i].burst_time = burst_time;
	}
	
	bool cont = true;
	
	while(cont) {
		double quanta = INFINITY;
		bool preemptive = false;
		PriorityFunc pf = NULL;
		num_gantt_entries = 1;

		for(int i = 0; i < num_procs; i++) {
			procs[i]->priority = 0;
		}

		printf("Scheduling Algorithms: \n");
		printf("1. FCFS\n");
		printf("2. SJF\n");
		printf("3. Round Robin\n");
		printf("4. Priority\n");
		printf("Enter your option: ");
		int op;
		scanf("%d", &op);
		switch(op) {
			case 1:
				pf = no_priority;
				break;
			case 2:
				pf = sjfs_priority;
				preemptive = get_preemptive_pref();
				break;
			case 3:
				pf = no_priority;
				get_quanta_pref(&quanta);
				break;
			case 4:
				pf = priority_priority;
				get_priority_prefs(procs, num_procs);
				preemptive = get_preemptive_pref();
				break;
			default:
				printf("Invalid Option!\n");
				continue;
				break;
		}

		for(int i = 0; i < num_procs; i++) {
			event_queue_add(event_queue, init_event(proc_infos[i].arrival_time, copy_pcb(procs[i]), Arrived));
		}

		double prev_event_time = 0;

		while(true) {
			Event *event = event_queue_peek(event_queue);
			if(event == NULL) break;

			PCB *old_pcb = cur_pcb;
			double cur_time = event->time;

			update_time(cur_time - prev_event_time);

			do {
				event_queue_extract_min(event_queue);
				handle_event(pf, event);
				free(event);
				event = event_queue_peek(event_queue);
			} while(event != NULL && event->time == cur_time);

			try_switch_proc(pf, cur_time, preemptive, quanta);

			if(old_pcb != cur_pcb) {
				strcpy(gantt_entries[num_gantt_entries].pid,cur_pcb == NULL ? "Idle" : cur_pcb->pid);
				gantt_entries[num_gantt_entries].start = cur_time;
				num_gantt_entries++;
			}

			prev_event_time = cur_time;
		}
		print_gantt_entries(gantt_entries, num_gantt_entries);
		printf("Press Enter to continue...\n");
		char buf[4096];
		scanf("%c%c", buf, buf);
		print_proc_info(num_procs);
		printf("Again?(Y/N): ");
		scanf("%s", &buf);
		if (tolower(buf[0]) == 'n') {
			cont = false;
		}
	}
}
