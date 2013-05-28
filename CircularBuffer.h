typedef struct {unsigned int src_ip;
		unsigned int src_port;
		unsigned int dest_ip;
		unsigned int dest_port;
		} ElemType;

typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    ElemType   *elems;  /* vector of elements                   */
} CircularBuffer;

static CircularBuffer cb;

static CircularBuffer *bufferPointer;

static ElemType e;

static int PortSequence[8];

void cbInit(CircularBuffer *cb, int size);
void cbFree(CircularBuffer *cb);
int cbIsFull(CircularBuffer *cb);
int cbIsEmpty(CircularBuffer *cb);
void cbWrite(CircularBuffer *cb, ElemType *elem);
void cbRead(CircularBuffer *cb, ElemType *elem);