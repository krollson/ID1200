#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
typedef struct page
{
    int page_id;
    int reference_bit;
    struct page *next;
    // @other auxiliary
} Node;
typedef struct nodeList
{
    Node *head;
    Node *last;
} NodeList;

void generate_reference_string();
void move_from_inactive_to_active_list(int pageid);
int pop_active();
int pop_inactive();
void pop_20_percent_of_active();
int active_ids[10000] = {0};
Node *page_success(int pageid);
int num_hits = 0;
int num_misses = 0;
int ARRAY_SIZE = 10000;
int ACTIVE_SIZE = 0;
int *array;
NodeList activeList;
NodeList inactiveList;
int n, m;
pthread_mutex_t lock;
int verbose;
volatile int running = 1;

void mutex_lock()
{
    pthread_mutex_lock(&lock);
}

void mutex_unlock()
{
    pthread_mutex_unlock(&lock);
}

int remove_from_list(int id, NodeList *list)
{
    Node *curr = list->head;
    Node *previous = NULL;
    int list_is_active = list == &activeList;
    if (curr == NULL)
    {
        return 0;
    }
    if (curr->page_id == id)
    {
        list->head = curr->next;
        free(curr);
        if (list_is_active)
        {
            ACTIVE_SIZE--;
        }
        return 1;
    }
    while (curr != NULL)
    {
        if (curr->page_id == id)
        {
            if (curr == list->last)
            {
                previous->next = NULL;
                list->last = previous;
                free(curr);
                if (list_is_active)
                {
                    ACTIVE_SIZE--;
                }
                return 1;
            }
            previous->next = curr->next;
            free(curr);
            if (list_is_active)
            {
                ACTIVE_SIZE--;
            }
            return 1;
        }
        previous = curr;
        curr = curr->next;
    }
    return 0;
}

void *player_thread_func()
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        mutex_lock();
        int id = array[i];
        Node *node = page_success(id);
        if (node == NULL)
        {
            num_misses++;
            move_from_inactive_to_active_list(id);
        }
        else
        {
            num_hits++;
            node->reference_bit = 1;
        }
        if (ACTIVE_SIZE >= 0.7 * n)
        {
            pop_20_percent_of_active();
        }
        mutex_unlock();
        usleep(10);
    }
    pthread_exit(0);
}

void *checker_thread_func()
{
    while (running)
    {
        usleep(m);
        mutex_lock();
        Node *node = activeList.head;
        while (node != NULL)
        {
            active_ids[node->page_id] += node->reference_bit;
            node->reference_bit = 0;
            node = node->next;
        }
        mutex_unlock();
    }
    pthread_exit(0);
}

Node *generate_inactive_list()
{
    mutex_lock();
    Node *next = (Node *)malloc(sizeof(Node));
    Node *first = next;
    for (int i = 0; i < n - 1; i++)
    {
        next->page_id = i;
        next->reference_bit = 0;
        next->next = (Node *)malloc(sizeof(Node));
        next = next->next;
    }
    next->page_id = n - 1;
    next->reference_bit = 0;
    inactiveList.head = first;
    inactiveList.last = next;
    mutex_unlock();
    return first;
}
void move_from_inactive_to_active_list(int pageid)
{
    Node *curr = inactiveList.head;
    Node *previous = NULL;
    while (curr != NULL)
    {
        if (curr->page_id == pageid)
        {
            if (previous == NULL)
            {
                inactiveList.head = curr->next;
            }
            else
            {
                previous->next = curr->next;
            }
            if (curr == inactiveList.last)
            {
                inactiveList.last = previous;
            }
            curr->next = NULL;
            curr->reference_bit = 1;
            if (activeList.head == NULL)
            {
                activeList.head = curr;
                activeList.last = curr;
            }
            else
            {
                activeList.last->next = curr;
                activeList.last = curr;
            }
            ACTIVE_SIZE++;
            return;
        }
        previous = curr;
        curr = curr->next;
    }
}

int pop_active()
{
    Node *curr = activeList.head;
    int id = curr->page_id;
    activeList.head = curr->next;
    free(curr);
    return id;
}

int pop_inactive()
{
    Node *curr = inactiveList.head;
    int id = curr->page_id;
    inactiveList.head = curr->next;
    free(curr);
    return id;
}

void append_to_inactive(int id)
{
    if (inactiveList.head == NULL)
    {
        // borde inte komma hit
        inactiveList.head = (Node *)malloc(sizeof(Node));
        inactiveList.head->page_id = id;
        inactiveList.head->next = NULL;
        inactiveList.head->reference_bit = 0;
        inactiveList.last = inactiveList.head;
        return;
    }
    remove_from_list(id, &inactiveList);
    Node *node = (Node *)malloc(sizeof(Node));
    node->page_id = id;
    node->next = NULL;
    node->reference_bit = 0;
    inactiveList.last->next = node;
    inactiveList.last = node;
}

void pop_20_percent_of_active()
{
    Node *curr;
    for (int i = 0; i < 0.2 * ACTIVE_SIZE; i++)
    {
        curr = activeList.head;
        activeList.head = curr->next;
        if (activeList.head == NULL)
        {
            activeList.last = NULL;
        }
        ACTIVE_SIZE--;
        curr->next = NULL;
        curr->reference_bit = 0;
        if (inactiveList.head == NULL)
        {
            inactiveList.head = curr;
            inactiveList.last = curr;
        }
        else
        {
            inactiveList.last->next = curr;
            inactiveList.last = curr;
        }
    }
}

Node *page_success(int pageid)
{
    Node *curr = activeList.head;
    while (curr != NULL)
    {
        if (curr->page_id == pageid)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void setup()
{
    activeList.head = NULL;
    activeList.last = NULL;
    generate_inactive_list();
    generate_reference_string();
    pthread_mutex_init(&lock, NULL);
}

void print_reference_string()
{
    printf("printing reference string\n");
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        printf("%d, ", array[i]);
    }
    printf("reference string done\n");
}

void generate_reference_string()
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        array[i] = rand() % n;
    }
}

int main(int argc, char *argv[])
{
    verbose = 0;
    n = atoi(argv[1]);
    m = atoi(argv[2]);
    generate_inactive_list();
    // @create a random reference string of N
    array = malloc(ARRAY_SIZE * sizeof(int));
    setup();
    // print_reference_string();
    /* Create two workers */
    pthread_t player;
    pthread_t checker;
    pthread_create(&player, NULL, player_thread_func, NULL);
    pthread_create(&checker, NULL, checker_thread_func, NULL);
    pthread_join(player, NULL);
    running = 0;
    pthread_join(checker, NULL);
    printf("Hits: %d\n", num_hits);
    printf("Misses: %d\n", num_misses);
    printf("Page_id, Total_Referenced\n");
    int total_accesses = 0;
    for (int i = 0; i < n; i++)
    {
        printf("%d, %d\n", i, active_ids[i]);
        total_accesses += active_ids[i];
    }
    printf("Total accesses: %d\n", total_accesses);
    printf("Pages in active list: \n");
    Node *curr = activeList.head;
    while (curr != NULL)
    {
        printf("%d, ", curr->page_id);
        pop_active();
        curr = activeList.head;
    }
    printf("\nPages in inactive list: \n");
    curr = inactiveList.head;
    while (curr != NULL)
    {
        printf("%d, ", curr->page_id);
        pop_inactive();
        curr = inactiveList.head;
    }
    free(array);
}