#ifndef NOMALLOC_INCLUDE_BOX_H
#define NOMALLOC_INCLUDE_BOX_H

#include <stddef.h>

typedef struct BoxLayout Box;

typedef void *Address;
typedef size_t Size;

Box *CreateBox(const char *file, Size heapSize);

Size GetBoxSize(const Box *box);

int DeployBox(Box *box, const Address address);

void PreExecute(const Box *box);

void *GetExecutedFunction(const Box *box, const char *function);

#define RunBox(box, type, function) \
    PreExecute(box);                \
    ((type)GetExecutedFunction(box, function))

#define RunBoxL(box, type, literal) RunBox(box, type, #literal)

void ReleaseBox(Box *box);

#endif