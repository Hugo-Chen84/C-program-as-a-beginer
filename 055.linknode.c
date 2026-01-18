#include<stdio.h>
#include<stdlib.h>

typedef struct linknode {
	int data;
	struct linknode* next;
}node;

node* creat_linknode(int n){
	node* head = (node*)malloc(sizeof(node));
	if (head == NULL) {
		printf("malloc failed\n");
		exit(EXIT_FAILURE);
	}
	head->data = n;
	head->next = NULL;
	return head;
}

void add_linknode(node* head, int n) {
	node* temp = head;
	while (temp->next != NULL) {
		temp = temp->next;
	}
	node* newNode = (node*)malloc(sizeof(node));
	if (newNode == NULL) {
		printf("malloc failed\n");
		exit(EXIT_FAILURE);
	}
	newNode->data = n;
	newNode->next = NULL;
	temp->next = newNode;
}//add at the tail

void traverseList(node* head) {
	node* p = head;
	printf("The data is£º");
	while (p != NULL) {
		printf("%d ", p->data);
		p = p->next;
	}
	printf("\n");
}

void free_list(node* head) {
	node* p = head;
	while (p != NULL) {
		node* t = p->next;
		free(p);
		p = t;
	}
}

int main() {
	node* head = creat_linknode(1);
	add_linknode(head, 3);
	traverseList(head);
	free_list(head);
	return 0;

}
