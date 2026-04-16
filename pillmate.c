/*
 * PillMate Backend - Medicine Reminder & Pill Tracker
 * Data Structures: Linked List, Queue, Priority Queue, Hash Table
 * Language: C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LEN    64
#define MAX_TYPE_LEN    32
#define MAX_DOSAGE_LEN  32
#define MAX_FREQ_LEN    32
#define MAX_TIMES       5
#define MAX_TIME_LEN    8
#define MAX_COLOR_LEN   16
#define MAX_PRIORITY_LEN 16
#define HASH_TABLE_SIZE 64
#define JSON_FILE       "../medicines.json"

/* ─── Medicine Node (Linked List) ──────────────────────────────────────── */
typedef struct Medicine {
    int    id;
    char   name[MAX_NAME_LEN];
    char   type[MAX_TYPE_LEN];
    char   dosage[MAX_DOSAGE_LEN];
    char   frequency[MAX_FREQ_LEN];
    char   times[MAX_TIMES][MAX_TIME_LEN];
    int    timeCount;
    int    pillsLeft;
    int    refillThreshold;
    char   priority[MAX_PRIORITY_LEN];
    char   color[MAX_COLOR_LEN];
    int    takenToday[MAX_TIMES];
    char   addedDate[16];
    struct Medicine *next;
} Medicine;

/* ─── Queue Node (Reminder Scheduling) ─────────────────────────────────── */
typedef struct QueueNode {
    int    medicineId;
    char   time[MAX_TIME_LEN];
    char   medicineName[MAX_NAME_LEN];
    char   dosage[MAX_DOSAGE_LEN];
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int        size;
} ReminderQueue;

/* ─── Priority Queue Node (Urgent Alerts) ──────────────────────────────── */
typedef struct PQNode {
    int    medicineId;
    char   name[MAX_NAME_LEN];
    int    pillsLeft;
    int    refillThreshold;
    int    priority;       /* 3=Critical, 2=High, 1=Medium, 0=Low */
    struct PQNode *next;
} PQNode;

typedef struct {
    PQNode *head;
    int     size;
} AlertPriorityQueue;

/* ─── Hash Table (Fast Search) ──────────────────────────────────────────── */
typedef struct HashEntry {
    char          key[MAX_NAME_LEN];
    Medicine     *medicine;
    struct HashEntry *next;
} HashEntry;

typedef struct {
    HashEntry *buckets[HASH_TABLE_SIZE];
} HashTable;

/* ─── Global State ──────────────────────────────────────────────────────── */
Medicine          *medicineList    = NULL;
int                nextId          = 6;
ReminderQueue      reminderQueue   = {NULL, NULL, 0};
AlertPriorityQueue alertPQ         = {NULL, 0};
HashTable          searchTable;

/* ═══════════════════════ HASH TABLE ═══════════════════════════════════════ */

unsigned int hashFn(const char *key) {
    unsigned int h = 5381;
    while (*key)
        h = ((h << 5) + h) ^ (unsigned char)*key++;
    return h % HASH_TABLE_SIZE;
}

void htInsert(HashTable *ht, const char *key, Medicine *m) {
    unsigned int idx = hashFn(key);
    HashEntry *e = malloc(sizeof(HashEntry));
    strncpy(e->key, key, MAX_NAME_LEN - 1);
    e->key[MAX_NAME_LEN - 1] = '\0';
    e->medicine = m;
    e->next = ht->buckets[idx];
    ht->buckets[idx] = e;
}

Medicine *htSearch(HashTable *ht, const char *key) {
    unsigned int idx = hashFn(key);
    HashEntry *e = ht->buckets[idx];
    while (e) {
        if (strcasecmp(e->key, key) == 0) return e->medicine;
        e = e->next;
    }
    return NULL;
}

void htDelete(HashTable *ht, const char *key) {
    unsigned int idx = hashFn(key);
    HashEntry *e = ht->buckets[idx], *prev = NULL;
    while (e) {
        if (strcasecmp(e->key, key) == 0) {
            if (prev) prev->next = e->next;
            else       ht->buckets[idx] = e->next;
            free(e);
            return;
        }
        prev = e;
        e    = e->next;
    }
}

void htInit(HashTable *ht) {
    memset(ht->buckets, 0, sizeof(ht->buckets));
}

/* ═══════════════════════ REMINDER QUEUE ════════════════════════════════════ */

void enqueueReminder(ReminderQueue *q, int id, const char *time,
                     const char *name, const char *dosage) {
    QueueNode *node = malloc(sizeof(QueueNode));
    node->medicineId = id;
    strncpy(node->time,          time,   MAX_TIME_LEN  - 1);
    strncpy(node->medicineName,  name,   MAX_NAME_LEN  - 1);
    strncpy(node->dosage,        dosage, MAX_DOSAGE_LEN - 1);
    node->time[MAX_TIME_LEN - 1]         = '\0';
    node->medicineName[MAX_NAME_LEN - 1] = '\0';
    node->dosage[MAX_DOSAGE_LEN - 1]     = '\0';
    node->next = NULL;
    if (!q->rear) { q->front = q->rear = node; }
    else          { q->rear->next = node; q->rear = node; }
    q->size++;
}

QueueNode *dequeueReminder(ReminderQueue *q) {
    if (!q->front) return NULL;
    QueueNode *node = q->front;
    q->front = q->front->next;
    if (!q->front) q->rear = NULL;
    q->size--;
    return node;
}

/* ═══════════════════════ PRIORITY QUEUE ════════════════════════════════════ */

int priorityLevel(const char *p) {
    if (strcasecmp(p, "Critical") == 0) return 3;
    if (strcasecmp(p, "High")     == 0) return 2;
    if (strcasecmp(p, "Medium")   == 0) return 1;
    return 0;
}

void pqInsert(AlertPriorityQueue *pq, Medicine *m) {
    int lvl = priorityLevel(m->priority);
    PQNode *node = malloc(sizeof(PQNode));
    node->medicineId       = m->id;
    node->pillsLeft        = m->pillsLeft;
    node->refillThreshold  = m->refillThreshold;
    node->priority         = lvl;
    strncpy(node->name, m->name, MAX_NAME_LEN - 1);
    node->name[MAX_NAME_LEN - 1] = '\0';

    /* Insert in descending priority order */
    PQNode **cur = &pq->head;
    while (*cur && (*cur)->priority >= lvl) cur = &(*cur)->next;
    node->next = *cur;
    *cur       = node;
    pq->size++;
}

void pqClear(AlertPriorityQueue *pq) {
    PQNode *cur = pq->head;
    while (cur) { PQNode *t = cur->next; free(cur); cur = t; }
    pq->head = NULL;
    pq->size = 0;
}

/* ═══════════════════════ JSON HELPERS ══════════════════════════════════════ */

/* Very minimal JSON escape (only for values we write) */
void jsonStr(FILE *f, const char *s) {
    fputc('"', f);
    while (*s) {
        if      (*s == '"')  fputs("\\\"", f);
        else if (*s == '\\') fputs("\\\\", f);
        else if (*s == '\n') fputs("\\n",  f);
        else                 fputc(*s, f);
        s++;
    }
    fputc('"', f);
}

/* ─── Parse helpers ─────────────────────────────────────────────────────── */

/* Extract first string value for a key; buf must be at least buflen bytes */
int parseStrField(const char *json, const char *key, char *buf, int buflen) {
    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) return 0;
    p = strchr(p + strlen(search), '"');   /* skip : and whitespace to " */
    if (!p) return 0;
    p++;  /* skip opening quote */
    int i = 0;
    while (*p && *p != '"' && i < buflen - 1) {
        if (*p == '\\') { p++; if (*p) buf[i++] = *p; }
        else              buf[i++] = *p;
        p++;
    }
    buf[i] = '\0';
    return 1;
}

int parseIntField(const char *json, const char *key) {
    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) return 0;
    p = strchr(p + strlen(search), ':');
    if (!p) return 0;
    while (*p == ':' || *p == ' ') p++;
    return atoi(p);
}

/* ═══════════════════════ CORE FUNCTIONS ════════════════════════════════════ */

/* addMedicine – appends to linked list, inserts into hash table */
Medicine *addMedicine(const char *name, const char *type, const char *dosage,
                      const char *frequency, char times[][MAX_TIME_LEN],
                      int timeCount, int pillsLeft, int refillThreshold,
                      const char *priority, const char *color) {
    Medicine *m = calloc(1, sizeof(Medicine));
    m->id               = nextId++;
    strncpy(m->name,       name,       MAX_NAME_LEN  - 1);
    strncpy(m->type,       type,       MAX_TYPE_LEN  - 1);
    strncpy(m->dosage,     dosage,     MAX_DOSAGE_LEN - 1);
    strncpy(m->frequency,  frequency,  MAX_FREQ_LEN  - 1);
    strncpy(m->priority,   priority,   MAX_PRIORITY_LEN - 1);
    strncpy(m->color,      color,      MAX_COLOR_LEN - 1);
    m->pillsLeft        = pillsLeft;
    m->refillThreshold  = refillThreshold;
    m->timeCount        = timeCount;
    time_t now          = time(NULL);
    struct tm *t        = localtime(&now);
    sprintf(m->addedDate, "%04d-%02d-%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

    for (int i = 0; i < timeCount && i < MAX_TIMES; i++) {
        strncpy(m->times[i], times[i], MAX_TIME_LEN - 1);
        m->takenToday[i] = 0;
    }

    /* Append to linked list */
    if (!medicineList) {
        medicineList = m;
    } else {
        Medicine *cur = medicineList;
        while (cur->next) cur = cur->next;
        cur->next = m;
    }

    /* Hash table insert */
    htInsert(&searchTable, name, m);

    /* Enqueue reminders */
    for (int i = 0; i < timeCount; i++)
        enqueueReminder(&reminderQueue, m->id, m->times[i], m->name, m->dosage);

    printf("[ADD] Added medicine: %s (ID %d)\n", name, m->id);
    return m;
}

/* deleteMedicine – removes from linked list and hash table */
int deleteMedicine(int id) {
    Medicine *cur = medicineList, *prev = NULL;
    while (cur) {
        if (cur->id == id) {
            if (prev) prev->next = cur->next;
            else       medicineList = cur->next;
            htDelete(&searchTable, cur->name);
            printf("[DEL] Deleted medicine: %s (ID %d)\n", cur->name, id);
            free(cur);
            return 1;
        }
        prev = cur; cur = cur->next;
    }
    printf("[DEL] Medicine ID %d not found\n", id);
    return 0;
}

/* displayMedicines – walks linked list */
void displayMedicines(void) {
    printf("\n╔══════════════════════ MEDICINE LIST ══════════════════════╗\n");
    Medicine *cur = medicineList;
    if (!cur) { printf("  (no medicines)\n"); }
    while (cur) {
        printf("  [%d] %s | %s | %s | Pills: %d | Priority: %s\n",
               cur->id, cur->name, cur->type, cur->dosage,
               cur->pillsLeft, cur->priority);
        cur = cur->next;
    }
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
}

/* searchMedicine – O(1) via hash table */
Medicine *searchMedicine(const char *name) {
    Medicine *m = htSearch(&searchTable, name);
    if (m) printf("[SEARCH] Found: %s (ID %d)\n", m->name, m->id);
    else   printf("[SEARCH] '%s' not found\n", name);
    return m;
}

/* markAsTaken – decrements pillsLeft */
int markAsTaken(int id, int timeIndex) {
    Medicine *cur = medicineList;
    while (cur) {
        if (cur->id == id) {
            if (timeIndex < 0 || timeIndex >= cur->timeCount) {
                printf("[TAKEN] Invalid time index\n"); return 0;
            }
            if (cur->takenToday[timeIndex]) {
                printf("[TAKEN] Already marked taken\n"); return 0;
            }
            cur->takenToday[timeIndex] = 1;
            cur->pillsLeft--;
            printf("[TAKEN] Marked %s at %s as taken. Pills left: %d\n",
                   cur->name, cur->times[timeIndex], cur->pillsLeft);
            return 1;
        }
        cur = cur->next;
    }
    printf("[TAKEN] Medicine ID %d not found\n", id);
    return 0;
}

/* checkRefillAlerts – builds priority queue for low-stock medicines */
void checkRefillAlerts(void) {
    pqClear(&alertPQ);
    Medicine *cur = medicineList;
    while (cur) {
        if (cur->pillsLeft <= cur->refillThreshold)
            pqInsert(&alertPQ, cur);
        cur = cur->next;
    }
    printf("\n╔══════════════════════ REFILL ALERTS ═══════════════════════╗\n");
    if (alertPQ.size == 0) {
        printf("  ✓ All medicines are sufficiently stocked.\n");
    } else {
        PQNode *p = alertPQ.head;
        while (p) {
            printf("  ⚠ %s | Pills left: %d | Threshold: %d | Priority: %d\n",
                   p->name, p->pillsLeft, p->refillThreshold, p->priority);
            p = p->next;
        }
    }
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
}

/* ═══════════════════════ JSON LOAD ═════════════════════════════════════════ */

void loadFromJSON(void) {
    FILE *f = fopen(JSON_FILE, "r");
    if (!f) { printf("[JSON] Could not open %s – starting fresh.\n", JSON_FILE); return; }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    htInit(&searchTable);

    /* Find "medicines": [ ... ] */
    const char *arr = strstr(buf, "\"medicines\"");
    if (!arr) { free(buf); return; }
    arr = strchr(arr, '[');
    if (!arr) { free(buf); return; }
    arr++;

    int maxId = 0;

    while (*arr) {
        /* Skip whitespace */
        while (*arr == ' ' || *arr == '\n' || *arr == '\r' || *arr == '\t') arr++;
        if (*arr == ']') break;
        if (*arr != '{') { arr++; continue; }

        /* Find matching } */
        const char *objStart = arr;
        int depth = 0;
        const char *p = arr;
        while (*p) {
            if      (*p == '{') depth++;
            else if (*p == '}') { depth--; if (depth == 0) break; }
            p++;
        }
        int objLen = (int)(p - objStart) + 1;
        char *obj  = malloc(objLen + 1);
        strncpy(obj, objStart, objLen);
        obj[objLen] = '\0';

        Medicine *m = calloc(1, sizeof(Medicine));

        char tmp[64];
        parseStrField(obj, "id",    tmp, sizeof(tmp));
        m->id = atoi(tmp);
        if (m->id == 0) {
            /* try numeric parse */
            const char *idp = strstr(obj, "\"id\"");
            if (idp) {
                idp = strchr(idp + 4, ':');
                if (idp) m->id = atoi(idp + 1);
            }
        }
        if (m->id > maxId) maxId = m->id;

        parseStrField(obj, "name",      m->name,      MAX_NAME_LEN);
        parseStrField(obj, "type",      m->type,      MAX_TYPE_LEN);
        parseStrField(obj, "dosage",    m->dosage,    MAX_DOSAGE_LEN);
        parseStrField(obj, "frequency", m->frequency, MAX_FREQ_LEN);
        parseStrField(obj, "priority",  m->priority,  MAX_PRIORITY_LEN);
        parseStrField(obj, "color",     m->color,     MAX_COLOR_LEN);
        parseStrField(obj, "addedDate", m->addedDate, 16);
        m->pillsLeft       = parseIntField(obj, "pillsLeft");
        m->refillThreshold = parseIntField(obj, "refillThreshold");

        /* Parse times array */
        const char *ta = strstr(obj, "\"times\"");
        if (ta) {
            ta = strchr(ta, '['); if (ta) ta++;
            int ti = 0;
            while (ta && *ta && *ta != ']' && ti < MAX_TIMES) {
                while (*ta == ' ' || *ta == ',') ta++;
                if (*ta == '"') {
                    ta++;
                    int ci = 0;
                    while (*ta && *ta != '"' && ci < MAX_TIME_LEN - 1)
                        m->times[ti][ci++] = *ta++;
                    m->times[ti][ci] = '\0';
                    ti++; if (*ta == '"') ta++;
                } else ta++;
            }
            m->timeCount = ti;
        }

        /* takenToday */
        const char *tta = strstr(obj, "\"takenToday\"");
        if (tta) {
            tta = strchr(tta, '['); if (tta) tta++;
            for (int i = 0; i < m->timeCount && tta && *tta && *tta != ']'; ) {
                while (*tta == ' ' || *tta == ',') tta++;
                if (*tta == 't' || *tta == '1') { m->takenToday[i++] = 1; tta += (*tta=='t'?4:1); }
                else if (*tta == 'f' || *tta == '0') { m->takenToday[i++] = 0; tta += (*tta=='f'?5:1); }
                else tta++;
            }
        }

        m->next = NULL;
        if (!medicineList) { medicineList = m; }
        else {
            Medicine *cur = medicineList;
            while (cur->next) cur = cur->next;
            cur->next = m;
        }
        htInsert(&searchTable, m->name, m);

        free(obj);
        arr = p + 1;
    }

    nextId = maxId + 1;
    free(buf);
    printf("[JSON] Loaded medicines from %s. Next ID: %d\n", JSON_FILE, nextId);
}

/* ═══════════════════════ JSON SAVE ═════════════════════════════════════════ */

void saveToJSON(void) {
    FILE *f = fopen(JSON_FILE, "w");
    if (!f) { printf("[JSON] Cannot write to %s\n", JSON_FILE); return; }

    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    char ts[32];
    sprintf(ts, "%04d-%02d-%02dT%02d:%02d:%02dZ",
            t->tm_year+1900, t->tm_mon+1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    fprintf(f, "{\n  \"medicines\": [\n");
    Medicine *cur = medicineList;
    while (cur) {
        fprintf(f, "    {\n");
        fprintf(f, "      \"id\": %d,\n",         cur->id);
        fprintf(f, "      \"name\": ");      jsonStr(f, cur->name);      fprintf(f, ",\n");
        fprintf(f, "      \"type\": ");      jsonStr(f, cur->type);      fprintf(f, ",\n");
        fprintf(f, "      \"dosage\": ");    jsonStr(f, cur->dosage);    fprintf(f, ",\n");
        fprintf(f, "      \"frequency\": "); jsonStr(f, cur->frequency); fprintf(f, ",\n");
        fprintf(f, "      \"times\": [");
        for (int i = 0; i < cur->timeCount; i++) {
            if (i) fprintf(f, ", ");
            jsonStr(f, cur->times[i]);
        }
        fprintf(f, "],\n");
        fprintf(f, "      \"pillsLeft\": %d,\n",       cur->pillsLeft);
        fprintf(f, "      \"refillThreshold\": %d,\n", cur->refillThreshold);
        fprintf(f, "      \"priority\": ");   jsonStr(f, cur->priority);  fprintf(f, ",\n");
        fprintf(f, "      \"color\": ");      jsonStr(f, cur->color);     fprintf(f, ",\n");
        fprintf(f, "      \"takenToday\": [");
        for (int i = 0; i < cur->timeCount; i++) {
            if (i) fprintf(f, ", ");
            fprintf(f, "%s", cur->takenToday[i] ? "true" : "false");
        }
        fprintf(f, "],\n");
        fprintf(f, "      \"addedDate\": "); jsonStr(f, cur->addedDate); fprintf(f, "\n");
        fprintf(f, "    }%s\n", cur->next ? "," : "");
        cur = cur->next;
    }
    fprintf(f, "  ],\n");
    fprintf(f, "  \"lastUpdated\": \"%s\"\n", ts);
    fprintf(f, "}\n");
    fclose(f);
    printf("[JSON] Saved to %s\n", JSON_FILE);
}

/* ═══════════════════════ FREE MEMORY ═══════════════════════════════════════ */

void freeAll(void) {
    Medicine *cur = medicineList;
    while (cur) { Medicine *t = cur->next; free(cur); cur = t; }
    medicineList = NULL;

    QueueNode *qn = reminderQueue.front;
    while (qn) { QueueNode *t = qn->next; free(qn); qn = t; }
    reminderQueue.front = reminderQueue.rear = NULL;

    pqClear(&alertPQ);
}

/* ═══════════════════════ MAIN ══════════════════════════════════════════════ */

int main(void) {
    printf("╔════════════════════════════════════════╗\n");
    printf("║     PillMate - Medicine Backend C      ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    htInit(&searchTable);
    loadFromJSON();
    displayMedicines();
    checkRefillAlerts();

    /* Demo: add a new medicine */
    char demoTimes[2][MAX_TIME_LEN] = {"07:30", "19:30"};
    addMedicine("Aspirin", "Tablet", "75mg", "Twice daily",
                demoTimes, 2, 20, 10, "Medium", "#c3f0ca");

    /* Demo: search */
    searchMedicine("Paracetamol");
    searchMedicine("Unknown");

    /* Demo: mark as taken */
    markAsTaken(1, 0);

    /* Demo: show queue */
    printf("\n[QUEUE] Reminder queue size: %d\n", reminderQueue.size);

    /* Save back */
    saveToJSON();
    displayMedicines();
    checkRefillAlerts();

    freeAll();
    printf("\n[EXIT] Backend done. JSON updated.\n");
    return 0;
}
