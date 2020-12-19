//

#define _GNU_SOURCE // get rid of IntelliSence error on `u_char`
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dlfcn.h>
#include <malloc.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <pcap.h>
#include "Box.h"

const char *LIBRARY_FILES[] = {
    "./libMatchFunc.so",
    NULL,
};

typedef void (*OnInitialize)(void);
typedef void (*OnProcess)(uint8_t *, size_t);

typedef struct
{
    union
    {
        void *bare;
        struct
        {
            Box *box;
            void *address;
        } protected;
    };
    OnProcess process;
} Library;

typedef struct
{
    Library *libraries;
    int libraryCount;
    int useSandbox;
    size_t totalSize;
} OnPacketArgument;

void onPacket(u_char *raw, const struct pcap_pkthdr *header, const u_char *packet)
{
    OnPacketArgument *arg = (OnPacketArgument *)raw;
    static uint8_t packetBuffer[2048];
    memcpy(packetBuffer, packet, header->caplen);
    for (int i = 0; i < arg->libraryCount; i += 1)
    {
        if (arg->useSandbox)
        {
            PreExecute(arg->libraries[i].protected.box);
        }
        arg->libraries[i].process(packetBuffer, header->caplen);
    }
    arg->totalSize += header->caplen;
}

int main(int argc, const char *argv[])
{
    if (argc < 3)
    {
        printf("usage: %s [sandbox|bare] [path to pcap file]\n", argv[0]);
        return 0;
    }
    int useSandbox;
    if (strcmp(argv[1], "sandbox") == 0)
    {
        useSandbox = 1;
    }
    else if (strcmp(argv[1], "bare") == 0)
    {
        useSandbox = 0;
    }
    else
    {
        fprintf(stderr, "unknown argument: %s\n", argv[1]);
        return 1;
    }
    const char *pcapFile = argv[2];

    Library libraries[16];
    int libraryCount = 0;
    for (int i = 0; LIBRARY_FILES[i]; i += 1, libraryCount += 1)
    {
        if (useSandbox)
        {
            libraries[i].protected.box = CreateBox(LIBRARY_FILES[i], 1u << 30u);
            libraries[i].protected.address =
                memalign(getpagesize(), GetBoxSize(libraries[i].protected.box));
            if (DeployBox(libraries[i].protected.box, libraries[i].protected.address))
            {
                fprintf(stderr, "DeployBox: failed to deploy %s\n", LIBRARY_FILES[i]);
                return 1;
            }
            RunBoxL(libraries[i].protected.box, OnInitialize, InitializeFunction)();
            libraries[i].process = GetExecutedFunction(libraries[i].protected.box, "ProcessPacket");
        }
        else
        {
            libraries[i].bare = dlopen(LIBRARY_FILES[i], RTLD_NOW);
            if (!libraries[i].bare)
            {
                fprintf(stderr, "dlopen: failed on %s\n", LIBRARY_FILES[i]);
                return 1;
            }
            ((OnInitialize)dlsym(libraries[i].bare, "InitializeFunction"))();
            libraries[i].process = dlsym(libraries[i].bare, "ProcessPacket");
        }
    }
    if (useSandbox)
    {
        libraries[libraryCount].protected.box = NULL;
    }
    else
    {
        libraries[libraryCount].bare = NULL;
    }

    char errorBuffer[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_offline(pcapFile, errorBuffer);

    struct timespec startTime, finishTime;
    clock_gettime(CLOCK_REALTIME, &startTime);
    OnPacketArgument arg = {
        .libraries = libraries,
        .libraryCount = libraryCount,
        .useSandbox = useSandbox,
        .totalSize = 0};
    pcap_loop(handle, 0, onPacket, (u_char *)&arg);
    clock_gettime(CLOCK_REALTIME, &finishTime);

    double interval = (finishTime.tv_nsec - startTime.tv_nsec) / 1e9f +
                      (finishTime.tv_sec - startTime.tv_sec);
    printf("Throughput: %fMbps\n", arg.totalSize / interval / 1e6 * 8);

    pcap_close(handle);
    for (int i = 0; i < libraryCount; i += 1)
    {
        if (useSandbox)
        {
            ReleaseBox(libraries[i].protected.box);
            free(libraries[i].protected.address);
        }
        else
        {
            dlclose(libraries[i].bare);
        }
    }
    return 0;
}